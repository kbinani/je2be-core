#pragma once

#include "_props.hpp"
#include "_reversible-map.hpp"

namespace je2be {

class LootTable {
  LootTable() = delete;

public:
  enum class State {
    HasLootTable,
    NoLootTable,
  };

  static State JavaToBedrock(CompoundTag const &j, CompoundTag &b) {
    auto lootTable = j.string(u8"LootTable"); // "minecraft:chests/simple_dungeon"
    auto lootTableSeed = j.int64(u8"LootTableSeed");
    if (lootTable && lootTableSeed) {
      auto tableName = BedrockTableNameFromJava(*lootTable);
      if (tableName) {
        b[u8"LootTable"] = String(*tableName);
        b[u8"LootTableSeed"] = Int(props::SquashI64ToI32(*lootTableSeed));
        return State::HasLootTable;
      }
    }
    return State::NoLootTable;
  }

  static std::optional<std::u8string> BedrockTableNameFromJava(std::u8string const &java) {
    auto const &exceptional = GetJavaToBedrockTable();
    auto found = exceptional.forward(java);
    if (found) {
      return *found;
    }
    auto slash = java.find_first_of(u8'/');
    if (java.starts_with(u8"minecraft:") && slash != std::u8string::npos) {
      return u8"loot_tables/" + java.substr(10) + u8".json"; // "loot_tables/chests/simple_dungeon.json"
    } else {
      return std::nullopt;
    }
  }

  static std::optional<std::u8string> JavaTableNameFromBedrock(std::u8string const &bedrock) {
    auto const &exceptional = GetJavaToBedrockTable();
    auto found = exceptional.backward(bedrock);
    if (found) {
      return *found;
    }
    if (bedrock.starts_with(u8"loot_tables/") && bedrock.ends_with(u8".json")) {
      std::u8string name = strings::RemoveSuffix(bedrock.substr(12), u8".json");
      return u8"minecraft:" + name;
    }
    return std::nullopt;
  }

  static State BedrockToJava(CompoundTag const &b, CompoundTag &j) {
    auto lootTable = b.string(u8"LootTable");
    auto lootTableSeed = b.int32(u8"LootTableSeed");
    if (lootTable && lootTableSeed) {
      auto tableName = JavaTableNameFromBedrock(*lootTable);
      if (tableName) {
        j[u8"LootTable"] = String(*tableName);
        j[u8"LootTableSeed"] = Long(*lootTableSeed);
        return State::HasLootTable;
      }
    }
    return State::NoLootTable;
  }

  static State Box360ToJava(CompoundTag const &b, CompoundTag &j) {
    using namespace std;
    auto lootTable = b.string(u8"LootTable");
    auto lootTableSeed = b.int64(u8"LootTableSeed");
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
    static unordered_map<u8string, u8string> const mapping = {
        {u8"minecraft:chests/shipwrecksupply", u8"minecraft:chests/shipwreck_supply"},
        {u8"minecraft:chests/shipwrecktreasure", u8"minecraft:chests/shipwreck_treasure"},
        {u8"minecraft:chests/shipwreck", u8"minecraft:chests/shipwreck_map"},
        {u8"minecraft:chests/buriedtreasure", u8"minecraft:chests/buried_treasure"},
        {u8"minecraft:chests/village_blacksmith", u8"minecraft:chests/village/village_toolsmith"},
    };
    u8string table = *lootTable;
    auto found = mapping.find(table);
    if (found != mapping.end()) {
      table = found->second;
    }
    j[u8"LootTable"] = String(table);
    j[u8"LootTableSeed"] = Long(*lootTableSeed);
    return State::HasLootTable;
  }

private:
  static ReversibleMap<std::u8string, std::u8string> *CreateJavaToBedrockTable() {
    return new ReversibleMap<std::u8string, std::u8string>({
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

        // Java(1.19.4 Pre Release 2)          Bedrock(Preview 1.19.70.24)
        "minecraft:archaeology/desert_pyramid" "loot_tables/entities/desert_pyramid_suspicious_sand.json"

        // Java(1.21)                                          Bedrock(1.21)
        "minecraft:chests/trial_chambers/corridor"             "loot_tables/chests/trial_chambers/corridor.json"
        "minecraft:chests/trial_chambers/entrance"             "loot_tables/chests/trial_chambers/entrance.json"
        "minecraft:chests/trial_chambers/intersection_barrel"  "loot_tables/chests/trial_chambers/intersection_barrel.json"
        "minecraft:chests/trial_chambers/reward"               "loot_tables/chests/trial_chambers/reward.json"
        "minecraft:dispensers/trial_chambers/chamber"          "loot_tables/dispensers/trial_chambers/chamber.json"
        "minecraft:pots/trial_chambers/corridor"               "loot_tables/pots/trial_chambers/corridor.json"
        */

        {u8"minecraft:chests/buried_treasure", u8"loot_tables/chests/buriedtreasure.json"},
        {u8"minecraft:chests/jungle_temple_dispenser", u8"loot_tables/chests/dispenser_trap.json"},
        {u8"minecraft:chests/jungle_temple", u8"loot_tables/chests/jungle_temple.json"},
        {u8"minecraft:chests/shipwreck_map", u8"loot_tables/chests/shipwreck.json"},
        {u8"minecraft:chests/shipwreck_supply", u8"loot_tables/chests/shipwrecksupply.json"},
        {u8"minecraft:chests/shipwreck_treasure", u8"loot_tables/chests/shipwrecktreasure.json"},
        {u8"minecraft:archaeology/desert_pyramid", u8"loot_tables/entities/desert_pyramid_brushable_block.json"},
        {u8"minecraft:archaeology/desert_well", u8"loot_tables/entities/desert_well_brushable_block.json"},
        {u8"minecraft:archaeology/ocean_ruin_cold", u8"loot_tables/entities/cold_ocean_ruins_brushable_block.json"},
        {u8"minecraft:archaeology/ocean_ruin_warm", u8"loot_tables/entities/warm_ocean_ruins_brushable_block.json"},
        {u8"minecraft:archaeology/trail_ruins_common", u8"loot_tables/entities/trail_ruins_brushable_block_common.json"},
        {u8"minecraft:archaeology/trail_ruins_rare", u8"loot_tables/entities/trail_ruins_brushable_block_rare.json"},
    });
  }

  static ReversibleMap<std::u8string, std::u8string> const &GetJavaToBedrockTable() {
    using namespace std;
    static unique_ptr<ReversibleMap<u8string, u8string> const> sTable(CreateJavaToBedrockTable());
    return *sTable.get();
  }
};

} // namespace je2be
