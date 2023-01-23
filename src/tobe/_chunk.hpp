#pragma once

#include "enums/_game-mode.hpp"
#include "tobe/_java-edition-map.hpp"

namespace je2be {

class DbInterface;

}

namespace je2be::tobe {

class WorldData;

class Chunk {
  class Impl;
  Chunk() = delete;

public:
  struct Result {
    Result() : fData(nullptr), fOk(false) {}

    std::shared_ptr<WorldData> fData;
    bool fOk;
  };

  struct PlayerAttachedEntities {
    CompoundTagPtr fVehicle;
    std::vector<CompoundTagPtr> fPassengers;
    std::vector<CompoundTagPtr> fShoulderRiders;
    i64 fLocalPlayerUid;
  };

public:
  static Result Convert(mcfile::Dimension dim,
                        DbInterface &db,
                        mcfile::je::Region region,
                        int cx, int cz,
                        JavaEditionMap mapInfo,
                        std::filesystem::path entitiesDir,
                        std::optional<PlayerAttachedEntities> playerAttachedEntities,
                        i64 gameTick,
                        int difficultyBedrock,
                        bool allowCommand,
                        GameMode gameType);
};

} // namespace je2be::tobe
