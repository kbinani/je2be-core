#pragma once

namespace je2be::tobe {

class BufferedDb : public DbInterface {
  struct Entry {
    std::string fKey;
    uint64_t fSizeCompressed;
    uint64_t fOffsetCompressed;
  };

public:
  explicit BufferedDb(std::filesystem::path const &dir)
      : fDir(dir), fPool(1) {
    fBuffer = mcfile::File::Open(dir / "tmp.bin", mcfile::File::Mode::Write);
    fPool.init();
  }

  BufferedDb(BufferedDb &&) = delete;
  BufferedDb &operator=(BufferedDb &&) = delete;

  ~BufferedDb() {
    try {
      unsafeFinalize();
    } catch (...) {
    }
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
    mcfile::Compression::compress(buffer);
    string cvalue((char const *)buffer.data(), buffer.size());
    vector<uint8_t>().swap(buffer);

    vector<future<void>> popped;

    {
      std::lock_guard<std::mutex> lk(fMut);
      int pop = fFutures.size() - 2;
      for (int i = 0; i < pop; i++) {
        popped.emplace_back(move(fFutures.front()));
        fFutures.pop_front();
      }
      fFutures.push_back(std::move(fPool.submit(std::bind(&BufferedDb::putImpl, this, _1, _2), key, cvalue)));
    }

    for (auto &f : popped) {
      f.get();
    }
  }

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
    entry.fOffsetCompressed = fPos;
    entry.fSizeCompressed = value.size();
    fEntries.push_back(entry);
    fPos += value.size();
  }

  void del(std::string const &key) override {
    //nop because write-only
  }

  void abandon() override {
    fAbandoned = true;
  }

private:
  void unsafeFinalize() {
    using namespace std;
    using namespace leveldb;
    namespace fs = std::filesystem;

    for (auto &f : fFutures) {
      f.get();
    }
    fPool.shutdown();

    if (!fBuffer) {
      return;
    }
    fclose(fBuffer);
    fBuffer = nullptr;

    if (fAbandoned) {
      error_code ec;
      fs::remove(fDir / "tmp.bin", ec);
      return;
    }

    vector<InternalKey> internalKeys;
    for (uint64_t sequence = 0; sequence < fEntries.size(); sequence++) {
      Entry entry = fEntries[sequence];
      InternalKey ik(entry.fKey, sequence, kTypeValue);
      internalKeys.push_back(ik);
    }

    Comparator const *cmp = BytewiseComparator();
    InternalKeyComparator icmp(cmp);
    sort(internalKeys.begin(), internalKeys.end(), [&icmp](InternalKey const &lhs, InternalKey const &rhs) {
      return icmp.Compare(lhs, rhs) < 0;
    });

    FILE *fp = mcfile::File::Open(fDir / "tmp.bin", mcfile::File::Mode::Read);
    if (!fp) {
      return;
    }

    Options o;
    o.compression = kZlibRawCompression;

    Options bo = o;
    bo.comparator = &icmp;

    VersionEdit edit;

    uint64_t fileNumber = 1;
    unique_ptr<WritableFile> file(openTableFile(fileNumber));
    unique_ptr<TableBuilder> builder(new TableBuilder(bo, file.get()));
    vector<uint8_t> buffer;
    optional<InternalKey> smallest = nullopt;
    InternalKey largest;
    for (SequenceNumber sequence = 0; sequence < internalKeys.size(); sequence++) {
      InternalKey ik = internalKeys[sequence];
      if (!smallest) {
        smallest = ik;
      }
      largest = ik;

      if (fEntries.size() <= sequence) {
        continue;
      }
      Entry entry = fEntries[sequence];
      buffer.resize(entry.fSizeCompressed);
#if defined(_WIN32)
      auto ret = _fseeki64(fp, entry.fOffsetCompressed, SEEK_SET);
#else
      auto ret = fseeko(fp, entry.fOffset, SEEK_SET);
#endif
      if (ret != 0) {
        break;
      }
      if (fread(buffer.data(), entry.fSizeCompressed, 1, fp) != 1) {
        break;
      }
      mcfile::Compression::decompress(buffer);
      Slice value((char const *)buffer.data(), buffer.size());

      builder->Add(ik.Encode(), value);

      if (builder->FileSize() > o.max_file_size) {
        builder->Finish();
        file->Close();
        edit.AddFile(1, fileNumber, builder->FileSize(), *smallest, largest);

        smallest = nullopt;

        fileNumber++;
        file.reset(openTableFile(fileNumber));
        builder.reset(new TableBuilder(bo, file.get()));
      }
    }

    fclose(fp);
    fp = nullptr;
    error_code ec;
    fs::remove(fDir / "tmp.bin", ec);

    builder->Finish();
    file->Close();

    uint64_t nextFile;
    if (smallest) {
      edit.AddFile(1, fileNumber, builder->FileSize(), *smallest, largest);
      nextFile = fileNumber + 1;
    } else {
      nextFile = fileNumber;
    }

    builder.reset();
    file.reset();

    Env *env = Env::Default();

    edit.SetLastSequence(fEntries.size());
    edit.SetNextFile(nextFile);
    edit.SetLogNumber(0);
    string manifestRecord;
    edit.EncodeTo(&manifestRecord);

    WritableFile *meta = nullptr;
    Status st = env->NewWritableFile(fDir / "MANIFEST-000001", &meta);
    if (!st.ok()) {
      return;
    }
    leveldb::log::Writer writer(meta);
    st = writer.AddRecord(manifestRecord);
    if (!st.ok()) {
      return;
    }
    meta->Close();
    delete meta;
    meta = nullptr;

    WritableFile *current = nullptr;
    st = env->NewWritableFile(fDir / "CURRENT", &current);
    if (!st.ok()) {
      return;
    }
    st = current->Append("MANIFEST-000001\x0a");
    if (!st.ok()) {
      return;
    }
    current->Close();
    delete current;
    current = nullptr;
  }

  leveldb::WritableFile *openTableFile(uint64_t tableNumber) const {
    using namespace leveldb;
    Env *env = Env::Default();
    WritableFile *file = nullptr;
    Status st = env->NewWritableFile(tableFilePath(tableNumber), &file);
    if (!st.ok()) {
      return nullptr;
    }
    return file;
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
  bool fAbandoned = false;
  ::ThreadPool fPool;
  std::deque<std::future<void>> fFutures;
};

} // namespace je2be::tobe
