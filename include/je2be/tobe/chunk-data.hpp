#pragma once

namespace je2be::tobe {

class ChunkData {
public:
  ChunkData(int32_t chunkX, int32_t chunkZ, mcfile::Dimension dim) : fChunkX(chunkX), fChunkZ(chunkZ), fDimension(dim) {}

  [[nodiscard]] bool put(DbInterface &db) {
    auto status = putChunkSections(db);
    if (status == ChunkStatus::NotEmpty) {
      putVersion(db);
      putData2D(db);
      putBlockEntity(db);
      putEntity(db);
      if (!putChecksums(db)) {
        return false;
      }
      putFinalizedState(db);
      putPendingTicks(db);
    }
    return true;
  }

private:
  void putFinalizedState(DbInterface &db) const {
    auto key = mcfile::be::DbKey::FinalizedState(fChunkX, fChunkZ, fDimension);
    if (fFinalizedState) {
      int32_t v = *fFinalizedState;
      db.put(key, leveldb::Slice((char const *)&v, sizeof(v)));
    } else {
      db.del(key);
    }
  }

  void putEntity(DbInterface &db) const {
    auto key = mcfile::be::DbKey::Entity(fChunkX, fChunkZ, fDimension);
    if (fEntity.empty()) {
      db.del(key);
    } else {
      leveldb::Slice d((char *)fEntity.data(), fEntity.size());
      db.put(key, d);
    }
  }

  [[nodiscard]] bool putChecksums(DbInterface &db) const {
    auto sum = checksums();
    if (!sum) {
      return false;
    }
    auto checksumKey = mcfile::be::DbKey::Checksums(fChunkX, fChunkZ, fDimension);
    db.put(checksumKey, *sum);
    return true;
  }

  void putBlockEntity(DbInterface &db) const {
    auto key = mcfile::be::DbKey::BlockEntity(fChunkX, fChunkZ, fDimension);
    if (fBlockEntity.empty()) {
      db.del(key);
    } else {
      leveldb::Slice blockEntity((char *)fBlockEntity.data(), fBlockEntity.size());
      db.put(key, blockEntity);
    }
  }

  void putData2D(DbInterface &db) const {
    auto data2DKey = mcfile::be::DbKey::Data2D(fChunkX, fChunkZ, fDimension);
    if (fData2D.empty()) {
      db.del(data2DKey);
    } else {
      leveldb::Slice data2D((char *)fData2D.data(), fData2D.size());
      db.put(data2DKey, data2D);
    }
  }

  enum class ChunkStatus {
    Empty,
    NotEmpty,
  };

  ChunkStatus putChunkSections(DbInterface &db) const {
    bool empty = true;
    for (int i = 0; i < 16; i++) {
      auto key = mcfile::be::DbKey::SubChunk(fChunkX, i, fChunkZ, fDimension);
      if (fSubChunks[i].empty()) {
        db.del(key);
      } else {
        leveldb::Slice subchunk((char *)fSubChunks[i].data(), fSubChunks[i].size());
        db.put(key, subchunk);
        empty = false;
      }
    }
    if (empty) {
      if (fFinalizedState && *fFinalizedState == 2) {
        return ChunkStatus::NotEmpty;
      } else {
        return ChunkStatus::Empty;
      }
    } else {
      return ChunkStatus::NotEmpty;
    }
  }

  void putVersion(DbInterface &db) const {
    auto const &versionKey = mcfile::be::DbKey::Version(fChunkX, fChunkZ, fDimension);
    leveldb::Slice version(&kSubChunkVersion, 1);
    db.put(versionKey, version);

    db.del(mcfile::be::DbKey::VersionLegacy(fChunkX, fChunkZ, fDimension));
  }

  void putPendingTicks(DbInterface &db) const {
    auto key = mcfile::be::DbKey::PendingTicks(fChunkX, fChunkZ, fDimension);
    if (fPendingTicks.empty()) {
      db.del(key);
    } else {
      leveldb::Slice data((char *)fPendingTicks.data(), fPendingTicks.size());
      db.put(key, data);
    }
  }

  std::optional<std::string> checksums() const {
    using namespace std;
    using namespace mcfile::stream;
    auto s = make_shared<ByteStream>();
    OutputStreamWriter w(s, {.fLittleEndian = true});
    if (!w.write((uint32_t)0)) {
      return nullopt;
    }

    // SubChunk
    uint32_t count = 0;
    for (int i = 0; i < 16; i++) {
      if (fSubChunks[i].empty()) {
        continue;
      }
      if (!w.write(static_cast<uint8_t>(mcfile::be::DbKey::Tag::SubChunk))) {
        return nullopt;
      }
      if (!w.write((uint8_t)0)) {
        return nullopt;
      }
      if (!w.write((uint8_t)i)) {
        return nullopt;
      }
      uint64_t hash = GetXXHSum(fSubChunks[i]);
      if (!w.write(hash)) {
        return nullopt;
      }
      count++;
    }

    // Data2D
    if (!fData2D.empty()) {
      if (!w.write(static_cast<uint8_t>(mcfile::be::DbKey::Tag::Data2D))) {
        return nullopt;
      }
      if (!w.write((uint8_t)0)) {
        return nullopt;
      }
      if (!w.write((uint8_t)0)) {
        return nullopt;
      }
      uint64_t hash = GetXXHSum(fData2D);
      if (!w.write(hash)) {
        return nullopt;
      }
      count++;
    }

    // BlockEntity
    if (!fBlockEntity.empty()) {
      if (!w.write(static_cast<uint8_t>(mcfile::be::DbKey::Tag::BlockEntity))) {
        return nullopt;
      }
      if (!w.write((uint8_t)0)) {
        return nullopt;
      }
      if (!w.write((uint8_t)0)) {
        return nullopt;
      }
      uint64_t hash = GetXXHSum(fBlockEntity);
      if (!w.write(hash)) {
        return nullopt;
      }
      count++;
    }

    // Entity
    if (!fEntity.empty()) {
      if (!w.write(static_cast<uint8_t>(mcfile::be::DbKey::Tag::Entity))) {
        return nullopt;
      }
      if (!w.write((uint8_t)0)) {
        return nullopt;
      }
      if (!w.write((uint8_t)0)) {
        return nullopt;
      }
      uint64_t hash = GetXXHSum(fEntity);
      if (!w.write(hash)) {
        return nullopt;
      }
      count++;
    }

    // PendingTicks
    if (!fPendingTicks.empty()) {
      if (!w.write(static_cast<uint8_t>(mcfile::be::DbKey::Tag::PendingTicks))) {
        return nullopt;
      }
      if (!w.write((uint8_t)0)) {
        return nullopt;
      }
      if (!w.write((uint8_t)0)) {
        return nullopt;
      }
      uint64_t hash = GetXXHSum(fPendingTicks);
      if (!w.write(hash)) {
        return nullopt;
      }
      count++;
    }

    if (!s->seek(0)) {
      return nullopt;
    }
    if (!w.write(count)) {
      return nullopt;
    }

    std::vector<uint8_t> buffer;
    s->drain(buffer);
    string r((char *)buffer.data(), buffer.size());
    return r;
  }

  static uint64_t GetXXHSum(std::vector<uint8_t> const &v) {
    int64_t d = XXHash::Digest(v.data(), v.size());
    return *(uint64_t *)&d;
  }

public:
  int32_t const fChunkX;
  int32_t const fChunkZ;
  mcfile::Dimension const fDimension;

  std::vector<uint8_t> fData2D;
  std::vector<uint8_t> fSubChunks[16];
  std::vector<uint8_t> fBlockEntity;
  std::vector<uint8_t> fEntity;
  std::optional<int32_t> fFinalizedState;
  std::vector<uint8_t> fPendingTicks;
};

} // namespace je2be::tobe
