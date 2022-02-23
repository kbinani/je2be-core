#pragma once

namespace je2be::tobe {

struct Context {
  Context(JavaEditionMap const &mapInfo, WorldData &wd, int64_t gameTick) : fMapInfo(mapInfo), fWorldData(wd), fGameTick(gameTick) {}

  JavaEditionMap const &fMapInfo;
  WorldData &fWorldData;
  int64_t const fGameTick;
};

} // namespace je2be::tobe
