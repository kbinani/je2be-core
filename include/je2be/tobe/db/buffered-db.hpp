#pragma once

namespace je2be::tobe {

class BufferedDb : public DbInterface {
  struct Entry {
    std::string fKey;
    uint64_t fSize;
    uint64_t fOffset;
  };

public:
  explicit BufferedDb(std::filesystem::path const &dir)
      : fDir(dir) {
    fBuffer = mcfile::File::Open(dir / "tmp.bin", mcfile::File::Mode::Write);
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
    std::lock_guard<std::mutex> lk(fMut);

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
    entry.fOffset = fPos;
    entry.fSize = value.size();
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

    vector<string> internalKeys;
    for (uint64_t sequence = 0; sequence < fEntries.size(); sequence++) {
      Entry entry = fEntries[sequence];
      InternalKey ik(entry.fKey, sequence, kTypeValue);
      string encoded = ik.Encode().ToString();
      internalKeys.push_back(encoded);
    }

    Comparator const *cmp = BytewiseComparator();
    InternalKeyComparator icmp(cmp);
    sort(internalKeys.begin(), internalKeys.end(), [&icmp](string const &lhs, string const &rhs) {
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
    vector<char> buffer;
    optional<InternalKey> smallest = nullopt;
    InternalKey largest;
    for (string const &internalKey : internalKeys) {
      ParsedInternalKey pik;
      if (!ParseInternalKey(internalKey, &pik)) {
        continue;
      }
      InternalKey ik(pik.user_key, pik.sequence, pik.type);
      if (!smallest) {
        smallest = ik;
      }
      largest = ik;

      if (fEntries.size() <= pik.sequence) {
        continue;
      }
      Entry entry = fEntries[pik.sequence];
      buffer.resize(entry.fSize);
#if defined(_WIN32)
      auto ret = _fseeki64(fp, entry.fOffset, SEEK_SET);
#else
      auto ret = fseeko(fp, entry.fOffset, SEEK_SET);
#endif
      if (ret != 0) {
        break;
      }
      if (fread(buffer.data(), entry.fSize, 1, fp) != 1) {
        break;
      }
      Slice value(buffer.data(), entry.fSize);
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

    if (smallest) {
      edit.AddFile(1, fileNumber, builder->FileSize(), *smallest, largest);
    }

    builder.reset();
    file.reset();

    Env *env = Env::Default();

    edit.SetLastSequence(fEntries.size());
    edit.SetNextFile(fileNumber + 1);
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
};

} // namespace je2be::tobe
