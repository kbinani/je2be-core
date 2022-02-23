#pragma once

namespace je2be::tobe {

class LevelData {
public:
  LevelData(std::filesystem::path const &input, Options const &opt) : fInput(input), fJavaEditionMap(input, opt), fOptions(opt) {}

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

    auto list = std::make_shared<ListTag>(Tag::Type::Compound);
    for (auto const &e : fAutonomousEntities) {
      list->push_back(e);
    }
    auto root = std::make_shared<CompoundTag>();
    root->set("AutonomousEntityList", list);

    auto s = std::make_shared<ByteStream>();
    OutputStreamWriter w(s, {.fLittleEndian = true});
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
  std::unordered_map<int32_t, std::shared_ptr<CompoundTag>> fMapItems;
  std::vector<std::shared_ptr<CompoundTag>> fAutonomousEntities;
  std::unordered_set<Pos3i, Pos3iHasher> fEndPortalsInEndDimension;
  Options fOptions;
  Structures fStructures;
  Statistics fStat;
  int64_t fMaxChunkLastUpdate = 0;

  struct RootVehicle {
    mcfile::Dimension fDim;
    Pos2i fChunk;
    int64_t fLocalPlayerUid;
    std::shared_ptr<CompoundTag> fVehicle;
  };
  std::optional<RootVehicle> fRootVehicle;
};

} // namespace je2be::tobe
