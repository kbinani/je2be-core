#include <doctest/doctest.h>

#include <je2be.hpp>

#include <iostream>

#if defined(_WIN32)
#include <fcntl.h>
#include <io.h>
#include <shlobj_core.h>
#endif

using namespace std;
using namespace mcfile;
namespace fs = std::filesystem;

namespace {
optional<wstring> GetLocalApplicationDirectory() {
#if __has_include(<shlobj_core.h>)
  int csidType = CSIDL_LOCAL_APPDATA;
  wchar_t path[MAX_PATH + 256];

  if (SHGetSpecialFolderPathW(nullptr, path, csidType, FALSE)) {
    return wstring(path);
  }
#endif
  return nullopt;
}

optional<fs::path> GetWorldDirectory(string const &name) {
  auto appDir = GetLocalApplicationDirectory(); // X:/Users/whoami/AppData/Local
  if (!appDir) {
    return nullopt;
  }
  return fs::path(*appDir) / L"Packages" / L"Microsoft.MinecraftUWP_8wekyb3d8bbwe" / L"LocalState" / L"games" / L"com.mojang" / L"minecraftWorlds" / name / L"db";
}

leveldb::DB *Open(string const &name) {
  using namespace leveldb;
  auto dir = GetWorldDirectory(name);
  if (!dir) {
    return nullptr;
  }
  Options o;
  o.compression = kZlibRawCompression;
  DB *db;
  Status st = DB::Open(o, *dir, &db);
  if (!st.ok()) {
    return nullptr;
  }
  return db;
}

void VisitDbUntil(string const &name, function<bool(string const &key, string const &value, leveldb::DB *db)> callback) {
  using namespace leveldb;
  unique_ptr<DB> db(Open(name));
  if (!db) {
    return;
  }
  ReadOptions ro;
  unique_ptr<Iterator> itr(db->NewIterator(ro));
  for (itr->SeekToFirst(); itr->Valid(); itr->Next()) {
    string k = itr->key().ToString();
    string v = itr->value().ToString();
    if (!callback(k, v, db.get())) {
      break;
    }
  }
}

void VisitDb(string const &name, function<void(string const &key, string const &value, leveldb::DB *db)> callback) {
  VisitDbUntil(name, [callback](string const &k, string const &v, leveldb::DB *db) {
    callback(k, v, db);
    return true;
  });
}

void Data2D(string const &k, string const &value, leveldb::DB *db) {
  using namespace mcfile::be;
  auto p = DbKey::Parse(k);
  if (!p) {
    return;
  }
  if (!p->fIsTagged) {
    return;
  }
  if (p->fTagged.fTag != static_cast<uint8_t>(DbKey::Tag::Version)) {
    return;
  }
  auto dim = p->fTagged.dimension();
  if (!dim) {
    return;
  }
  int const cx = p->fTagged.fChunk.fX;
  int const cz = p->fTagged.fChunk.fZ;
  int maxChunkY = -99;
  leveldb::ReadOptions ro;
  for (int cy = -4; cy <= 20; cy++) {
    auto key = DbKey::SubChunk(cx, cy, cz, *dim);
    string v;
    auto st = db->Get(ro, key, &v);
    if (!st.ok()) {
      continue;
    }
    maxChunkY = (std::max)(maxChunkY, cy);
  }
  int minChunkY = 0;
  if (*dim == Dimension::Overworld) {
    minChunkY = -4;
  }
  int numSubChunks = maxChunkY - minChunkY + 1;
  if (numSubChunks < 1) {
    return;
  }
  string data2d;
  {
    auto key = DbKey::Data2D(cx, cz, *dim);
    if (!db->Get(ro, key, &data2d).ok()) {
      cerr << "can't get Data2D" << endl;
      return;
    }
  }
  auto map = BiomeMap::Decode(minChunkY, data2d, 512);
  if (!map) {
    cerr << "failed decoding BiomeMap" << endl;
    return;
  }
  if (map->numSections() != numSubChunks) {
    cerr << "extra data contained in Data2D" << endl;
  }
}
} // namespace

TEST_CASE("research") {
  string name = "DBWoYX5RAAA=";
  VisitDb(name, Data2D);
}
