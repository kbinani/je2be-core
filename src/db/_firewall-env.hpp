#pragma once

#if __has_include(<leveldb/env.h>)
#include <leveldb/env.h>

namespace je2be {

class FirewallEnv : public leveldb::Env {
public:
  explicit FirewallEnv(std::filesystem::path const &allowedDirectory) : fE(nullptr) {
    namespace fs = std::filesystem;
    fs::path dir = allowedDirectory;
    dir.make_preferred();

    std::error_code ec;
    auto canonical = fs::canonical(dir, ec);
    if (ec) {
      return;
    }
    auto native = canonical.native();
    if (!native.ends_with(fs::path::preferred_separator)) {
      native.push_back(fs::path::preferred_separator);
    }
    fAllowed = native;
    fE = leveldb::Env::Default();
  }

  leveldb::Status NewSequentialFile(std::filesystem::path const &fname, leveldb::SequentialFile **result) override {
    if (!fE) {
      return IOError();
    }
    return fE->NewSequentialFile(fname, result);
  }

  leveldb::Status NewRandomAccessFile(std::filesystem::path const &fname, leveldb::RandomAccessFile **result) override {
    if (!fE) {
      return IOError();
    }
    return fE->NewRandomAccessFile(fname, result);
  }

  leveldb::Status NewWritableFile(std::filesystem::path const &fname, leveldb::WritableFile **result) override {
    if (!fE) {
      return IOError();
    }
    if (isAllowed(fname)) {
      return fE->NewWritableFile(fname, result);
    } else {
      return IOError();
    }
  }

  bool FileExists(std::filesystem::path const &fname) override {
    if (!fE) {
      return false;
    }
    return fE->FileExists(fname);
  }

  leveldb::Status GetChildren(std::filesystem::path const &dir, std::vector<std::filesystem::path> *result) override {
    if (!fE) {
      return IOError();
    }
    return fE->GetChildren(dir, result);
  }

  leveldb::Status RemoveFile(std::filesystem::path const &fname) override {
    if (!fE) {
      return IOError();
    }
    if (isAllowed(fname)) {
      return fE->RemoveFile(fname);
    } else {
      return IOError();
    }
  }

  leveldb::Status CreateDir(std::filesystem::path const &dirname) override {
    if (!fE) {
      return IOError();
    }
    if (isAllowed(dirname)) {
      return fE->CreateDir(dirname);
    } else {
      return IOError();
    }
  }

  leveldb::Status RemoveDir(std::filesystem::path const &dirname) override {
    if (!fE) {
      return IOError();
    }
    if (isAllowed(dirname)) {
      return fE->RemoveDir(dirname);
    } else {
      return IOError();
    }
  }

  leveldb::Status GetFileSize(std::filesystem::path const &fname, uint64_t *file_size) override {
    if (!fE) {
      return IOError();
    }
    return fE->GetFileSize(fname, file_size);
  }

  leveldb::Status RenameFile(std::filesystem::path const &src, std::filesystem::path const &target) override {
    if (!fE) {
      return IOError();
    }
    if (isAllowed(src) && isAllowed(target)) {
      return fE->RenameFile(src, target);
    } else {
      return IOError();
    }
  }

  leveldb::Status LockFile(std::filesystem::path const &fname, leveldb::FileLock **lock) override {
    if (!fE) {
      return IOError();
    }
    if (isAllowed(fname)) {
      return fE->LockFile(fname, lock);
    } else {
      return IOError();
    }
  }

  leveldb::Status UnlockFile(leveldb::FileLock *lock) override {
    if (!fE) {
      return IOError();
    }
    return fE->UnlockFile(lock);
  }

  void Schedule(void (*function)(void *arg), void *arg) override {
    if (!fE) {
      return;
    }
    fE->Schedule(function, arg);
  }

  void StartThread(void (*function)(void *arg), void *arg) override {
    if (!fE) {
      return;
    }
    fE->StartThread(function, arg);
  }

  leveldb::Status GetTestDirectory(std::filesystem::path *path) override {
    return IOError();
  }

  leveldb::Status NewLogger(std::filesystem::path const &fname, leveldb::Logger **result) override {
    if (!fE) {
      return IOError();
    }
    if (isAllowed(fname)) {
      return fE->NewLogger(fname, result);
    } else {
      return IOError();
    }
  }

  uint64_t NowMicros() override {
    if (!fE) {
      return 0;
    }
    return fE->NowMicros();
  }

  void SleepForMicroseconds(int micros) override {
    if (!fE) {
      return;
    }
    fE->SleepForMicroseconds(micros);
  }

  bool Valid() const {
    return (bool)fE;
  }

private:
  static leveldb::Status IOError() {
    return leveldb::Status::IOError({});
  }

  bool isAllowed(std::filesystem::path const &p) {
    namespace fs = std::filesystem;

    fs::path path = p;
    path.make_preferred();

    std::error_code ec;
    auto canonical = fs::weakly_canonical(path, ec);
    if (ec) {
      return false;
    }
    return canonical.native().starts_with(fAllowed);
  }

private:
  leveldb::Env *fE;
  std::filesystem::path::string_type fAllowed;
};

} // namespace je2be

#endif
