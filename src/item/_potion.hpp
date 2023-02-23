#pragma once

#include "_reversible-map.hpp"

namespace je2be {

class Potion {
public:
  struct TypeAndItemName {
    std::string fItemName;
    std::string fPotionType;
  };
  static std::optional<TypeAndItemName> JavaPotionTypeAndItemNameFromLegacyJavaDamage(i16 damage) {
    using namespace std;
    string type;
    // 0x2001: 0010000000000001 regeneration 0:45
    // 0x2021: 0010000000100001 regeneration II 0:22
    // 0x2041: 0010000001000001 regeneration 2:00

    // 0x202e: invisibility 3:00
    // 0x204e: invisibility 8:00
    // 0x402e: invisibility (splash) 2:15
    // 0x404e: invisibility (splasy) 6:00

    // 0x2026: night_vision 3:00

    // 0x2028: weakness: 1:30

    // 0x202d: water_breathing: 3:00

    // 0x2045: 0010 0000 0100 0101 healing
    // 0x2025: 0010 0000 0010 0101 strong_healing

    // 0x2023: 0010 0000 0010 0011 fire_resistance 3:00
    // 0x2043: 0010 0000 0100 0011 fire_resistance 8:00

    bool isStrong = (0x20 & damage) == 0x20;
    bool isLong = (0x40 & damage) == 0x40;

    switch (damage & 0x1f) {
    case 0:
      type = "water";
      break;
    case 1:
      type = "regeneration";
      break;
    case 2:
      type = "swiftness";
      break;
    case 3:
      type = "fire_resistance";
      // fire_resistance potion is default "strong" without prefix
      if (isStrong) {
        isStrong = false;
      }
      break;
    case 4:
      type = "poison";
      break;
    case 5:
      type = "healing";
      // healing potion is default "long" without prefix
      if (isLong) {
        isLong = false;
      }
      break;
    case 6:
      type = "night_vision";
      // night_vision potion is default "strong" without prefix
      if (isStrong) {
        isStrong = false;
      }
      break;
    case 8:
      type = "weakness";
      // weakness potion is default "strong" without prefix
      if (isStrong) {
        isStrong = false;
      }
      break;
    case 9:
      type = "strength";
      break;
    case 10:
      type = "slowness";
      break;
    case 12:
      type = "harming";
      // harming potion is default "long" without prefix
      if (isLong) {
        isLong = false;
      }
      break;
    case 13:
      type = "water_breathing";
      // water_breathing potion is default "strong" without prefix
      if (isStrong) {
        isStrong = false;
      }
      break;
    case 14:
      type = "invisibility";
      // invisibility potion is default "strong" without prefix
      if (isStrong) {
        isStrong = false;
      }
      break;
    case 16:
      type = "mundane";
      break;
    default:
      return std::nullopt;
    }

    string prefix;
    if (isLong) {
      prefix = "long_";
    }
    if (isStrong) {
      prefix = "strong_";
    }

    TypeAndItemName ret;
    ret.fPotionType = "minecraft:" + prefix + type;
    if ((0x4000 & damage) == 0x4000) {
      ret.fItemName = "minecraft:splash_potion";
    } else {
      ret.fItemName = "minecraft:potion";
    }
    return ret;
  }

  static ReversibleMap<std::string, i16> const *GetPotionTypeTableJtoB() {
    static std::unique_ptr<ReversibleMap<std::string, i16> const> const sTable(CreatePotionTypeTableJtoB());
    return sTable.get();
  }

  static ReversibleMap<std::string, i16> const *CreatePotionTypeTableJtoB() {
    return new ReversibleMap<std::string, i16>({
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

  static i16 BedrockPotionTypeFromJava(std::string const &name) {
    i16 type = 0;
    auto table = GetPotionTypeTableJtoB();
    auto found = table->forward(name);
    if (found) {
      type = *found;
    }
    return type;
  }

  static std::string JavaPotionTypeFromBedrock(i16 t) {
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
