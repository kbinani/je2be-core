#pragma once

#include "enums/_game-mode.hpp"
#include "java/_java-edition-map.hpp"

namespace je2be {

class DbInterface;

}

namespace je2be::java {

class WorldData;
class EntityStore;
class LodestoneRegistrar;

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
    i64 fLocalPlayerUid = -1;
  };

public:
  static Result Convert(mcfile::Dimension dim,
                        DbInterface &db,
                        mcfile::je::McaEditor &terrain,
                        mcfile::je::McaEditor *entities,
                        int cx, int cz,
                        JavaEditionMap mapInfo,
                        std::shared_ptr<LodestoneRegistrar> const &lodestones,
                        std::shared_ptr<EntityStore> const &entityStore,
                        std::optional<PlayerAttachedEntities> playerAttachedEntities,
                        i64 gameTick,
                        int difficultyBedrock,
                        bool allowCommand,
                        GameMode gameType);
};

} // namespace je2be::java
