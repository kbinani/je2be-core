#pragma once

namespace je2be::box360 {

class Item {
  Item() = delete;

  using Converter = std::function<std::string(CompoundTag const &in, CompoundTagPtr &out, int16_t damage)>;

public:
  static CompoundTagPtr Convert(CompoundTag const &in) {
    using namespace std;
    auto rawId = in.string("id");
    if (!rawId) {
      return nullptr;
    }
    if (!rawId->starts_with("minecraft:")) {
      return nullptr;
    }
    auto id = rawId->substr(10);
    auto const &table = GetTable();
    auto found = table.find(id);
    if (found == table.end()) {
      return nullptr;
    }
    auto damage = in.int16("Damage", 0);

    auto out = Compound();
    CopyByteValues(in, *out, {{"Count"}, {"Slot"}});

    auto changedId = found->second(in, out, damage);
    if (out) {
      if (changedId.empty()) {
        out->set("id", String(*rawId));
      } else {
        out->set("id", String("minecraft:" + changedId));
      }
      return out;
    } else {
      return nullptr;
    }
  }

private:
#pragma region Converter
  static std::string Dirt(CompoundTag const &in, CompoundTagPtr &out, int16_t damage) {
    switch (damage) {
    case 1:
      return "coarse_dirt";
    case 2:
      return "podzol";
    case 0:
    default:
      return "dirt";
    }
  }

  static std::string Log(CompoundTag const &in, CompoundTagPtr &out, int16_t damage) {
    switch (damage) {
    case 1:
      return "spruce_log";
    case 2:
      return "birch_log";
    case 3:
      return "jungle_log";
    case 0:
    default:
      return "oak_log";
    }
  }

  static std::string Log2(CompoundTag const &in, CompoundTagPtr &out, int16_t damage) {
    switch (damage) {
    case 1:
      return "dark_oak_log";
    case 0:
    default:
      return "acacia_log";
    }
  }

  static std::string MonsterEgg(CompoundTag const &in, CompoundTagPtr &out, int16_t damage) {
    switch (damage) {
    case 1:
      return "infested_cobblestone";
    case 2:
      return "infested_stone_bricks";
    case 3:
      return "infested_mossy_stone_bricks";
    case 4:
      return "infested_cracked_stone_bricks";
    case 5:
      return "infested_chiseled_stone_bricks";
    case 0:
    default:
      return "infested_stone";
    }
  }

  static std::string Planks(CompoundTag const &in, CompoundTagPtr &out, int16_t damage) {
    switch (damage) {
    case 1:
      return "spruce_planks";
    case 2:
      return "birch_planks";
    case 3:
      return "jungle_planks";
    case 4:
      return "acacia_planks";
    case 5:
      return "dark_oak_planks";
    case 0:
    default:
      return "oak_planks";
    }
  }

  static std::string Prismarine(CompoundTag const &in, CompoundTagPtr &out, int16_t damage) {
    switch (damage) {
    case 1:
      return "prismarine_bricks";
    case 2:
      return "dark_prismarine";
    case 0:
    default:
      return "prismarine";
    }
  }

  static std::string PrismarineSlab(CompoundTag const &in, CompoundTagPtr &out, int16_t damage) {
    switch (damage) {
    case 1:
      return "prismarine_brick_slab";
    case 2:
      return "dark_prismarine_slab";
    case 0:
    default:
      return "prismarine_slab";
    }
  }

  static std::string QuartzBlock(CompoundTag const &in, CompoundTagPtr &out, int16_t damage) {
    switch (damage) {
    case 1:
      return "chiseled_quartz_block";
    case 2:
      return "quartz_pillar";
    case 0:
      return "quartz_block";
    }
  }

  static std::string RedSandstone(CompoundTag const &in, CompoundTagPtr &out, int16_t damage) {
    switch (damage) {
    case 1:
      return "chiseled_red_sandstone";
    case 2:
      return "cut_red_sandstone";
    case 0:
    default:
      return "red_sandstone";
    }
  }

  static std::string Same(CompoundTag const &in, CompoundTagPtr &out, int16_t damage) {
    return "";
  }

  static std::string Sand(CompoundTag const &in, CompoundTagPtr &out, int16_t damage) {
    switch (damage) {
    case 1:
      return "red_sand";
    case 0:
    default:
      return "sand";
    }
  }

  static std::string Sandstone(CompoundTag const &in, CompoundTagPtr &out, int16_t damage) {
    switch (damage) {
    case 1:
      return "chiseled_sandstone";
    case 2:
      return "cut_sandstone";
    case 0:
    default:
      return "sandstone";
    }
  }

  static std::string StainedHardenedClay(CompoundTag const &in, CompoundTagPtr &out, int16_t damage) {
    switch (damage) {
    case 1:
      return "orange_terracotta";
    case 2:
      return "magenta_terracotta";
    case 3:
      return "light_blue_terracotta";
    case 4:
      return "yellow_terracotta";
    case 5:
      return "lime_terracotta";
    case 6:
      return "pink_terracotta";
    case 7:
      return "gray_terracotta";
    case 8:
      return "light_gray_terracotta";
    case 9:
      return "cyan_terracotta";
    case 10:
      return "purple_terracotta";
    case 11:
      return "blue_terracotta";
    case 12:
      return "brown_terracotta";
    case 13:
      return "green_terracotta";
    case 14:
      return "red_terracotta";
    case 15:
      return "black_terracotta";
    case 0:
    default:
      return "white_terracotta";
    }
  }

  static std::string Stone(CompoundTag const &in, CompoundTagPtr &out, int16_t damage) {
    switch (damage) {
    case 1:
      return "granite";
    case 2:
      return "polished_granite";
    case 3:
      return "diorite";
    case 4:
      return "polished_diorite";
    case 5:
      return "andesite";
    case 6:
      return "polished_andesite";
    case 0:
    default:
      return "stone";
    }
  }

  static std::string Stonebrick(CompoundTag const &in, CompoundTagPtr &out, int16_t damage) {
    switch (damage) {
    case 1:
      return "mossy_stone_bricks";
    case 2:
      return "cracked_stone_bricks";
    case 3:
      return "chiseled_stone_bricks";
    case 0:
    default:
      return "stone_bricks";
    }
  }

  static std::string StoneSlab(CompoundTag const &in, CompoundTagPtr &out, int16_t damage) {
    switch (damage) {
    case 1:
      return "sandstone_slab";
    case 3:
      return "cobblestone_slab";
    case 4:
      return "brick_slab";
    case 5:
      return "stone_brick_slab";
    case 6:
      return "nether_brick_slab";
    case 7:
      return "quartz_slab";
    case 0:
    default:
      return "smooth_stone_slab";
    }
  }

  static std::string StoneStairs(CompoundTag const &in, CompoundTagPtr &out, int16_t damage) {
    switch (damage) {
    case 2:
    default:
      return "cobblestone_stairs";
    }
  }

  static std::string WoodenSlab(CompoundTag const &in, CompoundTagPtr &out, int16_t damage) {
    switch (damage) {
    case 1:
      return "spruce_slab";
    case 2:
      return "birch_slab";
    case 3:
      return "jungle_slab";
    case 4:
      return "acacia_slab";
    case 5:
      return "dark_oak_slab";
    case 0:
    default:
      return "oak_slab";
    }
  }
#pragma endregion

#pragma region Converter_Generator
  static Converter Rename(std::string const &name) {
    return [name](CompoundTag const &in, CompoundTagPtr &out, int16_t damage) {
      return name;
    };
  }
#pragma endregion

  static std::unordered_map<std::string, Converter> const &GetTable() {
    static std::unique_ptr<std::unordered_map<std::string, Converter> const> const sTable(CreateTable());
    return *sTable;
  }

  static std::unordered_map<std::string, Converter> const *CreateTable() {
    using namespace std;
    auto ret = new unordered_map<string, Converter>();
#define E(__name, __conv)                   \
  assert(ret->find(#__name) == ret->end()); \
  ret->insert(std::make_pair(#__name, __conv))

    E(comparator, Same);
    E(daylight_detector, Same);
    E(spruce_fence_gate, Same);
    E(fence_gate, Same);
    E(birch_fence_gate, Same);
    E(jungle_fence_gate, Same);
    E(acacia_fence_gate, Same);
    E(dark_oak_fence_gate, Same);
    E(birch_stairs, Same);
    E(spruce_stairs, Same);
    E(oak_stairs, Same);
    E(jungle_stairs, Same);
    E(acacia_stairs, Same);
    E(dark_oak_stairs, Same);
    E(brick_stairs, Same);
    E(stone_brick_stairs, Same);
    E(nether_brick_stairs, Same);
    E(sandstone_stairs, Same);
    E(red_sandstone_stairs, Same);
    E(quartz_stairs, Same);
    E(purpur_stairs, Same);
    E(prismarine_stairs, Same);
    E(prismarine_bricks_stairs, Same);
    E(dark_prismarine_stairs, Same);

    E(stone, Stone);
    E(red_sandstone, RedSandstone);
    E(sand, Sand);
    E(sandstone, Sandstone);
    E(log, Log);
    E(planks, Planks);
    E(log2, Log2);
    E(prismarine, Prismarine);
    E(stonebrick, Stonebrick);
    E(monster_egg, MonsterEgg);
    E(dirt, Dirt);
    E(quartz_block, QuartzBlock);
    E(stone_slab, StoneSlab);
    E(wooden_slab, WoodenSlab);
    E(prismarine_slab, PrismarineSlab);
    E(stone_stairs, StoneStairs);
    E(stained_hardened_clay, StainedHardenedClay);

#undef E
    return ret;
  }
};

} // namespace je2be::box360
