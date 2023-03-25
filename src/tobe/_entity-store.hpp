#pragma once

#include <je2be/fs.hpp>

#include "_mem.hpp"
#include "_pos2i-set.hpp"

namespace je2be::tobe {

class EntityStore {
public:
  static EntityStore *Open(std::filesystem::path const &dir) {
    using namespace leveldb;
    DB *db = nullptr;
    leveldb::Options o;
    o.compression = kZlibRawCompression;
    o.create_if_missing = true;
    if (!DB::Open(o, dir, &db).ok()) {
      return nullptr;
    }
    return new EntityStore(db, dir);
  }

  ~EntityStore() {
    if (!fDb) {
      return;
    }
    fDb.reset();
    Fs::DeleteAll(fDir);
  }

  Status add(std::vector<i64> const &entityUuids, Pos2i const &chunk, Pos2i const &fromChunk) {
    if (!fDb) {
      return JE2BE_ERROR;
    }
    auto stream = std::make_shared<mcfile::stream::ByteStream>();
    mcfile::stream::OutputStreamWriter writer(stream, mcfile::Endian::Little);
    for (u64 uuid : entityUuids) {
      if (!writer.write(uuid)) {
        return JE2BE_ERROR;
      }
    }
    auto key = Key(chunk, fromChunk);
    std::string value;
    stream->drain(value);
    if (fDb->Put({}, leveldb::Slice(key), leveldb::Slice(value)).ok()) {
      std::lock_guard<std::mutex> lock(fMut);
      fChunks.insert(chunk);
      return Status::Ok();
    } else {
      return JE2BE_ERROR;
    }
  }

  Status entityUuids(Pos2i const &chunk, std::function<Status(i64)> cb) {
    using namespace std;
    if (!fDb) {
      return JE2BE_ERROR;
    }
    leveldb::ReadOptions ro;
    ro.fill_cache = false;
    for (int dx = -1; dx <= 1; dx++) {
      for (int dz = -1; dz <= 1; dz++) {
        Pos2i fromChunk(chunk.fX + dx, chunk.fZ + dz);
        auto key = Key(chunk, fromChunk);
        string value;
        if (!fDb->Get(ro, leveldb::Slice(key), &value).ok()) {
          continue;
        }
        auto s = make_shared<mcfile::stream::ByteInputStream>(value);
        mcfile::stream::InputStreamReader r(s, mcfile::Endian::Little);
        size_t count = value.size() / sizeof(i64);
        for (size_t i = 0; i < count; i++) {
          i64 uuid;
          if (!r.read(&uuid)) {
            return JE2BE_ERROR;
          }
          if (auto st = cb(uuid); !st.ok()) {
            return JE2BE_ERROR_PUSH(st);
          }
        }
      }
    }
    return Status::Ok();
  }

private:
  explicit EntityStore(leveldb::DB *db, std::filesystem::path const &dir) : fDb(db), fDir(dir) {
  }

  std::string Key(Pos2i const &chunk, Pos2i const &fromChunk) {
    std::string k;
    k.resize(16);
    Mem::Write<i32>(k, 0, chunk.fX);
    Mem::Write<i32>(k, 4, chunk.fZ);
    Mem::Write<i32>(k, 8, fromChunk.fX);
    Mem::Write<i32>(k, 12, fromChunk.fZ);
    return k;
  }

public:
  Pos2iSet fChunks;

private:
  std::unique_ptr<leveldb::DB> fDb;
  std::filesystem::path fDir;
  std::mutex fMut;
};

} // namespace je2be::tobe
