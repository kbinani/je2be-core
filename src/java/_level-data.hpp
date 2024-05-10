#pragma once

#include <je2be/java/options.hpp>
#include <je2be/status.hpp>

#include "enums/_game-mode.hpp"
#include "java/_context.hpp"
#include "java/_java-edition-map.hpp"
#include "java/_level.hpp"
#include "java/_lodestone-registrar.hpp"
#include "java/_map.hpp"
#include "java/portal/_portals.hpp"
#include "java/structure/_structures.hpp"

namespace je2be::java {

class LevelData {
public:
  LevelData(std::filesystem::path const &input,
            Options const &opt,
            i64 gameTick,
            int difficultyBedrock,
            bool allowCommand,
            GameMode gameType,
            int dataVersion)
      : fInput(input),
        fJavaEditionMap(input, opt),
        fOptions(opt),
        fGameTick(gameTick),
        fDifficultyBedrock(difficultyBedrock),
        fAllowCommand(allowCommand),
        fGameType(gameType),
        fDataVersion(dataVersion),
        fLodestones(std::make_shared<LodestoneRegistrar>()),
        fUuids(std::make_shared<UuidRegistrar>()) {}

  [[nodiscard]] Status put(DbInterface &db, CompoundTag const &javaLevelData, std::shared_ptr<UuidRegistrar> const &uuids) {
    Status st;
    if (st = fPortals.putInto(db); !st.ok()) {
      return JE2BE_ERROR_PUSH(st);
    }
    st = fJavaEditionMap.each([this, &db](i32 mapId) {
      auto found = fMapItems.find(mapId);
      if (found == fMapItems.end()) {
        return Status::Ok();
      }
      return Map::Convert(mapId, *found->second, fInput, fOptions, db);
    });
    if (!st.ok()) {
      return JE2BE_ERROR_PUSH(st);
    }
    if (st = putAutonomousEntities(db); !st.ok()) {
      return JE2BE_ERROR_PUSH(st);
    }

    auto theEnd = Level::TheEndData(javaLevelData, fAutonomousEntities.size(), fEndPortalsInEndDimension, uuids);
    if (theEnd) {
      if (st = db.put(mcfile::be::DbKey::TheEnd(), *theEnd); !st.ok()) {
        return JE2BE_ERROR_PUSH(st);
      }
    } else {
      if (st = db.del(mcfile::be::DbKey::TheEnd()); !st.ok()) {
        return JE2BE_ERROR_PUSH(st);
      }
    }

    if (st = fStructures.put(db); !st.ok()) {
      return JE2BE_ERROR_PUSH(st);
    }

    auto mobEvents = Level::MobEvents(javaLevelData);
    if (mobEvents) {
      if (st = db.put(mcfile::be::DbKey::MobEvents(), *mobEvents); !st.ok()) {
        return JE2BE_ERROR_PUSH(st);
      }
    } else {
      if (st = db.del(mcfile::be::DbKey::MobEvents()); !st.ok()) {
        return JE2BE_ERROR_PUSH(st);
      }
    }

    return fLodestones->put(db);
  }

private:
  [[nodiscard]] Status putAutonomousEntities(DbInterface &db) {
    using namespace mcfile::stream;

    auto list = List<Tag::Type::Compound>();
    for (auto const &e : fAutonomousEntities) {
      list->push_back(e);
    }
    auto root = Compound();
    root->set(u8"AutonomousEntityList", list);

    auto buffer = CompoundTag::Write(*root, mcfile::Encoding::LittleEndian);
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
  int const fDataVersion;
  std::unordered_set<std::u8string> fExperiments;

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
  std::shared_ptr<LodestoneRegistrar> const fLodestones;
  std::shared_ptr<UuidRegistrar> const fUuids;
};

} // namespace je2be::java
