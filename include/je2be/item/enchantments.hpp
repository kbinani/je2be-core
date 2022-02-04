#pragma once

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
      return 0;
    }
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
    });
  }
};

} // namespace je2be
