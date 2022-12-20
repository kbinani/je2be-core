#pragma once

#include <je2be/reversible-map.hpp>

namespace je2be {

class Enchantments {
  Enchantments() = delete;

public:
  static int16_t BedrockEnchantmentIdFromJava(std::string const &id) {
    auto const *table = GetTable();
    auto found = table->forward(id);
    if (found) {
      return *found;
    } else {
      return 0;
    }
  }

  static std::string JavaEnchantmentIdFromBedrock(int16_t id) {
    auto const *table = GetTable();
    auto found = table->backward(id);
    if (found) {
      return *found;
    } else {
      return "";
    }
  }

  static std::string JavaEnchantmentIdFromBox360(int16_t id) {
    switch (id) {
    case 5:
      return "minecraft:respiration";
    case 6:
      return "minecraft:aqua_affinity";
    case 7:
      return "minecraft:thorns";
    case 8:
      return "minecraft:depth_strider";
    case 9:
      return "minecraft:frost_walker";
    case 10:
      return "minecraft:binding_curse";
    case 16:
      return "minecraft:sharpness";
    case 17:
      return "minecraft:smite";
    case 18:
      return "minecraft:bane_of_arthropods";
    case 19:
      return "minecraft:knockback";
    case 20:
      return "minecraft:fire_aspect";
    case 21:
      return "minecraft:looting";
    case 32:
      return "minecraft:efficiency";
    case 33:
      return "minecraft:silk_touch";
    case 34:
      return "minecraft:unbreaking";
    case 35:
      return "minecraft:fortune";
    case 48:
      return "minecraft:power";
    case 49:
      return "minecraft:punch";
    case 50:
      return "minecraft:flame";
    case 51:
      return "minecraft:infinity";
    case 61:
      return "minecraft:luck_of_the_sea";
    case 62:
      return "minecraft:lure";
    case 70:
      return "minecraft:mending";
    case 71:
      return "minecraft:vanishing_curse";
    case 80:
      return "minecraft:impaling";
    case 81:
      return "minecraft:riptide";
    case 82:
      return "minecraft:loyalty";
    case 83:
      return "minecraft:channeling";
    }
    return JavaEnchantmentIdFromBedrock(id);
  }

  static ReversibleMap<std::string, int16_t> const *GetTable() {
    static std::unique_ptr<ReversibleMap<std::string, int16_t> const> const sTable(CreateTable());
    return sTable.get();
  }

  static ReversibleMap<std::string, int16_t> const *CreateTable() {
    return new ReversibleMap<std::string, int16_t>({
        {"minecraft:protection", 0},
        {"minecraft:fire_protection", 1},
        {"minecraft:feather_falling", 2},
        {"minecraft:blast_protection", 3},
        {"minecraft:projectile_protection", 4},
        {"minecraft:thorns", 5},
        {"minecraft:respiration", 6},
        {"minecraft:depth_strider", 7},
        {"minecraft:aqua_affinity", 8},
        {"minecraft:sharpness", 9},
        {"minecraft:smite", 10},
        {"minecraft:bane_of_arthropods", 11},
        {"minecraft:knockback", 12},
        {"minecraft:fire_aspect", 13},
        {"minecraft:looting", 14},
        {"minecraft:efficiency", 15},
        {"minecraft:silk_touch", 16},
        {"minecraft:unbreaking", 17},
        {"minecraft:fortune", 18},
        {"minecraft:power", 19},
        {"minecraft:punch", 20},
        {"minecraft:flame", 21},
        {"minecraft:infinity", 22},
        {"minecraft:luck_of_the_sea", 23},
        {"minecraft:lure", 24},
        {"minecraft:frost_walker", 25},
        {"minecraft:mending", 26},
        {"minecraft:binding_curse", 27},
        {"minecraft:vanishing_curse", 28},
        {"minecraft:impaling", 29},
        {"minecraft:riptide", 30},
        {"minecraft:loyalty", 31},
        {"minecraft:channeling", 32},
        {"minecraft:multishot", 33},
        {"minecraft:piercing", 34},
        {"minecraft:quick_charge", 35},
        {"minecraft:soul_speed", 36},
        {"minecraft:swift_sneak", 37},
    });
  }
};

} // namespace je2be
