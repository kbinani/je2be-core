#pragma once

namespace je2be::tobe {

class LevelData {
public:
  LevelData(std::filesystem::path const &input, Options const &opt, int64_t gameTick) : fInput(input), fJavaEditionMap(input, opt), fOptions(opt), fGameTick(gameTick) {}

  [[nodiscard]] bool put(DbInterface &db, CompoundTag const &javaLevelData) {
    if (!fPortals.putInto(db)) {
      return false;
    }
    bool ok = fJavaEditionMap.each([this, &db](int32_t mapId) {
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

    auto s = std::make_shared<ByteStream>();
    OutputStreamWriter w(s, mcfile::Endian::Little);
    if (!root->writeAsRoot(w)) {
      return false;
    }

    std::vector<uint8_t> buffer;
    s->drain(buffer);

    leveldb::Slice v((char const *)buffer.data(), buffer.size());
    db.put(mcfile::be::DbKey::AutonomousEntities(), v);

    return true;
  }

private:
  std::filesystem::path fInput;

public:
  Portals fPortals;
  JavaEditionMap fJavaEditionMap;
  std::unordered_map<int32_t, CompoundTagPtr> fMapItems;
  std::vector<CompoundTagPtr> fAutonomousEntities;
  std::unordered_set<Pos3i, Pos3iHasher> fEndPortalsInEndDimension;
  Options fOptions;
  Structures fStructures;
  Statistics fStat;
  int64_t fMaxChunkLastUpdate = 0;
  int64_t const fGameTick;
  bool fSpectatorUsed = false;

  struct VehicleAndPassengers {
    Pos2i fChunk;
    CompoundTagPtr fVehicle;
    std::vector<CompoundTagPtr> fPassengers;
  };
  struct PlayerAttachedEntities {
    mcfile::Dimension fDim;
    int64_t fLocalPlayerUid;
    std::optional<VehicleAndPassengers> fVehicle;
    std::vector<std::pair<Pos2i, CompoundTagPtr>> fShoulderRiders;
  };
  std::optional<PlayerAttachedEntities> fPlayerAttachedEntities;
};

} // namespace je2be::tobe
