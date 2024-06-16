#include "bedrock/_context.hpp"

#if !defined(EMSCRIPTEN)
#include "db/_async-iterator.hpp"
#endif
#include "_props.hpp"
#include "db/_readonly-db.hpp"
#include "structure/_structure-piece.hpp"

namespace je2be::bedrock {

class Context::Impl {
  struct Accum {
    std::map<i64, MapInfo::Map> fMaps;
    std::map<mcfile::Dimension, std::unordered_map<Pos2i, ChunksInRegion, Pos2iHasher>> fRegions;
    std::map<mcfile::Dimension, std::vector<StructurePiece>> fStructurePieces;
    std::unordered_map<i32, std::pair<mcfile::Dimension, Pos3i>> fLodestones;
    int fNumChunks = 0;

    Options fOpt;
    mcfile::Encoding fEncoding;

    Accum(Options opt, mcfile::Encoding encoding) : fOpt(opt), fEncoding(encoding) {}

    Accum(Accum const &other) : fOpt(other.fOpt), fEncoding(other.fEncoding) {
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
          auto &dest = out.fRegions[dim][region].fChunks;
          for (auto const &k : j.second.fChunks) {
            dest.insert(k);
          }
        }
      }
      for (auto const &i : fStructurePieces) {
        std::copy(i.second.begin(), i.second.end(), std::back_inserter(out.fStructurePieces[i.first]));
      }
      out.fNumChunks += fNumChunks;
      for (auto const &it : fLodestones) {
        out.fLodestones[it.first] = it.second;
      }
    }

    void accept(std::string const &key, std::string const &value) {
      using namespace std;
      auto parsed = mcfile::be::DbKey::Parse(key);
      if (parsed.fIsTagged) {
        u8 tag = parsed.fTagged.fTag;
        mcfile::Dimension d = parsed.fTagged.fDimension;
        if (!fOpt.fDimensionFilter.empty()) {
          if (fOpt.fDimensionFilter.find(d) == fOpt.fDimensionFilter.end()) {
            return;
          }
        }
        switch (tag) {
        case static_cast<u8>(mcfile::be::DbKey::Tag::Data3D):
        case static_cast<u8>(mcfile::be::DbKey::Tag::Data2D): {
          int cx = parsed.fTagged.fChunk.fX;
          int cz = parsed.fTagged.fChunk.fZ;
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
        case static_cast<u8>(mcfile::be::DbKey::Tag::StructureBounds): {
          std::vector<StructurePiece> buffer;
          StructurePiece::Parse(value, buffer);
          copy(buffer.begin(), buffer.end(), back_inserter(fStructurePieces[d]));
          break;
        }
        }
      } else {
        if (parsed.fUnTagged.starts_with("map_")) {
          i64 mapId;
          auto mapInfo = MapInfo::Parse(value, mapId, fEncoding);
          if (!mapInfo) {
            return;
          }
          fMaps[mapId] = *mapInfo;
        } else if (parsed.fUnTagged.starts_with("PosTrackDB-")) {
          if (auto c = CompoundTag::Read(value, fEncoding); c) {
            if (auto dim = c->int32(u8"dim"); dim) {
              std::optional<mcfile::Dimension> d;
              switch (*dim) {
              case 0:
                d = mcfile::Dimension::Overworld;
                break;
              case 1:
                d = mcfile::Dimension::Nether;
                break;
              case 2:
                d = mcfile::Dimension::End;
                break;
              }
              if (d) {
                if (auto id = c->string(u8"id"); id && id->starts_with(u8"0x")) {
                  std::string str;
                  std::copy(id->begin() + 2, id->end(), std::back_inserter(str));
                  auto numId = std::stoi(str, nullptr, 16);
                  if (auto pos = props::GetPos3iFromListTag(*c, u8"pos"); pos) {
                    fLodestones[numId] = std::make_pair(*d, *pos);
                  }
                }
              }
            }
          }
        }
      }
    }

    static void Merge(Accum const &from, Accum &to) {
      from.mergeInto(to);
    }

    static void Accept(std::string const &key, std::string const &value, Accum &out) {
      out.accept(key, value);
    }
  };

public:
  static Status Init(std::filesystem::path const &dbname,
                     Options opt,
                     mcfile::Encoding encoding,
                     std::map<mcfile::Dimension, std::vector<std::pair<Pos2i, ChunksInRegion>>> &regions,
                     u64 &totalChunks,
                     i64 gameTick,
                     GameMode gameMode,
                     unsigned int concurrency,
                     std::unique_ptr<Context> &out) {
    using namespace std;
    using namespace leveldb;
    using namespace mcfile;
    namespace fs = std::filesystem;

    DB *dbPtr = nullptr;
    unique_ptr<ReadonlyDb::Closer> closer;
    if (auto st = ReadonlyDb::Open(dbname, &dbPtr, opt.getTempDirectory(), closer); !st.ok()) {
      return JE2BE_ERROR_PUSH(st);
    }
    unique_ptr<DB> db;
    if (!dbPtr) {
      return JE2BE_ERROR;
    }
    db.reset(dbPtr);

#if defined(EMSCRIPTEN)
    Accum accum(opt, encoding);
    unique_ptr<Iterator> itr(db->NewIterator({}));
    if (auto st = itr->status(); !st.ok()) {
      return JE2BE_ERROR_PUSH(Status::FromLevelDBStatus(st));
    }
    for (itr->SeekToFirst(); itr->Valid(); itr->Next()) {
      if (auto st = itr->status(); !st.ok()) {
        return JE2BE_ERROR_PUSH(Status::FromLevelDBStatus(st));
      }
      accum.accept(itr->key().ToString(), itr->value().ToString());
    }
#else
    auto [accum, status] = AsyncIterator::IterateUnordered<Accum>(
        *db,
        concurrency,
        Accum(opt, encoding),
        Accum::Accept,
        Accum::Merge);
    if (!status.ok()) {
      return JE2BE_ERROR_PUSH(status);
    }
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
    for (auto &i : regions) {
      sort(i.second.begin(), i.second.end(), [](pair<Pos2i, Context::ChunksInRegion> const &a, pair<Pos2i, Context::ChunksInRegion> const &b) {
        return a.second.fChunks.size() > b.second.fChunks.size();
      });
    }

    totalChunks = accum.fNumChunks;

    auto structureInfo = make_shared<StructureInfo>();
    for (auto const &[dim, pieces] : accum.fStructurePieces) {
      unordered_map<StructureType, vector<StructurePiece>> categorized;
      for (StructurePiece const &piece : pieces) {
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
          structureInfo->add(dim, s);
        }
      }
    }

    fs::path temp = opt.getTempDirectory();
    out.reset(new Context(encoding, temp, mapInfo, structureInfo, gameTick, gameMode, accum.fLodestones));
    return Status::Ok();
  }
};

Status Context::Init(std::filesystem::path const &dbname,
                     Options opt,
                     mcfile::Encoding encoding,
                     std::map<mcfile::Dimension, std::vector<std::pair<Pos2i, ChunksInRegion>>> &regions,
                     u64 &totalChunks,
                     i64 gameTick,
                     GameMode gameMode,
                     unsigned int concurrency,
                     std::unique_ptr<Context> &out) {
  return Impl::Init(dbname, opt, encoding, regions, totalChunks, gameTick, gameMode, concurrency, out);
}

void Context::markMapUuidAsUsed(i64 uuid) {
  fUsedMapUuids.insert(uuid);
}

void Context::mergeInto(Context &other) const {
  for (i64 uuid : fUsedMapUuids) {
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
}

Status Context::postProcess(std::filesystem::path root, mcfile::be::DbInterface &db) const {
  if (auto st = exportMaps(root, db); !st.ok()) {
    return JE2BE_ERROR_PUSH(st);
  }
  if (auto st = exportPoi(root); !st.ok()) {
    return JE2BE_ERROR_PUSH(st);
  }
  return Status::Ok();
}

std::optional<MapInfo::Map> Context::mapFromUuid(i64 mapUuid) const {
  return fMapInfo->mapFromUuid(mapUuid);
}

void Context::structures(mcfile::Dimension d, Pos2i chunk, std::vector<StructureInfo::Structure> &buffer) {
  fStructureInfo->structures(d, chunk, buffer);
}

std::shared_ptr<Context> Context::make() const {
  auto ret = std::shared_ptr<Context>(new Context(fEncoding, fTempDirectory, fMapInfo, fStructureInfo, fGameTick, fGameMode, fLodestones));
  ret->fLocalPlayer = fLocalPlayer;
  if (fRootVehicle) {
    ret->fRootVehicle = *fRootVehicle;
  }
  ret->fShoulderEntityLeftId = fShoulderEntityLeftId;
  ret->fShoulderEntityRightId = fShoulderEntityRightId;
  ret->fDataPackBundle = fDataPackBundle;
  return ret;
}

void Context::setLocalPlayerIds(i64 entityIdB, Uuid const &entityIdJ) {
  LocalPlayer lp;
  lp.fBedrockId = entityIdB;
  lp.fJavaId = entityIdJ;
  fLocalPlayer = lp;
}

std::optional<Uuid> Context::mapLocalPlayerId(i64 entityIdB) const {
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

void Context::setShoulderEntityLeft(i64 uid) {
  fShoulderEntityLeftId = uid;
}

void Context::setShoulderEntityRight(i64 uid) {
  fShoulderEntityRightId = uid;
}

bool Context::setShoulderEntityIfItIs(i64 uid, CompoundTagPtr entityB) {
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

Status Context::exportMaps(std::filesystem::path const &root, mcfile::be::DbInterface &db) const {
  using namespace mcfile;

  if (!Fs::CreateDirectories(root / "data")) {
    return JE2BE_ERROR;
  }

  std::optional<int> maxMapNumber;

  for (i64 uuid : fUsedMapUuids) {
    auto map = fMapInfo->mapFromUuid(uuid);
    if (!map) {
      continue;
    }
    int number = map->fNumber;
    auto key = mcfile::be::DbKey::Map(uuid);
    auto str = db.get(key);
    if (!str) {
      continue;
    }
    auto dataB = CompoundTag::Read(*str, fEncoding);
    if (!dataB) {
      continue;
    }
    auto dataJ = Compound();
    auto dimensionB = dataB->byte(u8"dimension", 0);
    Dimension dim = Dimension::Overworld;
    if (auto dimension = DimensionFromBedrockDimension(dimensionB); dimension) {
      dim = *dimension;
    }
    dataJ->set(u8"dimension", JavaStringFromDimension(dim));
    CopyBoolValues(*dataB, *dataJ, {{u8"mapLocked", u8"locked"}});
    CopyByteValues(*dataB, *dataJ, {{u8"scale"}, {u8"unlimitedTracking"}});
    CopyIntValues(*dataB, *dataJ, {{u8"xCenter"}, {u8"zCenter"}});
    auto colorsTagB = dataB->byteArrayTag(u8"colors");
    if (!colorsTagB) {
      continue;
    }
    auto const &colorsB = colorsTagB->value();
    if (colorsB.size() != 65536) {
      continue;
    }
    std::vector<u8> colorsJ(16384);
    for (int i = 0; i < colorsJ.size(); i++) {
      u8 r = colorsB[i * 4];
      u8 g = colorsB[i * 4 + 1];
      u8 b = colorsB[i * 4 + 2];
      u8 a = colorsB[i * 4 + 3];
      Rgba rgb(r, g, b, a);
      u8 id = MapColor::MostSimilarColorId(rgb);
      colorsJ[i] = id;
    }
    dataJ->set(u8"colors", std::make_shared<ByteArrayTag>(colorsJ));
    auto tagJ = Compound();
    tagJ->set(u8"data", dataJ);
    tagJ->set(u8"DataVersion", Int(kJavaDataVersion));

    auto path = root / "data" / ("map_" + std::to_string(number) + ".dat");
    auto s = std::make_shared<mcfile::stream::GzFileOutputStream>(path);
    if (!CompoundTag::Write(*tagJ, s, mcfile::Encoding::Java)) {
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
    d->set(u8"map", Int(*maxMapNumber));
    idcounts->set(u8"data", d);
    idcounts->set(u8"DataVersion", Int(kJavaDataVersion));
    auto path = root / "data" / "idcounts.dat";
    auto s = std::make_shared<mcfile::stream::GzFileOutputStream>(path);
    if (!CompoundTag::Write(*idcounts, s, mcfile::Encoding::Java)) {
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
    if (!poi.write(dir, kJavaDataVersion)) {
      return JE2BE_ERROR;
    }
  }
  return Status::Ok();
}

std::optional<std::pair<mcfile::Dimension, Pos3i>> Context::getLodestone(i32 trackingHandle) const {
  auto found = fLodestones.find(trackingHandle);
  if (found == fLodestones.end()) {
    return std::nullopt;
  } else {
    return found->second;
  }
}

} // namespace je2be::bedrock
