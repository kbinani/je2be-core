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
    for (auto const &it : fVehicleEntities) {
      if (it.second.fPassengers.empty()) {
        continue;
      }
      other.fVehicleEntities[it.first] = it.second;
    }
    for (auto const &it : fLeashedEntities) {
      other.fLeashedEntities[it.first] = it.second;
    }
    for (auto const &it : fLeashKnots) {
      other.fLeashKnots[it.first] = it.second;
    }
  }

  bool postProcess(std::filesystem::path root, leveldb::DB &db) {
    //TODO:
    return true;
  }

  std::optional<MapInfo::Map> mapFromUuid(int64_t mapUuid) const {
    return fMapInfo->mapFronUuid(mapUuid);
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

private:
  std::shared_ptr<MapInfo const> fMapInfo;
  std::unordered_set<int64_t> fUsedMapUuids;
};

} // namespace je2be::toje
