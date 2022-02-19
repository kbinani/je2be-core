#pragma once

namespace je2be::toje {

class Context {
  Context(std::shared_ptr<MapInfo const> const &mapInfo, std::shared_ptr<StructureInfo const> const &structureInfo) : fMapInfo(mapInfo), fStructureInfo(structureInfo) {}

public:
  struct ChunksInRegion {
    std::unordered_set<Pos2i, Pos2iHasher> fChunks;
  };

  static std::shared_ptr<Context> Init(leveldb::DB &db, std::map<mcfile::Dimension, std::unordered_map<Pos2i, ChunksInRegion, Pos2iHasher>> &regions, int &totalChunks) {
    using namespace std;
    using namespace leveldb;
    using namespace mcfile;

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
        switch (tag) {
        case static_cast<uint8_t>(mcfile::be::DbKey::Tag::Data3D):
        case static_cast<uint8_t>(mcfile::be::DbKey::Tag::Data2D): {
          int cx = parsed->fTagged.fChunk.fX;
          int cz = parsed->fTagged.fChunk.fZ;
          int rx = Coordinate::RegionFromChunk(cx);
          int rz = Coordinate::RegionFromChunk(cz);
          Pos2i c(cx, cz);
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
        auto parsed = MapInfo::Parse(itr->value().ToString(), mapId);
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

    return std::shared_ptr<Context>(new Context(mapInfo, structureInfo));
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
  }

  bool postProcess(std::filesystem::path root, leveldb::DB &db) {
    using namespace mcfile;
    using namespace props;

    if (!Fs::CreateDirectories(root / "data")) {
      return false;
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
      auto b = CompoundTag::Read(str, {.fLittleEndian = true});
      if (!b) {
        continue;
      }
      auto j = std::make_shared<CompoundTag>();
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
      auto tagJ = std::make_shared<CompoundTag>();
      tagJ->set("data", j);
      tagJ->set("DataVersion", Int(mcfile::je::Chunk::kDataVersion));

      auto path = root / "data" / ("map_" + std::to_string(number) + ".dat");
      auto s = std::make_shared<mcfile::stream::GzFileOutputStream>(path);
      mcfile::stream::OutputStreamWriter osw(s);
      if (!tagJ->writeAsRoot(osw)) {
        return false;
      }
      if (maxMapNumber) {
        maxMapNumber = (std::max)(*maxMapNumber, number);
      } else {
        maxMapNumber = number;
      }
    }
    if (maxMapNumber) {
      auto idcounts = std::make_shared<CompoundTag>();
      auto d = std::make_shared<CompoundTag>();
      d->set("map", Int(*maxMapNumber));
      idcounts->set("data", d);
      idcounts->set("DataVersion", Int(mcfile::je::Chunk::kDataVersion));
      auto path = root / "data" / "idcounts.dat";
      auto s = std::make_shared<mcfile::stream::GzFileOutputStream>(path);
      mcfile::stream::OutputStreamWriter osw(s);
      if (!idcounts->writeAsRoot(osw)) {
        return false;
      }
    }
    return true;
  }

  std::optional<MapInfo::Map> mapFromUuid(int64_t mapUuid) const {
    return fMapInfo->mapFromUuid(mapUuid);
  }

  void structures(mcfile::Dimension d, Pos2i chunk, std::vector<StructureInfo::Structure> &buffer) {
    fStructureInfo->structures(d, chunk, buffer);
  }

  std::shared_ptr<Context> make() const {
    return std::shared_ptr<Context>(new Context(fMapInfo, fStructureInfo));
  }

public:
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

private:
  std::shared_ptr<MapInfo const> fMapInfo;
  std::shared_ptr<StructureInfo const> fStructureInfo;
  std::unordered_set<int64_t> fUsedMapUuids;
};

} // namespace je2be::toje
