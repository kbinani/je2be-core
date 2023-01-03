#pragma once

#include <je2be/_reversible-map.hpp>

namespace je2be {

class Potion {
public:
  static ReversibleMap<std::string, int16_t> const *GetPotionTypeTableJtoB() {
    static std::unique_ptr<ReversibleMap<std::string, int16_t> const> const sTable(CreatePotionTypeTableJtoB());
    return sTable.get();
  }

  static ReversibleMap<std::string, int16_t> const *CreatePotionTypeTableJtoB() {
    return new ReversibleMap<std::string, int16_t>({
        {"minecraft:water", 0},
        {"minecraft:mundane", 1},
        {"minecraft:night_vision", 5},
        {"minecraft:long_night_vision", 6},
        {"minecraft:thick", 2},
        {"minecraft:awkward", 3},
        {"minecraft:invisibility", 7},
        {"minecraft:long_invisibility", 8},
        {"minecraft:leaping", 9},
        {"minecraft:long_leaping", 10},
        {"minecraft:strong_leaping", 11},
        {"minecraft:fire_resistance", 12},
        {"minecraft:long_fire_resistance", 13},
        {"minecraft:swiftness", 14},
        {"minecraft:long_swiftness", 15},
        {"minecraft:strong_swiftness", 16},
        {"minecraft:slowness", 17},
        {"minecraft:long_slowness", 18},
        {"minecraft:strong_slowness", 42},
        {"minecraft:water_breathing", 19},
        {"minecraft:long_water_breathing", 20},
        {"minecraft:healing", 21},
        {"minecraft:strong_healing", 22},
        {"minecraft:harming", 23},
        {"minecraft:strong_harming", 24},
        {"minecraft:poison", 25},
        {"minecraft:long_poison", 26},
        {"minecraft:strong_poison", 27},
        {"minecraft:regeneration", 28},
        {"minecraft:long_regeneration", 29},
        {"minecraft:strong_regeneration", 30},
        {"minecraft:strength", 31},
        {"minecraft:long_strength", 32},
        {"minecraft:strong_strength", 33},
        {"minecraft:weakness", 34},
        {"minecraft:long_weakness", 35},
        {"minecraft:turtle_master", 37},
        {"minecraft:long_turtle_master", 38},
        {"minecraft:strong_turtle_master", 39},
        {"minecraft:slow_falling", 40},
        {"minecraft:long_slow_falling", 41},
    });
  }

  static int16_t BedrockPotionTypeFromJava(std::string const &name) {
    int16_t type = 0;
    auto table = GetPotionTypeTableJtoB();
    auto found = table->forward(name);
    if (found) {
      type = *found;
    }
    return type;
  }

  static std::string JavaPotionTypeFromBedrock(int16_t t) {
    std::string name = "minecraft:water";
    auto table = GetPotionTypeTableJtoB();
    auto found = table->backward(t);
    if (found) {
      name = *found;
    }
    return name;
  }

private:
  Potion() = delete;
};

} // namespace je2be
