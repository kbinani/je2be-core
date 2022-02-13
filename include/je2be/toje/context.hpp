#pragma once

namespace je2be::toje {

class Context {
public:
  explicit Context(std::shared_ptr<MapInfo const> const &mapInfo) : fMapInfo(mapInfo) {}

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

  std::shared_ptr<Context> make() const {
    return std::make_shared<Context>(fMapInfo);
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
  std::unordered_set<int64_t> fUsedMapUuids;
};

} // namespace je2be::toje
