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

  static State Box360ToJava(CompoundTag const &b, CompoundTag &j) {
    using namespace std;
    auto lootTable = b.string("LootTable");
    auto lootTableSeed = b.int64("LootTableSeed");
    if (!lootTable || !lootTableSeed) {
      return State::NoLootTable;
    }
    /*
    minecraft:chests/buriedtreasure         => buried_treasure
    minecraft:chests/shipwreck              => shipwreck_map
    minecraft:chests/shipwrecksupply        => shipwreck_supply
    minecraft:chests/shipwrecktreasure      => shipwreck_treasure
    minecraft:chests/simple_dungeon         => same
    minecraft:chests/stronghold_corridor    => same
    minecraft:chests/stronghold_library     => same
    minecraft:chests/underwater_ruin_big    => same
    minecraft:chests/underwater_ruin_small  => same
    minecraft:chests/village_blacksmith
    */
    static unordered_map<string, string> const mapping = {
        {"minecraft:chests/shipwrecksupply", "minecraft:chests/shipwreck_supply"},
        {"minecraft:chests/shipwrecktreasure", "minecraft:chests/shipwreck_treasure"},
        {"minecraft:chests/shipwreck", "minecraft:chests/shipwreck_map"},
        {"minecraft:chests/buriedtreasure", "minecraft:chests/buried_treasure"},
    };
    string table = *lootTable;
    auto found = mapping.find(table);
    if (found != mapping.end()) {
      table = found->second;
    }
    j["LootTable"] = String(table);
    j["LootTableSeed"] = Long(*lootTableSeed);
    return State::HasLootTable;
  }
};

} // namespace je2be
