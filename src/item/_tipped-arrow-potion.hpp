#pragma once

#include "_static-reversible-map.hpp"

namespace je2be {

class TippedArrowPotion {
  TippedArrowPotion() = delete;

  static i32 constexpr kTicksDefault = 22 * 20;
  static i32 constexpr kTicksLong = 60 * 20;
  static i32 constexpr kTicksStrong = 11 * 20;

  static std::map<std::u8string, std::pair<i16, i32>> *CreateTable() {
    return new std::map<std::u8string, std::pair<i16, i32>>({
        // java, {bedrock, duration(java)}
        {u8"minecraft:night_vision", {6, kTicksDefault}},
        {u8"minecraft:long_night_vision", {7, kTicksLong}},
        {u8"minecraft:invisibility", {8, kTicksDefault}},
        {u8"minecraft:long_invisibility", {9, kTicksLong}},
        {u8"minecraft:leaping", {10, kTicksDefault}},
        {u8"minecraft:long_leaping", {11, kTicksLong}},
        {u8"minecraft:strong_leaping", {12, kTicksStrong}},
        {u8"minecraft:fire_resistance", {13, kTicksDefault}},
        {u8"minecraft:long_fire_resistance", {14, kTicksLong}},
        {u8"minecraft:swiftness", {15, kTicksDefault}},
        {u8"minecraft:long_swiftness", {16, kTicksLong}},
        {u8"minecraft:strong_swiftness", {17, kTicksStrong}},
        {u8"minecraft:slowness", {18, 11 * 20}},
        {u8"minecraft:long_slowness", {19, 30 * 20}},
        {u8"minecraft:water_breathing", {20, kTicksDefault}},
        {u8"minecraft:long_water_breathing", {21, kTicksLong}},
        {u8"minecraft:healing", {22, 0}},
        {u8"minecraft:strong_healing", {23, 0}},
        {u8"minecraft:harming", {24, 0}},
        {u8"minecraft:strong_harming", {25, 0}},
        {u8"minecraft:poison", {26, 5 * 20}},
        {u8"minecraft:long_poison", {27, 11 * 20}},
        {u8"minecraft:strong_poison", {28, 2 * 20}},
        {u8"minecraft:regeneration", {29, 5 * 20}},
        {u8"minecraft:long_regeneration", {30, 11 * 20}},
        {u8"minecraft:strong_regeneration", {31, 2 * 20}},
        {u8"minecraft:strength", {32, kTicksDefault}},
        {u8"minecraft:long_strength", {33, kTicksLong}},
        {u8"minecraft:strong_strength", {34, kTicksStrong}},
        {u8"minecraft:weakness", {35, 11 * 20}},
        {u8"minecraft:long_weakness", {36, 30 * 20}},
        {u8"minecraft:wither", {37, -1}}, // NOTE: wither tipped arrow doesn't exist in JE
        {u8"minecraft:turtle_master", {38, 2 * 20}},
        {u8"minecraft:long_turtle_master", {39, 5 * 20}},
        {u8"minecraft:strong_turtle_master", {40, 2 * 20}},
        {u8"minecraft:slow_falling", {41, 11 * 20}},
        {u8"minecraft:long_slow_falling", {42, 30 * 20}},
        {u8"minecraft:strong_slowness", {43, 2 * 20}},
        {u8"minecraft:wind_charged", {44, 22 * 20}},
        {u8"minecraft:weaving", {45, 22 * 20}},
        {u8"minecraft:oozing", {46, 22 * 20}},
        {u8"minecraft:infested", {47, 22 * 20}},
    });
  }

  static std::map<std::u8string, std::pair<i16, i32>> const &Table() {
    static std::unique_ptr<std::map<std::u8string, std::pair<i16, i32>> const> sTable(CreateTable());
    return *sTable;
  }

public:
  static i16 BedrockPotionType(std::u8string const &java) {
    auto const &table = Table();
    if (auto found = table.find(java); found != table.end()) {
      return found->second.first;
    }
    return 6;
  }

  static std::u8string JavaPotionType(i16 bedrock) {
    for (auto const &it : Table()) {
      if (it.second.first == bedrock) {
        return it.first;
      }
    }
    return u8"minecraft:night_vision";
  }

  static i32 JavaPotionDuration(std::u8string const &java) {
    auto const &table = Table();
    if (auto found = table.find(java); found != table.end()) {
      return found->second.second;
    }
    return 0;
  }
};

} // namespace je2be
