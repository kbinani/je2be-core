#pragma once

namespace je2be::tobe {

class RawDb : public DbInterface {
  struct Entry {
    uint64_t fOffset;
  };

  struct TableBuildPlan {
    size_t fFrom;
    size_t fTo;
  };

  struct TableBuildResult {
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
    return fValuesFile != nullptr;
  }

  void put(std::string const &key, leveldb::Slice const &value) override {
    using namespace std;
    using namespace std::placeholders;

    vector<uint8_t> buffer;
    buffer.resize(value.size());
    copy_n(value.data(), value.size(), buffer.begin());
    if (!mcfile::Compression::compress(buffer, Z_BEST_SPEED)) {
      return;
    }
    string cvalue((char const *)buffer.data(), buffer.size());
    vector<uint8_t>().swap(buffer);

    vector<future<void>> popped;
    {
      std::lock_guard<std::mutex> lk(fMut);
      FutureSupport::Drain(2, fFutures, popped);
      fFutures.push_back(std::move(fPool.submit(std::bind(&RawDb::putImpl, this, _1, _2), key, cvalue)));
    }

    for (auto &f : popped) {
      f.get();
    }
  }

  void del(std::string const &key) override {
    //nop because write-only
  }

  bool close(std::optional<std::function<void(double progress)>> progress = std::nullopt) override {
    using namespace std;
    using namespace std::placeholders;
    using namespace leveldb;
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

    if (!fKeysFile) {
      return false;
    }
    fclose(fKeysFile);
    fKeysFile = nullptr;

    if (!fValuesFile) {
      return false;
    }
    fclose(fValuesFile);
    fValuesFile = nullptr;

    Options o;
    o.compression = kZlibRawCompression;

    vector<TableBuildPlan> plans;
    {
      struct Key {
        string fKey;
        SequenceNumber fSequenceNumber;
        uint64_t fSizeCompressed;
      };

      file::ScopedFile keys(mcfile::File::Open(keysFile(), mcfile::File::Mode::Read));
      vector<Key> internalKeys;
      for (SequenceNumber i = 0; i < fEntries.size(); i++) {
        Entry entry = fEntries[i];
        if (_fseeki64(keys, entry.fOffset + sizeof(uint64_t), SEEK_SET) != 0) {
          return false;
        }
        uint64_t sizeCompressed = 0;
        if (fread(&sizeCompressed, sizeof(sizeCompressed), 1, keys) != 1) {
          return false;
        }
        size_t keyLength = 0;
        if (fread(&keyLength, sizeof(keyLength), 1, keys) != 1) {
          return false;
        }
        vector<char> buffer;
        buffer.resize(keyLength);
        if (fread(buffer.data(), keyLength, 1, keys) != 1) {
          return false;
        }
        string key(buffer.data(), buffer.size());
        Key k;
        k.fKey = key;
        k.fSequenceNumber = i;
        k.fSizeCompressed = sizeCompressed;
        internalKeys.push_back(k);
      }

      Comparator const *cmp = BytewiseComparator();
      InternalKeyComparator icmp(cmp);
      std::sort(std::execution::par_unseq, internalKeys.begin(), internalKeys.end(), [&icmp](Key const &lhs, Key const &rhs) {
        InternalKey ikL(lhs.fKey, lhs.fSequenceNumber, kTypeValue);
        InternalKey ikR(rhs.fKey, rhs.fSequenceNumber, kTypeValue);
        return icmp.Compare(ikL, ikR) < 0;
      });

      uint64_t currentSize = 0;
      TableBuildPlan currentPlan;
      currentPlan.fFrom = 0;
      currentPlan.fTo = 0;
      for (size_t i = 0; i < internalKeys.size(); i++) {
        Key k = internalKeys[i];
        fSortedEntryIndices.push_back(k.fSequenceNumber);
        currentSize += k.fSizeCompressed;
        currentPlan.fTo = i;
        if (currentSize > o.max_file_size) {
          plans.push_back(currentPlan);
          currentPlan.fFrom = i + 1;
          currentPlan.fTo = i + 1;
          currentSize = 0;
        }
      }
      currentPlan.fTo = internalKeys.size() - 1;
      if (currentPlan.fTo >= currentPlan.fFrom) {
        plans.push_back(currentPlan);
      }
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
    fs::remove(keysFile(), ec);

    Env *env = Env::Default();

    edit.SetLastSequence(fEntries.size() + 1);
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
    fClosed = true;
  }

private:
  void putImpl(std::string key, std::string value) {
    if (!fValuesFile) {
      return;
    }
    if (!fKeysFile) {
      return;
    }

    if (fwrite(value.data(), value.size(), 1, fValuesFile) != 1) {
      fclose(fValuesFile);
      fValuesFile = nullptr;
      return;
    }

    uint64_t offsetCompressed = fValuesPos;
    uint64_t sizeCompressed = value.size();
    if (fwrite(&offsetCompressed, sizeof(offsetCompressed), 1, fKeysFile) != 1) {
      fclose(fKeysFile);
      fKeysFile = nullptr;
      return;
    }
    if (fwrite(&sizeCompressed, sizeof(sizeCompressed), 1, fKeysFile) != 1) {
      fclose(fKeysFile);
      fKeysFile = nullptr;
      return;
    }
    size_t keySize = key.size();
    if (fwrite(&keySize, sizeof(keySize), 1, fKeysFile) != 1) {
      fclose(fKeysFile);
      fKeysFile = nullptr;
      return;
    }
    if (fwrite(key.data(), key.size(), 1, fKeysFile) != 1) {
      fclose(fKeysFile);
      fKeysFile = nullptr;
      return;
    }

    Entry entry;
    entry.fOffset = fKeysPos;
    fEntries.push_back(entry);

    fKeysPos += sizeof(offsetCompressed) + sizeof(sizeCompressed) + sizeof(keySize) + key.size();
    fValuesPos += value.size();
  }

  std::optional<TableBuildResult> buildTable(TableBuildPlan plan, size_t idx) const {
    using namespace std;
    using namespace leveldb;
    file::ScopedFile fp(mcfile::File::Open(valuesFile(), mcfile::File::Mode::Read));
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

    file::ScopedFile key(mcfile::File::Open(keysFile(), mcfile::File::Mode::Read));
    if (!key) {
      return nullopt;
    }

    vector<uint8_t> valueBuffer;
    vector<char> keyBuffer;

    InternalKey smallest;
    InternalKey largest;

    for (size_t index = plan.fFrom; index <= plan.fTo; index++) {
      size_t entryIndex = fSortedEntryIndices[index];
      Entry entry = fEntries[entryIndex];
      if (_fseeki64(key, entry.fOffset, SEEK_SET) != 0) {
        return nullopt;
      }
      uint64_t offsetCompressed = 0;
      if (fread(&offsetCompressed, sizeof(offsetCompressed), 1, key) != 1) {
        return nullopt;
      }
      uint64_t sizeCompressed = 0;
      if (fread(&sizeCompressed, sizeof(sizeCompressed), 1, key) != 1) {
        return nullopt;
      }
      size_t keySize = 0;
      if (fread(&keySize, sizeof(keySize), 1, key) != 1) {
        return nullopt;
      }
      keyBuffer.resize(keySize);
      if (fread(keyBuffer.data(), keySize, 1, key) != 1) {
        return nullopt;
      }
      valueBuffer.resize(sizeCompressed);
#if defined(_WIN32)
      auto ret = _fseeki64(fp, offsetCompressed, SEEK_SET);
#else
      auto ret = fseeko(fp, entry.fOffsetCompressed, SEEK_SET);
#endif
      if (ret != 0) {
        return nullopt;
      }
      if (fread(valueBuffer.data(), sizeCompressed, 1, fp) != 1) {
        return nullopt;
      }
      if (!mcfile::Compression::decompress(valueBuffer)) {
        return nullopt;
      }
      Slice value((char const *)valueBuffer.data(), valueBuffer.size());

      Slice userKey(keyBuffer.data(), keyBuffer.size());
      InternalKey ik(userKey, entryIndex, kTypeValue);
      builder->Add(ik.Encode(), value);

      if (index == plan.fFrom) {
        smallest = ik;
      } else if (index == plan.fTo) {
        largest = ik;
      }
    }
    builder->Finish();
    file->Close();

    TableBuildResult result;
    result.fPlan = plan;
    result.fFileSize = builder->FileSize();
    result.fFileNumber = fileNumber;
    result.fSmallest = smallest;
    result.fLargest = largest;
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

  std::filesystem::path tableFilePath(uint32_t tableNumber) const {
    std::vector<char> buffer(11, (char)0);
#if defined(_WIN32)
    sprintf_s(buffer.data(), buffer.size(), "%06d.ldb", tableNumber);
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

private:
  uint64_t fValuesPos = 0;
  uint64_t fKeysPos = 0;
  FILE *fValuesFile;
  FILE *fKeysFile;
  std::filesystem::path const fDir;
  std::mutex fMut;
  std::vector<Entry> fEntries;
  std::vector<leveldb::SequenceNumber> fSortedEntryIndices;
  ::ThreadPool fPool;
  std::deque<std::future<void>> fFutures;
  unsigned int const fConcurrency;
  bool fClosed = false;
};

} // namespace je2be::tobe
