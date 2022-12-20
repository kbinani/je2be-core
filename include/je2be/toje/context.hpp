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
          GameMode gameMode,
          std::function<std::optional<BlockEntityConvertResult>(Pos3i const &pos, mcfile::be::Block const &block, CompoundTag const &tag, mcfile::je::Block const &blockJ, Context &ctx)> fromBlockAndBlockEntity)
      : fEndian(endian), fTempDirectory(tempDirectory), fMapInfo(mapInfo), fStructureInfo(structureInfo), fGameTick(gameTick), fGameMode(gameMode), fFromBlockAndBlockEntity(fromBlockAndBlockEntity) {}

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
                                       unsigned int concurrency,
                                       std::function<std::optional<BlockEntityConvertResult>(Pos3i const &pos, mcfile::be::Block const &block, CompoundTag const &tag, mcfile::je::Block const &blockJ, Context &ctx)> fromBlockAndBlockEntity);

  void markMapUuidAsUsed(int64_t uuid) {
    fUsedMapUuids.insert(uuid);
  }

  void mergeInto(Context &other) const {
    for (int64_t uuid : fUsedMapUuids) {
      other.fUsedMapUuids.insert(uuid);
    }
    for (auto const &it : fLeashedEntities) {
      other.fLeashedEntities[it.first] = it.second;
    }
    for (auto const &it : fLeashKnots) {
      other.fLeashKnots[it.first] = it.second;
    }
    if (!other.fRootVehicle && fRootVehicle) {
      other.fRootVehicle = fRootVehicle;
    }
    if (!other.fShoulderEntityLeft && fShoulderEntityLeft) {
      other.fShoulderEntityLeft = fShoulderEntityLeft;
    }
    if (!other.fShoulderEntityRight && fShoulderEntityRight) {
      other.fShoulderEntityRight = fShoulderEntityRight;
    }
    for (auto const &it : fPoiBlocks) {
      mcfile::Dimension dim = it.first;
      auto &dest = other.fPoiBlocks[dim];
      it.second.mergeInto(dest);
    }
    other.fDataPackBundle = other.fDataPackBundle || fDataPackBundle;
    other.fDataPack1_20Update = other.fDataPack1_20Update || fDataPack1_20Update;
  }

  Status postProcess(std::filesystem::path root, leveldb::DB &db) const {
    if (auto st = exportMaps(root, db); !st.ok()) {
      return st;
    }
    return exportPoi(root);
  }

  std::optional<MapInfo::Map> mapFromUuid(int64_t mapUuid) const {
    return fMapInfo->mapFromUuid(mapUuid);
  }

  void structures(mcfile::Dimension d, Pos2i chunk, std::vector<StructureInfo::Structure> &buffer) {
    fStructureInfo->structures(d, chunk, buffer);
  }

  std::shared_ptr<Context> make() const {
    auto ret = std::shared_ptr<Context>(new Context(fEndian, fTempDirectory, fMapInfo, fStructureInfo, fGameTick, fGameMode, fFromBlockAndBlockEntity));
    ret->fLocalPlayer = fLocalPlayer;
    if (fRootVehicle) {
      ret->fRootVehicle = *fRootVehicle;
    }
    ret->fShoulderEntityLeftId = fShoulderEntityLeftId;
    ret->fShoulderEntityRightId = fShoulderEntityRightId;
    ret->fDataPackBundle = fDataPackBundle;
    ret->fDataPack1_20Update = fDataPack1_20Update;
    return ret;
  }

  void setLocalPlayerIds(int64_t entityIdB, Uuid entityIdJ) {
    LocalPlayer lp;
    lp.fBedrockId = entityIdB;
    lp.fJavaId = entityIdJ;
    fLocalPlayer = lp;
  }

  std::optional<Uuid> mapLocalPlayerId(int64_t entityIdB) const {
    if (fLocalPlayer && fLocalPlayer->fBedrockId == entityIdB) {
      return fLocalPlayer->fJavaId;
    } else {
      return std::nullopt;
    }
  }

  bool isLocalPlayerId(Uuid uuid) const {
    if (fLocalPlayer) {
      return UuidPred{}(uuid, fLocalPlayer->fJavaId);
    } else {
      return false;
    }
  }

  void setRootVehicle(Uuid vehicleUid) {
    RootVehicle rv;
    rv.fUid = vehicleUid;
    fRootVehicle = rv;
  }

  void setRootVehicleEntity(CompoundTagPtr const &vehicleEntity) {
    assert(fRootVehicle);
    if (!fRootVehicle) {
      return;
    }
    fRootVehicle->fVehicle = vehicleEntity;
  }

  bool isRootVehicle(Uuid uuid) const {
    if (fRootVehicle) {
      return UuidPred{}(uuid, fRootVehicle->fUid);
    } else {
      return false;
    }
  }

  std::optional<std::pair<Uuid, CompoundTagPtr>> drainRootVehicle() {
    if (!fRootVehicle) {
      return std::nullopt;
    }
    auto uid = fRootVehicle->fUid;
    auto vehicle = fRootVehicle->fVehicle;
    fRootVehicle = std::nullopt;
    if (vehicle) {
      return std::make_pair(uid, vehicle);
    }
    return std::nullopt;
  }

  void setShoulderEntityLeft(int64_t uid) {
    fShoulderEntityLeftId = uid;
  }

  void setShoulderEntityRight(int64_t uid) {
    fShoulderEntityRightId = uid;
  }

  bool setShoulderEntityIfItIs(int64_t uid, CompoundTagPtr entityB) {
    if (fShoulderEntityLeftId == uid) {
      fShoulderEntityLeft = entityB;
      return true;
    } else if (fShoulderEntityRightId == uid) {
      fShoulderEntityRight = entityB;
      return true;
    } else {
      return false;
    }
  }

  void drainShoulderEntities(CompoundTagPtr &left, CompoundTagPtr &right) {
    fShoulderEntityLeft.swap(left);
    fShoulderEntityRight.swap(right);
  }

  void addToPoiIfItIs(mcfile::Dimension dim, Pos3i const &pos, mcfile::je::Block const &block) {
    if (PoiBlocks::Interest(block)) {
      fPoiBlocks[dim].add(pos, block.fId);
    }
  }

private:
  Status exportMaps(std::filesystem::path const &root, leveldb::DB &db) const {
    using namespace mcfile;

    if (!Fs::CreateDirectories(root / "data")) {
      return JE2BE_ERROR;
    }

    std::optional<int> maxMapNumber;

    for (int64_t uuid : fUsedMapUuids) {
      auto map = fMapInfo->mapFromUuid(uuid);
      if (!map) {
        continue;
      }
      int number = map->fNumber;
      auto key = mcfile::be::DbKey::Map(uuid);
      std::string str;
      if (auto st = db.Get(leveldb::ReadOptions{}, key, &str); !st.ok()) {
        continue;
      }
      auto b = CompoundTag::Read(str, fEndian);
      if (!b) {
        continue;
      }
      auto j = Compound();
      auto dimensionB = b->byte("dimension", 0);
      Dimension dim = Dimension::Overworld;
      if (auto dimension = DimensionFromBedrockDimension(dimensionB); dimension) {
        dim = *dimension;
      }
      j->set("dimension", String(JavaStringFromDimension(dim)));
      CopyBoolValues(*b, *j, {{"mapLocked", "locked"}});
      CopyByteValues(*b, *j, {{"scale"}, {"unlimitedTracking"}});
      CopyIntValues(*b, *j, {{"xCenter"}, {"zCenter"}});
      auto colorsTagB = b->byteArrayTag("colors");
      if (!colorsTagB) {
        continue;
      }
      auto const &colorsB = colorsTagB->value();
      if (colorsB.size() != 65536) {
        continue;
      }
      std::vector<uint8_t> colorsJ(16384);
      for (int i = 0; i < colorsJ.size(); i++) {
        uint8_t r = colorsB[i * 4];
        uint8_t g = colorsB[i * 4 + 1];
        uint8_t b = colorsB[i * 4 + 2];
        uint8_t a = colorsB[i * 4 + 3];
        Rgba rgb(r, g, b, a);
        uint8_t id = MapColor::MostSimilarColorId(rgb);
        colorsJ[i] = id;
      }
      j->set("colors", std::make_shared<ByteArrayTag>(colorsJ));
      auto tagJ = Compound();
      tagJ->set("data", j);
      tagJ->set("DataVersion", Int(toje::kDataVersion));

      auto path = root / "data" / ("map_" + std::to_string(number) + ".dat");
      auto s = std::make_shared<mcfile::stream::GzFileOutputStream>(path);
      if (!CompoundTag::Write(*tagJ, s, mcfile::Endian::Big)) {
        return JE2BE_ERROR;
      }
      if (maxMapNumber) {
        maxMapNumber = (std::max)(*maxMapNumber, number);
      } else {
        maxMapNumber = number;
      }
    }
    if (maxMapNumber) {
      auto idcounts = Compound();
      auto d = Compound();
      d->set("map", Int(*maxMapNumber));
      idcounts->set("data", d);
      idcounts->set("DataVersion", Int(toje::kDataVersion));
      auto path = root / "data" / "idcounts.dat";
      auto s = std::make_shared<mcfile::stream::GzFileOutputStream>(path);
      if (!CompoundTag::Write(*idcounts, s, mcfile::Endian::Big)) {
        return JE2BE_ERROR;
      }
    }

    return Status::Ok();
  }

  Status exportPoi(std::filesystem::path const &root) const {
    namespace fs = std::filesystem;

    for (auto const &it : fPoiBlocks) {
      mcfile::Dimension d = it.first;
      PoiBlocks const &poi = it.second;
      fs::path dir;
      if (d == mcfile::Dimension::Overworld) {
        dir = root / "poi";
      } else if (d == mcfile::Dimension::Nether) {
        dir = root / "DIM-1" / "poi";
      } else {
        continue;
      }
      if (!poi.write(dir, kDataVersion)) {
        return JE2BE_ERROR;
      }
    }
    return Status::Ok();
  }

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

  // NOTE: This std::function must be BlockEntity::FromBlockAndBlockEntity. By doing this, the "Item" class can use the function (the "Item" class is #includ'ed before "BlockEntity")
  std::function<std::optional<BlockEntityConvertResult>(Pos3i const &pos, mcfile::be::Block const &block, CompoundTag const &tag, mcfile::je::Block const &blockJ, Context &ctx)> const fFromBlockAndBlockEntity;

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
