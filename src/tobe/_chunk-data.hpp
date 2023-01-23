#pragma once

#include "db/_db-interface.hpp"
#include "enums/_chunk-conversion-mode.hpp"
#include "tobe/_versions.hpp"

#include <minecraft-file.hpp>

#include <cstdint>

namespace je2be::tobe {

class ChunkData {
public:
  ChunkData(i32 chunkX, i32 chunkZ, mcfile::Dimension dim, ChunkConversionMode mode) : fChunkX(chunkX), fChunkZ(chunkZ), fDimension(dim), fMode(mode) {}

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
      i32 v = *fFinalizedState;
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
      i8 y = it.first;
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
      vernum = kChunkVersionMaxLegacy;
      break;
    case ChunkConversionMode::CavesAndCliffs2:
    default:
      vernum = kChunkVersion;
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
  i32 const fChunkX;
  i32 const fChunkZ;
  mcfile::Dimension const fDimension;
  ChunkConversionMode const fMode;

  std::vector<u8> fData2D;
  std::map<i8, std::vector<u8>> fSubChunks;
  std::vector<u8> fBlockEntity;
  std::optional<i32> fFinalizedState;
  std::vector<u8> fPendingTicks;
};

} // namespace je2be::tobe
