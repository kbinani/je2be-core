#pragma once

#include "db/_db-interface.hpp"
#include "enums/_chunk-conversion-mode.hpp"
#include "java/_versions.hpp"

#include <minecraft-file.hpp>

#include <cstdint>

namespace je2be {
struct DataVersion;
namespace java {
class ChunkData {
public:
  ChunkData(i32 chunkX, i32 chunkZ, mcfile::Dimension dim, ChunkConversionMode mode, DataVersion const &dataVersion) : fChunkX(chunkX), fChunkZ(chunkZ), fDimension(dim), fMode(mode), fDataVersion(dataVersion) {}

  [[nodiscard]] Status put(DbInterface &db) {
    auto [chunkStatus, status] = putChunkSections(db);
    if (!status.ok()) {
      return JE2BE_ERROR_PUSH(status);
    }
    if (chunkStatus == ChunkStatus::NotEmpty) {
      if (auto st = putVersion(db); !st.ok()) {
        return JE2BE_ERROR_PUSH(st);
      }
      if (auto st = putData2D(db); !st.ok()) {
        return JE2BE_ERROR_PUSH(st);
      }
      if (auto st = putBlockEntity(db); !st.ok()) {
        return JE2BE_ERROR_PUSH(st);
      }
      if (auto st = putFinalizedState(db); !st.ok()) {
        return JE2BE_ERROR_PUSH(st);
      }
      return putPendingTicks(db);
    } else {
      return Status::Ok();
    }
  }

private:
  Status putFinalizedState(DbInterface &db) const {
    auto key = mcfile::be::DbKey::FinalizedState(fChunkX, fChunkZ, fDimension);
    if (fFinalizedState) {
      i32 v = *fFinalizedState;
      return db.put(key, leveldb::Slice((char const *)&v, sizeof(v)));
    } else {
      return db.del(key);
    }
  }

  Status putBlockEntity(DbInterface &db) const {
    auto key = mcfile::be::DbKey::BlockEntity(fChunkX, fChunkZ, fDimension);
    if (fBlockEntity.empty()) {
      return db.del(key);
    } else {
      leveldb::Slice blockEntity((char *)fBlockEntity.data(), fBlockEntity.size());
      return db.put(key, blockEntity);
    }
  }

  Status putData2D(DbInterface &db) const {
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
      return db.del(key);
    } else {
      leveldb::Slice data2D((char *)fData2D.data(), fData2D.size());
      return db.put(key, data2D);
    }
  }

  enum class ChunkStatus {
    Empty,
    NotEmpty,
  };

  std::pair<std::optional<ChunkStatus>, Status> putChunkSections(DbInterface &db) const {
    using namespace std;
    bool empty = true;
    for (auto const &it : fSubChunks) {
      i8 y = it.first;
      auto const &section = it.second;
      auto key = mcfile::be::DbKey::SubChunk(fChunkX, y, fChunkZ, fDimension);
      if (section.empty()) {
        if (auto st = db.del(key); !st.ok()) {
          return make_pair(nullopt, JE2BE_ERROR_PUSH(st));
        }
      } else {
        leveldb::Slice subchunk((char *)section.data(), section.size());
        if (auto st = db.put(key, subchunk); !st.ok()) {
          return make_pair(nullopt, JE2BE_ERROR_PUSH(st));
        }
        empty = false;
      }
    }
    if (empty) {
      if (fFinalizedState && *fFinalizedState == 2) {
        return make_pair(ChunkStatus::NotEmpty, Status::Ok());
      } else {
        return make_pair(ChunkStatus::Empty, Status::Ok());
      }
    } else {
      return make_pair(ChunkStatus::NotEmpty, Status::Ok());
    }
  }

  Status putVersion(DbInterface &db) const {
    auto const &versionKey = mcfile::be::DbKey::Version(fChunkX, fChunkZ, fDimension);
    uint8_t vernum;
    switch (fMode) {
    case ChunkConversionMode::Legacy:
      vernum = kChunkVersionMaxLegacy;
      break;
    case ChunkConversionMode::CavesAndCliffs2:
    default:
      vernum = kChunkVersion;
      break;
    }

    leveldb::Slice version((char const *)&vernum, sizeof(vernum));
    if (auto st = db.put(versionKey, version); !st.ok()) {
      return JE2BE_ERROR_PUSH(st);
    }

    return db.del(mcfile::be::DbKey::VersionLegacy(fChunkX, fChunkZ, fDimension));
  }

  Status putPendingTicks(DbInterface &db) const {
    auto key = mcfile::be::DbKey::PendingTicks(fChunkX, fChunkZ, fDimension);
    if (fPendingTicks.empty()) {
      return db.del(key);
    } else {
      leveldb::Slice data((char *)fPendingTicks.data(), fPendingTicks.size());
      return db.put(key, data);
    }
  }

public:
  i32 const fChunkX;
  i32 const fChunkZ;
  mcfile::Dimension const fDimension;
  ChunkConversionMode const fMode;
  DataVersion const &fDataVersion;

  std::vector<u8> fData2D;
  std::map<i8, std::vector<u8>> fSubChunks;
  std::vector<u8> fBlockEntity;
  std::optional<i32> fFinalizedState;
  std::vector<u8> fPendingTicks;
};

} // namespace java
} // namespace je2be
