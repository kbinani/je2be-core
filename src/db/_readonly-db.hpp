#pragma once

#if __has_include(<leveldb/db.h>)
#include <leveldb/db.h>

#include <je2be/fs.hpp>

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
    }

  protected:
    std::optional<std::filesystem::path> fTempDir;
    std::unique_ptr<FirewallEnv> fFirewall;
    std::unique_ptr<ProxyEnv> fProxy;
  };

  static std::unique_ptr<Closer> Open(std::filesystem::path const &db, leveldb::DB **ptr, std::filesystem::path const &tempRoot) {
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
    if (auto st = leveldb::DB::Open(o, db, ptr); st.ok()) {
      return std::move(closer);
    } else {
      return nullptr;
    }
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
