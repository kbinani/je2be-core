#pragma once

#include "enums/_game-mode.hpp"

namespace je2be::tobe {

class JavaEditionMap;
class WorldData;

struct Context {
  Context(JavaEditionMap const &mapInfo,
          WorldData &wd,
          i64 gameTick,
          int difficultyBedrock,
          bool allowCommand,
          GameMode gameType)
      : fMapInfo(mapInfo), fWorldData(wd), fGameTick(gameTick), fDifficultyBedrock(difficultyBedrock), fAllowCommand(allowCommand), fGameType(gameType) {}

  JavaEditionMap const &fMapInfo;
  WorldData &fWorldData;
  i64 const fGameTick;
  int const fDifficultyBedrock;
  bool const fAllowCommand;
  GameMode const fGameType;
};

} // namespace je2be::tobe
