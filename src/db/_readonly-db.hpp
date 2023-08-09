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

  static std::unique_ptr<Closer> Open(std::filesystem::path const &db, leveldb::DB **ptr, std::filesystem::path const &tempRoot) {
    namespace fs = std::filesystem;

    auto dir = mcfile::File::CreateTempDir(tempRoot);
    if (!dir) {
      return nullptr;
    }
    std::unique_ptr<Closer> closer(new Closer);
    closer->fTempDir = *dir;

    closer->fFirewall.reset(new FirewallEnv(*dir));
    if (!closer->fFirewall->Valid()) {
      return nullptr;
    }
    closer->fProxy.reset(new ProxyEnv(closer->fFirewall.get(), db, *dir));
    if (!closer->fProxy->Valid()) {
      return nullptr;
    }
    leveldb::Options o;
    o.env = closer->fProxy.get();
    *ptr = nullptr;
    if (auto st = leveldb::DB::Open(o, db, ptr); !st.ok()) {
      return nullptr;
    }

    auto lockFile = db / "LOCK";
    if (auto st = leveldb::Env::Default()->LockFile(lockFile, &closer->fLockLock); !st.ok()) {
      return nullptr;
    }

    // Bedrock game client doesn't create or lock the "LOCK" file.
    // The locking process above is for other app reading the db in regular manner.
    // For bedrock game client, additionally lock the manifest file.
    auto currentFile = db / "CURRENT";
    std::vector<u8> content;
    if (!file::GetContents(currentFile, content)) {
      return nullptr;
    }

    std::u8string manifestName;
    manifestName.assign((char8_t const *)content.data(), content.size());
    manifestName = strings::Trim(manifestName);
    auto manifestFile = db / manifestName;
    if (!Fs::Exists(manifestFile)) {
      return nullptr;
    }
    if (auto st = leveldb::Env::Default()->LockFile(manifestFile, &closer->fManifestLock); !st.ok()) {
      return nullptr;
    }

    return std::move(closer);
  }

  ReadonlyDb(std::filesystem::path const &db, std::filesystem::path const &tempRoot) {
    leveldb::DB *ptr = nullptr;
    fCloser = std::move(Open(db, &ptr, tempRoot));
    fDb.reset(ptr);
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
  std::unique_ptr<leveldb::DB> fDb;
  std::unique_ptr<Closer> fCloser;
};

} // namespace je2be

#endif
