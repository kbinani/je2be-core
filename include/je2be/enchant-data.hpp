#pragma once

namespace j2b {

class EnchantData {
  using CompoundTag = mcfile::nbt::CompoundTag;

public:
  static std::shared_ptr<CompoundTag> From(CompoundTag const &item) {
    using namespace props;
    using namespace std;
    auto tag = std::make_shared<CompoundTag>();
    auto name = item.string("id");
    if (!name)
      return nullptr;
    auto lvl = item.int16("lvl");
    if (!lvl)
      return nullptr;
    int16_t id = 0;
    static unordered_map<string, int16_t> const mapping = {
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
    };
    auto found = mapping.find(*name);
    if (found != mapping.end()) {
      id = found->second;
    }
    tag->set("id", props::Short(id));
    tag->set("lvl", props::Short(*lvl));
    return tag;
  }

private:
  EnchantData() = delete;
};

} // namespace j2b
