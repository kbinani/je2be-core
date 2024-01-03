#include "lce/_item.hpp"

#include "_namespace.hpp"
#include "_nbt-ext.hpp"
#include "_pos3.hpp"
#include "_props.hpp"
#include "lce/_context.hpp"
#include "enums/_color-code-java.hpp"
#include "item/_enchantments.hpp"
#include "item/_potion.hpp"

namespace je2be::box360 {

class Item::Impl {
  Impl() = delete;

  using Converter = std::function<std::u8string(CompoundTag const &in, CompoundTagPtr &out, i16 *damage, Context const &ctx)>;

public:
  static CompoundTagPtr Convert(CompoundTag const &in, Context const &ctx) {
    using namespace std;
    optional<u8string> rawId = in.string(u8"id"); // tu31, tu33, tu37, tu45, tu75
    if (!rawId) {
      auto i16Id = in.int16(u8"id"); // tu16, tu30
      if (!i16Id) {
        return nullptr;
      }
      rawId = MigrateId(*i16Id);
    }
    if (!rawId) {
      return nullptr;
    }
    if (!rawId->starts_with(u8"minecraft:")) {
      return nullptr;
    }
    auto id = Namespace::Remove(*rawId);
    auto const &table = GetTable();
    auto found = table.find(id);

    auto out = Compound();
    CopyByteValues(in, *out, {{u8"Count"}, {u8"Slot"}});

    i16 damage = in.int16(u8"Damage", 0);
    u8string changedId;
    if (found == table.end()) {
      changedId = Same(in, out, &damage, ctx);
    } else {
      changedId = found->second(in, out, &damage, ctx);
    }
    if (!out) {
      return nullptr;
    }
    if (changedId.empty()) {
      out->set(u8"id", *rawId);
    } else {
      out->set(u8"id", u8"minecraft:" + changedId);
    }
    auto tagJ = Compound();
    if (auto tagB = in.compoundTag(u8"tag"); tagB) {
      if (auto j = out->compoundTag(u8"tag"); j) {
        tagJ = j;
      }

      if (auto blockEntityTagB = tagB->compoundTag(u8"BlockEntityTag"); blockEntityTagB) {
        auto tileEntityId = GetTileEntityNameFromItemName(id);
        blockEntityTagB->set(u8"id", u8"minecraft:" + tileEntityId);
        if (auto converted = ctx.fTileEntityConverter(*blockEntityTagB, nullptr, Pos3i(0, 0, 0), ctx); converted && converted->fTileEntity) {
          tagJ->set(u8"BlockEntityTag", converted->fTileEntity);
        }
      }

      if (auto displayB = tagB->compoundTag(u8"display"); displayB) {
        auto displayJ = tagJ->compoundTag(u8"display");
        if (!displayJ) {
          displayJ = Compound();
        }
        if (auto name = displayB->string(u8"Name"); name) {
          props::Json obj;
          props::SetJsonString(obj, u8"text", *name);
          displayJ->set(u8"Name", props::StringFromJson(obj));
        }
        if (!displayJ->empty()) {
          tagJ->set(u8"display", displayJ);
        }
      }

      CopyIntValues(*tagB, *tagJ, {{u8"RepairCost"}});

      if (auto enchB = tagB->listTag(u8"ench"); enchB) {
        auto enchantmentsJ = List<Tag::Type::Compound>();
        for (auto const &item : *enchB) {
          auto c = item->asCompound();
          if (!c) {
            continue;
          }
          auto idB = c->int16(u8"id");
          if (!idB) {
            continue;
          }
          auto lvl = c->int16(u8"lvl", 1);
          u8string idJ = Enchantments::JavaEnchantmentIdFromBox360(*idB);
          auto itemJ = Compound();
          itemJ->set(u8"lvl", Short(lvl));
          itemJ->set(u8"id", idJ);
          enchantmentsJ->push_back(itemJ);
        }
        if (!enchantmentsJ->empty()) {
          tagJ->set(u8"Enchantments", enchantmentsJ);
        }
      }
    }
    if (damage != 0) {
      tagJ->set(u8"Damage", Int(damage));
    }
    if (!tagJ->empty()) {
      out->set(u8"tag", tagJ);
    }
    return out;
  }

private:
#pragma region Converter
  static std::u8string Banner(CompoundTag const &in, CompoundTagPtr &out, i16 *damage, Context const &ctx) {
    i16 d = *damage;
    *damage = 0;
    auto color = ColorCodeJavaFromBannerColorCodeBedrock(static_cast<BannerColorCodeBedrock>(d));
    std::u8string colorName = JavaNameFromColorCodeJava(color);
    return colorName + u8"_banner";
  }

  static std::u8string Coal(CompoundTag const &in, CompoundTagPtr &out, i16 *damage, Context const &) {
    i16 d = *damage;
    *damage = 0;
    switch (d) {
    case 1:
      return u8"charcoal";
    case 0:
    default:
      return u8"coal";
    }
  }

  static std::u8string CobblestoneWall(CompoundTag const &in, CompoundTagPtr &out, i16 *damage, Context const &) {
    i16 d = *damage;
    *damage = 0;
    switch (d) {
    case 1:
      return u8"mossy_cobblestone_wall";
    case 0:
    default:
      return u8"cobblestone_wall";
    }
  }

  static std::u8string CookedFish(CompoundTag const &in, CompoundTagPtr &out, i16 *damage, Context const &) {
    i16 d = *damage;
    *damage = 0;
    switch (d) {
    case 1:
      return u8"cooked_salmon";
    case 0:
    default:
      return u8"cooked_cod";
    }
  }

  static std::u8string Coral(CompoundTag const &in, CompoundTagPtr &out, i16 *damage, Context const &) {
    i16 d = *damage;
    *damage = 0;
    switch (d) {
    case 1:
      return u8"brain_coral";
    case 2:
      return u8"bubble_coral";
    case 3:
      return u8"fire_coral";
    case 4:
      return u8"horn_coral";
    case 0:
    default:
      return u8"tube_coral";
    }
  }

  static std::u8string CoralBlock(CompoundTag const &in, CompoundTagPtr &out, i16 *damage, Context const &) {
    i16 d = *damage;
    *damage = 0;
    std::u8string prefix = (d & 0x8) == 0x8 ? u8"dead_" : u8"";
    switch (d & 0x7) {
    case 1:
      return prefix + u8"brain_coral_block";
    case 2:
      return prefix + u8"bubble_coral_block";
    case 3:
      return prefix + u8"fire_coral_block";
    case 4:
      return prefix + u8"horn_coral_block";
    case 0:
    default:
      return prefix + u8"tube_coral_block";
    }
  }

  static std::u8string CoralFan(CompoundTag const &in, CompoundTagPtr &out, i16 *damage, Context const &) {
    i16 d = *damage;
    *damage = 0;
    switch (d) {
    case 1:
      return u8"brain_coral_fan";
    case 2:
      return u8"bubble_coral_fan";
    case 3:
      return u8"fire_coral_fan";
    case 4:
      return u8"horn_coral_fan";
    case 0:
    default:
      return u8"tube_coral_fan";
    }
  }

  static std::u8string CoralFanDead(CompoundTag const &in, CompoundTagPtr &out, i16 *damage, Context const &) {
    i16 d = *damage;
    *damage = 0;
    switch (d) {
    case 1:
      return u8"dead_brain_coral_fan";
    case 2:
      return u8"dead_bubble_coral_fan";
    case 3:
      return u8"dead_fire_coral_fan";
    case 4:
      return u8"dead_horn_coral_fan";
    case 0:
    default:
      return u8"dead_tube_coral_fan";
    }
  }

  static std::u8string Dirt(CompoundTag const &in, CompoundTagPtr &out, i16 *damage, Context const &) {
    i16 d = *damage;
    *damage = 0;
    switch (d) {
    case 1:
      return u8"coarse_dirt";
    case 2:
      return u8"podzol";
    case 0:
    default:
      return u8"dirt";
    }
  }

  static std::u8string DoublePlant(CompoundTag const &in, CompoundTagPtr &out, i16 *damage, Context const &) {
    i16 d = *damage;
    *damage = 0;
    switch (d) {
    case 1:
      return u8"lilac";
    case 2:
      return u8"tall_grass";
    case 3:
      return u8"large_fern";
    case 4:
      return u8"rose_bush";
    case 5:
      return u8"peony";
    case 0:
    default:
      return u8"sunflower";
    }
  }

  static std::u8string Dye(CompoundTag const &in, CompoundTagPtr &out, i16 *damage, Context const &) {
    i16 d = *damage;
    *damage = 0;
    switch (d) {
    case 1:
      return u8"red_dye";
    case 2:
      return u8"green_dye";
    case 3:
      return u8"cocoa_beans";
    case 4:
      return u8"lapis_lazuli";
    case 5:
      return u8"purple_dye";
    case 6:
      return u8"cyan_dye";
    case 7:
      return u8"light_gray_dye";
    case 8:
      return u8"gray_dye";
    case 9:
      return u8"pink_dye";
    case 10:
      return u8"lime_dye";
    case 11:
      return u8"yellow_dye";
    case 12:
      return u8"light_blue_dye";
    case 13:
      return u8"magenta_dye";
    case 14:
      return u8"orange_dye";
    case 15:
      return u8"bone_meal";
    case 0:
    default:
      return u8"ink_sac";
    }
  }

  static std::u8string EnchantedBook(CompoundTag const &in, CompoundTagPtr &out, i16 *damage, Context const &) {
    if (auto tagB = in.compoundTag(u8"tag"); tagB) {
      auto tagJ = Compound();
      if (auto storedB = tagB->listTag(u8"StoredEnchantments"); storedB) {
        auto storedJ = List<Tag::Type::Compound>();
        for (auto const &it : *storedB) {
          auto enchB = it->asCompound();
          if (!enchB) {
            continue;
          }
          auto idB = enchB->int16(u8"id");
          if (!idB) {
            continue;
          }
          auto idJ = Enchantments::JavaEnchantmentIdFromBox360(*idB);
          auto lvl = enchB->int16(u8"lvl");
          if (!lvl) {
            continue;
          }
          auto enchJ = Compound();
          enchJ->set(u8"id", idJ);
          enchJ->set(u8"lvl", Short(*lvl));
          storedJ->push_back(enchJ);
        }
        tagJ->set(u8"StoredEnchantments", storedJ);
      }
      if (!tagJ->empty()) {
        out->set(u8"tag", tagJ);
      }
    }
    return u8"";
  }

  static std::u8string FilledMap(CompoundTag const &in, CompoundTagPtr &out, i16 *damage, Context const &) {
    i16 d = *damage;
    *damage = 0;
    auto tagJ = Compound();
    tagJ->set(u8"map", Int(d));
    out->set(u8"tag", tagJ);
    return u8"";
  }

  static std::u8string Fireworks(CompoundTag const &in, CompoundTagPtr &out, i16 *damage, Context const &) {
    if (auto tag = in.compoundTag(u8"tag"); tag) {
      out->set(u8"tag", tag->copy());
    }
    return u8"firework_rocket";
  }

  static std::u8string Fish(CompoundTag const &in, CompoundTagPtr &out, i16 *damage, Context const &) {
    i16 d = *damage;
    *damage = 0;
    switch (d) {
    case 1:
      return u8"salmon";
    case 2:
      return u8"tropical_fish";
    case 3:
      return u8"pufferfish";
    case 0:
    default:
      return u8"cod";
    }
  }

  static std::u8string Leaves(CompoundTag const &in, CompoundTagPtr &out, i16 *damage, Context const &) {
    i16 d = *damage;
    *damage = 0;
    switch (d & 0x7) {
    case 1:
      return u8"spruce_leaves";
    case 2:
      return u8"birch_leaves";
    case 3:
      return u8"jungle_leaves";
    case 0:
    default:
      return u8"oak_leaves";
    }
  }

  static std::u8string Leaves2(CompoundTag const &in, CompoundTagPtr &out, i16 *damage, Context const &) {
    i16 d = *damage;
    *damage = 0;
    switch (d & 0x7) {
    case 1:
      return u8"dark_oak_leaves";
    case 0:
    default:
      return u8"acacia_leaves";
    }
  }

  static std::u8string Log(CompoundTag const &in, CompoundTagPtr &out, i16 *damage, Context const &) {
    i16 d = *damage;
    *damage = 0;
    switch (d) {
    case 1:
      return u8"spruce_log";
    case 2:
      return u8"birch_log";
    case 3:
      return u8"jungle_log";
    case 0:
    default:
      return u8"oak_log";
    }
  }

  static std::u8string Log2(CompoundTag const &in, CompoundTagPtr &out, i16 *damage, Context const &) {
    i16 d = *damage;
    *damage = 0;
    switch (d) {
    case 1:
      return u8"dark_oak_log";
    case 0:
    default:
      return u8"acacia_log";
    }
  }

  static std::u8string MonsterEgg(CompoundTag const &in, CompoundTagPtr &out, i16 *damage, Context const &) {
    i16 d = *damage;
    *damage = 0;
    switch (d) {
    case 1:
      return u8"infested_cobblestone";
    case 2:
      return u8"infested_stone_bricks";
    case 3:
      return u8"infested_mossy_stone_bricks";
    case 4:
      return u8"infested_cracked_stone_bricks";
    case 5:
      return u8"infested_chiseled_stone_bricks";
    case 0:
    default:
      return u8"infested_stone";
    }
  }

  static std::u8string MushroomBlock(CompoundTag const &in, CompoundTagPtr &out, i16 *damage, Context const &) {
    i16 d = *damage;
    *damage = 0;
    switch (d) {
    case 10:
      return u8"mushroom_stem";
    default:
      return u8"";
    }
  }

  static std::u8string Planks(CompoundTag const &in, CompoundTagPtr &out, i16 *damage, Context const &) {
    i16 d = *damage;
    *damage = 0;
    switch (d) {
    case 1:
      return u8"spruce_planks";
    case 2:
      return u8"birch_planks";
    case 3:
      return u8"jungle_planks";
    case 4:
      return u8"acacia_planks";
    case 5:
      return u8"dark_oak_planks";
    case 0:
    default:
      return u8"oak_planks";
    }
  }

  static std::u8string Potion(CompoundTag const &in, CompoundTagPtr &out, i16 *damage, Context const &) {
    std::u8string id;
    if (auto tag = in.compoundTag(u8"tag"); tag) {
      // TU67
      out->set(u8"tag", tag);
    } else {
      // TU25
      i16 d = DrainDamage(damage);
      auto ret = je2be::Potion::JavaPotionTypeAndItemNameFromLegacyJavaDamage(d);
      if (ret) {
        id = Namespace::Remove(ret->fItemName);
        auto t = Compound();
        t->set(u8"Potion", ret->fPotionType);
        out->set(u8"tag", t);
      }
    }
    return id;
  }

  static std::u8string Prismarine(CompoundTag const &in, CompoundTagPtr &out, i16 *damage, Context const &) {
    i16 d = *damage;
    *damage = 0;
    switch (d) {
    case 1:
      return u8"prismarine_bricks";
    case 2:
      return u8"dark_prismarine";
    case 0:
    default:
      return u8"prismarine";
    }
  }

  static std::u8string PrismarineSlab(CompoundTag const &in, CompoundTagPtr &out, i16 *damage, Context const &) {
    i16 d = *damage;
    *damage = 0;
    switch (d) {
    case 1:
      return u8"prismarine_brick_slab";
    case 2:
      return u8"dark_prismarine_slab";
    case 0:
    default:
      return u8"prismarine_slab";
    }
  }

  static std::u8string QuartzBlock(CompoundTag const &in, CompoundTagPtr &out, i16 *damage, Context const &) {
    i16 d = *damage;
    *damage = 0;
    switch (d) {
    case 1:
      return u8"chiseled_quartz_block";
    case 2:
      return u8"quartz_pillar";
    case 0:
    default:
      return u8"quartz_block";
    }
  }

  static std::u8string RedFlower(CompoundTag const &in, CompoundTagPtr &out, i16 *damage, Context const &) {
    i16 d = *damage;
    *damage = 0;
    switch (d) {
    case 1:
      return u8"blue_orchid";
    case 2:
      return u8"allium";
    case 3:
      return u8"azure_bluet";
    case 4:
      return u8"red_tulip";
    case 5:
      return u8"orange_tulip";
    case 6:
      return u8"white_tulip";
    case 7:
      return u8"pink_tulip";
    case 8:
      return u8"oxeye_daisy";
    case 0:
    default:
      return u8"poppy";
    }
  }

  static std::u8string RedSandstone(CompoundTag const &in, CompoundTagPtr &out, i16 *damage, Context const &) {
    i16 d = *damage;
    *damage = 0;
    switch (d) {
    case 1:
      return u8"chiseled_red_sandstone";
    case 2:
      return u8"cut_red_sandstone";
    case 0:
    default:
      return u8"red_sandstone";
    }
  }

  static std::u8string Same(CompoundTag const &in, CompoundTagPtr &out, i16 *damage, Context const &) {
    return u8"";
  }

  static std::u8string Sand(CompoundTag const &in, CompoundTagPtr &out, i16 *damage, Context const &) {
    i16 d = *damage;
    *damage = 0;
    switch (d) {
    case 1:
      return u8"red_sand";
    case 0:
    default:
      return u8"sand";
    }
  }

  static std::u8string Sandstone(CompoundTag const &in, CompoundTagPtr &out, i16 *damage, Context const &) {
    i16 d = *damage;
    *damage = 0;
    switch (d) {
    case 1:
      return u8"chiseled_sandstone";
    case 2:
      return u8"cut_sandstone";
    case 0:
    default:
      return u8"sandstone";
    }
  }

  static std::u8string Sapling(CompoundTag const &in, CompoundTagPtr &out, i16 *damage, Context const &) {
    i16 d = *damage;
    *damage = 0;
    switch (d) {
    case 1:
      return u8"spruce_sapling";
    case 2:
      return u8"birch_sapling";
    case 3:
      return u8"jungle_sapling";
    case 4:
      return u8"acacia_sapling";
    case 5:
      return u8"dark_oak_sapling";
    case 0:
    default:
      return u8"oak_sapling";
    }
  }

  static std::u8string Skull(CompoundTag const &in, CompoundTagPtr &out, i16 *damage, Context const &) {
    i16 d = *damage;
    *damage = 0;
    switch (d) {
    case 1:
      return u8"wither_skeleton_skull";
    case 2:
      return u8"zombie_head";
    case 3:
      return u8"player_head";
    case 4:
      return u8"creeper_head";
    case 5:
      return u8"dragon_head";
    case 0:
    default:
      return u8"skeleton_skull";
    }
  }

  static std::u8string SpawnEgg(CompoundTag const &in, CompoundTagPtr &out, i16 *damage, Context const &ctx) {
    std::u8string name;
    i16 d = DrainDamage(damage);
    switch (d) {
    case 50:
      name = u8"creeper";
      break;
    case 51:
      name = u8"skeleton";
      break;
    case 52:
      name = u8"spider";
      break;
    case 54:
      name = u8"zombie";
      break;
    case 55:
      name = u8"slime";
      break;
    case 56:
      name = u8"ghast";
      break;
    case 57:
      name = u8"zombified_piglin";
      break;
    case 58:
      name = u8"enderman";
      break;
    case 59:
      name = u8"cave_spider";
      break;
    case 60:
      name = u8"silverfish";
      break;
    case 61:
      name = u8"blaze";
      break;
    case 62:
      name = u8"magma_cube";
      break;
    case 65:
      name = u8"bat";
      break;
    case 66:
      name = u8"witch";
      break;
    case 90:
      name = u8"pig";
      break;
    case 91:
      name = u8"sheep";
      break;
    case 92:
      name = u8"cow";
      break;
    case 93:
      name = u8"chicken";
      break;
    case 94:
      name = u8"squid";
      break;
    case 95:
      name = u8"wolf";
      break;
    case 96:
      name = u8"mooshroom";
      break;
    case 98:
      name = u8"ocelot";
      break;
    case 100:
      name = u8"horse";
      break;
    case 120:
      name = u8"villager";
      break;
    case 8292:
      name = u8"donkey";
      break;
    case 12388:
      name = u8"mule";
      break;
    }
    if (auto tag = in.compoundTag(u8"tag"); tag) {
      if (auto entityTag = tag->compoundTag(u8"EntityTag"); entityTag) {
        if (auto id = entityTag->string(u8"id"); id) {
          auto n = ctx.fEntityNameMigrator(*id);
          if (n.starts_with(u8"minecraft:")) {
            name = n.substr(10);
          }
        }
      }
    }
    if (name.empty()) {
      return u8"";
    } else {
      return name + u8"_spawn_egg";
    }
  }

  static std::u8string Sponge(CompoundTag const &in, CompoundTagPtr &out, i16 *damage, Context const &) {
    i16 d = *damage;
    *damage = 0;
    switch (d) {
    case 1:
      return u8"wet_sponge";
    case 0:
    default:
      return u8"sponge";
    }
  }

  static std::u8string Stone(CompoundTag const &in, CompoundTagPtr &out, i16 *damage, Context const &) {
    i16 d = *damage;
    *damage = 0;
    switch (d) {
    case 1:
      return u8"granite";
    case 2:
      return u8"polished_granite";
    case 3:
      return u8"diorite";
    case 4:
      return u8"polished_diorite";
    case 5:
      return u8"andesite";
    case 6:
      return u8"polished_andesite";
    case 0:
    default:
      return u8"stone";
    }
  }

  static std::u8string Stonebrick(CompoundTag const &in, CompoundTagPtr &out, i16 *damage, Context const &) {
    i16 d = *damage;
    *damage = 0;
    switch (d) {
    case 1:
      return u8"mossy_stone_bricks";
    case 2:
      return u8"cracked_stone_bricks";
    case 3:
      return u8"chiseled_stone_bricks";
    case 0:
    default:
      return u8"stone_bricks";
    }
  }

  static std::u8string StoneSlab(CompoundTag const &in, CompoundTagPtr &out, i16 *damage, Context const &) {
    i16 d = *damage;
    *damage = 0;
    switch (d) {
    case 1:
      return u8"sandstone_slab";
    case 2:
      return u8"oak_slab";
    case 3:
      return u8"cobblestone_slab";
    case 4:
      return u8"brick_slab";
    case 5:
      return u8"stone_brick_slab";
    case 6:
      return u8"nether_brick_slab";
    case 7:
      return u8"quartz_slab";
    case 0:
    default:
      return u8"smooth_stone_slab";
    }
  }

  static std::u8string StoneStairs(CompoundTag const &in, CompoundTagPtr &out, i16 *damage, Context const &) {
    i16 d = *damage;
    *damage = 0;
    switch (d) {
    case 2:
    default:
      return u8"cobblestone_stairs";
    }
  }

  static std::u8string Tallgrass(CompoundTag const &in, CompoundTagPtr &out, i16 *damage, Context const &) {
    i16 d = *damage;
    *damage = 0;
    switch (d) {
    case 2:
      return u8"fern";
    case 0:
      return u8"grass"; // shrub
    case 1:
    default:
      return u8"grass";
    }
  }

  static std::u8string WoodenSlab(CompoundTag const &in, CompoundTagPtr &out, i16 *damage, Context const &) {
    i16 d = *damage;
    *damage = 0;
    switch (d) {
    case 1:
      return u8"spruce_slab";
    case 2:
      return u8"birch_slab";
    case 3:
      return u8"jungle_slab";
    case 4:
      return u8"acacia_slab";
    case 5:
      return u8"dark_oak_slab";
    case 0:
    default:
      return u8"oak_slab";
    }
  }
#pragma endregion

#pragma region Converter_Generator
  static Converter Colored(std::u8string const &suffix) {
    return [suffix](CompoundTag const &in, CompoundTagPtr &out, i16 *damage, Context const &ctx) {
      i16 d = *damage;
      *damage = 0;
      auto colorName = JavaNameFromColorCodeJava(static_cast<ColorCodeJava>(d));
      return colorName + u8"_" + suffix;
    };
  }

  static Converter Rename(std::u8string const &name) {
    return [name](CompoundTag const &in, CompoundTagPtr &out, i16 *damage, Context const &ctx) {
      return name;
    };
  }
#pragma endregion

  static std::optional<std::u8string> MigrateId(u16 id) {
    using namespace std;
    static unique_ptr<vector<u8string> const> const sTable(CreateIdTable());
    if (sTable->size() <= id) {
      return nullopt;
    }
    u8string mapped = (*sTable)[id];
    if (mapped.empty()) {
      return nullopt;
    }
    return u8"minecraft:" + mapped;
  }

  static i16 DrainDamage(i16 *damage) {
    i16 d = *damage;
    *damage = 0;
    return d;
  }

  static std::vector<std::u8string> const *CreateIdTable() {
    using namespace std;
    int maxId = -1;
    unordered_map<u16, u8string> t;

#define R(id, name)              \
  assert(t.count(id) == 0);      \
  maxId = (std::max)(maxId, id); \
  t[id] = u8"" #name

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
    R(95, stained_glass);
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
    R(123, redstone_lamp);
    R(126, wooden_slab);
    R(128, sandstone_stairs);
    R(129, emerald_ore);
    R(130, ender_chest);
    R(131, tripwire_hook);
    R(133, emerald_block);
    R(134, spruce_stairs);
    R(135, birch_stairs);
    R(136, jungle_stairs);
    R(138, beacon);
    R(139, cobblestone_wall);
    R(143, wooden_button);
    R(145, anvil);
    R(146, trapped_chest);
    R(147, light_weighted_pressure_plate);
    R(148, heavy_weighted_pressure_plate);
    R(151, daylight_detector);
    R(152, redstone_block);
    R(153, quartz_ore);
    R(155, quartz_block);
    R(156, quartz_stairs);
    R(157, activator_rail);
    R(158, dropper);
    R(154, hopper);
    R(159, stained_hardened_clay);
    R(160, stained_glass_pane);
    R(161, log2);
    R(163, acacia_stairs);
    R(164, dark_oak_stairs);
    R(167, iron_trapdoor);
    R(170, hay_block);
    R(171, carpet);
    R(172, hardened_clay);
    R(173, coal_block);
    R(183, spruce_fence_gate);
    R(184, birch_fence_gate);
    R(185, jungle_fence_gate);
    R(186, acacia_fence_gate);
    R(187, dark_oak_fence_gate);
    R(188, spruce_fence);
    R(189, birch_fence);
    R(190, jungle_fence);
    R(191, acacia_fence);
    R(192, dark_oak_fence);
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
    R(386, writable_book);
    R(388, emerald);
    R(389, item_frame);
    R(390, flower_pot);
    R(391, carrot);
    R(392, potato);
    R(393, baked_potato);
    R(394, poisonous_potato);
    R(395, map); // empty map
    R(396, golden_carrot);
    R(397, skull);
    R(398, carrot_on_a_stick);
    R(399, nether_star);
    R(400, pumpkin_pie);
    R(401, fireworks);
    R(403, enchanted_book);
    R(404, comparator);
    R(405, netherbrick);
    R(406, quartz);
    R(407, tnt_minecart);
    R(408, hopper_minecart);
    R(417, iron_horse_armor);
    R(418, golden_horse_armor);
    R(419, diamond_horse_armor);
    R(420, lead);
    R(421, name_tag);
    R(427, spruce_door);
    R(428, birch_door);
    R(429, jungle_door);
    R(430, acacia_door);
    R(431, dark_oak_door);
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

    auto ret = new vector<u8string>();
    ret->resize(maxId + 1);
    for (auto [id, name] : t) {
      (*ret)[id] = name;
    }
    return ret;
  }

  static std::u8string GetTileEntityNameFromItemName(std::u8string const &name) {
    if (name.find(u8"shulker_box") != std::u8string::npos) {
      return u8"shulker_box";
    }
    return name;
  }

  static std::unordered_map<std::u8string, Converter> const &GetTable() {
    static std::unique_ptr<std::unordered_map<std::u8string, Converter> const> const sTable(CreateTable());
    return *sTable;
  }

  static std::unordered_map<std::u8string, Converter> const *CreateTable() {
    using namespace std;
    auto ret = new unordered_map<u8string, Converter>();
#define E(__name, __conv)                        \
  assert(ret->find(u8"" #__name) == ret->end()); \
  ret->try_emplace(u8"" #__name, __conv)

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
    E(stained_hardened_clay, Colored(u8"terracotta"));
    E(sapling, Sapling);
    E(sponge, Sponge);
    E(skull, Skull);
    E(leaves, Leaves);
    E(leaves2, Leaves2);
    E(tallgrass, Tallgrass);
    E(double_plant, DoublePlant);
    E(red_flower, RedFlower);
    E(concrete, Colored(u8"concrete"));
    E(cobblestone_wall, CobblestoneWall);
    E(concrete_powder, Colored(u8"concrete_powder"));
    E(silver_glazed_terracotta, Rename(u8"light_gray_glazed_terracotta"));
    E(banner, Banner);
    E(wool, Colored(u8"wool"));
    E(carpet, Colored(u8"carpet"));
    E(stained_glass, Colored(u8"stained_glass"));
    E(stained_glass_pane, Colored(u8"stained_glass_pane"));
    E(coral_fan, CoralFan);
    E(coral, Coral);
    E(brown_mushroom_block, MushroomBlock);
    E(red_mushroom_block, MushroomBlock);
    E(coral_fan_dead, CoralFanDead);
    E(coral_block, CoralBlock);
    E(melon_block, Rename(u8"melon"));
    E(lit_pumpkin, Rename(u8"jack_o_lantern"));
    E(waterlily, Rename(u8"lily_pad"));
    E(deadbush, Rename(u8"dead_bush"));
    E(yellow_flower, Rename(u8"dandelion"));
    E(snow_layer, Rename(u8"snow"));
    E(web, Rename(u8"cobweb"));
    E(sign, Rename(u8"oak_sign"));
    E(sea_grass, Rename(u8"seagrass"));
    E(grass_path, Same);
    E(quartz_ore, Rename(u8"nether_quartz_ore"));
    E(stripped_log_oak, Rename(u8"stripped_oak_log"));
    E(stripped_log_spruce, Rename(u8"stripped_spruce_log"));
    E(stripped_log_birch, Rename(u8"stripped_birch_log"));
    E(stripped_log_jungle, Rename(u8"stripped_jungle_log"));
    E(stripped_log_acacia, Rename(u8"stripped_acacia_log"));
    E(stripped_log_dark_oak, Rename(u8"stripped_dark_oak_log"));
    E(brick_block, Rename(u8"bricks"));
    E(magma, Rename(u8"magma_block"));
    E(slime, Rename(u8"slime_block"));
    E(fence, Rename(u8"oak_fence"));
    E(nether_brick, Rename(u8"nether_bricks"));
    E(red_nether_brick, Rename(u8"red_nether_bricks"));
    E(end_bricks, Rename(u8"end_stone_bricks"));
    E(trapdoor, Rename(u8"oak_trapdoor"));
    E(wooden_door, Rename(u8"oak_door"));
    E(stone_slab2, Rename(u8"red_sandstone_slab"));
    E(prismarine_bricks_stairs, Rename(u8"prismarine_brick_stairs"));
    E(hardened_clay, Rename(u8"terracotta"));
    E(fence_gate, Rename(u8"oak_fence_gate"));
    E(golden_rail, Rename(u8"powered_rail"));
    E(boat, Rename(u8"oak_boat"));
    E(noteblock, Rename(u8"note_block"));
    E(wooden_button, Rename(u8"oak_button"));
    E(wooden_pressure_plate, Rename(u8"oak_pressure_plate"));
    E(netherbrick, Rename(u8"nether_brick"));
    E(nautilus, Rename(u8"nautilus_shell"));
    E(nautilus_core, Rename(u8"heart_of_the_sea"));
    E(reeds, Rename(u8"sugar_cane"));
    E(chorus_fruit_popped, Rename(u8"popped_chorus_fruit"));
    E(turtle_shell_piece, Rename(u8"scute"));
    E(dye, Dye);
    E(cooked_fish, CookedFish);
    E(fish, Fish);
    E(speckled_melon, Rename(u8"glistering_melon_slice"));
    E(potion, Potion);
    E(lingering_potion, Potion);
    E(splash_potion, Potion);
    E(enchanted_book, EnchantedBook);
    E(tipped_arrow, Potion);
    E(silver_shulker_box, Rename(u8"light_gray_shulker_box"));
    E(bed, Colored(u8"bed"));
    E(fish_bucket, Rename(u8"cod_bucket"));
    E(puffer_bucket, Rename(u8"pufferfish_bucket"));
    E(tropical_bucket, Rename(u8"tropical_fish_bucket"));
    E(mob_spawner, Rename(u8"spawner"));
    E(spawn_egg, SpawnEgg);
    E(record_13, Rename(u8"music_disc_13"));
    E(record_cat, Rename(u8"music_disc_cat"));
    E(record_blocks, Rename(u8"music_disc_blocks"));
    E(record_chirp, Rename(u8"music_disc_chirp"));
    E(record_far, Rename(u8"music_disc_far"));
    E(record_mall, Rename(u8"music_disc_mall"));
    E(record_mellohi, Rename(u8"music_disc_mellohi"));
    E(record_stal, Rename(u8"music_disc_stal"));
    E(record_strad, Rename(u8"music_disc_strad"));
    E(record_ward, Rename(u8"music_disc_ward"));
    E(record_11, Rename(u8"music_disc_11"));
    E(record_wait, Rename(u8"music_disc_wait"));
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
