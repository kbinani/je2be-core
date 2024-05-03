#include "bedrock/_item.hpp"

#include "_namespace.hpp"
#include "_props.hpp"
#include "bedrock/_block-data.hpp"
#include "bedrock/_block-entity.hpp"
#include "bedrock/_context.hpp"
#include "entity/_axolotl.hpp"
#include "entity/_tropical-fish.hpp"
#include "enums/_effect.hpp"
#include "enums/_skull-type.hpp"
#include "item/_banner.hpp"
#include "item/_enchantments.hpp"
#include "item/_fireworks.hpp"
#include "item/_goat-horn.hpp"
#include "item/_map-type.hpp"
#include "item/_potion.hpp"
#include "item/_tipped-arrow-potion.hpp"
#include "java/_block-data.hpp"
#include "java/_components.hpp"
#include "java/_versions.hpp"

namespace je2be::bedrock {

class Item::Impl {
  Impl() = delete;

public:
  static CompoundTagPtr From(CompoundTag const &tagB, Context &ctx, int dataVersion, Options const &opt) {
    using namespace std;
    auto name = tagB.string(u8"Name");
    if (!name) {
      return nullptr;
    }
    static unique_ptr<unordered_map<u8string_view, Converter> const> const sTable(CreateTable());
    auto ret = Compound();
    if (name == u8"") {
      return ret;
    }
    u8string_view key(*name);
    auto found = sTable->find(Namespace::Remove(key));
    if (found == sTable->end()) {
      Default(*name, tagB, *ret, ctx, dataVersion);
    } else {
      auto renamed = found->second(*name, tagB, *ret, ctx, dataVersion, opt);
      Default(renamed, tagB, *ret, ctx, dataVersion);
    }
    return ret;
  }

  static void Default(std::u8string const &nameB, CompoundTag const &itemB, CompoundTag &itemJ, Context &ctx, int dataVersion) {
    using namespace std;
    using namespace mcfile::blocks::minecraft;

    auto tagB = itemB.compoundTag(u8"tag");
    if (!tagB) {
      tagB = Compound();
    }

    auto componentsJ = itemJ.compoundTag(u8"components");
    if (!componentsJ) {
      componentsJ = Compound();
    }

    u8string nameJ = nameB;
    auto blockTag = itemB.compoundTag(u8"Block");
    if (nameB.ends_with(u8"sign") && !tagB->empty()) {
      // NOTE: "tag" of standing_sign should be converted to "BlockEntityTag", but itemB doesn't have "Block" tag.
      //  Therefore, we need a dummy blockTag here.
      blockTag = Compound();
      blockTag->set(u8"name", u8"minecraft:standing_sign");
      blockTag->set(u8"version", Int(java::kBlockDataVersion));
    }
    if (blockTag) {
      auto blockB = mcfile::be::Block::FromCompound(*blockTag);
      if (blockB) {
        auto blockJ = BlockData::From(*blockB, dataVersion);
        if (blockJ) {
          nameJ = blockJ->fName;

          Pos3i dummy(0, 0, 0);
          if (!tagB->empty()) {
            if (auto converted = BlockEntity::FromBlockAndBlockEntity(dummy, *blockB, *tagB, *blockJ, ctx, dataVersion); converted && converted->fTileEntity) {
              static unordered_set<u8string> const sExclude({u8"x", u8"y", u8"z", u8"keepPacked", u8"RecipesUsed"});
              auto blockEntityDataJ = converted->fTileEntity;
              for (auto const &e : sExclude) {
                blockEntityDataJ->erase(e);
              }
              if (!blockEntityDataJ->empty()) {
                componentsJ->set(u8"minecraft:block_entity_data", blockEntityDataJ);
              }
            }
          }

          if (!ctx.fDataPackUpdate1_21) {
            if (java::BlockData::IsUpdate121Block(blockJ->fId)) {
              ctx.fDataPackUpdate1_21 = true;
            }
          }
        }
      }
    }
    if (!ctx.fDataPackBundle) {
      if (nameJ == u8"minecraft:bundle") {
        ctx.fDataPackBundle = true;
      }
    }
    if (!ctx.fDataPackUpdate1_21) {
      if (nameJ == u8"minecraft:trial_key") {
        ctx.fDataPackUpdate1_21 = true;
      }
    }
    itemJ.set(u8"id", nameJ);

    if (auto countB = itemB.byte(u8"Count"); countB) {
      itemJ[u8"count"] = Int(*countB);
    }
    CopyByteValues(itemB, itemJ, {{u8"Slot"}});
    CopyIntValues(*tagB, *componentsJ, {{u8"Damage", u8"minecraft:damage"}, {u8"RepairCost", u8"minecraft:repair_cost"}});

    auto customColor = tagB->int32(u8"customColor");
    if (customColor) {
      auto dyedColor = Compound();
      dyedColor->set(u8"rgb", Int(RgbFromCustomColor(*customColor)));
      java::AppendComponent(itemJ, u8"dyed_color", dyedColor);
    }

    auto displayB = tagB->compoundTag(u8"display");
    if (displayB) {
      auto displayName = displayB->string(u8"Name");
      if (displayName) {
        props::Json json;
        props::SetJsonString(json, u8"text", *displayName);
        componentsJ->set(u8"minecraft:custom_name", props::StringFromJson(json));
      }

      if (auto loreB = displayB->listTag(u8"Lore"); loreB) {
        auto loreJ = List<Tag::Type::String>();
        for (auto const &it : *loreB) {
          if (auto item = it->asString(); item) {
            if (item->fValue == u8"(+DATA)") {
              loreJ->push_back(String(u8"\"(+NBT)\""));
            } else {
              loreJ->push_back(String(item->fValue));
            }
          }
        }
        componentsJ->set(u8"minecraft:lore", loreJ);
      }
    }

    auto customName = tagB->string(u8"CustomName");
    auto customNameVisible = tagB->boolean(u8"CustomNameVisible", false);
    if (customName && customNameVisible) {
      props::Json json;
      props::SetJsonString(json, u8"text", *customName);
      componentsJ->set(u8"minecraft:custom_name", props::StringFromJson(json));
    }

    auto enchB = tagB->listTag(u8"ench");
    if (enchB) {
      auto levelsJ = Compound();
      for (auto const &it : *enchB) {
        auto cB = it->asCompound();
        if (!cB) {
          continue;
        }
        auto idB = cB->int16(u8"id");
        auto lvlB = cB->int16(u8"lvl");
        if (idB && lvlB) {
          auto idJ = Enchantments::JavaEnchantmentIdFromBedrock(*idB);
          levelsJ->set(idJ, Int(*lvlB));
        }
      }
      if (!levelsJ->empty()) {
        auto enchJ = Compound();
        enchJ->set(u8"levels", levelsJ);
        if (nameB == u8"minecraft:enchanted_book") {
          componentsJ->set(u8"minecraft:stored_enchantments", enchJ);
        } else {
          componentsJ->set(u8"minecraft:enchantments", enchJ);
        }
      }
    }

    if (auto trimB = tagB->compoundTag(u8"Trim"); trimB) {
      auto materialB = trimB->string(u8"Material");
      auto patternB = trimB->string(u8"Pattern");
      auto trimJ = Compound();
      trimJ->set(u8"material", Namespace::Add(*materialB));
      trimJ->set(u8"pattern", Namespace::Add(*patternB));
      componentsJ->set(u8"minecraft:trim", trimJ);
    }

    if (!componentsJ->empty()) {
      itemJ[u8"components"] = componentsJ;
    }
  }

#pragma region Converters
  static std::u8string Arrow(std::u8string const &nameB, CompoundTag const &itemB, CompoundTag &itemJ, Context &ctx, int dataVersion, Options const &opt) {
    auto damage = itemB.int16(u8"Damage", 0);
    if (damage == 0) {
      return nameB;
    } else {
      auto potionJ = TippedArrowPotion::JavaPotionType(damage);
      auto tagJ = Compound();
      tagJ->set(u8"potion", potionJ);
      java::AppendComponent(itemJ, u8"potion_contents", tagJ);
      return Ns() + u8"tipped_arrow";
    }
  }

  static std::u8string AxolotlBucket(std::u8string const &nameB, CompoundTag const &itemB, CompoundTag &itemJ, Context &ctx, int dataVersion, Options const &opt) {
    auto tagB = itemB.compoundTag(u8"tag");
    if (tagB) {
      auto tagJ = Compound();

      CopyIntValues(*tagB, *tagJ, {{u8"Age", u8"Age", 0}});

      auto attributes = tagB->listTag(u8"Attributes");
      if (attributes) {
        for (auto const &it : *attributes) {
          auto c = it->asCompound();
          if (!c) {
            continue;
          }
          auto name = c->string(u8"Name");
          if (name != u8"minecraft:health") {
            continue;
          }
          auto current = c->float32(u8"Current");
          if (!current) {
            continue;
          }
          tagJ->set(u8"Health", Float(*current));
          break;
        }
      }

      auto variantB = tagB->int32(u8"Variant", 0);
      tagJ->set(u8"Variant", Int(Axolotl::JavaVariantFromBedrockVariant(variantB)));

      itemJ[u8"tag"] = tagJ;
    }
    return nameB;
  }

  static std::u8string Banner(std::u8string const &nameB, CompoundTag const &itemB, CompoundTag &itemJ, Context &ctx, int dataVersion, Options const &opt) {
    auto damageB = itemB.int16(u8"Damage", 0);
    BannerColorCodeBedrock colorB = static_cast<BannerColorCodeBedrock>(damageB);
    ColorCodeJava colorJ = ColorCodeJavaFromBannerColorCodeBedrock(colorB);
    std::u8string colorNameJ = JavaNameFromColorCodeJava(colorJ);
    std::u8string nameJ = u8"minecraft:" + colorNameJ + u8"_banner";

    auto tagB = itemB.compoundTag(u8"tag");
    if (tagB) {
      auto typeB = tagB->int32(u8"Type");
      bool ominous = typeB == 1;
      if (ominous) {
        nameJ = u8"minecraft:white_banner";

        auto bannerPatterns = Banner::OminousBannerPatterns();
        java::AppendComponent(itemJ, u8"banner_patterns", bannerPatterns);

        props::Json json;
        json["color"] = "gold";
        json["translate"] = "block.minecraft.ominous_banner";
        java::AppendComponent(itemJ, u8"item_name", String(props::StringFromJson(json)));

        java::AppendComponent(itemJ, u8"hide_additional_tooltip", Compound());
      } else {
        auto patternsB = tagB->listTag(u8"Patterns");
        if (patternsB) {
          auto patternsJ = List<Tag::Type::Compound>();
          for (auto const &it : *patternsB) {
            auto patternB = it->asCompound();
            if (!patternB) {
              continue;
            }
            auto patternColorB = patternB->int32(u8"Color");
            auto patternStringB = patternB->string(u8"Pattern");
            if (!patternColorB || !patternStringB) {
              continue;
            }
            auto patternJ = Compound();
            BannerColorCodeBedrock bccb = static_cast<BannerColorCodeBedrock>(*patternColorB);
            ColorCodeJava ccj = ColorCodeJavaFromBannerColorCodeBedrock(bccb);
            patternJ->set(u8"color", String(JavaNameFromColorCodeJava(ccj)));
            patternJ->set(u8"pattern", *patternStringB);

            patternsJ->push_back(patternJ);
          }
          if (!patternsJ->empty()) {
            java::AppendComponent(itemJ, u8"banner_patterns", patternsJ);
          }
        }
      }
    }
    return nameJ;
  }

  static std::u8string Bed(std::u8string const &name, CompoundTag const &itemB, CompoundTag &itemJ, Context &ctx, int dataVersion, Options const &opt) {
    auto damage = itemB.int16(u8"Damage", 0);
    ColorCodeJava ccj = static_cast<ColorCodeJava>(damage);
    return u8"minecraft:" + JavaNameFromColorCodeJava(ccj) + u8"_bed";
  }

  static std::u8string Book(std::u8string const &name, CompoundTag const &itemB, CompoundTag &itemJ, Context &ctx, int dataVersion, Options const &opt) {
    using namespace std;
    auto tagB = itemB.compoundTag(u8"tag");
    if (!tagB) {
      return name;
    }
    auto tagJ = Compound();
    auto pagesB = tagB->listTag(u8"pages");
    if (pagesB) {
      auto pagesJ = List<Tag::Type::String>();
      for (auto const &pageB : *pagesB) {
        auto const *c = pageB->asCompound();
        if (!c) {
          continue;
        }
        auto text = c->string(u8"text");
        if (!text) {
          continue;
        }
        if (name == u8"minecraft:written_book") {
          props::Json json;
          props::SetJsonString(json, u8"text", *text);
          u8string jsonString = props::StringFromJson(json);
          pagesJ->push_back(String(jsonString));
        } else {
          pagesJ->push_back(String(*text));
        }
      }
      tagJ->set(u8"pages", pagesJ);
    }
    auto author = tagB->string(u8"author");
    if (author) {
      tagJ->set(u8"author", *author);
    }
    auto title = tagB->string(u8"title");
    if (title) {
      tagJ->set(u8"title", *title);
    }
    itemJ.set(u8"tag", tagJ);
    return name;
  }

  static std::u8string Bucket(std::u8string const &name, CompoundTag const &itemB, CompoundTag &itemJ, Context &ctx, int dataVersion, Options const &opt) {
    auto damage = itemB.int16(u8"Damage", 0);
    std::u8string prefix = u8"";
    switch (damage) {
    case 1:
      prefix = u8"milk_";
      break;
    case 2:
      prefix = u8"cod_";
      break;
    case 3:
      prefix = u8"salmon_";
      break;
    case 5:
      prefix = u8"pufferfish_";
      break;
    case 10:
      prefix = u8"lava_";
      break;
    case 4: {
      prefix = u8"tropical_fish_";
      auto tagB = itemB.compoundTag(u8"tag");
      if (tagB) {
        TropicalFish tf = TropicalFish::FromBedrockBucketTag(*tagB);
        auto tagJ = Compound();
        tagJ->set(u8"BucketVariantTag", Int(tf.toJavaVariant()));
        itemJ[u8"tag"] = tagJ;
      }
      break;
    }
    }
    return u8"minecraft:" + prefix + u8"bucket";
  }

  static std::u8string Crossbow(std::u8string const &name, CompoundTag const &itemB, CompoundTag &itemJ, Context &ctx, int dataVersion, Options const &opt) {
    if (auto tagB = itemB.compoundTag(u8"tag"); tagB) {
      auto tagJ = Compound();
      auto chargedProjectiles = List<Tag::Type::Compound>();
      if (auto chargedItem = tagB->compoundTag(u8"chargedItem"); chargedItem) {
        if (auto projectileJ = From(*chargedItem, ctx, dataVersion, opt); projectileJ) {
          chargedProjectiles->push_back(projectileJ);
        }
        tagJ->set(u8"ChargedProjectiles", chargedProjectiles);
        tagJ->set(u8"Charged", Bool(!chargedProjectiles->empty()));
        itemJ[u8"tag"] = tagJ;
      }
    }
    return name;
  }

  static std::u8string FireworkRocket(std::u8string const &name, CompoundTag const &itemB, CompoundTag &itemJ, Context &ctx, int dataVersion, Options const &opt) {
    auto tagB = itemB.compoundTag(u8"tag");
    if (tagB) {
      auto fireworksB = tagB->compoundTag(u8"Fireworks");
      if (fireworksB) {
        FireworksData data = FireworksData::FromBedrock(*fireworksB);
        auto fireworksJ = data.toJavaCompoundTag();
        java::AppendComponent(itemJ, u8"fireworks", fireworksJ);
      }
    }
    return u8"minecraft:firework_rocket";
  }

  static std::u8string FireworkStar(std::u8string const &name, CompoundTag const &itemB, CompoundTag &itemJ, Context &ctx, int dataVersion, Options const &opt) {
    auto tagB = itemB.compoundTag(u8"tag");
    if (tagB) {
      tagB->erase(u8"customColor");
      auto fireworksB = tagB->compoundTag(u8"FireworksItem");
      if (fireworksB) {
        FireworksExplosion explosion = FireworksExplosion::FromBedrock(*fireworksB);
        auto explosionJ = explosion.toJavaCompoundTag();
        java::AppendComponent(itemJ, u8"firework_explosion", explosionJ);
      }
    }
    return u8"minecraft:firework_star";
  }

  static std::u8string GoatHorn(std::u8string const &name, CompoundTag const &itemB, CompoundTag &itemJ, Context &ctx, int dataVersion, Options const &opt) {
    auto damage = itemB.int16(u8"Damage", 0);
    auto instrument = GoatHorn::JavaInstrumentFromBedrockDamage(damage);
    auto tagJ = Compound();
    tagJ->set(u8"instrument", instrument);
    itemJ[u8"tag"] = tagJ;
    return name;
  }

  static std::u8string LegacyBannerPattern(std::u8string const &name, CompoundTag const &itemB, CompoundTag &itemJ, Context &ctx, int dataVersion, Options const &opt) {
    auto damage = itemB.int16(u8"Damage", 0);
    std::u8string prefix;
    switch (damage) {
    case 2:
      prefix = u8"flower_";
      break;
    case 1:
      prefix = u8"creeper_";
      break;
    case 6:
      prefix = u8"piglin_";
      break;
    case 3:
      prefix = u8"mojang_";
      break;
    case 0:
    default:
      prefix = u8"skull_";
      break;
    }
    return Ns() + prefix + u8"banner_pattern";
  }

  static std::u8string LegacyBoat(std::u8string const &name, CompoundTag const &itemB, CompoundTag &itemJ, Context &ctx, int dataVersion, Options const &opt) {
    auto damage = itemB.int16(u8"Damage", 0);
    std::u8string type = u8"oak";
    switch (damage) {
    case 1:
      type = u8"spruce";
      break;
    case 2:
      type = u8"birch";
      break;
    case 3:
      type = u8"jungle";
      break;
    case 4:
      type = u8"acacia";
      break;
    case 5:
      type = u8"dark_oak";
      break;
    case 0:
    default:
      type = u8"oak";
      break;
    }
    return u8"minecraft:" + type + u8"_boat";
  }

  static std::u8string LegacyDye(std::u8string const &name, CompoundTag const &itemB, CompoundTag &itemJ, Context &ctx, int dataVersion, Options const &opt) {
    auto damage = itemB.int16(u8"Damage", 0);
    switch (damage) {
    case 1:
      return Ns() + u8"red_dye";
    case 2:
      return Ns() + u8"green_dye";
    case 3:
      return Ns() + u8"cocoa_beans";
    case 4:
      return Ns() + u8"lapis_lazuli";
    case 5:
      return Ns() + u8"purple_dye";
    case 6:
      return Ns() + u8"cyan_dye";
    case 7:
      return Ns() + u8"light_gray_dye";
    case 8:
      return Ns() + u8"gray_dye";
    case 9:
      return Ns() + u8"pink_dye";
    case 10:
      return Ns() + u8"lime_dye";
    case 11:
      return Ns() + u8"yellow_dye";
    case 12:
      return Ns() + u8"light_blue_dye";
    case 13:
      return Ns() + u8"magenta_dye";
    case 14:
      return Ns() + u8"orange_dye";
    case 15:
      return Ns() + u8"bone_meal";
    case 16:
      return Ns() + u8"black_dye";
    case 17:
      return Ns() + u8"brown_dye";
    case 18:
      return Ns() + u8"blue_dye";
    case 19:
      return Ns() + u8"white_dye";
    case 0:
    default:
      return Ns() + u8"ink_sac";
    }
  }

  static std::u8string LegacySpawnEgg(std::u8string const &name, CompoundTag const &itemB, CompoundTag &itemJ, Context &ctx, int dataVersion, Options const &opt) {
    auto damage = itemB.int16(u8"Damage", 0);
    std::u8string prefix;
    switch (damage) {
    case 122:
      prefix = u8"bee_";
      break;
    case 10:
      prefix = u8"chicken_";
      break;
    case 16:
      prefix = u8"mooshroom_";
      break;
    case 28:
      prefix = u8"polar_bear_";
      break;
    case 11:
      prefix = u8"cow_";
      break;
    case 12:
      prefix = u8"pig_";
      break;
    case 13:
      prefix = u8"sheep_";
      break;
    case 14:
      prefix = u8"wolf_";
      break;
    case 22:
      prefix = u8"ocelot_";
      break;
    case 75:
      prefix = u8"cat_";
      break;
    case 30:
      prefix = u8"parrot_";
      break;
    case 19:
      prefix = u8"bat_";
      break;
    case 74:
      prefix = u8"turtle_";
      break;
    case 25:
      prefix = u8"mule_";
      break;
    case 18:
      prefix = u8"rabbit_";
      break;
    case 29:
      prefix = u8"llama_";
      break;
    case 23:
      prefix = u8"horse_";
      break;
    case 24:
      prefix = u8"donkey_";
      break;
    case 26:
      prefix = u8"skeleton_horse_";
      break;
    case 27:
      prefix = u8"zombie_horse_";
      break;
    case 121:
      prefix = u8"fox_";
      break;
    case 113:
      prefix = u8"panda_";
      break;
    case 31:
      prefix = u8"dolphin_";
      break;
    case 108:
      prefix = u8"pufferfish_";
      break;
    case 109:
      prefix = u8"salmon_";
      break;
    case 112:
      prefix = u8"cod_";
      break;
    case 38:
      prefix = u8"enderman_";
      break;
    case 33:
      prefix = u8"creeper_";
      break;
    case 39:
      prefix = u8"silverfish_";
      break;
    case 34:
      prefix = u8"skeleton_";
      break;
    case 45:
      prefix = u8"witch_";
      break;
    case 37:
      prefix = u8"slime_";
      break;
    case 48:
      prefix = u8"wither_skeleton_";
      break;
    case 46:
      prefix = u8"stray_";
      break;
    case 35:
      prefix = u8"spider_";
      break;
    case 32:
      prefix = u8"zombie_";
      break;
    case 50:
      prefix = u8"elder_guardian_";
      break;
    case 49:
      prefix = u8"guardian_";
      break;
    case 40:
      prefix = u8"cave_spider_";
      break;
    case 17:
      prefix = u8"squid_";
      break;
    case 110:
      prefix = u8"drowned_";
      break;
    case 47:
      prefix = u8"husk_";
      break;
    case 42:
      prefix = u8"magma_cube_";
      break;
    case 55:
      prefix = u8"endermite_";
      break;
    case 118:
      prefix = u8"wandering_trader_";
      break;
    case 127:
      prefix = u8"piglin_brute_";
      break;
    case 125:
      prefix = u8"strider_";
      break;
    case 124:
      prefix = u8"hoglin_";
      break;
    case 123:
      prefix = u8"piglin_";
      break;
    case 126:
      prefix = u8"zoglin_";
      break;
    case 41:
      prefix = u8"ghast_";
      break;
    case 43:
      prefix = u8"blaze_";
      break;
    case 58:
      prefix = u8"phantom_";
      break;
    case 105:
      prefix = u8"vex_";
      break;
    case 57:
      prefix = u8"vindicator_";
      break;
    case 54:
      prefix = u8"shulker_";
      break;
    case 59:
      prefix = u8"ravager_";
      break;
    case 114:
      prefix = u8"pillager_";
      break;
    case 36:
      prefix = u8"zombified_piglin_";
      break;
    case 111:
      prefix = u8"tropical_fish_";
      break;
    case 115:
      prefix = u8"villager_";
      break;
    case 104:
      prefix = u8"evoker_";
      break;
    case 116:
      prefix = u8"zombie_villager_";
      break;
    }
    return u8"minecraft:" + prefix + u8"spawn_egg";
  }

  static std::u8string Map(std::u8string const &name, CompoundTag const &itemB, CompoundTag &itemJ, Context &ctx, int dataVersion, Options const &opt) {
    auto tagB = itemB.compoundTag(u8"tag");
    if (!tagB) {
      return name;
    }
    auto uuid = tagB->int64(u8"map_uuid");
    if (!uuid) {
      return name;
    }

    auto damage = itemB.int16(u8"Damage", 0);
    if (auto translate = MapType::JavaTranslationKeyFromBedrockDamage(damage); translate) {
      props::Json json;
      props::SetJsonString(json, u8"translate", *translate);
      java::AppendComponent(itemJ, u8"item_name", String(props::StringFromJson(json)));

      if (auto mapColor = MapType::JavaMapColorFromBedrockDamage(damage); mapColor) {
        java::AppendComponent(itemJ, u8"map_color", Int(*mapColor));
      }
    }

    auto info = ctx.mapFromUuid(*uuid);
    if (info) {
      java::AppendComponent(itemJ, u8"map_id", Int(info->fNumber));
      ctx.markMapUuidAsUsed(*uuid);

      if (damage != 0 && !info->fDecorations.empty()) {
        auto decorationsJ = Compound();
        for (MapInfo::Decoration const &decoration : info->fDecorations) {
          decorationsJ->set(decoration.fId, decoration.toCompoundTag());
        }
        java::AppendComponent(itemJ, u8"map_decorations", decorationsJ);
      }
    }

    return name;
  }

  static std::u8string Potion(std::u8string const &name, CompoundTag const &itemB, CompoundTag &itemJ, Context &ctx, int dataVersion, Options const &opt) {
    auto damage = itemB.int16(u8"Damage", 0);
    auto potionName = je2be::Potion::JavaPotionTypeFromBedrock(damage);
    auto potionContents = Compound();
    potionContents->set(u8"potion", potionName);
    java::AppendComponent(itemJ, u8"potion_contents", potionContents);
    return name;
  }

  static std::u8string Skull(std::u8string const &name, CompoundTag const &itemB, CompoundTag &itemJ, Context &ctx, int dataVersion, Options const &opt) {
    auto damage = itemB.int16(u8"Damage", 0);
    SkullType st = static_cast<SkullType>(damage);
    return u8"minecraft:" + JavaNameFromSkullType(st);
  }

  static std::u8string SuspiciousStew(std::u8string const &name, CompoundTag const &itemB, CompoundTag &itemJ, Context &ctx, int dataVersion, Options const &opt) {
    auto damage = itemB.int16(u8"Damage", 0);
    auto effects = List<Tag::Type::Compound>();
    Effect::SuspiciousStewEffect sse = Effect::JavaEffectFromBedrockSuspiciousStew(damage, opt.fOfferItem);
    if (auto id = Effect::NamespacedIdFromLegacyId(sse.fEffectId); id) {
      auto effect = Compound();
      effect->set(u8"id", *id);
      if (sse.fDuration) {
        effect->set(u8"duration", Int(*sse.fDuration));
      }
      effects->push_back(effect);
    }
    java::AppendComponent(itemJ, u8"suspicious_stew_effects", effects);
    return name;
  }

  static std::u8string TropicalFishBucket(std::u8string const &name, CompoundTag const &itemB, CompoundTag &itemJ, Context &ctx, int dataVersion, Options const &opt) {
    auto tagB = itemB.compoundTag(u8"tag");
    if (tagB) {
      TropicalFish tf = TropicalFish::FromBedrockBucketTag(*tagB);
      auto tagJ = Compound();
      tagJ->set(u8"BucketVariantTag", Int(tf.toJavaVariant()));
      itemJ[u8"tag"] = tagJ;
    }
    return name;
  }

  static std::u8string WolfArmor(std::u8string const &name, CompoundTag const &itemB, CompoundTag &itemJ, Context &ctx, int dataVersion, Options const &opt) {
    if (auto tagB = itemB.compoundTag(u8"tag"); tagB) {
      if (auto customColorB = tagB->int32(u8"customColor"); customColorB) {
        auto dyedColor = Compound();
        dyedColor->set(u8"rgb", Int(RgbFromCustomColor(*customColorB)));
        java::AppendComponent(itemJ, u8"dyed_color", dyedColor);
      }
    }
    return name;
  }
#pragma endregion

#pragma region Converter generators
  static Converter Rename(std::u8string name) {
    return [name](std::u8string const &, CompoundTag const &itemB, CompoundTag &itemJ, Context &ctx, int dataVersion, Options const &opt) {
      return u8"minecraft:" + name;
    };
  }
#pragma endregion

  static i32 RgbFromCustomColor(i32 customColor) {
    u32 rgb = 0xffffff & *(u32 *)&customColor;
    return *(i32 *)&rgb;
  }

  static std::u8string Ns() {
    return u8"minecraft:";
  }

  static std::unordered_map<std::u8string_view, Converter> *CreateTable() {
    using namespace std;
    auto *ret = new unordered_map<u8string_view, Converter>();

#define E(__name, __conv)                        \
  assert(ret->find(u8"" #__name) == ret->end()); \
  ret->try_emplace(u8"" #__name, __conv)

    E(writable_book, Book);
    E(written_book, Book);
    E(fish, Rename(u8"cod"));
    E(potion, Potion);
    E(splash_potion, Potion);
    E(lingering_potion, Potion);
    E(filled_map, Map);
    E(firework_rocket, FireworkRocket);
    E(fireworks, FireworkRocket); // legacy
    E(firework_star, FireworkStar);
    E(fireworkscharge, FireworkStar); // legacy
    E(banner, Banner);
    E(skull, Skull);
    E(sign, Rename(u8"oak_sign"));              // legacy
    E(darkoak_sign, Rename(u8"dark_oak_sign")); // legacy
    E(bed, Bed);
    E(frame, Rename(u8"item_frame"));
    E(glow_frame, Rename(u8"glow_item_frame"));
    E(carrotonastick, Rename(u8"carrot_on_a_stick")); // legacy
    E(boat, LegacyBoat);                              // legacy
    E(wooden_door, Rename(u8"oak_door"));
    E(turtle_shell_piece, Rename(u8"scute")); // legacy
    E(bucket, Bucket);
    E(dye, LegacyDye);                                                     // legacy
    E(spawn_egg, LegacySpawnEgg);                                          // legacy
    E(fireball, Rename(u8"fire_charge"));                                  // legacy
    E(zombified_pigman_spawn_egg, Rename(u8"zombified_piglin_spawn_egg")); // legacy
    E(zombie_pigman_spawn_egg, Rename(u8"zombified_piglin_spawn_egg"));
    E(netherbrick, Rename(u8"nether_brick"));              // legacy
    E(netherstar, Rename(u8"nether_star"));                // legacy
    E(horsearmorleather, Rename(u8"leather_horse_armor")); // legacy
    E(horsearmoriron, Rename(u8"iron_horse_armor"));       // legacy
    E(horsearmorgold, Rename(u8"golden_horse_armor"));     // legacy
    E(horsearmordiamond, Rename(u8"diamond_horse_armor")); // legacy
    E(empty_map, Rename(u8"map"));
    E(chorus_fruit_popped, Rename(u8"popped_chorus_fruit")); // legacy
    E(banner_pattern, LegacyBannerPattern);                  // legacy
    E(appleenchanted, Rename(u8"enchanted_golden_apple"));   // legacy
    E(melon, Rename(u8"melon_slice"));                       // legacy
    E(muttoncooked, Rename(u8"cooked_mutton"));              // legacy
    E(muttonraw, Rename(u8"mutton"));                        // legacy
    E(cooked_fish, Rename(u8"cooked_cod"));                  // legacy
    E(clownfish, Rename(u8"tropical_fish"));                 // legacy
    E(tropical_fish_bucket, TropicalFishBucket);
    E(arrow, Arrow);
    E(totem, Rename(u8"totem_of_undying")); // legacy
    E(suspicious_stew, SuspiciousStew);
    E(axolotl_bucket, AxolotlBucket);
    E(crossbow, Crossbow);
    E(field_masoned_banner_pattern, Rename(u8"flower_banner_pattern"));    // field_masoned_banner_pattern doesn't exist in JE
    E(bordure_indented_banner_pattern, Rename(u8"flower_banner_pattern")); // bordure_indented_banner_pattern doesn't exist in JE

    // 1.19
    E(frog_spawn, Rename(u8"frogspawn"));
    E(goat_horn, GoatHorn);

    // 1.20
    E(archer_pottery_shard, Rename(u8"pottery_shard_archer"));
    E(prize_pottery_shard, Rename(u8"pottery_shard_prize"));
    E(arms_up_pottery_shard, Rename(u8"pottery_shard_arms_up"));
    E(skull_pottery_shard, Rename(u8"pottery_shard_skull"));

    // 1.20.5
    E(wolf_armor, WolfArmor);
#undef E
    return ret;
  }
};

CompoundTagPtr Item::From(CompoundTag const &tagB, Context &ctx, int dataVersion, Item::Options const &opt) {
  return Impl::From(tagB, ctx, dataVersion, opt);
}

} // namespace je2be::bedrock
