#pragma once

namespace je2be::tobe {

struct Context {
  Context(JavaEditionMap const &mapInfo, WorldData &wd) : fMapInfo(mapInfo), fWorldData(wd) {}

  std::vector<std::shared_ptr<CompoundTag>> fPassengers;
  std::unordered_map<Pos2i, std::vector<std::shared_ptr<CompoundTag>>, Pos2iHasher> fLeashKnots;
  JavaEditionMap const &fMapInfo;
  WorldData &fWorldData;
};

} // namespace je2be::tobe
