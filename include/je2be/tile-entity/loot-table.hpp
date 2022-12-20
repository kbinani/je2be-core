#pragma once

#include <je2be/props.hpp>
#include <je2be/reversible-map.hpp>

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
      auto tableName = BedrockTableNameFromJava(*lootTable);
      if (tableName) {
        b["LootTable"] = String(*tableName);
        b["LootTableSeed"] = Int(props::SquashI64ToI32(*lootTableSeed));
        return State::HasLootTable;
      }
    }
    return State::NoLootTable;
  }

  static std::optional<std::string> BedrockTableNameFromJava(std::string const &java) {
    auto const &exceptional = GetJavaToBedrockTable();
    auto found = exceptional.forward(java);
    if (found) {
      return *found;
    }
    auto slash = java.find_first_of('/');
    if (java.starts_with("minecraft:") && slash != std::string::npos) {
      return "loot_tables/" + java.substr(10) + ".json"; // "loot_tables/chests/simple_dungeon.json"
    } else {
      return std::nullopt;
    }
  }

  static std::optional<std::string> JavaTableNameFromBedrock(std::string const &bedrock) {
    auto const &exceptional = GetJavaToBedrockTable();
    auto found = exceptional.backward(bedrock);
    if (found) {
      return *found;
    }
    if (bedrock.starts_with("loot_tables/") && bedrock.ends_with(".json")) {
      std::string name = strings::RTrim(bedrock.substr(12), ".json");
      return "minecraft:" + name;
    }
    return std::nullopt;
  }

  static State BedrockToJava(CompoundTag const &b, CompoundTag &j) {
    auto lootTable = b.string("LootTable");
    auto lootTableSeed = b.int32("LootTableSeed");
    if (lootTable && lootTableSeed) {
      auto tableName = JavaTableNameFromBedrock(*lootTable);
      if (tableName) {
        j["LootTable"] = String(*tableName);
        j["LootTableSeed"] = Long(*lootTableSeed);
        return State::HasLootTable;
      }
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
    minecraft:chests/village_blacksmith     => village/village_toolsmith ?
    minecraft:chests/desert_pyramid         => same
    minecraft:chests/jungle_temple          => same
    minecraft:chests/stronghold_crossing    => same
    minecraft:chests/woodland_mansion       => same
    minecraft:chests/igloo_chest            => same
    minecraft:chests/abandoned_mineshaft    => same
    */
    static unordered_map<string, string> const mapping = {
        {"minecraft:chests/shipwrecksupply", "minecraft:chests/shipwreck_supply"},
        {"minecraft:chests/shipwrecktreasure", "minecraft:chests/shipwreck_treasure"},
        {"minecraft:chests/shipwreck", "minecraft:chests/shipwreck_map"},
        {"minecraft:chests/buriedtreasure", "minecraft:chests/buried_treasure"},
        {"minecraft:chests/village_blacksmith", "minecraft:chests/village/village_toolsmith"},
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

private:
  static ReversibleMap<std::string, std::string> *CreateJavaToBedrockTable() {
    return new ReversibleMap<std::string, std::string>({
        /* Java(1.19.2)                                   Bedrock(1.19.41)
        "minecraft:chests/abandoned_mineshaft"            "loot_tables/chests/abandoned_mineshaft.json"
        "minecraft:chests/ancient_city"                   "loot_tables/chests/ancient_city.json"
        "minecraft:chests/ancient_city_ice_box"           "loot_tables/chests/ancient_city_ice_box.json"
        "minecraft:chests/bastion_bridge"                 "loot_tables/chests/bastion_bridge.json"
        "minecraft:chests/bastion_hoglin_stable"          "loot_tables/chests/bastion_hoglin_stable.json"
        "minecraft:chests/bastion_other"                  "loot_tables/chests/bastion_other.json"
        "minecraft:chests/bastion_treasure"               "loot_tables/chests/bastion_treasure.json"
        "minecraft:chests/buried_treasure"                "loot_tables/chests/buriedtreasure.json"
        "minecraft:chests/desert_pyramid"                 "loot_tables/chests/desert_pyramid.json"
        "minecraft:chests/jungle_temple_dispenser"        "loot_tables/chests/dispenser_trap.json"
        "minecraft:chests/end_city_treasure"              "loot_tables/chests/end_city_treasure"
        "minecraft:chests/igloo_chest"                    "loot_tables/chests/igloo_chest.json"
        "minecraft:chests/jungle_temple"                  "loot_tables/chests/jungle_temple.json"
        "minecraft:chests/nether_bridge"                  "loot_tables/chests/nether_bridge.json"
        "minecraft:chests/pillager_outpost"               "loot_tables/chests/pillager_outpost.json"
        "minecraft:chests/ruined_portal"                  "loot_tables/chests/ruined_portal.json"
        "minecraft:chests/shipwreck_map"                  "loot_tables/chests/shipwreck.json"
        "minecraft:chests/shipwreck_supply"               "loot_tables/chests/shipwrecksupply.json"
        "minecraft:chests/shipwreck_treasure"             "loot_tables/chests/shipwrecktreasure.json"
        "minecraft:chests/simple_dungeon"                 "loot_tables/chests/simple_dungeon.json"
        "minecraft:chests/spawn_bonus_chest"              "loot_tables/chests/spawn_bonus_chest.json"
        "minecraft:chests/stronghold_corridor"            "loot_tables/chests/stronghold_corridor.json"
        "minecraft:chests/stronghold_crossing"            "loot_tables/chests/stronghold_crossing.json"
        "minecraft:chests/stronghold_library"             "loot_tables/chests/stronghold_library.json"
        "minecraft:chests/underwater_ruin_big"            "loot_tables/chests/underwater_ruin_big.json"
        "minecraft:chests/underwater_ruin_small"          "loot_tables/chests/underwater_ruin_small.json"
        "minecraft:chests/village/village_armorer"        "loot_tables/chests/village/village_armorer.json"
        "minecraft:chests/village/village_butcher"        "loot_tables/chests/village/village_butcher.json"
        "minecraft:chests/village/village_cartographer"   "loot_tables/chests/village/village_cartographer.json"
        "minecraft:chests/village/village_desert_house"   "loot_tables/chests/village/village_desert_house.json"
        "minecraft:chests/village/village_fisher"         ?
        "minecraft:chests/village/village_fletcher"       "loot_tables/chests/village/village_fletcher.json"
        "minecraft:chests/village/village_mason"          "loot_tables/chests/village/village_mason.json"
        "minecraft:chests/village/village_plains_house"   "loot_tables/chests/village/village_plains_house.json"
        "minecraft:chests/village/village_savanna_house"  "loot_tables/chests/village/village_savanna_house.json"
        "minecraft:chests/village/village_shepherd"       "loot_tables/chests/village/village_shepherd.json"
        "minecraft:chests/village/village_snowy_house"    "loot_tables/chests/village/village_snowy_house.json"
        "minecraft:chests/village/village_taiga_house"    "loot_tables/chests/village/village_taiga_house.json"
        "minecraft:chests/village/village_tannery"        "loot_tables/chests/village/village_tannery.json"
        "minecraft:chests/village/village_temple"         "loot_tables/chests/village/village_temple.json"
        "minecraft:chests/village/village_toolsmith"      "loot_tables/chests/village/village_toolsmith.json"
        "minecraft:chests/village/village_weaponsmith"    "loot_tables/chests/village/village_weaponsmith.json"
        "minecraft:chests/woodland_mansion"               "loot_tables/chests/woodland_mansion.json"
        */

        {"minecraft:chests/buried_treasure", "loot_tables/chests/buriedtreasure.json"},
        {"minecraft:chests/jungle_temple_dispenser", "loot_tables/chests/dispenser_trap.json"},
        {"minecraft:chests/shipwreck_map", "loot_tables/chests/shipwreck.json"},
        {"minecraft:chests/shipwreck_supply", "loot_tables/chests/shipwrecksupply.json"},
        {"minecraft:chests/shipwreck_treasure", "loot_tables/chests/shipwrecktreasure.json"},
    });
  }

  static ReversibleMap<std::string, std::string> const &GetJavaToBedrockTable() {
    using namespace std;
    static unique_ptr<ReversibleMap<string, string> const> sTable(CreateJavaToBedrockTable());
    return *sTable.get();
  }
};

} // namespace je2be
