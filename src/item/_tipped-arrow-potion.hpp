#pragma once

#include "_static-reversible-map.hpp"

namespace je2be {

class TippedArrowPotion : StaticReversibleMap<std::u8string, i16, TippedArrowPotion> {
  TippedArrowPotion() = delete;

public:
  static ReversibleMap<std::u8string, i16> const *CreateTable() {
    return new ReversibleMap<std::u8string, i16>({
        {u8"minecraft:night_vision", 6},
        {u8"minecraft:long_night_vision", 7},
        {u8"minecraft:invisibility", 8},
        {u8"minecraft:long_invisibility", 9},
        {u8"minecraft:leaping", 10},
        {u8"minecraft:long_leaping", 11},
        {u8"minecraft:strong_leaping", 12},
        {u8"minecraft:fire_resistance", 13},
        {u8"minecraft:long_fire_resistance", 14},
        {u8"minecraft:swiftness", 15},
        {u8"minecraft:long_swiftness", 16},
        {u8"minecraft:strong_swiftness", 17},
        {u8"minecraft:slowness", 18},
        {u8"minecraft:long_slowness", 19},
        {u8"minecraft:water_breathing", 20},
        {u8"minecraft:long_water_breathing", 21},
        {u8"minecraft:healing", 22},
        {u8"minecraft:strong_healing", 23},
        {u8"minecraft:harming", 24},
        {u8"minecraft:strong_harming", 25},
        {u8"minecraft:poison", 26},
        {u8"minecraft:long_poison", 27},
        {u8"minecraft:strong_poison", 28},
        {u8"minecraft:regeneration", 29},
        {u8"minecraft:long_regeneration", 30},
        {u8"minecraft:strong_regeneration", 31},
        {u8"minecraft:strength", 32},
        {u8"minecraft:long_strength", 33},
        {u8"minecraft:strong_strength", 34},
        {u8"minecraft:weakness", 35},
        {u8"minecraft:long_weakness", 36},
        {u8"minecraft:wither", 37}, // NOTE: wither tipped arrow doesn't exist in JE
        {u8"minecraft:turtle_master", 38},
        {u8"minecraft:long_turtle_master", 39},
        {u8"minecraft:strong_turtle_master", 40},
        {u8"minecraft:slow_falling", 41},
        {u8"minecraft:long_slow_falling", 42},
        {u8"minecraft:strong_slowness", 43},
    });
  }

  static i16 BedrockPotionType(std::u8string const &java) {
    return Forward(java, 6);
  }

  static std::u8string JavaPotionType(i16 bedrock) {
    return Backward(bedrock, u8"minecraft:night_vision");
  }
};

} // namespace je2be
