#pragma once

namespace je2be {

class Db : public DbInterface {
public:
  Db(std::string const &) = delete;
  Db(std::wstring const &) = delete;
  explicit Db(std::filesystem::path const &dir) : fDb(nullptr) {
    using namespace leveldb;

    DB *db;
    leveldb::Options options;
    options.compression = kZlibRawCompression;
    options.create_if_missing = true;
    leveldb::Status status = DB::Open(options, dir, &db);
    if (!status.ok()) {
      return;
    }
    fDb.reset(db);
  }

  bool valid() const override { return fDb != nullptr; }

  void put(std::string const &key, leveldb::Slice const &value) override {
    assert(fDb);
    if (fDb) {
      fDb->Put(fWriteOptions, key, value);
    }
  }

  void write(leveldb::WriteBatch &batch) {
    assert(fDb);
    if (fDb) {
      fDb->Write(leveldb::WriteOptions{}, &batch);
    }
  }

  std::optional<std::string> get(std::string const &key) {
    if (!fDb) {
      return std::nullopt;
    }
    leveldb::ReadOptions o;
    std::string v;
    leveldb::Status st = fDb->Get(o, key, &v);
    if (st.ok()) {
      return v;
    } else {
      return std::nullopt;
    }
  }

  void del(std::string const &key) override {
    if (fDb) {
      fDb->Delete(leveldb::WriteOptions{}, key);
    }
  }

  bool close(std::function<void(Rational<u64> const &progress)> progress = nullptr) override {
    if (!fDb) {
      return false;
    }
    if (progress) {
      progress({0, 1});
    }
    fDb.reset();
    if (progress) {
      progress({1, 1});
    }
    return true;
  }

  void abandon() override {
    if (fDb) {
      fDb.reset();
    }
  }

private:
  std::unique_ptr<leveldb::DB> fDb;
  leveldb::WriteOptions fWriteOptions;
};

} // namespace je2be
