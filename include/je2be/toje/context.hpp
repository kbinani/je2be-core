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
    for (auto const &it : fPassengers) {
      if (it.second.empty()) {
        continue;
      }
      other.fPassengers[it.first] = it.second;
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
  std::unordered_map<Uuid, std::map<size_t, Uuid>, UuidHasher, UuidPred> fPassengers;
  std::unordered_map<Uuid, int64_t, UuidHasher, UuidPred> fLeashedEntities;
  std::unordered_map<int64_t, std::shared_ptr<CompoundTag>> fLeashKnots;

private:
  std::shared_ptr<MapInfo const> fMapInfo;
  std::unordered_set<int64_t> fUsedMapUuids;
};

} // namespace je2be::toje
