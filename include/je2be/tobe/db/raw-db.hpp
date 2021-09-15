#pragma once

namespace je2be::tobe {

class RawDb : public DbInterface {
  struct ShardLocator {
    ShardLocator(uint8_t shard, uint64_t indexInShard, uint64_t keyFileOffset, leveldb::SequenceNumber sequence)
        : fShard(shard), fIndexInShard(indexInShard), fKeyFileOffset(keyFileOffset), fSequence(sequence) {}

    uint8_t fShard;
    uint64_t fIndexInShard;
    uint64_t fKeyFileOffset;
    leveldb::SequenceNumber fSequence;
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

public:
  RawDb(std::filesystem::path const &dir, unsigned int concurrency)
      : fDir(dir), fPool(1), fConcurrency(std::max(1u, concurrency)) {
    fValuesFile = mcfile::File::Open(valuesFile(), mcfile::File::Mode::Write);
    fKeysFile = mcfile::File::Open(keysFile(), mcfile::File::Mode::Write);
    fPool.init();
  }

  RawDb(RawDb &&) = delete;
  RawDb &operator=(RawDb &&) = delete;

  ~RawDb() {
    close();
  }

  bool valid() const override {
    return fValuesFile != nullptr && fKeysFile != nullptr;
  }

  void put(std::string const &key, leveldb::Slice const &value) override {
    using namespace std;
    using namespace std::placeholders;

    if (!valid()) {
      return;
    }

    vector<uint8_t> buffer;
    buffer.resize(value.size());
    copy_n(value.data(), value.size(), buffer.begin());
    if (!mcfile::Compression::compress(buffer, Z_BEST_SPEED)) {
      return;
    }
    string cvalue((char const *)buffer.data(), buffer.size());
    vector<uint8_t>().swap(buffer);

    vector<future<bool>> popped;
    {
      std::lock_guard<std::mutex> lk(fMut);
      FutureSupport::Drain(2, fFutures, popped);
      fFutures.push_back(std::move(fPool.submit(std::bind(&RawDb::putImpl, this, _1, _2), key, cvalue)));
    }

    bool ok = true;
    for (auto &f : popped) {
      ok &= f.get();
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
    fPool.shutdown();

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

    Options o;
    o.compression = kZlibRawCompression;

    if (!shardKeysFile()) {
      return false;
    }

    vector<TableBuildPlan> plans;
    if (!sortKeys(plans, o)) {
      return false;
    }

    ::ThreadPool pool(fConcurrency);
    pool.init();

    VersionEdit edit;
    uint64_t done = 0;
    uint64_t total = plans.size() + 1; // +1 stands for MANIFEST-000001, CURRENT, and remaining cleanup tasks

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
      futures.push_back(move(pool.submit(bind(&RawDb::buildTable, this, _1, _2), plan, idx)));
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

    pool.shutdown();

    error_code ec;
    fs::remove(valuesFile(), ec);
    removeKeysFiles();

    edit.SetLastSequence(fNumKeys + 1);
    edit.SetNextFile(plans.size() + 1);
    edit.SetLogNumber(0);
    string manifestRecord;
    edit.EncodeTo(&manifestRecord);

    string manifestFileName = "MANIFEST-000001";

    unique_ptr<WritableFile> meta(OpenWritable(fDir / manifestFileName));
    leveldb::log::Writer writer(meta.get());
    Status st = writer.AddRecord(manifestRecord);
    if (!st.ok()) {
      return false;
    }
    meta->Close();
    meta.reset();

    unique_ptr<WritableFile> current(OpenWritable(fDir / "CURRENT"));
    st = current->Append(manifestFileName + "\x0a");
    if (!st.ok()) {
      return false;
    }
    current->Close();
    current.reset();
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
    fPool.shutdown();
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
      if (fread(&offsetCompressed, sizeof(offsetCompressed), 1, fp) != 1) {
        goto cleanup;
      }
      if (fread(&sizeCompressed, sizeof(sizeCompressed), 1, fp) != 1) {
        goto cleanup;
      }
      if (fread(&keySize, sizeof(keySize), 1, fp) != 1) {
        goto cleanup;
      }
      keyBuffer.resize(keySize);
      if (fread(keyBuffer.data(), keySize, 1, fp) != 1) {
        goto cleanup;
      }
      uint8_t shard = keyBuffer[0];
      if (!keyFiles[shard]) {
        auto file = keysShardFile(shard);
        keyFiles[shard] = File::Open(file, File::Mode::Write);
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
      Key(string key, uint64_t offsetCompressed, uint64_t sizeCompressed)
          : fKey(key), fOffsetCompressed(offsetCompressed), fSizeCompressed(sizeCompressed) {}

#if defined(TBB_INTERFACE_VERSION)
      // The backend of std::sort(execution::par_unseq, ...) requires the element type to be default constructive.
      Key() = default;
#endif

      string fKey;
      uint64_t fOffsetCompressed;
      uint64_t fSizeCompressed;
    };

    SequenceNumber sequence = 0;

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

        for (uint64_t i = 0; i < numKeys; i++) {
          uint64_t offsetCompressed = 0;
          if (fread(&offsetCompressed, sizeof(offsetCompressed), 1, fp) != 1) {
            return false;
          }
          uint64_t sizeCompressed = 0;
          if (fread(&sizeCompressed, sizeof(sizeCompressed), 1, fp) != 1) {
            return false;
          }
          size_t keySize = 0;
          if (fread(&keySize, sizeof(keySize), 1, fp) != 1) {
            return false;
          }
          keyBuffer.resize(keySize + 1);
          if (fread(keyBuffer.data() + 1, keySize, 1, fp) != 1) {
            return false;
          }
          keyBuffer[0] = shard;
          string key(keyBuffer.data(), keyBuffer.size());
          Key k(key, offsetCompressed, sizeCompressed);
          keys.push_back(k);
        }
        Comparator const *cmp = BytewiseComparator();
        InternalKeyComparator icmp(cmp);
        sort(execution::par_unseq, keys.begin(), keys.end(), [cmp](Key const &lhs, Key const &rhs) {
          return cmp->Compare(lhs.fKey, rhs.fKey) < 0;
        });
      }
      {
        ScopedFile fp(File::Open(keysShardFile(shard), File::Mode::Write));
        uint64_t offset = 0;
        for (size_t i = 0; i < keys.size(); i++) {
          Key key = keys[i];

          if (fwrite(&key.fOffsetCompressed, sizeof(key.fOffsetCompressed), 1, fp) != 1) {
            return false;
          }
          if (fwrite(&key.fSizeCompressed, sizeof(key.fSizeCompressed), 1, fp) != 1) {
            return false;
          }
          size_t keySize = key.fKey.size() - 1;
          if (fwrite(&keySize, sizeof(keySize), 1, fp) != 1) {
            return false;
          }
          if (fwrite(key.fKey.data() + 1, keySize, 1, fp) != 1) {
            return false;
          }
          size += key.fSizeCompressed;
          last = ShardLocator(shard, i, offset, sequence);
          if (!from) {
            from = last;
          }
          if (size >= o.max_file_size) {
            TableBuildPlan plan(*from, *last);

            plans.push_back(plan);
            size = 0;
            from = nullopt;
          }
          sequence++;
          offset += sizeof(key.fOffsetCompressed) + sizeof(key.fSizeCompressed) + sizeof(keySize) + keySize;
        }
      }
    }

    if (from) {
      TableBuildPlan plan(*from, *last);
      plans.push_back(plan);
    }
    return true;
  }

  bool putImpl(std::string key, std::string value) {
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
    ScopedFile fp(File::Open(valuesFile(), File::Mode::Read));
    if (!fp) {
      return nullopt;
    }
    Options bo;
    bo.compression = kZlibRawCompression;
    InternalKeyComparator icmp(BytewiseComparator());
    bo.comparator = &icmp;

    uint64_t fileNumber = idx + 1;
    unique_ptr<WritableFile> file(openTableFile(fileNumber));
    if (!file) {
      return nullopt;
    }
    unique_ptr<TableBuilder> builder(new TableBuilder(bo, file.get()));

    vector<uint8_t> valueBuffer;
    vector<char> keyBuffer;
    SequenceNumber sequence = plan.fFrom.fSequence;

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
        if (!File::Fseek(keyFile, plan.fFrom.fKeyFileOffset, SEEK_SET)) {
          return nullopt;
        }
      }
      while (true) {
        uint64_t offsetCompressed = 0;
        if (fread(&offsetCompressed, sizeof(offsetCompressed), 1, keyFile) != 1) {
          if (feof(keyFile) != 0) {
            break;
          }
          return nullopt;
        }
        uint64_t sizeCompressed = 0;
        if (fread(&sizeCompressed, sizeof(sizeCompressed), 1, keyFile) != 1) {
          return nullopt;
        }
        size_t keySize = 0;
        if (fread(&keySize, sizeof(keySize), 1, keyFile) != 1) {
          return nullopt;
        }
        keyBuffer.resize(keySize + 1);
        if (fread(keyBuffer.data() + 1, keySize, 1, keyFile) != 1) {
          return nullopt;
        }
        keyBuffer[0] = shard;
        valueBuffer.resize(sizeCompressed);
        if (!File::Fseek(fp, offsetCompressed, SEEK_SET)) {
          return nullopt;
        }
        if (fread(valueBuffer.data(), sizeCompressed, 1, fp) != 1) {
          return nullopt;
        }
        if (!Compression::decompress(valueBuffer)) {
          return nullopt;
        }
        Slice value((char const *)valueBuffer.data(), valueBuffer.size());

        Slice userKey(keyBuffer.data(), keyBuffer.size());
        InternalKey ik(userKey, sequence, kTypeValue);
        builder->Add(ik.Encode(), value);

        if (!smallest) {
          smallest = ik;
        }
        largest = ik;
        if (shard == plan.fTo.fShard && localIndex == plan.fTo.fIndexInShard) {
          break;
        }

        ++localIndex;
        ++sequence;
      }
    }

    builder->Finish();
    file->Close();

    TableBuildResult result(plan, fileNumber, builder->FileSize(), *smallest, largest);
    return result;
  }

  static leveldb::WritableFile *OpenWritable(std::filesystem::path const &path) {
    using namespace leveldb;
    Env *env = Env::Default();
    WritableFile *file = nullptr;
    Status st = env->NewWritableFile(path, &file);
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
    sprintf_s(buffer.data(), buffer.size(), "%06lld.ldb", tableNumber);
#else
    sprintf(buffer.data(), "%06d.ldb", tableNumber);
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

private:
  uint64_t fValuesPos = 0;
  FILE *fValuesFile;
  uint64_t fKeysPos = 0;
  uint64_t fNumKeys = 0;
  FILE *fKeysFile;
  std::filesystem::path const fDir;
  std::mutex fMut;
  std::map<uint8_t, uint64_t> fNumKeysPerShard;
  ::ThreadPool fPool;
  std::deque<std::future<bool>> fFutures;
  unsigned int const fConcurrency;
  bool fClosed = false;
};

} // namespace je2be::tobe
