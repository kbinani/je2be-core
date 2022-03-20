#pragma once

namespace je2be::box360 {

class Item {
  Item() = delete;

  using Converter = std::function<std::string(CompoundTag const &in, CompoundTagPtr &out, int16_t damage, Context const &ctx)>;

public:
  static CompoundTagPtr Convert(CompoundTag const &in, Context const &ctx) {
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

    auto changedId = found->second(in, out, damage, ctx);
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
  static std::string CobblestoneWall(CompoundTag const &in, CompoundTagPtr &out, int16_t damage, Context const &) {
    switch (damage) {
    case 1:
      return "mossy_cobblestone_wall";
    case 0:
    default:
      return "cobblestone_wall";
    }
  }

  static std::string Concrete(CompoundTag const &in, CompoundTagPtr &out, int16_t damage, Context const &) {
    switch (damage) {
    case 1:
      return "orange_concrete_powder";
    case 2:
      return "magenta_concrete_powder";
    case 3:
      return "light_blue_concrete_powder";
    case 4:
      return "yellow_concrete_powder";
    case 5:
      return "lime_concrete_powder";
    case 6:
      return "pink_concrete_powder";
    case 7:
      return "gray_concrete_powder";
    case 8:
      return "light_gray_concrete_powder";
    case 9:
      return "cyan_concrete_powder";
    case 10:
      return "purple_concrete_powder";
    case 11:
      return "blue_concrete_powder";
    case 12:
      return "brown_concrete_powder";
    case 13:
      return "green_concrete_powder";
    case 14:
      return "red_concrete_powder";
    case 15:
      return "black_concrete_powder";
    case 0:
    default:
      return "white_concrete_powder";
    }
  }

  static std::string ConcretePowder(CompoundTag const &in, CompoundTagPtr &out, int16_t damage, Context const &) {
    switch (damage) {
    case 1:
      return "orange_concrete";
    case 2:
      return "magenta_concrete";
    case 3:
      return "light_blue_concrete";
    case 4:
      return "yellow_concrete";
    case 5:
      return "lime_concrete";
    case 6:
      return "pink_concrete";
    case 7:
      return "gray_concrete";
    case 8:
      return "light_gray_concrete";
    case 9:
      return "cyan_concrete";
    case 10:
      return "purple_concrete";
    case 11:
      return "blue_concrete";
    case 12:
      return "brown_concrete";
    case 13:
      return "green_concrete";
    case 14:
      return "red_concrete";
    case 15:
      return "black_concrete";
    case 0:
    default:
      return "white_concrete";
    }
  }

  static std::string Dirt(CompoundTag const &in, CompoundTagPtr &out, int16_t damage, Context const &) {
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

  static std::string DoublePlant(CompoundTag const &in, CompoundTagPtr &out, int16_t damage, Context const &) {
    switch (damage) {
    case 1:
      return "lilac";
    case 2:
      return "tall_grass";
    case 3:
      return "large_fern";
    case 4:
      return "rose_bush";
    case 5:
      return "peony";
    case 0:
    default:
      return "sunflower";
    }
  }

  static std::string Leaves(CompoundTag const &in, CompoundTagPtr &out, int16_t damage, Context const &) {
    switch (damage) {
    case 9:
      return "spruce_leaves";
    case 10:
      return "birch_leaves";
    case 11:
      return "jungle_leaves";
    case 8:
    default:
      return "oak_leaves";
    }
  }

  static std::string Leaves2(CompoundTag const &in, CompoundTagPtr &out, int16_t damage, Context const &) {
    switch (damage) {
    case 9:
      return "dark_oak_leaves";
    case 8:
    default:
      return "acacia_leaves";
    }
  }

  static std::string Log(CompoundTag const &in, CompoundTagPtr &out, int16_t damage, Context const &) {
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

  static std::string Log2(CompoundTag const &in, CompoundTagPtr &out, int16_t damage, Context const &) {
    switch (damage) {
    case 1:
      return "dark_oak_log";
    case 0:
    default:
      return "acacia_log";
    }
  }

  static std::string MonsterEgg(CompoundTag const &in, CompoundTagPtr &out, int16_t damage, Context const &) {
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

  static std::string Planks(CompoundTag const &in, CompoundTagPtr &out, int16_t damage, Context const &) {
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

  static std::string Prismarine(CompoundTag const &in, CompoundTagPtr &out, int16_t damage, Context const &) {
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

  static std::string PrismarineSlab(CompoundTag const &in, CompoundTagPtr &out, int16_t damage, Context const &) {
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

  static std::string QuartzBlock(CompoundTag const &in, CompoundTagPtr &out, int16_t damage, Context const &) {
    switch (damage) {
    case 1:
      return "chiseled_quartz_block";
    case 2:
      return "quartz_pillar";
    case 0:
    default:
      return "quartz_block";
    }
  }

  static std::string RedFlower(CompoundTag const &in, CompoundTagPtr &out, int16_t damage, Context const &) {
    switch (damage) {
    case 1:
      return "blue_orchid";
    case 2:
      return "allium";
    case 3:
      return "azure_bluet";
    case 4:
      return "red_tulip";
    case 5:
      return "orange_tulip";
    case 6:
      return "white_tulip";
    case 7:
      return "pink_tulip";
    case 8:
      return "oxeye_daisy";
    case 0:
    default:
      return "poppy";
    }
  }

  static std::string RedSandstone(CompoundTag const &in, CompoundTagPtr &out, int16_t damage, Context const &) {
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

  static std::string Same(CompoundTag const &in, CompoundTagPtr &out, int16_t damage, Context const &) {
    return "";
  }

  static std::string Sand(CompoundTag const &in, CompoundTagPtr &out, int16_t damage, Context const &) {
    switch (damage) {
    case 1:
      return "red_sand";
    case 0:
    default:
      return "sand";
    }
  }

  static std::string Sandstone(CompoundTag const &in, CompoundTagPtr &out, int16_t damage, Context const &) {
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

  static std::string Sapling(CompoundTag const &in, CompoundTagPtr &out, int16_t damage, Context const &) {
    switch (damage) {
    case 1:
      return "spruce_sapling";
    case 2:
      return "birch_sapling";
    case 3:
      return "jungle_sapling";
    case 4:
      return "acacia_sapling";
    case 5:
      return "dark_oak_sapling";
    case 0:
    default:
      return "oak_sapling";
    }
  }

  static std::string Skull(CompoundTag const &in, CompoundTagPtr &out, int16_t damage, Context const &) {
    switch (damage) {
    case 1:
      return "wither_skeleton_skull";
    case 2:
      return "zombie_head";
    case 3:
      return "player_head";
    case 4:
      return "creeper_head";
    case 5:
      return "dragon_head";
    case 0:
    default:
      return "skeleton_skull";
    }
  }

  static std::string Sponge(CompoundTag const &in, CompoundTagPtr &out, int16_t damage, Context const &) {
    switch (damage) {
    case 1:
      return "wet_sponge";
    case 0:
    default:
      return "sponge";
    }
  }

  static std::string StainedHardenedClay(CompoundTag const &in, CompoundTagPtr &out, int16_t damage, Context const &) {
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

  static std::string Stone(CompoundTag const &in, CompoundTagPtr &out, int16_t damage, Context const &) {
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

  static std::string Stonebrick(CompoundTag const &in, CompoundTagPtr &out, int16_t damage, Context const &) {
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

  static std::string StoneSlab(CompoundTag const &in, CompoundTagPtr &out, int16_t damage, Context const &) {
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

  static std::string StoneStairs(CompoundTag const &in, CompoundTagPtr &out, int16_t damage, Context const &) {
    switch (damage) {
    case 2:
    default:
      return "cobblestone_stairs";
    }
  }

  static std::string Tallgrass(CompoundTag const &in, CompoundTagPtr &out, int16_t damage, Context const &) {
    switch (damage) {
    case 2:
      return "fern";
    case 0:
      return "grass"; // shrub
    case 1:
    default:
      return "grass";
    }
  }

  static std::string WoodenSlab(CompoundTag const &in, CompoundTagPtr &out, int16_t damage, Context const &) {
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
    return [name](CompoundTag const &in, CompoundTagPtr &out, int16_t damage, Context const &ctx) {
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
    E(torch, Same);
    E(end_rod, Same);
    E(white_glazed_terracotta, Same);
    E(orange_glazed_terracotta, Same);
    E(magenta_glazed_terracotta, Same);
    E(light_blue_glazed_terracotta, Same);
    E(yellow_glazed_terracotta, Same);
    E(lime_glazed_terracotta, Same);
    E(pink_glazed_terracotta, Same);
    E(gray_glazed_terracotta, Same);
    E(cyan_glazed_terracotta, Same);
    E(purple_glazed_terracotta, Same);
    E(blue_glazed_terracotta, Same);
    E(brown_glazed_terracotta, Same);
    E(green_glazed_terracotta, Same);
    E(red_glazed_terracotta, Same);
    E(black_glazed_terracotta, Same);

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
    E(sapling, Sapling);
    E(sponge, Sponge);
    E(skull, Skull);
    E(leaves, Leaves);
    E(leaves2, Leaves2);
    E(tallgrass, Tallgrass);
    E(double_plant, DoublePlant);
    E(red_flower, RedFlower);
    E(concrete, Concrete);
    E(cobblestone_wall, CobblestoneWall);
    E(concrete_powder, ConcretePowder);
    E(silver_glazed_terracotta, Rename("light_gray_glazed_terracotta"));

#undef E
    return ret;
  }
};

} // namespace je2be::box360
