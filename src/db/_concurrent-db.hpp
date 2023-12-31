#pragma once

#include <je2be/defer.hpp>
#include <je2be/fs.hpp>
#include <je2be/strings.hpp>

#include "_parallel.hpp"
#include "_system.hpp"
#include "db/_db-interface.hpp"

#include <db/log_writer.h>
#include <db/version_edit.h>
#include <leveldb/env.h>
#include <table/block_builder.h>
#include <table/format.h>
#include <util/crc32c.h>

#include <execution>
#include <inttypes.h>

namespace je2be {

class ConcurrentDb : public DbInterface {
  struct TableBuildResult {
    TableBuildResult(u64 fileNumber, u64 fileSize, leveldb::InternalKey smallest, leveldb::InternalKey largest)
        : fFileNumber(fileNumber), fFileSize(fileSize), fSmallest(smallest), fLargest(largest) {}

    u64 fFileNumber;
    u64 fFileSize;
    leveldb::InternalKey fSmallest;
    leveldb::InternalKey fLargest;
  };

  struct Key {
    std::string fKey;
    u32 fValueSizeCompressed;
    u64 fOffset;
    u64 fSequence;
    u32 fWriterId;
  };

  class Writer {
  public:
    Writer(u32 id, std::filesystem::path const &directory, std::atomic_uint64_t &sequencer) : fId(id), fSequencer(sequencer), fNumKeys(), fTotalKeySize() {
      namespace fs = std::filesystem;
      fs::path dir = directory / std::to_string(id);
      Fs::DeleteAll(dir);
      if (!Fs::CreateDirectories(dir)) {
        fWhy = JE2BE_ERROR;
        return;
      }
      fDir = dir;

      fs::path keyFile = dir / "key.bin";
      FILE *key = mcfile::File::Open(keyFile, mcfile::File::Mode::Write);
      if (!key) {
        fWhy = JE2BE_ERROR_ERRNO;
        return;
      }

      fs::path valueFile = dir / "value.bin";
      FILE *value = mcfile::File::Open(valueFile, mcfile::File::Mode::Write);
      if (!value) {
        fWhy = JE2BE_ERROR_ERRNO;
        fclose(key);
        return;
      }

      fKey = key;
      fValue = value;
      std::fill_n(fNumKeys, sizeof(fNumKeys) / sizeof(fNumKeys[0]), 0);
      std::fill_n(fTotalKeySize, sizeof(fTotalKeySize) / sizeof(fTotalKeySize[0]), 0);
    }

    ~Writer() {
      abandon();
    }

    Status put(std::string const &key, std::string const &value) {
      using namespace std;
      if (!fValue || !fKey) {
        if (fWhy.ok()) {
          return JE2BE_ERROR;
        } else {
          return JE2BE_ERROR_PUSH(fWhy);
        }
      }
      if (key.empty()) {
        return JE2BE_ERROR;
      }
      u64 sequence = fSequencer.fetch_add(1);
      string block;
      Compress(key, value, sequence, &block);

      u32 valueSizeCompressed = block.size();
      u32 keySize = key.size();
      Status st;

      if (fwrite(block.data(), block.size(), 1, fValue) != 1) {
        st = JE2BE_ERROR_ERRNO;
        goto error;
      }

      if (fwrite(&keySize, sizeof(keySize), 1, fKey) != 1) {
        st = JE2BE_ERROR_ERRNO;
        goto error;
      }
      if (fwrite(key.data(), key.size(), 1, fKey) != 1) {
        st = JE2BE_ERROR_ERRNO;
        goto error;
      }
      if (fwrite(&valueSizeCompressed, sizeof(valueSizeCompressed), 1, fKey) != 1) {
        st = JE2BE_ERROR_ERRNO;
        goto error;
      }
      if (fwrite(&fOffset, sizeof(fOffset), 1, fKey) != 1) {
        st = JE2BE_ERROR_ERRNO;
        goto error;
      }
      if (fwrite(&sequence, sizeof(sequence), 1, fKey) != 1) {
        st = JE2BE_ERROR_ERRNO;
        goto error;
      }
      fOffset += block.size();
      {
        int idx = (unsigned char)key[0];
        fNumKeys[idx] += 1;
        fTotalKeySize[idx] += key.size();
      }
      return st;

    error:
      if (fKey) {
        fclose(fKey);
        fKey = nullptr;
      }
      if (fValue) {
        fclose(fValue);
        fValue = nullptr;
      }
      Fs::DeleteAll(fDir);
      fWhy = st;
      return st;
    }

    struct CloseResult {
      u32 fWriterId;
      u64 fNumKeys[256];
      u64 fTotalKeySize[256];
    };
    std::shared_ptr<CloseResult> close() {
      if (!fValue || !fKey) {
        if (fValue) {
          fclose(fValue);
          fValue = nullptr;
        }
        if (fKey) {
          fclose(fKey);
          fKey = nullptr;
        }
        return nullptr;
      }
      fclose(fValue);
      fValue = nullptr;
      fclose(fKey);
      fKey = nullptr;
      auto result = std::make_shared<CloseResult>();
      result->fWriterId = fId;
      std::copy_n(fNumKeys, 256, result->fNumKeys);
      std::copy_n(fTotalKeySize, 256, result->fTotalKeySize);
      return result;
    }

    void abandon() {
      if (fValue) {
        fclose(fValue);
        fValue = nullptr;
      }
      if (fKey) {
        fclose(fKey);
        fKey = nullptr;
      }
      Fs::DeleteAll(fDir);
    }

  private:
    static void Compress(std::string const &key, leveldb::Slice const &value, u64 seq, std::string *out) {
      using namespace std;
      using namespace leveldb;

      leveldb::Options o;
      o.compression = kZlibRawCompression;
      InternalKeyComparator icmp(BytewiseComparator());
      o.comparator = &icmp;

      leveldb::BlockBuilder bb(&o);
      InternalKey ik(key, seq, kTypeValue);
      bb.Add(ik.Encode(), value);
      Slice c = bb.Finish();

      mcfile::Compression::CompressDeflate((void *)c.data(), c.size(), *out);
    }

  private:
    u32 const fId;
    std::atomic_uint64_t &fSequencer;
    std::filesystem::path fDir;
    FILE *fKey = nullptr;
    FILE *fValue = nullptr;
    u64 fOffset = 0;
    u64 fNumKeys[256];
    Status fWhy;
    u64 fTotalKeySize[256];
  };

  class Gate : std::enable_shared_from_this<Gate> {
  public:
    std::shared_ptr<Writer> get(uintptr_t key, std::filesystem::path const &dir, std::atomic_uint64_t &keySequence, std::atomic_uint32_t &writerIdGenerator) {
      using namespace std;
      auto found = fWriters.find(key);
      if (found != fWriters.end()) {
        return found->second;
      }
      u32 id = writerIdGenerator.fetch_add(1);
      auto writer = make_shared<Writer>(id, dir, keySequence);
      fWriters[key] = writer;
      return writer;
    }

    static void Drain(uintptr_t key, std::vector<std::shared_ptr<Writer>> &out) {
      using namespace std;
      lock_guard<mutex> lock(Mut());
      auto &gates = Gates();
      for (auto &it : gates) {
        it->drain(key, out);
      }
    }

    void drain(uintptr_t key, std::vector<std::shared_ptr<Writer>> &out) {
      auto found = fWriters.find(key);
      if (found != fWriters.end()) {
        out.push_back(found->second);
        fWriters.erase(found);
      }
    }

    static std::mutex &Mut() {
      static std::mutex sMut;
      return sMut;
    }

    static std::set<std::shared_ptr<Gate>> &Gates() {
      static std::set<std::shared_ptr<Gate>> sGates;
      return sGates;
    }

  private:
    std::map<uintptr_t, std::shared_ptr<Writer>> fWriters;
  };

  class ZlibRawTableBuilder {
    struct Rep {
      Rep(const leveldb::Options &opt, leveldb::WritableFile *f)
          : options(opt),
            index_block_options(opt),
            file(f),
            offset(0),
            index_block(&index_block_options),
            num_entries(0),
            closed(false),
            pending_index_entry(false) {
        index_block_options.block_restart_interval = 1;
        assert(opt.filter_policy == nullptr);
        assert(opt.compression == leveldb::kZlibRawCompression);
      }

      leveldb::Options options;
      leveldb::Options index_block_options;
      leveldb::WritableFile *file;
      u64 offset;
      leveldb::Status status;
      leveldb::BlockBuilder index_block;
      std::string last_key;
      i64 num_entries;
      bool closed; // Either Finish() or Abandon() has been called.

      // We do not emit the index entry for a block until we have seen the
      // first key for the next data block.  This allows us to use shorter
      // keys in the index block.  For example, consider a block boundary
      // between the keys "the quick brown fox" and "the who".  We can use
      // "the r" as the key for the index block entry since it is >= all
      // entries in the first block and < all entries in subsequent
      // blocks.
      //
      // Invariant: r->pending_index_entry is true only if data_block is empty.
      bool pending_index_entry;
      leveldb::BlockHandle pending_handle; // Handle to add to index block

      std::string compressed_output;
    };

  public:
    // Create a builder that will store the contents of the table it is
    // building in *file.  Does not close the file.  It is up to the
    // caller to close the file after calling Finish().
    ZlibRawTableBuilder(const leveldb::Options &options, leveldb::WritableFile *file)
        : rep_(new Rep(options, file)) {
    }

    ZlibRawTableBuilder(const ZlibRawTableBuilder &) = delete;
    ZlibRawTableBuilder &operator=(const ZlibRawTableBuilder &) = delete;

    // REQUIRES: Either Finish() or Abandon() has been called.
    ~ZlibRawTableBuilder() {
      assert(rep_->closed); // Catch errors where caller forgot to call Finish()
      delete rep_;
    }

    // Add key,value to the table being constructed.
    // REQUIRES: key is after any previously added key according to comparator.
    // REQUIRES: Finish(), Abandon() have not been called
    void AddAlreadyCompressedAndFlush(const leveldb::Slice &key, const leveldb::Slice &value) {
      using namespace leveldb;
      Rep *r = rep_;
      assert(!r->closed);
      if (!ok())
        return;
      if (r->num_entries > 0) {
        assert(r->options.comparator->Compare(key, Slice(r->last_key)) > 0);
      }

      if (r->pending_index_entry) {
        r->options.comparator->FindShortestSeparator(&r->last_key, key);
        std::string handle_encoding;
        r->pending_handle.EncodeTo(&handle_encoding);
        r->index_block.Add(r->last_key, Slice(handle_encoding));
        r->pending_index_entry = false;
      }

      r->last_key.assign(key.data(), key.size());
      r->num_entries++;

      WriteRawBlock(value, kZlibRawCompression, &r->pending_handle);

      if (ok()) {
        r->pending_index_entry = true;
        r->status = r->file->Flush();
      }
    }

    // Return non-ok iff some error has been detected.
    leveldb::Status status() const { return rep_->status; }

    // Finish building the table.  Stops using the file passed to the
    // constructor after this function returns.
    // REQUIRES: Finish(), Abandon() have not been called
    leveldb::Status Finish() {
      using namespace leveldb;
      Rep *r = rep_;
      // Flush();
      assert(!r->closed);
      r->closed = true;

      BlockHandle filter_block_handle, metaindex_block_handle, index_block_handle;

      // Write metaindex block
      if (ok()) {
        BlockBuilder meta_index_block(&r->options);
        // TODO(postrelease): Add stats and other meta blocks
        WriteBlock(&meta_index_block, &metaindex_block_handle);
      }

      // Write index block
      if (ok()) {
        if (r->pending_index_entry) {
          r->options.comparator->FindShortSuccessor(&r->last_key);
          std::string handle_encoding;
          r->pending_handle.EncodeTo(&handle_encoding);
          r->index_block.Add(r->last_key, Slice(handle_encoding));
          r->pending_index_entry = false;
        }
        WriteBlock(&r->index_block, &index_block_handle);
      }

      // Write footer
      if (ok()) {
        Footer footer;
        footer.set_metaindex_handle(metaindex_block_handle);
        footer.set_index_handle(index_block_handle);
        std::string footer_encoding;
        footer.EncodeTo(&footer_encoding);
        r->status = r->file->Append(footer_encoding);
        if (r->status.ok()) {
          r->offset += footer_encoding.size();
        }
      }
      return r->status;
    }

    // Indicate that the contents of this builder should be abandoned.  Stops
    // using the file passed to the constructor after this function returns.
    // If the caller is not going to call Finish(), it must call Abandon()
    // before destroying this builder.
    // REQUIRES: Finish(), Abandon() have not been called
    void Abandon() {
      Rep *r = rep_;
      assert(!r->closed);
      r->closed = true;
    }

    // Number of calls to Add() so far.
    u64 NumEntries() const { return rep_->num_entries; }

    // Size of the file generated so far.  If invoked after a successful
    // Finish() call, returns the size of the final generated file.
    u64 FileSize() const { return rep_->offset; }

  private:
    bool ok() const { return status().ok(); }
    void WriteBlock(leveldb::BlockBuilder *block, leveldb::BlockHandle *handle) {
      using namespace leveldb;
      // File format contains a sequence of blocks where each block has:
      //    block_data: uint8[n]
      //    type: uint8
      //    crc: uint32
      assert(ok());
      Rep *r = rep_;
      Slice raw = block->Finish();

      Slice block_contents;
      CompressionType type = kZlibRawCompression;

      std::string *compressed = &r->compressed_output;
      mcfile::Compression::CompressDeflate((void *)raw.data(), raw.size(), *compressed, Z_BEST_COMPRESSION);
      if (compressed->size() < raw.size()) {
        block_contents = *compressed;
      } else {
        // Compression type not supported, or compressed less than equal, so just
        // store uncompressed form
        block_contents = raw;
        type = kNoCompression;
      }

      WriteRawBlock(block_contents, type, handle);
      r->compressed_output.clear();
      block->Reset();
    }

    void WriteRawBlock(const leveldb::Slice &block_contents, leveldb::CompressionType type, leveldb::BlockHandle *handle) {
      using namespace leveldb;
      Rep *r = rep_;
      handle->set_offset(r->offset);
      handle->set_size(block_contents.size());
      r->status = r->file->Append(block_contents);
      if (r->status.ok()) {
        char trailer[kBlockTrailerSize];
        trailer[0] = type;
        u32 crc = crc32c::Value(block_contents.data(), block_contents.size());
        crc = crc32c::Extend(crc, trailer, 1); // Extend crc to cover block type
        EncodeFixed32(trailer + 1, crc32c::Mask(crc));
        r->status = r->file->Append(Slice(trailer, kBlockTrailerSize));
        if (r->status.ok()) {
          r->offset += block_contents.size() + kBlockTrailerSize;
        }
      }
    }

    Rep *rep_;
  };

public:
  ConcurrentDb(std::filesystem::path const &dbname, unsigned int concurrency, std::optional<std::filesystem::path> tempDir = std::nullopt)
      : fDbName(dbname), fSequence(0), fWriterIdGenerator(0), fConcurrency(concurrency), fWriterDir(tempDir ? *tempDir : dbname) {
    leveldb::DestroyDB(dbname, {});
    Fs::CreateDirectories(dbname);
  }

  ~ConcurrentDb() {
    abandonImpl();
  }

  bool valid() const override {
    return fValid;
  }

  Status put(std::string const &key, leveldb::Slice const &value) override {
    return gate()->get((uintptr_t)this, fWriterDir, fSequence, fWriterIdGenerator)->put(key, value.ToString());
  }

  Status del(std::string const &key) override { return Status::Ok(); }

  Status close(std::function<bool(Rational<u64> const &progress)> progress = nullptr) override {
    using namespace std;
    using namespace std::placeholders;
    using namespace leveldb;
    namespace fs = std::filesystem;

    if (progress && !progress({0, 257})) {
      return JE2BE_ERROR;
    }

    vector<shared_ptr<Writer>> writers;
    Gate::Drain((uintptr_t)this, writers);

    if (writers.empty()) {
      if (progress && !progress({257, 257})) {
        return JE2BE_ERROR;
      }
      return Status::Ok();
    }

    auto [writerIds, status] = Parallel::Reduce<shared_ptr<Writer>, vector<Writer::CloseResult>>(
        writers,
        fConcurrency,
        vector<Writer::CloseResult>(),
        [](shared_ptr<Writer> const &writer) -> pair<vector<Writer::CloseResult>, Status> {
          shared_ptr<Writer::CloseResult> id = writer->close();
          vector<Writer::CloseResult> ret;
          if (id) {
            ret.push_back(*id);
            return make_pair(ret, Status::Ok());
          } else {
            return make_pair(ret, JE2BE_ERROR);
          }
        },
        Parallel::MergeVector<Writer::CloseResult>);
    if (!status.ok()) {
      return JE2BE_ERROR_PUSH(status);
    }

    struct BuildResult {
      vector<TableBuildResult> fResults;
      static void Merge(BuildResult const &from, BuildResult &to) {
        std::copy(from.fResults.begin(), from.fResults.end(), std::back_inserter(to.fResults));
      }
    };
    vector<u8> works;
    for (int i = 0; i < 256; i++) {
      works.push_back((u8)i);
    }
    atomic_uint64_t fileNumber(1);
    atomic_uint64_t done(0);
    u64 maxMemoryUsage = 0;
    if (fConcurrency > 0) {
      u64 const availableMemory = System::GetAvailableMemory();
      maxMemoryUsage = availableMemory / fConcurrency;
    }
    auto [result, buildStatus] = Parallel::Reduce<u8, BuildResult>(
        works,
        fConcurrency,
        BuildResult{},
        [&](u8 prefix) -> pair<BuildResult, Status> {
          BuildResult ret;
          if (auto s = BuildTable(fDbName, fWriterDir, writerIds, &fileNumber, ret.fResults, prefix, maxMemoryUsage); !s.ok()) {
            return make_pair(ret, JE2BE_ERROR_PUSH(s));
          }
          auto p = done.fetch_add(1) + 1;
          if (progress && !progress({p, 256 + 1})) {
            return make_pair(ret, JE2BE_ERROR);
          } else {
            return make_pair(ret, Status::Ok());
          }
        },
        BuildResult::Merge);
    if (!buildStatus.ok()) {
      return JE2BE_ERROR_PUSH(buildStatus);
    }
    sort(result.fResults.begin(), result.fResults.end(), [](TableBuildResult const &lhs, TableBuildResult const &rhs) {
      return lhs.fFileNumber < rhs.fFileNumber;
    });

    InternalKeyComparator icmp(BytewiseComparator());
    VersionEdit edit;
    u64 maxFileNumber = 0;
    for (auto const &it : result.fResults) {
      maxFileNumber = (std::max)(maxFileNumber, it.fFileNumber);
      edit.AddFile(1, it.fFileNumber, it.fFileSize, it.fSmallest, it.fLargest);
    }
    edit.SetLastSequence(fSequence.load());
    edit.SetNextFile(maxFileNumber + 1);
    edit.SetLogNumber(0);
    string manifestRecord;
    edit.EncodeTo(&manifestRecord);

    string manifestFileName = "MANIFEST-000001";
    fs::path manifestFile = fDbName / manifestFileName;
    auto [meta, metaOpenStatus] = OpenWritable(manifestFile);
    if (!meta) {
      return JE2BE_ERROR_WHAT(metaOpenStatus.ToString());
    }
    leveldb::log::Writer writer(meta.get());
    leveldb::Status st = writer.AddRecord(manifestRecord);
    if (!st.ok()) {
      return JE2BE_ERROR_WHAT(st.ToString());
    }
    st = meta->Close();
    if (!st.ok()) {
      return JE2BE_ERROR_WHAT(st.ToString());
    }
    meta.reset();

    fs::path currentFile = fDbName / "CURRENT";
    auto [current, currentOpenStatus] = OpenWritable(currentFile);
    if (!current) {
      return JE2BE_ERROR_WHAT(currentOpenStatus.ToString());
    }
    st = current->Append(manifestFileName + "\x0a");
    if (!st.ok()) {
      return JE2BE_ERROR_WHAT(st.ToString());
    }
    st = current->Close();
    if (!st.ok()) {
      return JE2BE_ERROR_WHAT(st.ToString());
    }
    current.reset();

    if (progress) {
      progress({257, 257});
    }

    return Status::Ok();
  }

  static Status BuildTable(
      std::filesystem::path const &dbname,
      std::filesystem::path const &writerDir,
      std::vector<Writer::CloseResult> const &writerIds,
      std::atomic_uint64_t *fileNumber,
      std::vector<TableBuildResult> &out,
      u8 prefix,
      u64 maxMemoryUsage) {
    using namespace std;
    using namespace leveldb;
    namespace fs = std::filesystem;

    u64 totalKeySizeWithPrefix = 0;
    u64 numKeysWithPrefix = 0;
    u64 numKeys = 0;
    for (Writer::CloseResult const &cr : writerIds) {
      totalKeySizeWithPrefix += cr.fTotalKeySize[prefix];
      numKeysWithPrefix += cr.fNumKeys[prefix];
      for (int i = 0; i < 256; i++) {
        numKeys += cr.fNumKeys[i];
      }
    }
    if (numKeysWithPrefix == 0) {
      return Status::Ok();
    }

    u64 const estimatedKeysSize = totalKeySizeWithPrefix + numKeysWithPrefix * sizeof(Key);

    int depth = 0;
    if (maxMemoryUsage > 0) {
      int div = (int)std::ceil(-std::log(maxMemoryUsage / double(estimatedKeysSize)) / std::log(256.0));
      depth = clamp<int>(div, 0, 7);
    }

    string lower;
    lower.resize(depth + 1);
    lower[0] = *(char *)&prefix;
    u64 const nmax = (u64(1) << (8 * depth));
    for (u64 n = 0; n < nmax; n++) {
      string upper = strings::Increment(lower);

      string begin = strings::RemoveSuffix(lower, string("\x0", 1));
      string end = strings::RemoveSuffix(upper, string("\x0", 1));

      vector<Key> keys;
      for (Writer::CloseResult const &cr : writerIds) {
        u64 expectedEncount = cr.fNumKeys[prefix];
        if (expectedEncount == 0) {
          continue;
        }
        fs::path fname = writerDir / to_string(cr.fWriterId) / "key.bin";
        mcfile::ScopedFile fp(mcfile::File::Open(fname, mcfile::File::Mode::Read));
        if (!fp) {
          return JE2BE_ERROR_ERRNO;
        }
        int encount = 0;
        for (u64 i = 0; i < numKeys; i++) {
          u32 keySize;
          if (fread(&keySize, sizeof(keySize), 1, fp.get()) != 1) {
            return JE2BE_ERROR_ERRNO;
          }
          u8 first;
          if (fread(&first, sizeof(first), 1, fp.get()) != 1) {
            return JE2BE_ERROR_ERRNO;
          }
          if (first != prefix) {
            if (!mcfile::File::Fseek(fp.get(), keySize + sizeof(u32) + sizeof(u64) + sizeof(u64) - 1, SEEK_CUR)) {
              return JE2BE_ERROR_ERRNO;
            }
            continue;
          }
          encount++;

          string keyString;
          keyString.resize(keySize);
          keyString[0] = (char)prefix;
          if (keySize > 1) {
            if (fread(keyString.data() + 1, keySize - 1, 1, fp.get()) != 1) {
              return JE2BE_ERROR_ERRNO;
            }
          }
          if (n != 0 && keyString < lower) {
            if (encount == expectedEncount) {
              break;
            } else {
              if (!mcfile::File::Fseek(fp.get(), sizeof(u32) + sizeof(u64) + sizeof(u64), SEEK_CUR)) {
                return JE2BE_ERROR_ERRNO;
              }
              continue;
            }
          }
          if (n != nmax - 1 && !(keyString < upper)) {
            if (encount == expectedEncount) {
              break;
            } else {
              if (!mcfile::File::Fseek(fp.get(), sizeof(u32) + sizeof(u64) + sizeof(u64), SEEK_CUR)) {
                return JE2BE_ERROR_ERRNO;
              }
              continue;
            }
          }

          Key key;
          key.fWriterId = cr.fWriterId;
          key.fKey = keyString;

          if (fread(&key.fValueSizeCompressed, sizeof(key.fValueSizeCompressed), 1, fp.get()) != 1) {
            return JE2BE_ERROR_ERRNO;
          }
          if (fread(&key.fOffset, sizeof(key.fOffset), 1, fp.get()) != 1) {
            return JE2BE_ERROR_ERRNO;
          }
          if (fread(&key.fSequence, sizeof(key.fSequence), 1, fp.get()) != 1) {
            return JE2BE_ERROR_ERRNO;
          }
          keys.push_back(key);
          if (encount == expectedEncount) {
            break;
          }
        }
      }
      Comparator const *cmp = BytewiseComparator();
      sort(keys.begin(), keys.end(), [cmp](Key const &lhs, Key const &rhs) {
        return cmp->Compare(lhs.fKey, rhs.fKey) < 0;
      });

      u64 size = 0;
      vector<Key> bin;
      for (int i = 0; i < keys.size(); i++) {
        Key key = keys[i];
        bin.push_back(key);
        size += key.fValueSizeCompressed;
        if (size >= kMaxFileSize) {
          u64 fn = fileNumber->fetch_add(1);
          if (auto st = Write(dbname, writerDir, bin, fn, out); !st.ok()) {
            return JE2BE_ERROR_PUSH(st);
          }
          size = 0;
          bin.clear();
        }
      }
      if (!bin.empty()) {
        u64 fn = fileNumber->fetch_add(1);
        if (auto st = Write(dbname, writerDir, bin, fn, out); !st.ok()) {
          return JE2BE_ERROR_PUSH(st);
        }
      }

      lower = upper;
    }

    return Status::Ok();
  }

  static Status Write(std::filesystem::path dbname, std::optional<std::filesystem::path> tempDir, std::vector<Key> &keys, u64 fileNumber, std::vector<TableBuildResult> &results) {
    using namespace std;
    using namespace leveldb;
    namespace fs = std::filesystem;

    if (keys.empty()) {
      return Status::Ok();
    }

    auto [file, status] = OpenWritable(TableFilePath(dbname, fileNumber));
    if (!status.ok() || !file) {
      return JE2BE_ERROR_WHAT(status.ToString());
    }

    leveldb::Options bo;
    bo.compression = kZlibRawCompression;
    InternalKeyComparator icmp(BytewiseComparator());
    bo.comparator = &icmp;
    auto builder = make_shared<ZlibRawTableBuilder>(bo, file.get());

    Status st;
    string value;
    optional<u32> openedFile;
    FILE *fp = nullptr;

    fs::path writerDir = dbname;
    if (tempDir) {
      writerDir = *tempDir;
    }

    for (auto const &it : keys) {
      Slice userKey(it.fKey);
      InternalKey ik(userKey, it.fSequence, kTypeValue);
      FILE *f = nullptr;
      if (openedFile != it.fWriterId && fp) {
        fclose(fp);
        fp = nullptr;
      }
      if (openedFile == it.fWriterId && fp) {
        f = fp;
      } else {
        fs::path fname = writerDir / to_string(it.fWriterId) / "value.bin";
        f = mcfile::File::Open(fname, mcfile::File::Mode::Read);
        if (!f) {
          return JE2BE_ERROR_ERRNO;
        }
        fp = f;
        openedFile = it.fWriterId;
      }
      if (!mcfile::File::Fseek(f, it.fOffset, SEEK_SET)) {
        st = JE2BE_ERROR_ERRNO;
        goto cleanup;
      }
      value.resize(it.fValueSizeCompressed);
      if (fread(value.data(), it.fValueSizeCompressed, 1, f) != 1) {
        st = JE2BE_ERROR_ERRNO;
        goto cleanup;
      }
      builder->AddAlreadyCompressedAndFlush(ik.Encode(), value);
    }
    builder->Finish();
    file->Close();

    {
      InternalKey smallest(keys[0].fKey, keys[0].fSequence, kTypeValue);
      InternalKey largest(keys[keys.size() - 1].fKey, keys[keys.size() - 1].fSequence, kTypeValue);
      TableBuildResult result(fileNumber, builder->FileSize(), smallest, largest);
      results.push_back(result);
    }

    keys.clear();

  cleanup:
    if (fp) {
      fclose(fp);
    }
    return st;
  }

  static std::pair<std::unique_ptr<leveldb::WritableFile>, leveldb::Status> OpenWritable(std::filesystem::path const &path) {
    using namespace leveldb;
    Env *env = Env::Default();
    WritableFile *file = nullptr;
    leveldb::Status st = env->NewWritableFile(path, &file);
    if (st.ok() && file) {
      return std::make_pair(std::unique_ptr<leveldb::WritableFile>(file), st);
    } else {
      return std::make_pair(nullptr, st);
    }
  }

  static std::filesystem::path TableFilePath(std::filesystem::path const &dbname, u64 tableNumber) {
    std::vector<char> buffer(11, (char)0);
#if defined(_WIN32)
    sprintf_s(buffer.data(), buffer.size(), "%06" PRIu64 ".ldb", tableNumber);
#else
    snprintf(buffer.data(), buffer.size(), "%06" PRIu64 ".ldb", tableNumber);
#endif
    std::string p(buffer.data(), 10);
    return dbname / p;
  }

  void abandon() override {
    abandonImpl();
  }

private:
  void abandonImpl() {
    using namespace std;
    namespace fs = std::filesystem;
    vector<shared_ptr<Writer>> writers;
    Gate::Drain((uintptr_t)this, writers);
    for (int i = 0; i < writers.size(); i++) {
      writers[i]->abandon();
    }
    fValid = false;
  }

  std::shared_ptr<Gate> gate() {
    using namespace std;
    thread_local shared_ptr<Gate> tGate(CreateGate());
    return tGate;
  }

  static std::shared_ptr<Gate> CreateGate() {
    using namespace std;
    auto ret = make_shared<Gate>();
    lock_guard<mutex> lock(Gate::Mut());
    Gate::Gates().insert(ret);
    return ret;
  }

private:
  std::filesystem::path const fDbName;
  std::atomic_uint64_t fSequence;
  std::atomic_uint32_t fWriterIdGenerator;
  bool fValid = true;
  unsigned int const fConcurrency;
  std::filesystem::path const fWriterDir;

  static constexpr u64 kMaxFileSize = 2 * 1024 * 1024;
};

} // namespace je2be
