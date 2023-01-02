#include <je2be/toje/context.hpp>

#if !defined(EMSCRIPTEN)
#include <je2be/db/async-iterator.hpp>
#endif
#include <je2be/structure/structure-piece.hpp>

namespace je2be::toje {

class Context::Impl {
  struct Accum {
    std::map<int64_t, MapInfo::Map> fMaps;
    std::map<mcfile::Dimension, std::unordered_map<Pos2i, ChunksInRegion, Pos2iHasher>> fRegions;
    std::map<mcfile::Dimension, std::vector<StructurePiece>> fStructurePieces;
    int fNumChunks = 0;

    Options fOpt;
    mcfile::Endian fEndian;

    Accum(Options opt, mcfile::Endian endian) : fOpt(opt), fEndian(endian) {}

    Accum(Accum const &other) : fOpt(other.fOpt), fEndian(other.fEndian) {
      other.mergeInto(*this);
    }
    Accum &operator=(Accum const &other) {
      if (this == &other) {
        return *this;
      }
      other.mergeInto(*this);
      return *this;
    }

    void mergeInto(Accum &out) const {
      for (auto const &i : fMaps) {
        out.fMaps[i.first] = i.second;
      }
      for (auto const &i : fRegions) {
        auto dim = i.first;
        std::unordered_map<Pos2i, ChunksInRegion, Pos2iHasher> const &regions = i.second;
        for (auto const &j : regions) {
          Pos2i region = j.first;
          std::copy(j.second.fChunks.begin(), j.second.fChunks.end(), std::inserter(out.fRegions[dim][region].fChunks, out.fRegions[dim][region].fChunks.end()));
        }
      }
      for (auto const &i : fStructurePieces) {
        std::copy(i.second.begin(), i.second.end(), std::back_inserter(out.fStructurePieces[i.first]));
      }
      out.fNumChunks += fNumChunks;
    }

    void accept(std::string const &key, std::string const &value) {
      using namespace std;
      auto parsed = mcfile::be::DbKey::Parse(key);
      if (!parsed) {
        return;
      }
      if (parsed->fIsTagged) {
        uint8_t tag = parsed->fTagged.fTag;
        mcfile::Dimension d = parsed->fTagged.fDimension;
        if (!fOpt.fDimensionFilter.empty()) {
          if (fOpt.fDimensionFilter.find(d) == fOpt.fDimensionFilter.end()) {
            return;
          }
        }
        switch (tag) {
        case static_cast<uint8_t>(mcfile::be::DbKey::Tag::Data3D):
        case static_cast<uint8_t>(mcfile::be::DbKey::Tag::Data2D): {
          int cx = parsed->fTagged.fChunk.fX;
          int cz = parsed->fTagged.fChunk.fZ;
          Pos2i c(cx, cz);
          if (!fOpt.fChunkFilter.empty()) {
            if (fOpt.fChunkFilter.find(c) == fOpt.fChunkFilter.end()) {
              return;
            }
          }
          int rx = mcfile::Coordinate::RegionFromChunk(cx);
          int rz = mcfile::Coordinate::RegionFromChunk(cz);
          Pos2i r(rx, rz);
          fRegions[d][r].fChunks.insert(c);
          fNumChunks++;
          break;
        }
        case static_cast<uint8_t>(mcfile::be::DbKey::Tag::StructureBounds): {
          std::vector<StructurePiece> buffer;
          StructurePiece::Parse(value, buffer);
          copy(buffer.begin(), buffer.end(), back_inserter(fStructurePieces[d]));
          break;
        }
        }
      } else if (parsed->fUnTagged.starts_with("map_")) {
        int64_t mapId;
        auto parsed = MapInfo::Parse(value, mapId, fEndian);
        if (!parsed) {
          return;
        }
        fMaps[mapId] = *parsed;
      }
    }
  };

public:
  static std::shared_ptr<Context> Init(leveldb::DB &db,
                                       Options opt,
                                       mcfile::Endian endian,
                                       std::map<mcfile::Dimension, std::vector<std::pair<Pos2i, ChunksInRegion>>> &regions,
                                       int &totalChunks,
                                       int64_t gameTick,
                                       GameMode gameMode,
                                       unsigned int concurrency) {
    using namespace std;
    using namespace leveldb;
    using namespace mcfile;
    namespace fs = std::filesystem;

#if defined(EMSCRIPTEN)
    Accum accum(opt, endian);
    unique_ptr<Iterator> itr(db.NewIterator({}));
    for (itr->SeekToFirst(); itr->Valid(); itr->Next()) {
      accum.accept(itr->key().ToString(), itr->value().ToString());
    }
#else
    Accum accum = AsyncIterator::IterateUnordered<Accum>(
        db,
        concurrency,
        Accum(opt, endian),
        [](string const &key, string const &value, Accum &out) -> void { out.accept(key, value); },
        [](Accum const &from, Accum &to) -> void { from.mergeInto(to); });
#endif

    auto mapInfo = make_shared<MapInfo>();
    for (auto const &it : accum.fMaps) {
      mapInfo->add(it.second, it.first);
    }

    for (auto const &i : accum.fRegions) {
      mcfile::Dimension dimension = i.first;
      for (auto const &j : i.second) {
        Pos2i region = j.first;
        regions[dimension].push_back(make_pair(region, j.second));
      }
    }

    totalChunks = accum.fNumChunks;

    auto structureInfo = make_shared<StructureInfo>();
    for (auto const &i : accum.fStructurePieces) {
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
        switch (type) {
        case StructureType::Monument:
          Volume::ConnectGreed(volumes, 58, 23, 58);
          break;
        default:
          break;
        }
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
    return std::shared_ptr<Context>(new Context(endian, temp, mapInfo, structureInfo, gameTick, gameMode));
  }
};

std::shared_ptr<Context> Context::Init(leveldb::DB &db,
                                       Options opt,
                                       mcfile::Endian endian,
                                       std::map<mcfile::Dimension, std::vector<std::pair<Pos2i, ChunksInRegion>>> &regions,
                                       int &totalChunks,
                                       int64_t gameTick,
                                       GameMode gameMode,
                                       unsigned int concurrency) {
  return Impl::Init(db, opt, endian, regions, totalChunks, gameTick, gameMode, concurrency);
}

void Context::markMapUuidAsUsed(int64_t uuid) {
  fUsedMapUuids.insert(uuid);
}

void Context::mergeInto(Context &other) const {
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

Status Context::postProcess(std::filesystem::path root, leveldb::DB &db) const {
  if (auto st = exportMaps(root, db); !st.ok()) {
    return st;
  }
  return exportPoi(root);
}

std::optional<MapInfo::Map> Context::mapFromUuid(int64_t mapUuid) const {
  return fMapInfo->mapFromUuid(mapUuid);
}

void Context::structures(mcfile::Dimension d, Pos2i chunk, std::vector<StructureInfo::Structure> &buffer) {
  fStructureInfo->structures(d, chunk, buffer);
}

std::shared_ptr<Context> Context::make() const {
  auto ret = std::shared_ptr<Context>(new Context(fEndian, fTempDirectory, fMapInfo, fStructureInfo, fGameTick, fGameMode));
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

void Context::setLocalPlayerIds(int64_t entityIdB, Uuid const &entityIdJ) {
  LocalPlayer lp;
  lp.fBedrockId = entityIdB;
  lp.fJavaId = entityIdJ;
  fLocalPlayer = lp;
}

std::optional<Uuid> Context::mapLocalPlayerId(int64_t entityIdB) const {
  if (fLocalPlayer && fLocalPlayer->fBedrockId == entityIdB) {
    return fLocalPlayer->fJavaId;
  } else {
    return std::nullopt;
  }
}

bool Context::isLocalPlayerId(Uuid const &uuid) const {
  if (fLocalPlayer) {
    return UuidPred{}(uuid, fLocalPlayer->fJavaId);
  } else {
    return false;
  }
}

void Context::setRootVehicle(Uuid const &vehicleUid) {
  RootVehicle rv;
  rv.fUid = vehicleUid;
  fRootVehicle = rv;
}

void Context::setRootVehicleEntity(CompoundTagPtr const &vehicleEntity) {
  assert(fRootVehicle);
  if (!fRootVehicle) {
    return;
  }
  fRootVehicle->fVehicle = vehicleEntity;
}

bool Context::isRootVehicle(Uuid const &uuid) const {
  if (fRootVehicle) {
    return UuidPred{}(uuid, fRootVehicle->fUid);
  } else {
    return false;
  }
}

std::optional<std::pair<Uuid, CompoundTagPtr>> Context::drainRootVehicle() {
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

void Context::setShoulderEntityLeft(int64_t uid) {
  fShoulderEntityLeftId = uid;
}

void Context::setShoulderEntityRight(int64_t uid) {
  fShoulderEntityRightId = uid;
}

bool Context::setShoulderEntityIfItIs(int64_t uid, CompoundTagPtr entityB) {
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

void Context::drainShoulderEntities(CompoundTagPtr &left, CompoundTagPtr &right) {
  fShoulderEntityLeft.swap(left);
  fShoulderEntityRight.swap(right);
}

void Context::addToPoiIfItIs(mcfile::Dimension dim, Pos3i const &pos, mcfile::je::Block const &block) {
  if (PoiBlocks::Interest(block)) {
    fPoiBlocks[dim].add(pos, block.fId);
  }
}

Status Context::exportMaps(std::filesystem::path const &root, leveldb::DB &db) const {
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

Status Context::exportPoi(std::filesystem::path const &root) const {
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

} // namespace je2be::toje
