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

  [[nodiscard]] Status put(DbInterface &db, CompoundTag const &javaLevelData) {
    Status st;
    if (st = fPortals.putInto(db); !st.ok()) {
      return st;
    }
    st = fJavaEditionMap.each([this, &db](i32 mapId) {
      auto found = fMapItems.find(mapId);
      if (found == fMapItems.end()) {
        return Status::Ok();
      }
      return Map::Convert(mapId, *found->second, fInput, fOptions, db);
    });
    if (!st.ok()) {
      return st;
    }
    if (st = putAutonomousEntities(db); !st.ok()) {
      return st;
    }

    auto theEnd = Level::TheEndData(javaLevelData, fAutonomousEntities.size(), fEndPortalsInEndDimension);
    if (theEnd) {
      if (st = db.put(mcfile::be::DbKey::TheEnd(), *theEnd); !st.ok()) {
        return st;
      }
    } else {
      if (st = db.del(mcfile::be::DbKey::TheEnd()); !st.ok()) {
        return st;
      }
    }

    if (st = fStructures.put(db); !st.ok()) {
      return st;
    }

    auto mobEvents = Level::MobEvents(javaLevelData);
    if (mobEvents) {
      return db.put(mcfile::be::DbKey::MobEvents(), *mobEvents);
    } else {
      return db.del(mcfile::be::DbKey::MobEvents());
    }
  }

private:
  [[nodiscard]] Status putAutonomousEntities(DbInterface &db) {
    using namespace mcfile::stream;

    auto list = List<Tag::Type::Compound>();
    for (auto const &e : fAutonomousEntities) {
      list->push_back(e);
    }
    auto root = Compound();
    root->set("AutonomousEntityList", list);

    auto buffer = CompoundTag::Write(*root, mcfile::Endian::Little);
    if (!buffer) {
      return JE2BE_ERROR;
    }

    leveldb::Slice v(*buffer);
    return db.put(mcfile::be::DbKey::AutonomousEntities(), v);
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
  std::unordered_set<std::string> fExperiments;

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
