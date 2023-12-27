#pragma once

#if __has_include(<leveldb/db.h>)
#include <leveldb/db.h>

#include <je2be/fs.hpp>

#include "_file.hpp"
#include "db/_firewall-env.hpp"
#include "db/_proxy-env.hpp"

#include <minecraft-file.hpp>

namespace je2be {

class ReadonlyDb : public mcfile::be::DbInterface {
public:
  struct Closer {
    friend class ReadonlyDb;

    ~Closer() {
      if (fProxy) {
        fProxy.reset();
      }
      if (fFirewall) {
        fFirewall.reset();
      }
      if (fTempDir) {
        Fs::DeleteAll(*fTempDir);
      }
      if (fLockLock) {
        leveldb::Env::Default()->UnlockFile(fLockLock);
        fLockLock = nullptr;
      }
      if (fManifestLock) {
        leveldb::Env::Default()->UnlockFile(fManifestLock);
        fManifestLock = nullptr;
      }
    }

  protected:
    std::optional<std::filesystem::path> fTempDir;
    leveldb::FileLock *fLockLock = nullptr;
    leveldb::FileLock *fManifestLock = nullptr;
    std::unique_ptr<FirewallEnv> fFirewall;
    std::unique_ptr<ProxyEnv> fProxy;
  };

  static Status Open(std::filesystem::path const &db, leveldb::DB **ptr, std::filesystem::path const &tempRoot, std::unique_ptr<Closer> &outCloser) {
    namespace fs = std::filesystem;

    *ptr = nullptr;

    auto dir = mcfile::File::CreateTempDir(tempRoot);
    if (!dir) {
      return JE2BE_ERROR;
    }
    std::unique_ptr<Closer> closer(new Closer);
    closer->fTempDir = *dir;

    closer->fFirewall.reset(new FirewallEnv(*dir));
    if (!closer->fFirewall->Valid()) {
      return JE2BE_ERROR;
    }
    closer->fProxy.reset(new ProxyEnv(closer->fFirewall.get(), db, *dir));
    if (!closer->fProxy->Valid()) {
      return JE2BE_ERROR;
    }
    leveldb::Options o;
    o.env = closer->fProxy.get();
    leveldb::DB *dbPtr = nullptr;
    if (auto st = leveldb::DB::Open(o, db, &dbPtr); !st.ok()) {
      return JE2BE_ERROR_PUSH(Status::FromLevelDBStatus(st));
    }

    auto lockFile = db / "LOCK";
    if (auto st = leveldb::Env::Default()->LockFile(lockFile, &closer->fLockLock); !st.ok()) {
      return JE2BE_ERROR;
    }

    // Bedrock game client doesn't create or lock the "LOCK" file.
    // The locking process above is for other app reading the db in regular manner.
    // For bedrock game client, additionally lock the manifest file.
    auto currentFile = db / "CURRENT";
    std::vector<u8> content;
    if (!file::GetContents(currentFile, content)) {
      return JE2BE_ERROR;
    }

    std::u8string manifestName;
    manifestName.assign((char8_t const *)content.data(), content.size());
    manifestName = strings::Trim(manifestName);
    auto manifestFile = db / manifestName;
    if (!Fs::Exists(manifestFile)) {
      return JE2BE_ERROR;
    }
    if (auto st = leveldb::Env::Default()->LockFile(manifestFile, &closer->fManifestLock); !st.ok()) {
      return JE2BE_ERROR;
    }

    *ptr = dbPtr;
    outCloser.swap(closer);
    return Status::Ok();
  }

  static Status Open(std::filesystem::path const &db, std::filesystem::path const &tempRoot, std::unique_ptr<ReadonlyDb> &out) {
    Status st;
    std::unique_ptr<ReadonlyDb> ptr(new ReadonlyDb(db, tempRoot, st));
    if (!ptr->fDb || !ptr->fCloser || !st.ok()) {
      return JE2BE_ERROR_PUSH(st);
    }
    out.swap(ptr);
    return Status::Ok();
  }

  ~ReadonlyDb() {
    if (fDb) {
      fDb.reset();
    }
    if (fCloser) {
      fCloser.reset();
    }
  }

  std::optional<std::string> get(std::string const &key) override {
    if (!fDb) {
      return std::nullopt;
    }
    std::string v;
    if (auto st = fDb->Get({}, key, &v); st.ok()) {
      return v;
    } else {
      return std::nullopt;
    }
  }

private:
  ReadonlyDb(std::filesystem::path const &db, std::filesystem::path const &tempRoot, Status &out) {
    leveldb::DB *ptr = nullptr;
    out = Open(db, &ptr, tempRoot, fCloser);
    fDb.reset(ptr);
  }

private:
  std::unique_ptr<leveldb::DB> fDb;
  std::unique_ptr<Closer> fCloser;
};

} // namespace je2be

#endif
