#pragma once

namespace je2be::toje {

class Context {
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
                                       std::function<std::optional<BlockEntityConvertResult>(Pos3i const &pos, mcfile::be::Block const &block, CompoundTag const &tag, mcfile::je::Block const &blockJ, Context &ctx)> fromBlockAndBlockEntity) {
    using namespace std;
    using namespace leveldb;
    using namespace mcfile;
    namespace fs = std::filesystem;

    totalChunks = 0;

    auto mapInfo = make_shared<MapInfo>();
    auto structureInfo = make_shared<StructureInfo>();
    unordered_map<Dimension, vector<StructurePiece>> pieces;

    unique_ptr<Iterator> itr(db.NewIterator({}));
    for (itr->SeekToFirst(); itr->Valid(); itr->Next()) {
      auto key = itr->key();
      auto parsed = mcfile::be::DbKey::Parse(key.ToString());
      if (!parsed) {
        continue;
      }
      if (parsed->fIsTagged) {
        uint8_t tag = parsed->fTagged.fTag;
        Dimension d = parsed->fTagged.fDimension;
        if (!opt.fDimensionFilter.empty()) {
          if (opt.fDimensionFilter.find(d) == opt.fDimensionFilter.end()) {
            continue;
          }
        }
        switch (tag) {
        case static_cast<uint8_t>(mcfile::be::DbKey::Tag::Data3D):
        case static_cast<uint8_t>(mcfile::be::DbKey::Tag::Data2D): {
          int cx = parsed->fTagged.fChunk.fX;
          int cz = parsed->fTagged.fChunk.fZ;
          Pos2i c(cx, cz);
          if (!opt.fChunkFilter.empty()) {
            if (opt.fChunkFilter.find(c) == opt.fChunkFilter.end()) {
              continue;
            }
          }
          int rx = Coordinate::RegionFromChunk(cx);
          int rz = Coordinate::RegionFromChunk(cz);
          Pos2i r(rx, rz);
          regions[d][r].fChunks.insert(c);
          totalChunks++;
          break;
        }
        case static_cast<uint8_t>(mcfile::be::DbKey::Tag::StructureBounds): {
          vector<StructurePiece> buffer;
          StructurePiece::Parse(itr->value().ToString(), buffer);
          copy(buffer.begin(), buffer.end(), back_inserter(pieces[d]));
          break;
        }
        }
      } else if (parsed->fUnTagged.starts_with("map_")) {
        int64_t mapId;
        auto parsed = MapInfo::Parse(itr->value().ToString(), mapId, endian);
        if (!parsed) {
          continue;
        }
        mapInfo->add(*parsed, mapId);
      }
    }

    for (auto const &i : pieces) {
      Dimension d = i.first;
      unordered_map<StructureType, vector<StructurePiece>> categorized;
      for (StructurePiece const &piece : i.second) {
        StructureType type = piece.fType;
        switch (type) {
        case StructureType::Monument:
        case StructureType::Fortress:
        case StructureType::Outpost:
          categorized[type].push_back(piece);
          break;
        }
      }
      for (auto const &i : categorized) {
        StructureType type = i.first;
        vector<Volume> volumes;
        for (StructurePiece const &piece : i.second) {
          volumes.push_back(piece.fVolume);
        }
        Volume::Connect(volumes);
        for (Volume const &v : volumes) {
          int cx;
          int cz;
          if (type == StructureType::Monument) {
            if (v.size<0>() == 58 && v.size<1>() == 23 && v.size<2>() == 58) {
              int x = v.fStart.fX;
              int z = v.fStart.fZ;
              cx = Coordinate::ChunkFromBlock(x) + 2;
              cz = Coordinate::ChunkFromBlock(z) + 2;
              // NW corner offset from chunk:
              // BE: (11, 11)
              // JE: (3, 3)
            } else {
              // TODO: bounds is incomplete for monument
              continue;
            }
          } else {
            int x = v.fStart.fX + v.size<0>() / 2;
            int z = v.fStart.fZ + v.size<2>() / 2;
            cx = Coordinate::ChunkFromBlock(x);
            cz = Coordinate::ChunkFromBlock(z);
          }
          Pos2i chunk(cx, cz);
          StructureInfo::Structure s(type, v, chunk);
          structureInfo->add(d, s);
        }
      }
    }

    fs::path temp = opt.fTempDirectory ? *opt.fTempDirectory : fs::temp_directory_path();
    return std::shared_ptr<Context>(new Context(endian, temp, mapInfo, structureInfo, gameTick, gameMode, fromBlockAndBlockEntity));
  }

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
    for (auto const &it : fPortalBlocks) {
      mcfile::Dimension dim = it.first;
      auto &dest = other.fPortalBlocks[dim];
      it.second.mergeInto(dest);
    }
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

  void addPortalBlock(mcfile::Dimension dim, Pos3i const &pos) {
    fPortalBlocks[dim].add(pos);
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

    for (auto const &it : fPortalBlocks) {
      mcfile::Dimension d = it.first;
      PoiPortals const &portals = it.second;
      fs::path dir;
      if (d == mcfile::Dimension::Overworld) {
        dir = root / "poi";
      } else if (d == mcfile::Dimension::Nether) {
        dir = root / "DIM-1" / "poi";
      } else {
        continue;
      }
      if (!portals.write(dir, kDataVersion)) {
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

  std::unordered_map<mcfile::Dimension, PoiPortals> fPortalBlocks;
};

} // namespace je2be::toje
