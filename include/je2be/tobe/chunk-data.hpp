#pragma once

namespace je2be::tobe {

class ChunkData {
public:
  ChunkData(int32_t chunkX, int32_t chunkZ, mcfile::Dimension dim, ChunkConversionMode mode) : fChunkX(chunkX), fChunkZ(chunkZ), fDimension(dim), fMode(mode) {}

  [[nodiscard]] bool put(DbInterface &db) {
    auto status = putChunkSections(db);
    if (status == ChunkStatus::NotEmpty) {
      putVersion(db);
      putData2D(db);
      putBlockEntity(db);
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
    std::string key;
    switch (fMode) {
    case ChunkConversionMode::Legacy:
      key = mcfile::be::DbKey::Data2D(fChunkX, fChunkZ, fDimension);
      break;
    case ChunkConversionMode::CavesAndCliffs2:
    default:
      key = mcfile::be::DbKey::Data3D(fChunkX, fChunkZ, fDimension);
      break;
    }
    if (fData2D.empty()) {
      db.del(key);
    } else {
      leveldb::Slice data2D((char *)fData2D.data(), fData2D.size());
      db.put(key, data2D);
    }
  }

  enum class ChunkStatus {
    Empty,
    NotEmpty,
  };

  ChunkStatus putChunkSections(DbInterface &db) const {
    bool empty = true;
    for (auto const &it : fSubChunks) {
      int8_t y = it.first;
      auto const &section = it.second;
      auto key = mcfile::be::DbKey::SubChunk(fChunkX, y, fChunkZ, fDimension);
      if (section.empty()) {
        db.del(key);
      } else {
        leveldb::Slice subchunk((char *)section.data(), section.size());
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
    char vernum;
    switch (fMode) {
    case ChunkConversionMode::Legacy:
      vernum = 16;
      break;
    case ChunkConversionMode::CavesAndCliffs2:
    default:
      vernum = kSubChunkVersion;
      break;
    }

    leveldb::Slice version(&vernum, sizeof(vernum));
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

public:
  int32_t const fChunkX;
  int32_t const fChunkZ;
  mcfile::Dimension const fDimension;
  ChunkConversionMode const fMode;

  std::vector<uint8_t> fData2D;
  std::map<int8_t, std::vector<uint8_t>> fSubChunks;
  std::vector<uint8_t> fBlockEntity;
  std::optional<int32_t> fFinalizedState;
  std::vector<uint8_t> fPendingTicks;
};

} // namespace je2be::tobe
