#pragma once

#include "enums/_game-mode.hpp"

namespace je2be::java {

class JavaEditionMap;
class WorldData;
class UuidRegistrar;
class LodestoneRegistrar;

struct Context {
  Context(JavaEditionMap const &mapInfo,
          std::shared_ptr<LodestoneRegistrar> const &lodestones,
          std::shared_ptr<UuidRegistrar> const &uuids,
          WorldData &wd,
          i64 gameTick,
          int difficultyBedrock,
          bool allowCommand,
          GameMode gameType)
      : fMapInfo(mapInfo), fLodestones(lodestones), fUuids(uuids), fWorldData(wd), fGameTick(gameTick), fDifficultyBedrock(difficultyBedrock), fAllowCommand(allowCommand), fGameType(gameType) {}

  JavaEditionMap const &fMapInfo;
  std::shared_ptr<LodestoneRegistrar> const fLodestones;
  std::shared_ptr<UuidRegistrar> const fUuids;
  WorldData &fWorldData;
  i64 const fGameTick;
  int const fDifficultyBedrock;
  bool const fAllowCommand;
  GameMode const fGameType;
  std::unordered_set<std::u8string> fExperiments;
};

} // namespace je2be::java
