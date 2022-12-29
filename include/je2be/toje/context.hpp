#pragma once

#include <je2be/color/rgba.hpp>
#include <je2be/dimension-ext.hpp>
#include <je2be/enums/game-mode.hpp>
#include <je2be/item/map-color.hpp>
#include <je2be/nbt-ext.hpp>
#include <je2be/poi-blocks.hpp>
#include <je2be/pos2.hpp>
#include <je2be/status.hpp>
#include <je2be/structure/structure-piece.hpp>
#include <je2be/toje/block-entity-convert-result.hpp>
#include <je2be/toje/constants.hpp>
#include <je2be/toje/map-info.hpp>
#include <je2be/toje/options.hpp>
#include <je2be/toje/structure-info.hpp>
#include <je2be/volume.hpp>

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
                                       std::map<mcfile::Dimension, std::unordered_map<Pos2i, ChunksInRegion, Pos2iHasher>> &regions,
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
