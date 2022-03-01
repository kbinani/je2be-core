#pragma once

namespace je2be {

class LootTable {
  LootTable() = delete;

public:
  enum class State {
    HasLootTable,
    NoLootTable,
  };

  static State JavaToBedrock(CompoundTag const &j, CompoundTag &b) {
    auto lootTable = j.string("LootTable"); // "minecraft:chests/simple_dungeon"
    auto lootTableSeed = j.int64("LootTableSeed");
    if (lootTable && lootTableSeed) {
      auto slash = lootTable->find('/');
      if (lootTable->starts_with("minecraft:") && slash != std::string::npos) {
        auto type = lootTable->substr(0, slash).substr(10);                             // "chests"
        std::string table = "loot_tables/" + type + lootTable->substr(slash) + ".json"; // "loot_tables/chests/simple_dungeon.json"
        b["LootTable"] = String(table);
        b["LootTableSeed"] = Int(props::SquashI64ToI32(*lootTableSeed));
        return State::HasLootTable;
      }
    }
    return State::NoLootTable;
  }

  static State BedrockToJava(CompoundTag const &b, CompoundTag &j) {
    auto lootTable = b.string("LootTable");
    auto lootTableSeed = b.int32("LootTableSeed");
    if (lootTable && lootTable->starts_with("loot_tables/") && lootTable->ends_with(".json") && lootTableSeed) {
      std::string name = strings::RTrim(lootTable->substr(12), ".json");
      j["LootTable"] = String("minecraft:" + name);
      j["LootTableSeed"] = Long(*lootTableSeed);
      return State::HasLootTable;
    }
    return State::NoLootTable;
  }
};

} // namespace je2be