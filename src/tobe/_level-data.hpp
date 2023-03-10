#pragma once

#include <je2be/status.hpp>
#include <je2be/tobe/options.hpp>

#include "enums/_game-mode.hpp"
#include "tobe/_java-edition-map.hpp"
#include "tobe/_level.hpp"
#include "tobe/_map.hpp"
#include "tobe/portal/_portals.hpp"
#include "tobe/structure/_structures.hpp"

namespace je2be::tobe {

class LevelData {
public:
  LevelData(std::filesystem::path const &input,
            Options const &opt,
            i64 gameTick,
            int difficultyBedrock,
            bool allowCommand,
            GameMode gameType) : fInput(input), fJavaEditionMap(input, opt), fOptions(opt), fGameTick(gameTick), fDifficultyBedrock(difficultyBedrock), fAllowCommand(allowCommand), fGameType(gameType) {}

  [[nodiscard]] bool put(DbInterface &db, CompoundTag const &javaLevelData) {
    if (!fPortals.putInto(db)) {
      return false;
    }
    bool ok = fJavaEditionMap.each([this, &db](i32 mapId) {
      auto found = fMapItems.find(mapId);
      if (found == fMapItems.end()) {
        return true;
      }
      return Map::Convert(mapId, *found->second, fInput, fOptions, db);
    });
    if (!ok) {
      return false;
    }
    if (!putAutonomousEntities(db)) {
      return false;
    }

    auto theEnd = Level::TheEndData(javaLevelData, fAutonomousEntities.size(), fEndPortalsInEndDimension);
    if (theEnd) {
      db.put(mcfile::be::DbKey::TheEnd(), *theEnd);
    } else {
      db.del(mcfile::be::DbKey::TheEnd());
    }

    if (!fStructures.put(db)) {
      return false;
    }

    auto mobEvents = Level::MobEvents(javaLevelData);
    if (mobEvents) {
      db.put(mcfile::be::DbKey::MobEvents(), *mobEvents);
    } else {
      db.del(mcfile::be::DbKey::MobEvents());
    }

    return true;
  }

private:
  [[nodiscard]] bool putAutonomousEntities(DbInterface &db) {
    using namespace mcfile::stream;

    auto list = List<Tag::Type::Compound>();
    for (auto const &e : fAutonomousEntities) {
      list->push_back(e);
    }
    auto root = Compound();
    root->set("AutonomousEntityList", list);

    auto buffer = CompoundTag::Write(*root, mcfile::Endian::Little);
    if (!buffer) {
      return false;
    }

    leveldb::Slice v(*buffer);
    db.put(mcfile::be::DbKey::AutonomousEntities(), v);

    return true;
  }

private:
  std::filesystem::path fInput;

public:
  Portals fPortals;
  JavaEditionMap fJavaEditionMap;
  std::unordered_map<i32, CompoundTagPtr> fMapItems;
  std::vector<CompoundTagPtr> fAutonomousEntities;
  std::unordered_set<Pos3i, Pos3iHasher> fEndPortalsInEndDimension;
  Options fOptions;
  Structures fStructures;
  std::optional<Status::ErrorData> fError;
  i64 fMaxChunkLastUpdate = 0;
  i64 const fGameTick;
  int const fDifficultyBedrock;
  bool const fAllowCommand;
  GameMode const fGameType;

  struct VehicleAndPassengers {
    Pos2i fChunk;
    CompoundTagPtr fVehicle;
    std::vector<CompoundTagPtr> fPassengers;
  };
  struct PlayerAttachedEntities {
    mcfile::Dimension fDim;
    i64 fLocalPlayerUid;
    std::optional<VehicleAndPassengers> fVehicle;
    std::vector<std::pair<Pos2i, CompoundTagPtr>> fShoulderRiders;
  };
  std::optional<PlayerAttachedEntities> fPlayerAttachedEntities;
};

} // namespace je2be::tobe
