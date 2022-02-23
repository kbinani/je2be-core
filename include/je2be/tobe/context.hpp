#pragma once

namespace je2be::tobe {

struct Context {
  Context(JavaEditionMap const &mapInfo, WorldData &wd) : fMapInfo(mapInfo), fWorldData(wd) {}

  JavaEditionMap const &fMapInfo;
  WorldData &fWorldData;
};

} // namespace je2be::tobe
