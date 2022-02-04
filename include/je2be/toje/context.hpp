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

private:
  std::shared_ptr<MapInfo const> fMapInfo;
  std::unordered_set<int64_t> fUsedMapUuids;
};

} // namespace je2be::toje
