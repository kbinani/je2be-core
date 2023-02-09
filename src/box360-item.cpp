#include "box360/_item.hpp"

#include "_nbt-ext.hpp"
#include "_pos3.hpp"
#include "box360/_context.hpp"
#include "enums/_color-code-java.hpp"
#include "item/_enchantments.hpp"

#include <nlohmann/json.hpp>

namespace je2be::box360 {

class Item::Impl {
  Impl() = delete;

  using Converter = std::function<std::string(CompoundTag const &in, CompoundTagPtr &out, i16 *damage, Context const &ctx)>;

public:
  static CompoundTagPtr Convert(CompoundTag const &in, Context const &ctx) {
    using namespace std;
    optional<string> rawId = in.string("id");
    if (!rawId) {
      auto i16Id = in.int16("id");
      if (!i16Id) {
        return nullptr;
      }
      rawId = MigrateId(*i16Id);
    }
    if (!rawId) {
      return nullptr;
    }
    if (!rawId->starts_with("minecraft:")) {
      return nullptr;
    }
    auto id = rawId->substr(10);
    auto const &table = GetTable();
    auto found = table.find(id);

    auto out = Compound();
    CopyByteValues(in, *out, {{"Count"}, {"Slot"}});

    i16 damage = in.int16("Damage", 0);
    string changedId = *rawId;
    if (found == table.end()) {
      changedId = Same(in, out, &damage, ctx);
    } else {
      changedId = found->second(in, out, &damage, ctx);
    }
    if (!out) {
      return nullptr;
    }
    if (changedId.empty()) {
      out->set("id", String(*rawId));
    } else {
      out->set("id", String("minecraft:" + changedId));
    }
    auto tagJ = Compound();
    if (auto tagB = in.compoundTag("tag"); tagB) {
      if (auto j = out->compoundTag("tag"); j) {
        tagJ = j;
      }

      if (auto blockEntityTagB = tagB->compoundTag("BlockEntityTag"); blockEntityTagB) {
        auto tileEntityId = GetTileEntityNameFromItemName(id);
        blockEntityTagB->set("id", String("minecraft:" + tileEntityId));
        if (auto converted = ctx.fTileEntityConverter(*blockEntityTagB, nullptr, Pos3i(0, 0, 0), ctx); converted && converted->fTileEntity) {
          tagJ->set("BlockEntityTag", converted->fTileEntity);
        }
      }

      if (auto displayB = tagB->compoundTag("display"); displayB) {
        auto displayJ = tagJ->compoundTag("display");
        if (!displayJ) {
          displayJ = Compound();
        }
        if (auto name = displayB->string("Name"); name) {
          nlohmann::json obj;
          obj["text"] = *name;
          displayJ->set("Name", String(nlohmann::to_string(obj)));
        }
        if (!displayJ->empty()) {
          tagJ->set("display", displayJ);
        }
      }

      CopyIntValues(*tagB, *tagJ, {{"RepairCost"}});

      if (auto enchB = tagB->listTag("ench"); enchB) {
        auto enchantmentsJ = List<Tag::Type::Compound>();
        for (auto const &item : *enchB) {
          auto c = item->asCompound();
          if (!c) {
            continue;
          }
          auto idB = c->int16("id");
          if (!idB) {
            continue;
          }
          auto lvl = c->int16("lvl", 1);
          string idJ = Enchantments::JavaEnchantmentIdFromBox360(*idB);
          auto itemJ = Compound();
          itemJ->set("lvl", Short(lvl));
          itemJ->set("id", String(idJ));
          enchantmentsJ->push_back(itemJ);
        }
        if (!enchantmentsJ->empty()) {
          tagJ->set("Enchantments", enchantmentsJ);
        }
      }
    }
    if (damage != 0) {
      tagJ->set("Damage", Int(damage));
    }
    if (!tagJ->empty()) {
      out->set("tag", tagJ);
    }
    return out;
  }

private:
#pragma region Converter
  static std::string Banner(CompoundTag const &in, CompoundTagPtr &out, i16 *damage, Context const &ctx) {
    i16 d = *damage;
    *damage = 0;
    auto color = ColorCodeJavaFromBannerColorCodeBedrock(static_cast<BannerColorCodeBedrock>(d));
    std::string colorName = JavaNameFromColorCodeJava(color);
    return colorName + "_banner";
  }

  static std::string Coal(CompoundTag const &in, CompoundTagPtr &out, i16 *damage, Context const &) {
    i16 d = *damage;
    *damage = 0;
    switch (d) {
    case 1:
      return "charcoal";
    case 0:
    default:
      return "coal";
    }
  }

  static std::string CobblestoneWall(CompoundTag const &in, CompoundTagPtr &out, i16 *damage, Context const &) {
    i16 d = *damage;
    *damage = 0;
    switch (d) {
    case 1:
      return "mossy_cobblestone_wall";
    case 0:
    default:
      return "cobblestone_wall";
    }
  }

  static std::string CookedFish(CompoundTag const &in, CompoundTagPtr &out, i16 *damage, Context const &) {
    i16 d = *damage;
    *damage = 0;
    switch (d) {
    case 1:
      return "cooked_salmon";
    case 0:
    default:
      return "cooked_cod";
    }
  }

  static std::string Coral(CompoundTag const &in, CompoundTagPtr &out, i16 *damage, Context const &) {
    i16 d = *damage;
    *damage = 0;
    switch (d) {
    case 1:
      return "brain_coral";
    case 2:
      return "bubble_coral";
    case 3:
      return "fire_coral";
    case 4:
      return "horn_coral";
    case 0:
    default:
      return "tube_coral";
    }
  }

  static std::string CoralBlock(CompoundTag const &in, CompoundTagPtr &out, i16 *damage, Context const &) {
    i16 d = *damage;
    *damage = 0;
    std::string prefix = (d & 0x8) == 0x8 ? "dead_" : "";
    switch (d & 0x7) {
    case 1:
      return prefix + "brain_coral_block";
    case 2:
      return prefix + "bubble_coral_block";
    case 3:
      return prefix + "fire_coral_block";
    case 4:
      return prefix + "horn_coral_block";
    case 0:
    default:
      return prefix + "tube_coral_block";
    }
  }

  static std::string CoralFan(CompoundTag const &in, CompoundTagPtr &out, i16 *damage, Context const &) {
    i16 d = *damage;
    *damage = 0;
    switch (d) {
    case 1:
      return "brain_coral_fan";
    case 2:
      return "bubble_coral_fan";
    case 3:
      return "fire_coral_fan";
    case 4:
      return "horn_coral_fan";
    case 0:
    default:
      return "tube_coral_fan";
    }
  }

  static std::string CoralFanDead(CompoundTag const &in, CompoundTagPtr &out, i16 *damage, Context const &) {
    i16 d = *damage;
    *damage = 0;
    switch (d) {
    case 1:
      return "dead_brain_coral_fan";
    case 2:
      return "dead_bubble_coral_fan";
    case 3:
      return "dead_fire_coral_fan";
    case 4:
      return "dead_horn_coral_fan";
    case 0:
    default:
      return "dead_tube_coral_fan";
    }
  }

  static std::string Dirt(CompoundTag const &in, CompoundTagPtr &out, i16 *damage, Context const &) {
    i16 d = *damage;
    *damage = 0;
    switch (d) {
    case 1:
      return "coarse_dirt";
    case 2:
      return "podzol";
    case 0:
    default:
      return "dirt";
    }
  }

  static std::string DoublePlant(CompoundTag const &in, CompoundTagPtr &out, i16 *damage, Context const &) {
    i16 d = *damage;
    *damage = 0;
    switch (d) {
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

  static std::string Dye(CompoundTag const &in, CompoundTagPtr &out, i16 *damage, Context const &) {
    i16 d = *damage;
    *damage = 0;
    switch (d) {
    case 1:
      return "red_dye";
    case 2:
      return "green_dye";
    case 3:
      return "cocoa_beans";
    case 4:
      return "lapis_lazuli";
    case 5:
      return "purple_dye";
    case 6:
      return "cyan_dye";
    case 7:
      return "light_gray_dye";
    case 8:
      return "gray_dye";
    case 9:
      return "pink_dye";
    case 10:
      return "lime_dye";
    case 11:
      return "yellow_dye";
    case 12:
      return "light_blue_dye";
    case 13:
      return "magenta_dye";
    case 14:
      return "orange_dye";
    case 15:
      return "bone_meal";
    case 0:
    default:
      return "ink_sac";
    }
  }

  static std::string EnchantedBook(CompoundTag const &in, CompoundTagPtr &out, i16 *damage, Context const &) {
    if (auto tagB = in.compoundTag("tag"); tagB) {
      auto tagJ = Compound();
      if (auto storedB = tagB->listTag("StoredEnchantments"); storedB) {
        auto storedJ = List<Tag::Type::Compound>();
        for (auto const &it : *storedB) {
          auto enchB = it->asCompound();
          if (!enchB) {
            continue;
          }
          auto idB = enchB->int16("id");
          if (!idB) {
            continue;
          }
          auto idJ = Enchantments::JavaEnchantmentIdFromBox360(*idB);
          auto lvl = enchB->int16("lvl");
          if (!lvl) {
            continue;
          }
          auto enchJ = Compound();
          enchJ->set("id", String(idJ));
          enchJ->set("lvl", Short(*lvl));
          storedJ->push_back(enchJ);
        }
        tagJ->set("StoredEnchantments", storedJ);
      }
      if (!tagJ->empty()) {
        out->set("tag", tagJ);
      }
    }
    return "";
  }

  static std::string FilledMap(CompoundTag const &in, CompoundTagPtr &out, i16 *damage, Context const &) {
    i16 d = *damage;
    *damage = 0;
    auto tagJ = Compound();
    tagJ->set("map", Int(d));
    out->set("tag", tagJ);
    return "";
  }

  static std::string Fireworks(CompoundTag const &in, CompoundTagPtr &out, i16 *damage, Context const &) {
    if (auto tag = in.compoundTag("tag"); tag) {
      out->set("tag", tag->copy());
    }
    return "firework_rocket";
  }

  static std::string Fish(CompoundTag const &in, CompoundTagPtr &out, i16 *damage, Context const &) {
    i16 d = *damage;
    *damage = 0;
    switch (d) {
    case 1:
      return "salmon";
    case 2:
      return "tropical_fish";
    case 3:
      return "pufferfish";
    case 0:
    default:
      return "cod";
    }
  }

  static std::string Leaves(CompoundTag const &in, CompoundTagPtr &out, i16 *damage, Context const &) {
    i16 d = *damage;
    *damage = 0;
    switch (d & 0x7) {
    case 1:
      return "spruce_leaves";
    case 2:
      return "birch_leaves";
    case 3:
      return "jungle_leaves";
    case 0:
    default:
      return "oak_leaves";
    }
  }

  static std::string Leaves2(CompoundTag const &in, CompoundTagPtr &out, i16 *damage, Context const &) {
    i16 d = *damage;
    *damage = 0;
    switch (d & 0x7) {
    case 1:
      return "dark_oak_leaves";
    case 0:
    default:
      return "acacia_leaves";
    }
  }

  static std::string Log(CompoundTag const &in, CompoundTagPtr &out, i16 *damage, Context const &) {
    i16 d = *damage;
    *damage = 0;
    switch (d) {
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

  static std::string Log2(CompoundTag const &in, CompoundTagPtr &out, i16 *damage, Context const &) {
    i16 d = *damage;
    *damage = 0;
    switch (d) {
    case 1:
      return "dark_oak_log";
    case 0:
    default:
      return "acacia_log";
    }
  }

  static std::string MonsterEgg(CompoundTag const &in, CompoundTagPtr &out, i16 *damage, Context const &) {
    i16 d = *damage;
    *damage = 0;
    switch (d) {
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

  static std::string MushroomBlock(CompoundTag const &in, CompoundTagPtr &out, i16 *damage, Context const &) {
    i16 d = *damage;
    *damage = 0;
    switch (d) {
    case 10:
      return "mushroom_stem";
    default:
      return "";
    }
  }

  static std::string Planks(CompoundTag const &in, CompoundTagPtr &out, i16 *damage, Context const &) {
    i16 d = *damage;
    *damage = 0;
    switch (d) {
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

  static std::string Potion(CompoundTag const &in, CompoundTagPtr &out, i16 *damage, Context const &) {
    std::string id;
    if (auto tag = in.compoundTag("tag"); tag) {
      out->set("tag", tag);
    } else {
      i16 d = DrainDamage(damage);
      std::optional<std::string> potion;
      switch (d & 0x1f) {
      case 0:
        potion = "water";
        break;
      case 1:
        potion = "regeneration";
        break;
      case 2:
        potion = "swiftness";
        break;
      case 3:
        potion = "fire_resistance";
        break;
      case 4:
        potion = "poison";
        break;
      case 5:
        potion = "healing";
        break;
      case 8:
        potion = "weakness";
        break;
      case 9:
        potion = "strength";
        break;
      case 10:
        potion = "slowness";
        break;
      case 12:
        potion = "harming";
        break;
      case 16:
        potion = "mundane";
        break;
      }
      if (potion) {
        auto t = Compound();
        t->set("Potion", String("minecraft:" + *potion));
        out->set("tag", t);
      }
      if ((0x4000 & d) == 0x4000) {
        id = "splash_potion";
      }
    }
    return id;
  }

  static std::string Prismarine(CompoundTag const &in, CompoundTagPtr &out, i16 *damage, Context const &) {
    i16 d = *damage;
    *damage = 0;
    switch (d) {
    case 1:
      return "prismarine_bricks";
    case 2:
      return "dark_prismarine";
    case 0:
    default:
      return "prismarine";
    }
  }

  static std::string PrismarineSlab(CompoundTag const &in, CompoundTagPtr &out, i16 *damage, Context const &) {
    i16 d = *damage;
    *damage = 0;
    switch (d) {
    case 1:
      return "prismarine_brick_slab";
    case 2:
      return "dark_prismarine_slab";
    case 0:
    default:
      return "prismarine_slab";
    }
  }

  static std::string QuartzBlock(CompoundTag const &in, CompoundTagPtr &out, i16 *damage, Context const &) {
    i16 d = *damage;
    *damage = 0;
    switch (d) {
    case 1:
      return "chiseled_quartz_block";
    case 2:
      return "quartz_pillar";
    case 0:
    default:
      return "quartz_block";
    }
  }

  static std::string RedFlower(CompoundTag const &in, CompoundTagPtr &out, i16 *damage, Context const &) {
    i16 d = *damage;
    *damage = 0;
    switch (d) {
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

  static std::string RedSandstone(CompoundTag const &in, CompoundTagPtr &out, i16 *damage, Context const &) {
    i16 d = *damage;
    *damage = 0;
    switch (d) {
    case 1:
      return "chiseled_red_sandstone";
    case 2:
      return "cut_red_sandstone";
    case 0:
    default:
      return "red_sandstone";
    }
  }

  static std::string Same(CompoundTag const &in, CompoundTagPtr &out, i16 *damage, Context const &) {
    return "";
  }

  static std::string Sand(CompoundTag const &in, CompoundTagPtr &out, i16 *damage, Context const &) {
    i16 d = *damage;
    *damage = 0;
    switch (d) {
    case 1:
      return "red_sand";
    case 0:
    default:
      return "sand";
    }
  }

  static std::string Sandstone(CompoundTag const &in, CompoundTagPtr &out, i16 *damage, Context const &) {
    i16 d = *damage;
    *damage = 0;
    switch (d) {
    case 1:
      return "chiseled_sandstone";
    case 2:
      return "cut_sandstone";
    case 0:
    default:
      return "sandstone";
    }
  }

  static std::string Sapling(CompoundTag const &in, CompoundTagPtr &out, i16 *damage, Context const &) {
    i16 d = *damage;
    *damage = 0;
    switch (d) {
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

  static std::string Skull(CompoundTag const &in, CompoundTagPtr &out, i16 *damage, Context const &) {
    i16 d = *damage;
    *damage = 0;
    switch (d) {
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

  static std::string SpawnEgg(CompoundTag const &in, CompoundTagPtr &out, i16 *damage, Context const &ctx) {
    std::string name;
    i16 d = DrainDamage(damage);
    switch (d) {
    case 50:
      name = "creeper";
      break;
    case 51:
      name = "skeleton";
      break;
    case 52:
      name = "spider";
      break;
    case 54:
      name = "zombie";
      break;
    case 55:
      name = "slime";
      break;
    case 56:
      name = "ghast";
      break;
    case 57:
      name = "zombified_piglin";
      break;
    case 58:
      name = "enderman";
      break;
    case 59:
      name = "cave_spider";
      break;
    case 60:
      name = "silverfish";
      break;
    case 61:
      name = "blaze";
      break;
    case 62:
      name = "magma_cube";
      break;
    case 90:
      name = "pig";
      break;
    case 91:
      name = "sheep";
      break;
    case 92:
      name = "cow";
      break;
    case 93:
      name = "chicken";
      break;
    case 94:
      name = "squid";
      break;
    case 95:
      name = "wolf";
      break;
    case 96:
      name = "mooshroom";
      break;
    case 120:
      name = "villager";
      break;
    }
    if (auto tag = in.compoundTag("tag"); tag) {
      if (auto entityTag = tag->compoundTag("EntityTag"); entityTag) {
        if (auto id = entityTag->string("id"); id) {
          auto n = ctx.fEntityNameMigrator(*id);
          if (n.starts_with("minecraft:")) {
            name = n.substr(10);
          }
        }
      }
    }
    if (name.empty()) {
      return "";
    } else {
      return name + "_spawn_egg";
    }
  }

  static std::string Sponge(CompoundTag const &in, CompoundTagPtr &out, i16 *damage, Context const &) {
    i16 d = *damage;
    *damage = 0;
    switch (d) {
    case 1:
      return "wet_sponge";
    case 0:
    default:
      return "sponge";
    }
  }

  static std::string Stone(CompoundTag const &in, CompoundTagPtr &out, i16 *damage, Context const &) {
    i16 d = *damage;
    *damage = 0;
    switch (d) {
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

  static std::string Stonebrick(CompoundTag const &in, CompoundTagPtr &out, i16 *damage, Context const &) {
    i16 d = *damage;
    *damage = 0;
    switch (d) {
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

  static std::string StoneSlab(CompoundTag const &in, CompoundTagPtr &out, i16 *damage, Context const &) {
    i16 d = *damage;
    *damage = 0;
    switch (d) {
    case 1:
      return "sandstone_slab";
    case 2:
      return "oak_slab";
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

  static std::string StoneStairs(CompoundTag const &in, CompoundTagPtr &out, i16 *damage, Context const &) {
    i16 d = *damage;
    *damage = 0;
    switch (d) {
    case 2:
    default:
      return "cobblestone_stairs";
    }
  }

  static std::string Tallgrass(CompoundTag const &in, CompoundTagPtr &out, i16 *damage, Context const &) {
    i16 d = *damage;
    *damage = 0;
    switch (d) {
    case 2:
      return "fern";
    case 0:
      return "grass"; // shrub
    case 1:
    default:
      return "grass";
    }
  }

  static std::string WoodenSlab(CompoundTag const &in, CompoundTagPtr &out, i16 *damage, Context const &) {
    i16 d = *damage;
    *damage = 0;
    switch (d) {
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
  static Converter Colored(std::string const &suffix) {
    return [suffix](CompoundTag const &in, CompoundTagPtr &out, i16 *damage, Context const &ctx) {
      i16 d = *damage;
      *damage = 0;
      auto colorName = JavaNameFromColorCodeJava(static_cast<ColorCodeJava>(d));
      return colorName + "_" + suffix;
    };
  }

  static Converter Rename(std::string const &name) {
    return [name](CompoundTag const &in, CompoundTagPtr &out, i16 *damage, Context const &ctx) {
      return name;
    };
  }
#pragma endregion

  static std::optional<std::string> MigrateId(u16 id) {
    using namespace std;
    static unique_ptr<vector<string> const> const sTable(CreateIdTable());
    if (sTable->size() <= id) {
      return nullopt;
    }
    string mapped = (*sTable)[id];
    if (mapped.empty()) {
      return nullopt;
    }
    return "minecraft:" + mapped;
  }

  static i16 DrainDamage(i16 *damage) {
    i16 d = *damage;
    *damage = 0;
    return d;
  }

  static std::vector<std::string> const *CreateIdTable() {
    using namespace std;
    int maxId = -1;
    unordered_map<u16, string> t;

#define R(id, name)              \
  assert(t.count(id) == 0);      \
  maxId = (std::max)(maxId, id); \
  t[id] = #name

    R(1, stone);
    R(2, grass_block);
    R(3, dirt);
    R(4, cobblestone);
    R(5, planks);
    R(6, sapling);
    R(7, bedrock);
    R(12, sand);
    R(13, gravel);
    R(14, gold_ore);
    R(15, iron_ore);
    R(16, coal_ore);
    R(17, log);
    R(18, leaves);
    R(19, sponge);
    R(20, glass);
    R(21, lapis_ore);
    R(22, lapis_block);
    R(23, dispenser);
    R(24, sandstone);
    R(25, noteblock);
    R(27, golden_rail);
    R(28, detector_rail);
    R(29, sticky_piston);
    R(30, cobweb);
    R(31, tallgrass);
    R(32, deadbush);
    R(33, piston);
    R(35, wool);
    R(37, yellow_flower);
    R(38, red_flower);
    R(39, brown_mushroom);
    R(40, red_mushroom);
    R(41, gold_block);
    R(42, iron_block);
    R(44, stone_slab);
    R(45, brick_block);
    R(46, tnt);
    R(47, bookshelf);
    R(48, mossy_cobblestone);
    R(49, obsidian);
    R(50, torch);
    R(53, oak_stairs);
    R(54, chest);
    R(56, diamond_ore);
    R(57, diamond_block);
    R(58, crafting_table);
    R(61, furnace);
    R(65, ladder);
    R(66, rail);
    R(67, stone_stairs);
    R(69, lever);
    R(70, stone_pressure_plate); // pressure_plate?
    R(72, wooden_pressure_plate);
    R(73, redstone_ore);
    R(76, redstone_torch);
    R(77, stone_button); // button?
    R(78, snow_layer);
    R(79, ice);
    R(80, snow_block);
    R(81, cactus);
    R(82, clay);
    R(84, jukebox);
    R(85, fence);
    R(86, carved_pumpkin);
    R(87, netherrack);
    R(88, soul_sand);
    R(89, glowstone);
    R(91, lit_pumpkin);
    R(96, trapdoor);
    R(97, monster_egg);
    R(98, stonebrick);
    R(101, iron_bars);
    R(102, glass_pane);
    R(103, melon_block);
    R(106, vine);
    R(107, fence_gate);
    R(108, brick_stairs);
    R(109, stone_brick_stairs);
    R(110, mycelium);
    R(111, waterlily);
    R(112, nether_brick);
    R(113, nether_brick_fence);
    R(114, nether_brick_stairs);
    R(116, enchanting_table);
    R(120, end_portal_frame);
    R(121, end_stone);
    R(126, wooden_slab);
    R(128, sandstone_stairs);
    R(134, spruce_stairs);
    R(135, birch_stairs);
    R(143, wooden_button);
    R(256, iron_shovel);
    R(257, iron_pickaxe);
    R(258, iron_axe);
    R(259, flint_and_steel);
    R(260, apple);
    R(261, bow);
    R(262, arrow);
    R(263, coal);
    R(264, diamond);
    R(265, iron_ingot);
    R(266, gold_ingot);
    R(267, iron_sword);
    R(268, wooden_sword);
    R(269, wooden_shovel);
    R(270, wooden_pickaxe);
    R(271, wooden_axe);
    R(272, stone_sword);
    R(273, stone_shovel);
    R(274, stone_pickaxe);
    R(275, stone_axe);
    R(276, diamond_sword);
    R(277, diamond_shovel);
    R(278, diamond_pickaxe);
    R(279, diamond_axe);
    R(280, stick);
    R(281, bowl);
    R(282, mushroom_stew);
    R(283, golden_sword);
    R(284, golden_shovel);
    R(285, golden_pickaxe);
    R(286, golden_axe);
    R(287, string);
    R(289, gunpowder);
    R(288, feather);
    R(290, wooden_hoe);
    R(291, stone_hoe);
    R(292, iron_hoe);
    R(293, diamond_hoe);
    R(294, golden_hoe);
    R(295, wheat_seeds);
    R(296, wheat);
    R(297, bread);
    R(298, leather_helmet);
    R(299, leather_chestplate);
    R(300, leather_leggings);
    R(301, leather_boots);
    R(302, chainmail_helmet);
    R(303, chainmail_chestplate);
    R(304, chainmail_leggings);
    R(305, chainmail_boots);
    R(306, iron_helmet);
    R(307, iron_chestplate);
    R(308, iron_leggings);
    R(309, iron_boots);
    R(310, diamond_helmet);
    R(311, diamond_chestplate);
    R(312, diamond_leggings);
    R(313, diamond_boots);
    R(314, golden_helmet);
    R(315, golden_chestplate);
    R(316, golden_leggings);
    R(317, golden_boots);
    R(318, flint);
    R(319, porkchop);
    R(320, cooked_porkchop);
    R(321, painting);
    R(322, golden_apple);
    R(323, sign);
    R(324, wooden_door);
    R(325, bucket);
    R(326, water_bucket);
    R(327, lava_bucket);
    R(328, minecart);
    R(329, saddle);
    R(330, iron_door);
    R(331, redstone);
    R(332, snowball);
    R(333, boat);
    R(334, leather);
    R(335, milk_bucket);
    R(336, brick);
    R(337, clay_ball);
    R(338, reeds);
    R(339, paper);
    R(340, book);
    R(341, slime_ball);
    R(342, chest_minecart);
    R(343, furnace_minecart);
    R(344, egg);
    R(345, compass);
    R(346, fishing_rod);
    R(347, clock);
    R(348, glowstone_dust);
    R(349, fish);
    R(350, cooked_fish);
    R(351, dye);
    R(352, bone);
    R(353, sugar);
    R(354, cake);
    R(355, bed);
    R(356, repeater);
    R(357, cookie);
    R(358, map);
    R(359, shears);
    R(360, melon_slice);
    R(361, pumpkin_seeds);
    R(362, melon_seeds);
    R(363, beef);
    R(364, cooked_beef);
    R(365, chicken);
    R(366, cooked_chicken);
    R(367, rotten_flesh);
    R(368, ender_pearl);
    R(369, blaze_rod);
    R(370, ghast_tear);
    R(371, gold_nugget);
    R(372, nether_wart);
    R(373, potion);
    R(374, glass_bottle);
    R(375, spider_eye);
    R(376, fermented_spider_eye);
    R(377, blaze_powder);
    R(378, magma_cream);
    R(379, brewing_stand);
    R(380, cauldron);
    R(381, ender_eye);
    R(382, speckled_melon);
    R(383, spawn_egg);
    R(384, experience_bottle);
    R(385, fire_charge);
    R(389, item_frame);
    R(405, netherbrick);
    R(2256, music_disc_13);
    R(2257, music_disc_cat);
    R(2258, music_disc_blocks);
    R(2259, music_disc_chirp);
    R(2260, music_disc_far);
    R(2261, music_disc_mall);
    R(2262, music_disc_mellohi);
    R(2267, music_disc_wait); // "where are we now"
    R(2263, music_disc_stal);
    R(2264, music_disc_strad);
    R(2265, music_disc_ward);
    R(2266, music_disc_11);

#undef R

    auto ret = new vector<string>();
    ret->resize(maxId + 1);
    for (auto [id, name] : t) {
      (*ret)[id] = name;
    }
    return ret;
  }

  static std::string GetTileEntityNameFromItemName(std::string const &name) {
    if (name.find("shulker_box")) {
      return "shulker_box";
    }
    return name;
  }

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
    E(stained_hardened_clay, Colored("terracotta"));
    E(sapling, Sapling);
    E(sponge, Sponge);
    E(skull, Skull);
    E(leaves, Leaves);
    E(leaves2, Leaves2);
    E(tallgrass, Tallgrass);
    E(double_plant, DoublePlant);
    E(red_flower, RedFlower);
    E(concrete, Colored("concrete"));
    E(cobblestone_wall, CobblestoneWall);
    E(concrete_powder, Colored("concrete_powder"));
    E(silver_glazed_terracotta, Rename("light_gray_glazed_terracotta"));
    E(banner, Banner);
    E(wool, Colored("wool"));
    E(carpet, Colored("carpet"));
    E(stained_glass, Colored("stained_glass"));
    E(stained_glass_pane, Colored("stained_glass_pane"));
    E(coral_fan, CoralFan);
    E(coral, Coral);
    E(brown_mushroom_block, MushroomBlock);
    E(red_mushroom_block, MushroomBlock);
    E(coral_fan_dead, CoralFanDead);
    E(coral_block, CoralBlock);
    E(melon_block, Rename("melon"));
    E(lit_pumpkin, Rename("jack_o_lantern"));
    E(waterlily, Rename("lily_pad"));
    E(deadbush, Rename("dead_bush"));
    E(yellow_flower, Rename("dandelion"));
    E(snow_layer, Rename("snow"));
    E(web, Rename("cobweb"));
    E(sign, Rename("oak_sign"));
    E(sea_grass, Rename("seagrass"));
    E(grass_path, Rename("dirt_path"));
    E(quartz_ore, Rename("nether_quartz_ore"));
    E(stripped_log_oak, Rename("stripped_oak_log"));
    E(stripped_log_spruce, Rename("stripped_spruce_log"));
    E(stripped_log_birch, Rename("stripped_birch_log"));
    E(stripped_log_jungle, Rename("stripped_jungle_log"));
    E(stripped_log_acacia, Rename("stripped_acacia_log"));
    E(stripped_log_dark_oak, Rename("stripped_dark_oak_log"));
    E(brick_block, Rename("bricks"));
    E(magma, Rename("magma_block"));
    E(slime, Rename("slime_block"));
    E(fence, Rename("oak_fence"));
    E(nether_brick, Rename("nether_bricks"));
    E(red_nether_brick, Rename("red_nether_bricks"));
    E(end_bricks, Rename("end_stone_bricks"));
    E(trapdoor, Rename("oak_trapdoor"));
    E(wooden_door, Rename("oak_door"));
    E(stone_slab2, Rename("red_sandstone_slab"));
    E(prismarine_bricks_stairs, Rename("prismarine_brick_stairs"));
    E(hardened_clay, Rename("terracotta"));
    E(fence_gate, Rename("oak_fence_gate"));
    E(golden_rail, Rename("powered_rail"));
    E(boat, Rename("oak_boat"));
    E(noteblock, Rename("note_block"));
    E(wooden_button, Rename("oak_button"));
    E(wooden_pressure_plate, Rename("oak_pressure_plate"));
    E(netherbrick, Rename("nether_brick"));
    E(nautilus, Rename("nautilus_shell"));
    E(nautilus_core, Rename("heart_of_the_sea"));
    E(reeds, Rename("sugar_cane"));
    E(chorus_fruit_popped, Rename("popped_chorus_fruit"));
    E(turtle_shell_piece, Rename("scute"));
    E(dye, Dye);
    E(cooked_fish, CookedFish);
    E(fish, Fish);
    E(speckled_melon, Rename("glistering_melon_slice"));
    E(potion, Potion);
    E(lingering_potion, Potion);
    E(splash_potion, Potion);
    E(enchanted_book, EnchantedBook);
    E(tipped_arrow, Potion);
    E(silver_shulker_box, Rename("light_gray_shulker_box"));
    E(bed, Colored("bed"));
    E(fish_bucket, Rename("cod_bucket"));
    E(puffer_bucket, Rename("pufferfish_bucket"));
    E(tropical_bucket, Rename("tropical_fish_bucket"));
    E(mob_spawner, Rename("spawner"));
    E(spawn_egg, SpawnEgg);
    E(record_13, Rename("music_disc_13"));
    E(record_cat, Rename("music_disc_cat"));
    E(record_blocks, Rename("music_disc_blocks"));
    E(record_chirp, Rename("music_disc_chirp"));
    E(record_far, Rename("music_disc_far"));
    E(record_mall, Rename("music_disc_mall"));
    E(record_mellohi, Rename("music_disc_mellohi"));
    E(record_stal, Rename("music_disc_stal"));
    E(record_strad, Rename("music_disc_strad"));
    E(record_ward, Rename("music_disc_ward"));
    E(record_11, Rename("music_disc_11"));
    E(record_wait, Rename("music_disc_wait"));
    E(fireworks, Fireworks);
    E(filled_map, FilledMap);
    E(coal, Coal); // TODO: charcoal(?) for TU76

#undef E
    return ret;
  }
};

CompoundTagPtr Item::Convert(CompoundTag const &in, Context const &ctx) {
  return Impl::Convert(in, ctx);
}

} // namespace je2be::box360
