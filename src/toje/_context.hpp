#pragma once

#include <je2be/pos2.hpp>
#include <je2be/status.hpp>
#include <je2be/toje/options.hpp>

#include "_dimension-ext.hpp"
#include "_nbt-ext.hpp"
#include "_poi-blocks.hpp"
#include "_volume.hpp"
#include "color/_rgba.hpp"
#include "enums/_game-mode.hpp"
#include "item/_map-color.hpp"
#include "structure/_structure-piece.hpp"
#include "toje/_block-entity-convert-result.hpp"
#include "toje/_constants.hpp"
#include "toje/_map-info.hpp"
#include "toje/_structure-info.hpp"

namespace je2be::toje {

class Context {
  class Impl;

  Context(mcfile::Endian endian,
          std::filesystem::path tempDirectory,
          std::shared_ptr<MapInfo const> const &mapInfo,
          std::shared_ptr<StructureInfo const> const &structureInfo,
          int64_t gameTick,
          GameMode gameMode)
      : fEndian(endian), fTempDirectory(tempDirectory), fGameTick(gameTick), fGameMode(gameMode), fMapInfo(mapInfo), fStructureInfo(structureInfo) {}

public:
  struct ChunksInRegion {
    std::unordered_set<Pos2i, Pos2iHasher> fChunks;
  };

  static std::shared_ptr<Context> Init(leveldb::DB &db,
                                       Options opt,
                                       mcfile::Endian endian,
                                       std::map<mcfile::Dimension, std::vector<std::pair<Pos2i, ChunksInRegion>>> &regions,
                                       int &totalChunks,
                                       int64_t gameTick,
                                       GameMode gameMode,
                                       unsigned int concurrency);

  void markMapUuidAsUsed(int64_t uuid);
  void mergeInto(Context &other) const;
  Status postProcess(std::filesystem::path root, leveldb::DB &db) const;
  std::optional<MapInfo::Map> mapFromUuid(int64_t mapUuid) const;
  void structures(mcfile::Dimension d, Pos2i chunk, std::vector<StructureInfo::Structure> &buffer);
  std::shared_ptr<Context> make() const;
  void setLocalPlayerIds(int64_t entityIdB, Uuid const &entityIdJ);
  std::optional<Uuid> mapLocalPlayerId(int64_t entityIdB) const;
  bool isLocalPlayerId(Uuid const &uuid) const;
  void setRootVehicle(Uuid const &vehicleUid);
  void setRootVehicleEntity(CompoundTagPtr const &vehicleEntity);
  bool isRootVehicle(Uuid const &uuid) const;
  std::optional<std::pair<Uuid, CompoundTagPtr>> drainRootVehicle();
  void setShoulderEntityLeft(int64_t uid);
  void setShoulderEntityRight(int64_t uid);
  bool setShoulderEntityIfItIs(int64_t uid, CompoundTagPtr entityB);
  void drainShoulderEntities(CompoundTagPtr &left, CompoundTagPtr &right);
  void addToPoiIfItIs(mcfile::Dimension dim, Pos3i const &pos, mcfile::je::Block const &block);

private:
  Status exportMaps(std::filesystem::path const &root, leveldb::DB &db) const;
  Status exportPoi(std::filesystem::path const &root) const;

public:
  mcfile::Endian const fEndian;
  std::filesystem::path const fTempDirectory;

  struct VehicleEntity {
    Pos2i fChunk;
    std::map<size_t, Uuid> fPassengers;
  };
  std::unordered_map<Uuid, VehicleEntity, UuidHasher, UuidPred> fVehicleEntities;

  struct LeashedEntity {
    Pos2i fChunk;
    int64_t fLeasherId;
  };
  std::unordered_map<Uuid, LeashedEntity, UuidHasher, UuidPred> fLeashedEntities;

  std::unordered_map<int64_t, Pos3i> fLeashKnots;
  std::unordered_map<Uuid, Pos2i, UuidHasher, UuidPred> fEntities;

  int64_t const fGameTick;
  GameMode const fGameMode;

  bool fDataPackBundle = false;
  bool fDataPack1_20Update = false;

private:
  std::shared_ptr<MapInfo const> fMapInfo;
  std::shared_ptr<StructureInfo const> fStructureInfo;
  std::unordered_set<int64_t> fUsedMapUuids;

  struct LocalPlayer {
    int64_t fBedrockId;
    Uuid fJavaId;
  };
  std::optional<LocalPlayer> fLocalPlayer;

  struct RootVehicle {
    Uuid fUid;
    CompoundTagPtr fVehicle;
  };
  std::optional<RootVehicle> fRootVehicle;

  std::optional<int64_t> fShoulderEntityLeftId;
  std::optional<int64_t> fShoulderEntityRightId;
  CompoundTagPtr fShoulderEntityLeft;
  CompoundTagPtr fShoulderEntityRight;

  std::unordered_map<mcfile::Dimension, PoiBlocks> fPoiBlocks;
};

} // namespace je2be::toje
