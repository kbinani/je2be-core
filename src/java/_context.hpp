#pragma once

#include "enums/_game-mode.hpp"

namespace je2be::java {

class JavaEditionMap;
class WorldData;
class LodestoneRegistrar;

struct Context {
  Context(JavaEditionMap const &mapInfo,
          std::shared_ptr<LodestoneRegistrar> const &lodestones,
          WorldData &wd,
          i64 gameTick,
          int difficultyBedrock,
          bool allowCommand,
          GameMode gameType)
      : fMapInfo(mapInfo), fLodestones(lodestones), fWorldData(wd), fGameTick(gameTick), fDifficultyBedrock(difficultyBedrock), fAllowCommand(allowCommand), fGameType(gameType) {}

  JavaEditionMap const &fMapInfo;
  std::shared_ptr<LodestoneRegistrar> fLodestones;
  WorldData &fWorldData;
  i64 const fGameTick;
  int const fDifficultyBedrock;
  bool const fAllowCommand;
  GameMode const fGameType;
  std::unordered_set<std::u8string> fExperiments;
};

} // namespace je2be::java
