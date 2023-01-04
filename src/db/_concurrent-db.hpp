#pragma once

#include <je2be/defer.hpp>

#include "_fs.hpp"
#include "_parallel.hpp"
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
    TableBuildResult(uint64_t fileNumber, uint64_t fileSize, leveldb::InternalKey smallest, leveldb::InternalKey largest)
        : fFileNumber(fileNumber), fFileSize(fileSize), fSmallest(smallest), fLargest(largest) {}

    uint64_t fFileNumber;
    uint64_t fFileSize;
    leveldb::InternalKey fSmallest;
    leveldb::InternalKey fLargest;
  };

  struct Key {
    std::string fKey;
    uint32_t fValueSizeCompressed;
    uint64_t fOffset;
    uint64_t fSequence;
    uint32_t fWriterId;
  };

  class Writer {
  public:
    Writer(uint32_t id, std::filesystem::path const &dbname, std::atomic_uint64_t &sequencer) : fId(id), fDbName(dbname), fSequencer(sequencer) {
      namespace fs = std::filesystem;
      fs::path dir = dbname / std::to_string(id);
      Fs::DeleteAll(dir);
      if (!Fs::CreateDirectories(dir)) {
        return;
      }
      fDir = dir;

      fs::path keyFile = dir / "key.bin";
      FILE *key = mcfile::File::Open(keyFile, mcfile::File::Mode::Write);
      if (!key) {
        return;
      }

      fs::path valueFile = dir / "value.bin";
      FILE *value = mcfile::File::Open(valueFile, mcfile::File::Mode::Write);
      if (!value) {
        fclose(key);
        return;
      }

      fKey = key;
      fValue = value;
      std::fill_n(fNumPrefix, 256, 0);
    }

    ~Writer() {
      abandon();
    }

    void put(std::string const &key, std::string const &value) {
      using namespace std;
      if (!fValue || !fKey) {
        return;
      }
      if (key.empty()) {
        return;
      }
      uint64_t sequence = fSequencer.fetch_add(1);
      string block;
      Compress(key, value, sequence, &block);

      uint32_t valueSizeCompressed = block.size();
      uint32_t keySize = key.size();

      if (fwrite(block.data(), block.size(), 1, fValue) != 1) {
        goto error;
      }

      if (fwrite(&keySize, sizeof(keySize), 1, fKey) != 1) {
        goto error;
      }
      if (fwrite(key.data(), key.size(), 1, fKey) != 1) {
        goto error;
      }
      if (fwrite(&valueSizeCompressed, sizeof(valueSizeCompressed), 1, fKey) != 1) {
        goto error;
      }
      if (fwrite(&fOffset, sizeof(fOffset), 1, fKey) != 1) {
        goto error;
      }
      if (fwrite(&sequence, sizeof(sequence), 1, fKey) != 1) {
        goto error;
      }
      fOffset += block.size();
      fNumKeys++;
      {
        int idx = (unsigned char)key[0];
        uint16_t num = fNumPrefix[idx];
        if (num < numeric_limits<uint16_t>::max()) {
          num++;
        }
        fNumPrefix[idx] = num;
      }
      return;

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
    }

    struct CloseResult {
      uint32_t fWriterId;
      uint64_t fNumKeys;
      uint16_t fNumPrefix[256];
    };
    std::optional<CloseResult> close() {
      if (!fValue || !fKey) {
        if (fValue) {
          fclose(fValue);
          fValue = nullptr;
        }
        if (fKey) {
          fclose(fKey);
          fKey = nullptr;
        }
        return std::nullopt;
      }
      fclose(fValue);
      fValue = nullptr;
      fclose(fKey);
      fKey = nullptr;
      CloseResult result;
      result.fNumKeys = fNumKeys;
      result.fWriterId = fId;
      std::copy_n(fNumPrefix, 256, result.fNumPrefix);
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
    static void Compress(std::string const &key, leveldb::Slice const &value, uint64_t seq, std::string *out) {
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
    uint32_t const fId;
    std::filesystem::path fDbName;
    std::atomic_uint64_t &fSequencer;
    std::filesystem::path fDir;
    FILE *fKey = nullptr;
    FILE *fValue = nullptr;
    uint64_t fOffset = 0;
    uint64_t fNumKeys = 0;
    uint16_t fNumPrefix[256];
  };

  class Gate {
  public:
    std::shared_ptr<Writer> get(uintptr_t key, std::filesystem::path const &dbname, std::atomic_uint64_t &keySequence, std::atomic_uint32_t &writerIdGenerator) {
      using namespace std;
      lock_guard<mutex> lock(Mut());
      auto found = fWriters.find(key);
      if (found != fWriters.end()) {
        return found->second;
      }
      uint32_t id = writerIdGenerator.fetch_add(1);
      auto writer = make_shared<Writer>(id, dbname, keySequence);
      fWriters[key] = writer;
      Jar().insert(make_pair(key, writer));
      return writer;
    }

    static void Drain(uintptr_t key, std::vector<std::shared_ptr<Writer>> &out) {
      using namespace std;
      lock_guard<mutex> lock(Mut());
      auto &jar = Jar();
      while (true) {
        auto node = jar.extract(key);
        if (node.empty()) {
          break;
        }
        out.push_back(node.mapped());
      }
    }

    static std::mutex &Mut() {
      static std::mutex sMut;
      return sMut;
    }

    static std::multimap<uintptr_t, std::shared_ptr<Writer>> &Jar() {
      static std::multimap<uintptr_t, std::shared_ptr<Writer>> sJar;
      return sJar;
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
      uint64_t offset;
      leveldb::Status status;
      leveldb::BlockBuilder index_block;
      std::string last_key;
      int64_t num_entries;
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
    uint64_t NumEntries() const { return rep_->num_entries; }

    // Size of the file generated so far.  If invoked after a successful
    // Finish() call, returns the size of the final generated file.
    uint64_t FileSize() const { return rep_->offset; }

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
      mcfile::Compression::CompressDeflate((void *)raw.data(), raw.size(), *compressed, Z_BEST_SPEED);
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
        uint32_t crc = crc32c::Value(block_contents.data(), block_contents.size());
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
  explicit ConcurrentDb(std::filesystem::path const &dbname) : fDbName(dbname), fSequence(0), fWriterIdGenerator(0) {
    leveldb::DestroyDB(dbname, {});
  }

  ~ConcurrentDb() {
    abandon();
  }

  bool valid() const override {
    return fValid;
  }

  void put(std::string const &key, leveldb::Slice const &value) override {
    gate().get((uintptr_t)this, fDbName, fSequence, fWriterIdGenerator)->put(key, value.ToString());
  }

  void del(std::string const &key) override {}

  bool close(std::function<void(double progress)> progress = nullptr) override {
    using namespace std;
    using namespace std::placeholders;
    using namespace leveldb;
    namespace fs = std::filesystem;

    vector<shared_ptr<Writer>> writers;
    Gate::Drain((uintptr_t)this, writers);

    atomic_bool ok = true;
    auto writerIds = Parallel::Reduce<shared_ptr<Writer>, vector<Writer::CloseResult>>(
        writers,
        vector<Writer::CloseResult>(),
        [&ok](shared_ptr<Writer> const &writer) -> vector<Writer::CloseResult> {
          auto id = writer->close();
          vector<Writer::CloseResult> ret;
          if (id) {
            ret.push_back(*id);
          } else {
            ok = false;
          }
          return ret;
        },
        Parallel::MergeVector<Writer::CloseResult>);
    if (!ok) {
      return false;
    }

    vector<uint8_t> works;
    for (int i = 0; i < 256; i++) {
      works.push_back((uint8_t)i);
    }
    atomic_uint64_t fileNumber(1);
    auto result = Parallel::Reduce<uint8_t, vector<TableBuildResult>>(
        works,
        vector<TableBuildResult>(),
        bind(BuildTable, fDbName, writerIds, &fileNumber, &ok, _1),
        Parallel::MergeVector<TableBuildResult>);
    if (!ok) {
      return false;
    }
    sort(result.begin(), result.end(), [](TableBuildResult const &lhs, TableBuildResult const &rhs) {
      return lhs.fFileNumber < rhs.fFileNumber;
    });

    InternalKeyComparator icmp(BytewiseComparator());
    VersionEdit edit;
    uint64_t maxFileNumber = 0;
    for (auto const &it : result) {
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
    unique_ptr<WritableFile> meta(OpenWritable(manifestFile));
    leveldb::log::Writer writer(meta.get());
    leveldb::Status st = writer.AddRecord(manifestRecord);
    if (!st.ok()) {
      return false;
    }
    meta->Close();
    meta.reset();

    fs::path currentFile = fDbName / "CURRENT";
    unique_ptr<WritableFile> current(OpenWritable(currentFile));
    st = current->Append(manifestFileName + "\x0a");
    if (!st.ok()) {
      return false;
    }
    current->Close();
    current.reset();

    return true;
  }

  static std::vector<TableBuildResult> BuildTable(
      std::filesystem::path const &dbname,
      std::vector<Writer::CloseResult> const &writerIds,
      std::atomic_uint64_t *fileNumber,
      std::atomic_bool *ok,
      uint8_t prefix) {
    using namespace std;
    using namespace leveldb;
    namespace fs = std::filesystem;

    vector<TableBuildResult> result;

    vector<Key> keys;
    for (Writer::CloseResult const &cr : writerIds) {
      if (!ok->load()) {
        break;
      }
      int expectedEncount = (int)cr.fNumPrefix[prefix];
      if (expectedEncount == 0) {
        continue;
      }
      fs::path fname = dbname / to_string(cr.fWriterId) / "key.bin";
      FILE *fp = mcfile::File::Open(fname, mcfile::File::Mode::Read);
      if (!fp) {
        ok->store(false);
        break;
      }
      int encount = 0;
      for (uint64_t i = 0; i < cr.fNumKeys; i++) {
        Key key;
        key.fWriterId = cr.fWriterId;
        uint32_t keySize;
        if (fread(&keySize, sizeof(keySize), 1, fp) != 1) {
          ok->store(false);
          fclose(fp);
          break;
        }
        uint8_t first;
        if (fread(&first, sizeof(first), 1, fp) != 1) {
          ok->store(false);
          fclose(fp);
          break;
        }
        if (first != prefix) {
          if (!mcfile::File::Fseek(fp, keySize + sizeof(uint32_t) + sizeof(uint64_t) + sizeof(uint64_t) - 1, SEEK_CUR)) {
            ok->store(false);
            break;
          }
          continue;
        }
        key.fKey.resize(keySize);
        key.fKey[0] = (char)prefix;
        if (keySize > 1) {
          if (fread(key.fKey.data() + 1, keySize - 1, 1, fp) != 1) {
            ok->store(false);
            break;
          }
        }
        if (fread(&key.fValueSizeCompressed, sizeof(key.fValueSizeCompressed), 1, fp) != 1) {
          ok->store(false);
          fclose(fp);
          break;
        }
        if (fread(&key.fOffset, sizeof(key.fOffset), 1, fp) != 1) {
          ok->store(false);
          fclose(fp);
          break;
        }
        if (fread(&key.fSequence, sizeof(key.fSequence), 1, fp) != 1) {
          ok->store(false);
          fclose(fp);
          break;
        }
        keys.push_back(key);
        encount++;
        if (expectedEncount != numeric_limits<uint16_t>::max() && encount == expectedEncount) {
          break;
        }
      }
      fclose(fp);
      if (!ok->load()) {
        break;
      }
    }
    if (!ok->load()) {
      return {};
    }
    Comparator const *cmp = BytewiseComparator();
    sort(keys.begin(), keys.end(), [cmp](Key const &lhs, Key const &rhs) {
      return cmp->Compare(lhs.fKey, rhs.fKey) < 0;
    });

    uint64_t size = 0;
    vector<Key> bin;
    for (int i = 0; i < keys.size(); i++) {
      Key key = keys[i];
      bin.push_back(key);
      size += key.fValueSizeCompressed;
      if (size >= kMaxFileSize) {
        uint64_t fn = fileNumber->fetch_add(1);
        if (!Write(dbname, bin, fn, result)) {
          ok->store(false);
          return {};
        }
        size = 0;
        bin.clear();
      }
    }
    if (!bin.empty()) {
      uint64_t fn = fileNumber->fetch_add(1);
      if (!Write(dbname, bin, fn, result)) {
        ok->store(false);
        return {};
      }
    }
    return result;
  }

  static bool Write(std::filesystem::path dbname, std::vector<Key> &keys, uint64_t fileNumber, std::vector<TableBuildResult> &results) {
    using namespace std;
    using namespace leveldb;
    namespace fs = std::filesystem;

    if (keys.empty()) {
      return true;
    }

    unique_ptr<WritableFile> file(OpenWritable(TableFilePath(dbname, fileNumber)));
    if (!file) {
      return false;
    }

    leveldb::Options bo;
    bo.compression = kZlibRawCompression;
    InternalKeyComparator icmp(BytewiseComparator());
    bo.comparator = &icmp;
    auto builder = make_shared<ZlibRawTableBuilder>(bo, file.get());

    bool ok = true;
    string value;
    optional<uint32_t> openedFile;
    FILE *fp = nullptr;

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
        fs::path fname = dbname / to_string(it.fWriterId) / "value.bin";
        f = mcfile::File::Open(fname, mcfile::File::Mode::Read);
        if (!f) {
          return false;
        }
        fp = f;
        openedFile = it.fWriterId;
      }
      if (!mcfile::File::Fseek(f, it.fOffset, SEEK_SET)) {
        ok = false;
        goto cleanup;
      }
      value.resize(it.fValueSizeCompressed);
      if (fread(value.data(), it.fValueSizeCompressed, 1, f) != 1) {
        ok = false;
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
    return ok;
  }

  static leveldb::WritableFile *OpenWritable(std::filesystem::path const &path) {
    using namespace leveldb;
    Env *env = Env::Default();
    WritableFile *file = nullptr;
    leveldb::Status st = env->NewWritableFile(path, &file);
    if (!st.ok()) {
      return nullptr;
    }
    return file;
  }

  static std::filesystem::path TableFilePath(std::filesystem::path const &dbname, uint64_t tableNumber) {
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
    using namespace std;
    namespace fs = std::filesystem;
    vector<shared_ptr<Writer>> writers;
    Gate::Drain((uintptr_t)this, writers);
    vector<fs::path> files;
    for (int i = 0; i < writers.size(); i++) {
      writers[i]->abandon();
    }
    fValid = false;
  }

private:
  Gate &gate() {
    using namespace std;
    thread_local unique_ptr<Gate> tGate(new Gate);
    return *tGate;
  }

private:
  std::filesystem::path const fDbName;
  std::atomic_uint64_t fSequence;
  std::atomic_uint32_t fWriterIdGenerator;
  bool fValid = true;

  static constexpr uint64_t kMaxFileSize = 2 * 1024 * 1024;
};

} // namespace je2be
