#pragma once

#include "_reversible-map.hpp"

namespace je2be {

class Enchantments {
  Enchantments() = delete;

public:
  static i16 BedrockEnchantmentIdFromJava(std::u8string const &id) {
    auto const *table = GetTable();
    auto found = table->forward(id);
    if (found) {
      return *found;
    } else {
      return 0;
    }
  }

  static std::u8string JavaEnchantmentIdFromBedrock(i16 id) {
    auto const *table = GetTable();
    auto found = table->backward(id);
    if (found) {
      return *found;
    } else {
      return u8"";
    }
  }

  static std::optional<std::u8string> JavaEnchantmentIdFromLegacyJava(i16 id) {
#define ns u8"minecraft:"
    switch (id) {
    case 0:
      return ns u8"protection";
    case 1:
      return ns u8"fire_protection";
    case 2:
      return ns u8"feather_falling";
    case 3:
      return ns u8"blast_protection";
    case 4:
      return ns u8"projectile_protection";
    case 5:
      return ns u8"respiration";
    case 6:
      return ns u8"aqua_affinity";
    case 7:
      return ns u8"thorns";
    case 16:
      return ns u8"sharpness";
    case 17:
      return ns u8"smite";
    case 18:
      return ns u8"bane_of_arthropods";
    case 19:
      return ns u8"knockback";
    case 20:
      return ns u8"fire_aspect";
    case 21:
      return ns u8"looting";
    case 32:
      return ns u8"efficiency";
    case 33:
      return ns u8"silk_touch";
    case 34:
      return ns u8"unbreaking";
    case 35:
      return ns u8"fortune";
    case 48:
      return ns u8"power";
    case 49:
      return ns u8"punch";
    case 50:
      return ns u8"flame";
    case 51:
      return ns u8"infinity";
    case 61:
      return ns u8"luck_of_the_sea";
    case 62:
      return ns u8"lure";
    default:
      return std::nullopt;
    }
#undef ns
  }

  static std::u8string JavaEnchantmentIdFromBox360(i16 id) {
    switch (id) {
    case 5:
      return u8"minecraft:respiration";
    case 6:
      return u8"minecraft:aqua_affinity";
    case 7:
      return u8"minecraft:thorns";
    case 8:
      return u8"minecraft:depth_strider";
    case 9:
      return u8"minecraft:frost_walker";
    case 10:
      return u8"minecraft:binding_curse";
    case 16:
      return u8"minecraft:sharpness";
    case 17:
      return u8"minecraft:smite";
    case 18:
      return u8"minecraft:bane_of_arthropods";
    case 19:
      return u8"minecraft:knockback";
    case 20:
      return u8"minecraft:fire_aspect";
    case 21:
      return u8"minecraft:looting";
    case 32:
      return u8"minecraft:efficiency";
    case 33:
      return u8"minecraft:silk_touch";
    case 34:
      return u8"minecraft:unbreaking";
    case 35:
      return u8"minecraft:fortune";
    case 48:
      return u8"minecraft:power";
    case 49:
      return u8"minecraft:punch";
    case 50:
      return u8"minecraft:flame";
    case 51:
      return u8"minecraft:infinity";
    case 61:
      return u8"minecraft:luck_of_the_sea";
    case 62:
      return u8"minecraft:lure";
    case 70:
      return u8"minecraft:mending";
    case 71:
      return u8"minecraft:vanishing_curse";
    case 80:
      return u8"minecraft:impaling";
    case 81:
      return u8"minecraft:riptide";
    case 82:
      return u8"minecraft:loyalty";
    case 83:
      return u8"minecraft:channeling";
    }
    return JavaEnchantmentIdFromBedrock(id);
  }

  static ReversibleMap<std::u8string, i16> const *GetTable() {
    static std::unique_ptr<ReversibleMap<std::u8string, i16> const> const sTable(CreateTable());
    return sTable.get();
  }

  static ReversibleMap<std::u8string, i16> const *CreateTable() {
    return new ReversibleMap<std::u8string, i16>({
        {u8"minecraft:protection", 0},
        {u8"minecraft:fire_protection", 1},
        {u8"minecraft:feather_falling", 2},
        {u8"minecraft:blast_protection", 3},
        {u8"minecraft:projectile_protection", 4},
        {u8"minecraft:thorns", 5},
        {u8"minecraft:respiration", 6},
        {u8"minecraft:depth_strider", 7},
        {u8"minecraft:aqua_affinity", 8},
        {u8"minecraft:sharpness", 9},
        {u8"minecraft:smite", 10},
        {u8"minecraft:bane_of_arthropods", 11},
        {u8"minecraft:knockback", 12},
        {u8"minecraft:fire_aspect", 13},
        {u8"minecraft:looting", 14},
        {u8"minecraft:efficiency", 15},
        {u8"minecraft:silk_touch", 16},
        {u8"minecraft:unbreaking", 17},
        {u8"minecraft:fortune", 18},
        {u8"minecraft:power", 19},
        {u8"minecraft:punch", 20},
        {u8"minecraft:flame", 21},
        {u8"minecraft:infinity", 22},
        {u8"minecraft:luck_of_the_sea", 23},
        {u8"minecraft:lure", 24},
        {u8"minecraft:frost_walker", 25},
        {u8"minecraft:mending", 26},
        {u8"minecraft:binding_curse", 27},
        {u8"minecraft:vanishing_curse", 28},
        {u8"minecraft:impaling", 29},
        {u8"minecraft:riptide", 30},
        {u8"minecraft:loyalty", 31},
        {u8"minecraft:channeling", 32},
        {u8"minecraft:multishot", 33},
        {u8"minecraft:piercing", 34},
        {u8"minecraft:quick_charge", 35},
        {u8"minecraft:soul_speed", 36},
        {u8"minecraft:swift_sneak", 37},
        {u8"minecraft:wind_burst", 38},
        {u8"minecraft:density", 39},
        {u8"minecraft:breach", 40},
    });
  }
};

} // namespace je2be
