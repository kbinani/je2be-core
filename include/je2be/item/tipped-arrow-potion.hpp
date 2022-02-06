#pragma once

namespace je2be {

class TippedArrowPotion : StaticReversibleMap<std::string, int16_t, TippedArrowPotion> {
  TippedArrowPotion() = delete;

public:
  static ReversibleMap<std::string, int16_t> const *CreateTable() {
    return new ReversibleMap<std::string, int16_t>({
        {"minecraft:night_vision", 6},
        {"minecraft:long_night_vision", 7},
        {"minecraft:invisibility", 8},
        {"minecraft:long_invisibility", 9},
        {"minecraft:leaping", 10},
        {"minecraft:long_leaping", 11},
        {"minecraft:strong_leaping", 12},
        {"minecraft:fire_resistance", 13},
        {"minecraft:long_fire_resistance", 14},
        {"minecraft:swiftness", 15},
        {"minecraft:long_swiftness", 16},
        {"minecraft:strong_swiftness", 17},
        {"minecraft:slowness", 18},
        {"minecraft:long_slowness", 19},
        {"minecraft:water_breathing", 20},
        {"minecraft:long_water_breathing", 21},
        {"minecraft:healing", 22},
        {"minecraft:strong_healing", 23},
        {"minecraft:harming", 24},
        {"minecraft:strong_harming", 25},
        {"minecraft:poison", 26},
        {"minecraft:long_poison", 27},
        {"minecraft:strong_poison", 28},
        {"minecraft:regeneration", 29},
        {"minecraft:long_regeneration", 30},
        {"minecraft:strong_regeneration", 31},
        {"minecraft:strength", 32},
        {"minecraft:long_strength", 33},
        {"minecraft:strong_strength", 34},
        {"minecraft:weakness", 35},
        {"minecraft:long_weakness", 36},
        {"minecraft:turtle_master", 38},
        {"minecraft:long_turtle_master", 39},
        {"minecraft:strong_turtle_master", 40},
        {"minecraft:slow_falling", 41},
        {"minecraft:long_slow_falling", 42},
        {"minecraft:strong_slowness", 43},
    });
  }

  static int16_t BedrockPotionType(std::string const &java) {
    return Forward(java, 6);
  }

  static std::string JavaPotionType(int16_t bedrock) {
    return Backward(bedrock, "minecraft:night_vision");
  }
};

} // namespace je2be
