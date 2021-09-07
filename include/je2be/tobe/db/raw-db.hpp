#pragma once

namespace je2be::tobe {

class RawDb : public DbInterface {
  struct Entry {
    std::string fKey;
    leveldb::SequenceNumber fSequenceNumber;
    uint64_t fSizeCompressed;
    uint64_t fOffsetCompressed;

    leveldb::InternalKey internalKey() const {
      leveldb::InternalKey ik(fKey, fSequenceNumber, leveldb::kTypeValue);
      return ik;
    }
  };

  struct TableBuildPlan {
    size_t fFrom;
    size_t fTo;
  };

  struct TableBuildResult {
    TableBuildPlan fPlan;
    uint64_t fFileNumber;
    uint64_t fFileSize;
  };

public:
  RawDb(std::filesystem::path const &dir, unsigned int concurrency)
      : fDir(dir), fPool(1), fConcurrency(std::max(1u, concurrency)) {
    fBuffer = mcfile::File::Open(dir / "tmp.bin", mcfile::File::Mode::Write);
    fPool.init();
  }

  RawDb(RawDb &&) = delete;
  RawDb &operator=(RawDb &&) = delete;

  ~RawDb() {
    close();
  }

  bool valid() const override {
    return fBuffer != nullptr;
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

    if (!fBuffer) {
      return false;
    }
    fclose(fBuffer);
    fBuffer = nullptr;

    Comparator const *cmp = BytewiseComparator();
    InternalKeyComparator icmp(cmp);
    std::sort(std::execution::par_unseq, fEntries.begin(), fEntries.end(), [&icmp](Entry const &lhs, Entry const &rhs) {
      return icmp.Compare(lhs.internalKey(), rhs.internalKey()) < 0;
    });

    Options o;
    o.compression = kZlibRawCompression;

    vector<TableBuildPlan> plans;
    uint64_t currentSize = 0;
    TableBuildPlan currentPlan;
    currentPlan.fFrom = 0;
    currentPlan.fTo = 0;

    for (size_t index = 0; index < fEntries.size(); index++) {
      Entry entry = fEntries[index];
      currentSize += entry.fSizeCompressed;
      currentPlan.fTo = index;
      if (currentSize > o.max_file_size) {
        plans.push_back(currentPlan);
        currentPlan.fFrom = index + 1;
        currentPlan.fTo = index + 1;
        currentSize = 0;
      }
    }
    currentPlan.fTo = fEntries.size() - 1;
    if (currentPlan.fTo >= currentPlan.fFrom) {
      plans.push_back(currentPlan);
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
        Entry smallest = fEntries[result->fPlan.fFrom];
        Entry largest = fEntries[result->fPlan.fTo];
        edit.AddFile(1, result->fFileNumber, result->fFileSize, smallest.internalKey(), largest.internalKey());
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
      Entry smallest = fEntries[result->fPlan.fFrom];
      Entry largest = fEntries[result->fPlan.fTo];
      edit.AddFile(1, result->fFileNumber, result->fFileSize, smallest.internalKey(), largest.internalKey());
      done++;
      if (progress) {
        double p = (double)done / (double)total;
        (*progress)(p);
      }
    }

    pool.shutdown();

    error_code ec;
    fs::remove(fDir / "tmp.bin", ec);

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
    if (fBuffer) {
      fclose(fBuffer);
      fBuffer = nullptr;
    }
    error_code ec;
    fs::remove(fDir / "tmp.bin", ec);
    fClosed = true;
  }

private:
  void putImpl(std::string key, std::string value) {
    if (!fBuffer) {
      return;
    }

    if (fwrite(value.data(), value.size(), 1, fBuffer) != 1) {
      fclose(fBuffer);
      fBuffer = nullptr;
      return;
    }
    Entry entry;
    entry.fKey = key;
    entry.fSequenceNumber = fEntries.size();
    entry.fOffsetCompressed = fPos;
    entry.fSizeCompressed = value.size();
    fEntries.push_back(entry);
    fPos += value.size();
  }

  void unsafeFinalize() {
  }

  std::optional<TableBuildResult> buildTable(TableBuildPlan plan, size_t idx) const {
    using namespace std;
    using namespace leveldb;
    file::ScopedFile fp(mcfile::File::Open(fDir / "tmp.bin", mcfile::File::Mode::Read));
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

    vector<uint8_t> buffer;
    for (size_t index = plan.fFrom; index <= plan.fTo; index++) {
      Entry entry = fEntries[index];
      buffer.resize(entry.fSizeCompressed);
#if defined(_WIN32)
      auto ret = _fseeki64(fp, entry.fOffsetCompressed, SEEK_SET);
#else
      auto ret = fseeko(fp, entry.fOffsetCompressed, SEEK_SET);
#endif
      if (ret != 0) {
        return nullopt;
      }
      if (fread(buffer.data(), entry.fSizeCompressed, 1, fp) != 1) {
        return nullopt;
      }
      if (!mcfile::Compression::decompress(buffer)) {
        return nullopt;
      }
      Slice value((char const *)buffer.data(), buffer.size());

      InternalKey ik = entry.internalKey();
      builder->Add(ik.Encode(), value);
    }
    builder->Finish();
    file->Close();

    TableBuildResult result;
    result.fPlan = plan;
    result.fFileSize = builder->FileSize();
    result.fFileNumber = fileNumber;
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

private:
  uint64_t fPos = 0;
  FILE *fBuffer;
  std::filesystem::path const fDir;
  std::mutex fMut;
  std::vector<Entry> fEntries;
  ::ThreadPool fPool;
  std::deque<std::future<void>> fFutures;
  unsigned int const fConcurrency;
  bool fClosed = false;
};

} // namespace je2be::tobe
