#pragma once

#include "_reversible-map.hpp"

namespace je2be {

class Potion {
public:
  struct TypeAndItemName {
    std::u8string fItemName;
    std::u8string fPotionType;
  };
  static std::optional<TypeAndItemName> JavaPotionTypeAndItemNameFromLegacyJavaDamage(i16 damage) {
    using namespace std;
    u8string type;
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
      type = u8"water";
      break;
    case 1:
      type = u8"regeneration";
      break;
    case 2:
      type = u8"swiftness";
      break;
    case 3:
      type = u8"fire_resistance";
      // fire_resistance potion is default "strong" without prefix
      if (isStrong) {
        isStrong = false;
      }
      break;
    case 4:
      type = u8"poison";
      break;
    case 5:
      type = u8"healing";
      // healing potion is default "long" without prefix
      if (isLong) {
        isLong = false;
      }
      break;
    case 6:
      type = u8"night_vision";
      // night_vision potion is default "strong" without prefix
      if (isStrong) {
        isStrong = false;
      }
      break;
    case 8:
      type = u8"weakness";
      // weakness potion is default "strong" without prefix
      if (isStrong) {
        isStrong = false;
      }
      break;
    case 9:
      type = u8"strength";
      break;
    case 10:
      type = u8"slowness";
      break;
    case 12:
      type = u8"harming";
      // harming potion is default "long" without prefix
      if (isLong) {
        isLong = false;
      }
      break;
    case 13:
      type = u8"water_breathing";
      // water_breathing potion is default "strong" without prefix
      if (isStrong) {
        isStrong = false;
      }
      break;
    case 14:
      type = u8"invisibility";
      // invisibility potion is default "strong" without prefix
      if (isStrong) {
        isStrong = false;
      }
      break;
    case 16:
      type = u8"mundane";
      break;
    default:
      return std::nullopt;
    }

    u8string prefix;
    if (isLong) {
      prefix = u8"long_";
    }
    if (isStrong) {
      prefix = u8"strong_";
    }

    TypeAndItemName ret;
    ret.fPotionType = u8"minecraft:" + prefix + type;
    if ((0x4000 & damage) == 0x4000) {
      ret.fItemName = u8"minecraft:splash_potion";
    } else {
      ret.fItemName = u8"minecraft:potion";
    }
    return ret;
  }

  static ReversibleMap<std::u8string, i16> const *GetPotionTypeTableJtoB() {
    static std::unique_ptr<ReversibleMap<std::u8string, i16> const> const sTable(CreatePotionTypeTableJtoB());
    return sTable.get();
  }

  static ReversibleMap<std::u8string, i16> const *CreatePotionTypeTableJtoB() {
    return new ReversibleMap<std::u8string, i16>({
        {u8"minecraft:water", 0},
        {u8"minecraft:mundane", 1},
        {u8"minecraft:thick", 3},
        {u8"minecraft:awkward", 4},
        {u8"minecraft:night_vision", 5},
        {u8"minecraft:long_night_vision", 6},
        {u8"minecraft:invisibility", 7},
        {u8"minecraft:long_invisibility", 8},
        {u8"minecraft:leaping", 9},
        {u8"minecraft:long_leaping", 10},
        {u8"minecraft:strong_leaping", 11},
        {u8"minecraft:fire_resistance", 12},
        {u8"minecraft:long_fire_resistance", 13},
        {u8"minecraft:swiftness", 14},
        {u8"minecraft:long_swiftness", 15},
        {u8"minecraft:strong_swiftness", 16},
        {u8"minecraft:slowness", 17},
        {u8"minecraft:long_slowness", 18},
        {u8"minecraft:water_breathing", 19},
        {u8"minecraft:long_water_breathing", 20},
        {u8"minecraft:healing", 21},
        {u8"minecraft:strong_healing", 22},
        {u8"minecraft:harming", 23},
        {u8"minecraft:strong_harming", 24},
        {u8"minecraft:poison", 25},
        {u8"minecraft:long_poison", 26},
        {u8"minecraft:strong_poison", 27},
        {u8"minecraft:regeneration", 28},
        {u8"minecraft:long_regeneration", 29},
        {u8"minecraft:strong_regeneration", 30},
        {u8"minecraft:strength", 31},
        {u8"minecraft:long_strength", 32},
        {u8"minecraft:strong_strength", 33},
        {u8"minecraft:weakness", 34},
        {u8"minecraft:long_weakness", 35},
        {u8"minecraft:wither", 36}, // NOTE: wither potion doesn't exist in JE
        {u8"minecraft:turtle_master", 37},
        {u8"minecraft:long_turtle_master", 38},
        {u8"minecraft:strong_turtle_master", 39},
        {u8"minecraft:slow_falling", 40},
        {u8"minecraft:long_slow_falling", 41},
        {u8"minecraft:strong_slowness", 42},
    });
  }

  static i16 BedrockPotionTypeFromJava(std::u8string const &name) {
    i16 type = 0;
    auto table = GetPotionTypeTableJtoB();
    auto found = table->forward(name);
    if (found) {
      type = *found;
    }
    return type;
  }

  static std::u8string JavaPotionTypeFromBedrock(i16 t) {
    std::u8string name = u8"minecraft:water";
    auto table = GetPotionTypeTableJtoB();
    auto found = table->backward(t);
    if (found) {
      name = *found;
    } else if (t == 2) {
      name = u8"minecraft:awkward";
    }
    return name;
  }

private:
  Potion() = delete;
};

} // namespace je2be
