#pragma once

namespace je2be {

class RawDb : public DbInterface {
  struct ShardLocator {
    ShardLocator(uint8_t shard, uint64_t indexInShard, uint64_t keyFileOffset)
        : fShard(shard), fIndexInShard(indexInShard), fKeyFileOffset(keyFileOffset) {}

    uint8_t fShard;
    uint64_t fIndexInShard;
    uint64_t fKeyFileOffset;
  };

  struct TableBuildPlan {
    TableBuildPlan(ShardLocator from, ShardLocator to) : fFrom(from), fTo(to) {}
    ShardLocator fFrom;
    ShardLocator fTo;
  };

  struct TableBuildResult {
    TableBuildResult(TableBuildPlan plan, uint64_t fileNumber, uint64_t fileSize, leveldb::InternalKey smallest, leveldb::InternalKey largest)
        : fPlan(plan), fFileNumber(fileNumber), fFileSize(fileSize), fSmallest(smallest), fLargest(largest) {}

    TableBuildPlan fPlan;
    uint64_t fFileNumber;
    uint64_t fFileSize;
    leveldb::InternalKey fSmallest;
    leveldb::InternalKey fLargest;
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
      //Flush();
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
  RawDb(std::filesystem::path const &dir, int concurrency)
      : fValuesFile(nullptr), fKeysFile(nullptr), fDir(dir), fConcurrency(concurrency), fLock(nullptr), fSeq(0) {

    if (concurrency > 0) {
      fQueue.reset(new hwm::task_queue(1));
    }

    leveldb::DestroyDB(dir, {});
    std::error_code ec;
    std::filesystem::create_directories(dir, ec);
    if (ec) {
      return;
    }

    auto env = leveldb::Env::Default();
    leveldb::FileLock *lock = nullptr;
    auto st = env->LockFile(dir / "LOCK", &lock);
    if (!st.ok()) {
      return;
    }
    fLock = lock;

    fValuesFile = mcfile::File::Open(valuesFile(), mcfile::File::Mode::Write);
    fKeysFile = mcfile::File::Open(keysFile(), mcfile::File::Mode::Write);
  }

  RawDb(RawDb &&) = delete;
  RawDb &operator=(RawDb &&) = delete;

  ~RawDb() {
    close();

    if (fLock) {
      auto env = leveldb::Env::Default();
      env->UnlockFile(fLock);
      env->RemoveFile(fDir / "LOCK");
      fLock = nullptr;
    }
  }

  bool valid() const override {
    return fValuesFile != nullptr && fKeysFile != nullptr;
  }

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

  void put(std::string const &key, leveldb::Slice const &value) override {
    using namespace std;
    using namespace std::placeholders;
    using namespace leveldb;

    if (!valid()) {
      return;
    }

    uint64_t sequence = fSeq.fetch_add(1);

    string block;
    Compress(key, value, sequence, &block);

    bool ok = true;
    if (fQueue) {
      vector<future<bool>> popped;
      {
        std::lock_guard<std::mutex> lk(fMut);
        FutureSupport::Drain(2, fFutures, popped);
        fFutures.push_back(fQueue->enqueue(std::bind(&RawDb::putImpl, this, _1, _2, _3), key, block, sequence));
      }

      for (auto &f : popped) {
        ok &= f.get();
      }
    } else {
      ok = putImpl(key, block, sequence);
    }

    if (!ok) {
      if (fValuesFile) {
        fclose(fValuesFile);
        fValuesFile = nullptr;
      }
      if (fKeysFile) {
        fclose(fKeysFile);
        fKeysFile = nullptr;
      }
    }
  }

  void del(std::string const &key) override {
    //nop because write-only
  }

  bool close(std::optional<std::function<void(double progress)>> progress = std::nullopt) override {
    using namespace std;
    using namespace std::placeholders;
    using namespace leveldb;
    using namespace mcfile;
    namespace fs = std::filesystem;

    if (fClosed) {
      return false;
    }
    fClosed = true;
    if (progress) {
      (*progress)(0.0);
    }

    for (auto &f : fFutures) {
      f.get();
    }
    fQueue.reset();

    if (!fValuesFile) {
      return false;
    }
    fclose(fValuesFile);
    fValuesFile = nullptr;

    if (!fKeysFile) {
      return false;
    }
    fclose(fKeysFile);
    fKeysFile = nullptr;

    leveldb::Options o;
    o.compression = kZlibRawCompression;

    if (!shardKeysFile()) {
      return false;
    }

    vector<TableBuildPlan> plans;
    if (!sortKeys(plans, o)) {
      return false;
    }

    VersionEdit edit;
    uint64_t done = 0;
    uint64_t total = plans.size() + 1; // +1 stands for MANIFEST-000001, CURRENT, and remaining cleanup tasks

    if (fConcurrency > 0) {
      auto queue = make_unique<hwm::task_queue>(fConcurrency);

      deque<future<optional<TableBuildResult>>> futures;
      for (size_t idx = 0; idx < plans.size(); idx++) {
        vector<future<optional<TableBuildResult>>> drain;
        FutureSupport::Drain<optional<TableBuildResult>>(fConcurrency + 1, futures, drain);
        for (auto &f : drain) {
          auto result = f.get();
          if (!result) {
            continue;
          }
          InternalKey smallest = result->fSmallest;
          InternalKey largest = result->fLargest;
          edit.AddFile(1, result->fFileNumber, result->fFileSize, smallest, largest);
          done++;
          if (progress) {
            double p = (double)done / (double)total;
            (*progress)(p);
          }
        }

        TableBuildPlan plan = plans[idx];
        futures.push_back(queue->enqueue(bind(&RawDb::buildTable, this, _1, _2), plan, idx));
      }

      for (auto &f : futures) {
        auto result = f.get();
        if (!result) {
          continue;
        }
        InternalKey smallest = result->fSmallest;
        InternalKey largest = result->fLargest;
        edit.AddFile(1, result->fFileNumber, result->fFileSize, smallest, largest);
        done++;
        if (progress) {
          double p = (double)done / (double)total;
          (*progress)(p);
        }
      }
    } else {
      for (size_t idx = 0; idx < plans.size(); idx++) {
        TableBuildPlan plan = plans[idx];
        auto result = buildTable(plan, idx);
        InternalKey smallest = result->fSmallest;
        InternalKey largest = result->fLargest;
        edit.AddFile(1, result->fFileNumber, result->fFileSize, smallest, largest);
        done++;
        if (progress) {
          double p = (double)done / (double)total;
          (*progress)(p);
        }
      }
    }

    error_code ec;
    fs::remove(valuesFile(), ec);
    removeKeysFiles();

    edit.SetLastSequence(fNumKeys + 1);
    edit.SetNextFile(plans.size() + 1);
    edit.SetLogNumber(0);
    string manifestRecord;
    edit.EncodeTo(&manifestRecord);

    string manifestFileName = "MANIFEST-000001";
    fs::path manifestFile = fDir / manifestFileName;
    unique_ptr<WritableFile> meta(OpenWritable(manifestFile));
    leveldb::log::Writer writer(meta.get());
    leveldb::Status st = writer.AddRecord(manifestRecord);
    if (!st.ok()) {
      return false;
    }
    meta->Close();
    meta.reset();
    if (fOnFileCreated) {
      (*fOnFileCreated)(manifestFile);
    }

    fs::path currentFile = fDir / "CURRENT";
    unique_ptr<WritableFile> current(OpenWritable(currentFile));
    st = current->Append(manifestFileName + "\x0a");
    if (!st.ok()) {
      return false;
    }
    current->Close();
    current.reset();
    if (fOnFileCreated) {
      (*fOnFileCreated)(currentFile);
    }
    if (progress) {
      (*progress)(1.0);
    }
    return true;
  }

  void abandon() override {
    using namespace std;
    namespace fs = std::filesystem;
    if (fClosed) {
      return;
    }
    for (auto &f : fFutures) {
      f.get();
    }
    fQueue.reset();
    if (fValuesFile) {
      fclose(fValuesFile);
      fValuesFile = nullptr;
    }
    if (fKeysFile) {
      fclose(fKeysFile);
      fKeysFile = nullptr;
    }
    error_code ec;
    fs::remove(valuesFile(), ec);
    removeKeysFiles();
    fClosed = true;
  }

private:
  void removeKeysFiles() {
    using namespace std;
    namespace fs = std::filesystem;
    for (auto it : fNumKeysPerShard) {
      auto file = keysShardFile(it.first);
      error_code ec;
      fs::remove(file, ec);
    }
  }

  bool shardKeysFile() {
    using namespace std;
    using namespace leveldb;
    using namespace mcfile;
    namespace fs = std::filesystem;

    FILE *keyFiles[256] = {nullptr};
    auto keysBin = keysFile();
    FILE *fp = File::Open(keysBin, File::Mode::Read);
    if (!fp) {
      return false;
    }

    vector<char> keyBuffer;

    bool ok = false;
    for (uint64_t i = 0; i < fNumKeys; i++) {
      uint64_t offsetCompressed;
      uint64_t sizeCompressed;
      size_t keySize;
      uint64_t sequence;
      if (fread(&offsetCompressed, sizeof(offsetCompressed), 1, fp) != 1) {
        goto cleanup;
      }
      if (fread(&sizeCompressed, sizeof(sizeCompressed), 1, fp) != 1) {
        goto cleanup;
      }
      if (fread(&keySize, sizeof(keySize), 1, fp) != 1) {
        goto cleanup;
      }
      if (fread(&sequence, sizeof(sequence), 1, fp) != 1) {
        goto cleanup;
      }
      keyBuffer.resize(keySize);
      if (fread(keyBuffer.data(), keySize, 1, fp) != 1) {
        goto cleanup;
      }
      uint8_t shard = keyBuffer[0];
      if (!keyFiles[shard]) {
        auto file = keysShardFile(shard);
        FILE *f = File::Open(file, File::Mode::Write);
        if (!f) {
          goto cleanup;
        }
        keyFiles[shard] = f;
      }
      fNumKeysPerShard[shard] += 1;
      FILE *out = keyFiles[shard];
      if (fwrite(&offsetCompressed, sizeof(offsetCompressed), 1, out) != 1) {
        goto cleanup;
      }
      if (fwrite(&sizeCompressed, sizeof(sizeCompressed), 1, out) != 1) {
        goto cleanup;
      }
      size_t keySizeWithoutCap = keySize - 1;
      if (fwrite(&keySizeWithoutCap, sizeof(keySizeWithoutCap), 1, out) != 1) {
        goto cleanup;
      }
      if (fwrite(&sequence, sizeof(sequence), 1, out) != 1) {
        goto cleanup;
      }
      if (fwrite(keyBuffer.data() + 1, keySizeWithoutCap, 1, out) != 1) {
        goto cleanup;
      }
    }
    ok = true;

  cleanup:
    fclose(fp);
    fp = nullptr;

    error_code ec;
    fs::remove(keysBin, ec);

    for (int i = 0; i < 256; i++) {
      if (keyFiles[i]) {
        fclose(keyFiles[i]);
        keyFiles[i] = nullptr;
      }
    }
    return ok;
  }

  bool sortKeys(std::vector<TableBuildPlan> &plans, leveldb::Options const &o) {
    using namespace std;
    using namespace leveldb;
    using namespace mcfile;

    struct Key {
      Key(string key, uint64_t offsetCompressed, uint64_t sizeCompressed, uint64_t sequence)
          : fKey(key), fOffsetCompressed(offsetCompressed), fSizeCompressed(sizeCompressed), fSequence(sequence) {}

      string fKey;
      uint64_t fOffsetCompressed;
      uint64_t fSizeCompressed;
      uint64_t fSequence;
    };

    uint64_t size = 0;
    optional<ShardLocator> from;
    optional<ShardLocator> last;

    vector<char> keyBuffer;
    vector<Key> keys;

    for (auto it : fNumKeysPerShard) {
      uint8_t shard = it.first;
      uint64_t numKeys = it.second;

      keys.clear();
      {
        ScopedFile fp(File::Open(keysShardFile(shard), File::Mode::Read));
        if (!fp) {
          return false;
        }

        for (uint64_t i = 0; i < numKeys; i++) {
          uint64_t offsetCompressed = 0;
          if (fread(&offsetCompressed, sizeof(offsetCompressed), 1, fp.get()) != 1) {
            return false;
          }
          uint64_t sizeCompressed = 0;
          if (fread(&sizeCompressed, sizeof(sizeCompressed), 1, fp.get()) != 1) {
            return false;
          }
          size_t keySize = 0;
          if (fread(&keySize, sizeof(keySize), 1, fp.get()) != 1) {
            return false;
          }
          uint64_t sequence;
          if (fread(&sequence, sizeof(sequence), 1, fp.get()) != 1) {
            return false;
          }
          keyBuffer.resize(keySize + 1);
          if (fread(keyBuffer.data() + 1, keySize, 1, fp.get()) != 1) {
            return false;
          }
          keyBuffer[0] = shard;
          string key(keyBuffer.data(), keyBuffer.size());
          Key k(key, offsetCompressed, sizeCompressed, sequence);
          keys.push_back(k);
        }
        Comparator const *cmp = BytewiseComparator();
#if defined(EMSCRIPTEN) || defined(__APPLE__)
        std::sort(
#else
        std::sort(execution::par_unseq,
#endif
            keys.begin(), keys.end(), [cmp](Key const &lhs, Key const &rhs) {
              return cmp->Compare(lhs.fKey, rhs.fKey) < 0;
            });
      }
      {
        ScopedFile fp(File::Open(keysShardFile(shard), File::Mode::Write));
        if (!fp) {
          return false;
        }
        uint64_t offset = 0;
        for (size_t i = 0; i < keys.size(); i++) {
          Key key = keys[i];

          if (fwrite(&key.fOffsetCompressed, sizeof(key.fOffsetCompressed), 1, fp.get()) != 1) {
            return false;
          }
          if (fwrite(&key.fSizeCompressed, sizeof(key.fSizeCompressed), 1, fp.get()) != 1) {
            return false;
          }
          size_t keySize = key.fKey.size() - 1;
          if (fwrite(&keySize, sizeof(keySize), 1, fp.get()) != 1) {
            return false;
          }
          uint64_t sequence = key.fSequence;
          if (fwrite(&sequence, sizeof(sequence), 1, fp.get()) != 1) {
            return false;
          }
          if (fwrite(key.fKey.data() + 1, keySize, 1, fp.get()) != 1) {
            return false;
          }
          size += key.fSizeCompressed;
          last = ShardLocator(shard, i, offset);
          if (!from) {
            from = last;
          }
          if (size >= o.max_file_size) {
            TableBuildPlan plan(*from, *last);

            plans.push_back(plan);
            size = 0;
            from = nullopt;
          }
          offset += sizeof(key.fOffsetCompressed) + sizeof(key.fSizeCompressed) + sizeof(keySize) + sizeof(sequence) + keySize;
        }
      }
    }

    if (from) {
      TableBuildPlan plan(*from, *last);
      plans.push_back(plan);
    }
    return true;
  }

  bool putImpl(std::string key, std::string value, uint64_t sequence) {
    using namespace mcfile;

    if (!fValuesFile) {
      return false;
    }
    if (!fKeysFile) {
      return false;
    }

    if (fwrite(value.data(), value.size(), 1, fValuesFile) != 1) {
      fclose(fValuesFile);
      fValuesFile = nullptr;
      return false;
    }

    uint64_t offsetCompressed = fValuesPos;
    uint64_t sizeCompressed = value.size();
    size_t keySize = key.size();
    if (fwrite(&offsetCompressed, sizeof(offsetCompressed), 1, fKeysFile) != 1) {
      fclose(fKeysFile);
      fKeysFile = nullptr;
      return false;
    }
    if (fwrite(&sizeCompressed, sizeof(sizeCompressed), 1, fKeysFile) != 1) {
      fclose(fKeysFile);
      fKeysFile = nullptr;
      return false;
    }
    if (fwrite(&keySize, sizeof(keySize), 1, fKeysFile) != 1) {
      fclose(fKeysFile);
      fKeysFile = nullptr;
      return false;
    }
    if (fwrite(&sequence, sizeof(sequence), 1, fKeysFile) != 1) {
      fclose(fKeysFile);
      fKeysFile = nullptr;
      return false;
    }
    if (fwrite(key.data(), key.size(), 1, fKeysFile) != 1) {
      fclose(fKeysFile);
      fKeysFile = nullptr;
      return false;
    }

    fValuesPos += sizeCompressed;
    fNumKeys++;

    return true;
  }

  std::optional<TableBuildResult> buildTable(TableBuildPlan plan, size_t idx) const {
    using namespace std;
    using namespace leveldb;
    using namespace mcfile;
    namespace fs = std::filesystem;
    ScopedFile fp(File::Open(valuesFile(), File::Mode::Read));
    if (!fp) {
      return nullopt;
    }
    leveldb::Options bo;
    bo.compression = kZlibRawCompression;
    InternalKeyComparator icmp(BytewiseComparator());
    bo.comparator = &icmp;

    uint64_t fileNumber = idx + 1;
    fs::path tableFilePath = this->tableFilePath(fileNumber);
    unique_ptr<WritableFile> file(OpenWritable(tableFilePath));
    if (!file) {
      return nullopt;
    }
    unique_ptr<ZlibRawTableBuilder> builder(new ZlibRawTableBuilder(bo, file.get()));

    vector<uint8_t> valueBuffer;
    vector<char> keyBuffer;

    optional<InternalKey> smallest;
    InternalKey largest;

    for (int shard = plan.fFrom.fShard; shard <= plan.fTo.fShard; shard++) {
      uint64_t localIndex = 0;
      if (shard == plan.fFrom.fShard) {
        localIndex = plan.fFrom.fIndexInShard;
      }

      if (fNumKeysPerShard.find(shard) == fNumKeysPerShard.end()) {
        continue;
      }

      ScopedFile keyFile(File::Open(keysShardFile(shard), File::Mode::Read));
      if (!keyFile) {
        return nullopt;
      }
      if (shard == plan.fFrom.fShard) {
        if (!File::Fseek(keyFile.get(), plan.fFrom.fKeyFileOffset, SEEK_SET)) {
          return nullopt;
        }
      }
      while (true) {
        uint64_t offsetCompressed = 0;
        if (fread(&offsetCompressed, sizeof(offsetCompressed), 1, keyFile.get()) != 1) {
          if (feof(keyFile.get()) != 0) {
            break;
          }
          return nullopt;
        }
        uint64_t sizeCompressed = 0;
        if (fread(&sizeCompressed, sizeof(sizeCompressed), 1, keyFile.get()) != 1) {
          return nullopt;
        }
        size_t keySize = 0;
        if (fread(&keySize, sizeof(keySize), 1, keyFile.get()) != 1) {
          return nullopt;
        }
        uint64_t sequence;
        if (fread(&sequence, sizeof(sequence), 1, keyFile.get()) != 1) {
          return nullopt;
        }
        keyBuffer.resize(keySize + 1);
        if (fread(keyBuffer.data() + 1, keySize, 1, keyFile.get()) != 1) {
          return nullopt;
        }
        keyBuffer[0] = shard;
        valueBuffer.resize(sizeCompressed);
        if (!File::Fseek(fp.get(), offsetCompressed, SEEK_SET)) {
          return nullopt;
        }
        if (fread(valueBuffer.data(), sizeCompressed, 1, fp.get()) != 1) {
          return nullopt;
        }
        Slice value((char const *)valueBuffer.data(), valueBuffer.size());

        Slice userKey(keyBuffer.data(), keyBuffer.size());
        InternalKey ik(userKey, sequence, kTypeValue);
        builder->AddAlreadyCompressedAndFlush(ik.Encode(), value);

        if (!smallest) {
          smallest = ik;
        }
        largest = ik;
        if (shard == plan.fTo.fShard && localIndex == plan.fTo.fIndexInShard) {
          break;
        }

        ++localIndex;
      }
    }

    builder->Finish();
    file->Close();

    if (fOnFileCreated) {
      (*fOnFileCreated)(tableFilePath);
    }

    TableBuildResult result(plan, fileNumber, builder->FileSize(), *smallest, largest);
    return result;
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

  leveldb::WritableFile *openTableFile(uint64_t tableNumber) const {
    return OpenWritable(tableFilePath(tableNumber));
  }

  std::filesystem::path tableFilePath(uint64_t tableNumber) const {
    std::vector<char> buffer(11, (char)0);
#if defined(_WIN32)
    sprintf_s(buffer.data(), buffer.size(), "%06llu.ldb", tableNumber);
#else
    snprintf(buffer.data(), buffer.size(), "%06llu.ldb", tableNumber);
#endif
    std::string p(buffer.data(), 10);
    return fDir / p;
  }

  std::filesystem::path valuesFile() const {
    return fDir / "values.bin";
  }

  std::filesystem::path keysFile() const {
    return fDir / "keys.bin";
  }

  std::filesystem::path keysShardFile(uint8_t shared) const {
    return fDir / ("keys" + std::to_string((int)shared) + ".bin");
  }

public:
  std::optional<std::function<void(std::filesystem::path)>> fOnFileCreated;

private:
  uint64_t fValuesPos = 0;
  FILE *fValuesFile;
  uint64_t fKeysPos = 0;
  uint64_t fNumKeys = 0;
  FILE *fKeysFile;
  std::filesystem::path const fDir;
  std::mutex fMut;
  std::map<uint8_t, uint64_t> fNumKeysPerShard;
  std::unique_ptr<hwm::task_queue> fQueue;
  std::deque<std::future<bool>> fFutures;
  int const fConcurrency;
  bool fClosed = false;
  leveldb::FileLock *fLock;
  std::atomic_uint64_t fSeq;
};

} // namespace je2be
