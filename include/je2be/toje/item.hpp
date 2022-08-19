#pragma once

namespace je2be::toje {

class Item {
  Item() = delete;
  using Converter = std::function<std::string(std::string const &, CompoundTag const &itemB, CompoundTag &itemJ, Context &ctx)>;

public:
  static CompoundTagPtr From(CompoundTag const &tagB, Context &ctx) {
    using namespace std;
    auto name = tagB.string("Name");
    if (!name) {
      return nullptr;
    }
    static unique_ptr<unordered_map<string, Converter> const> const sTable(CreateTable());
    auto ret = Compound();
    if (name == "") {
      return ret;
    }
    auto found = sTable->find(*name);
    if (found == sTable->end()) {
      Default(*name, tagB, *ret, ctx);
    } else {
      auto renamed = found->second(*name, tagB, *ret, ctx);
      Default(renamed, tagB, *ret, ctx);
    }
    return ret;
  }

  static void Default(std::string const &nameB, CompoundTag const &itemB, CompoundTag &itemJ, Context &ctx) {
    using namespace std;

    auto tagB = itemB.compoundTag("tag");
    if (!tagB) {
      tagB = Compound();
    }

    auto tagJ = itemJ.compoundTag("tag");
    if (!tagJ) {
      tagJ = Compound();
    }

    string nameJ = nameB;
    auto blockTag = itemB.compoundTag("Block");
    if (blockTag) {
      auto blockB = mcfile::be::Block::FromCompound(*blockTag);
      if (blockB) {
        auto blockJ = BlockData::From(*blockB);
        if (blockJ) {
          nameJ = blockJ->fName;

          Pos3i dummy(0, 0, 0);
          if (!tagB->empty()) {
            if (auto converted = ctx.fFromBlockAndBlockEntity(dummy, *blockB, *tagB, *blockJ, ctx); converted && converted->fTileEntity) {
              static unordered_set<string> const sExclude({"x", "y", "z", "keepPacked", "RecipesUsed"});
              auto blockEntityTag = converted->fTileEntity;
              for (auto const &e : sExclude) {
                blockEntityTag->erase(e);
              }
              if (!blockEntityTag->empty()) {
                tagJ->set("BlockEntityTag", blockEntityTag);
              }
            }
          }
        }
      }
    }
    itemJ.set("id", String(nameJ));

    CopyByteValues(itemB, itemJ, {{"Count"}, {"Slot"}});
    CopyIntValues(*tagB, *tagJ, {{"Damage"}, {"RepairCost"}});

    auto displayJ = Compound();

    auto customColor = tagB->int32("customColor");
    if (customColor) {
      int32_t c = *customColor;
      uint32_t rgb = 0xffffff & *(uint32_t *)&c;
      displayJ->set("color", Int(*(int32_t *)&rgb));
    }

    auto displayB = tagB->compoundTag("display");
    if (displayB) {
      auto displayName = displayB->string("Name");
      if (displayName) {
        nlohmann::json json;
        json["text"] = *displayName;
        displayJ->set("Name", String(nlohmann::to_string(json)));
      }

      if (auto loreB = displayB->listTag("Lore"); loreB) {
        auto loreJ = List<Tag::Type::String>();
        for (auto const &it : *loreB) {
          if (auto item = it->asString(); item) {
            if (item->fValue == "(+DATA)") {
              loreJ->push_back(String("\"(+NBT)\""));
            } else {
              loreJ->push_back(String(item->fValue));
            }
          }
        }
        displayJ->set("Lore", loreJ);
      }
    }

    auto customName = tagB->string("CustomName");
    auto customNameVisible = tagB->boolean("CustomNameVisible", false);
    if (customName && customNameVisible) {
      nlohmann::json json;
      json["text"] = *customName;
      displayJ->set("Name", String(nlohmann::to_string(json)));
    }

    auto enchB = tagB->listTag("ench");
    if (enchB) {
      auto enchJ = List<Tag::Type::Compound>();
      for (auto const &it : *enchB) {
        auto cB = it->asCompound();
        if (!cB) {
          continue;
        }
        auto idB = cB->int16("id");
        auto lvlB = cB->int16("lvl");
        if (idB && lvlB) {
          auto cJ = Compound();
          auto idJ = Enchantments::JavaEnchantmentIdFromBedrock(*idB);
          cJ->set("id", String(idJ));
          cJ->set("lvl", Short(*lvlB));
          enchJ->push_back(cJ);
        }
      }
      if (nameB == "minecraft:enchanted_book") {
        tagJ->set("StoredEnchantments", enchJ);
      } else {
        tagJ->set("Enchantments", enchJ);
      }
    }

    if (!displayJ->empty()) {
      tagJ->set("display", displayJ);
    }

    if (!tagJ->empty()) {
      itemJ["tag"] = tagJ;
    }
  }

#pragma region Converters
  static std::string Arrow(std::string const &nameB, CompoundTag const &itemB, CompoundTag &itemJ, Context &ctx) {
    auto damage = itemB.int16("Damage", 0);
    if (damage == 0) {
      return nameB;
    } else {
      auto potionJ = TippedArrowPotion::JavaPotionType(damage);
      auto tagJ = Compound();
      tagJ->set("Potion", String(potionJ));
      itemJ.set("tag", tagJ);
      return Ns() + "tipped_arrow";
    }
  }

  static std::string AxolotlBucket(std::string const &nameB, CompoundTag const &itemB, CompoundTag &itemJ, Context &ctx) {
    auto tagB = itemB.compoundTag("tag");
    if (tagB) {
      auto tagJ = Compound();

      CopyIntValues(*tagB, *tagJ, {{"Age", "Age", 0}});

      auto attributes = tagB->listTag("Attributes");
      for (auto const &it : *attributes) {
        auto c = it->asCompound();
        if (!c) {
          continue;
        }
        auto name = c->string("Name");
        if (name != "minecraft:health") {
          continue;
        }
        auto current = c->float32("Current");
        if (!current) {
          continue;
        }
        tagJ->set("Health", Float(*current));
        break;
      }

      auto variantB = tagB->int32("Variant", 0);
      tagJ->set("Variant", Int(Axolotl::JavaVariantFromBedrockVariant(variantB)));

      itemJ["tag"] = tagJ;
    }
    return nameB;
  }

  static std::string Banner(std::string const &nameB, CompoundTag const &itemB, CompoundTag &itemJ, Context &ctx) {
    auto damageB = itemB.int16("Damage", 0);
    BannerColorCodeBedrock colorB = static_cast<BannerColorCodeBedrock>(damageB);
    ColorCodeJava colorJ = ColorCodeJavaFromBannerColorCodeBedrock(colorB);
    std::string colorNameJ = JavaNameFromColorCodeJava(colorJ);
    std::string nameJ = "minecraft:" + colorNameJ + "_banner";

    auto tagB = itemB.compoundTag("tag");
    if (tagB) {
      auto tagJ = Compound();
      auto typeB = tagB->int32("Type");
      bool ominous = typeB == 1;
      if (ominous) {
        nameJ = "minecraft:white_banner";

        auto bannerPatterns = Banner::OminousBannerPatterns();
        auto blockEntityTag = Compound();
        blockEntityTag->set("Patterns", bannerPatterns);
        blockEntityTag->set("id", String("minecraft:banner"));
        tagJ->set("BlockEntityTag", blockEntityTag);

        auto displayJ = Compound();
        nlohmann::json json;
        json["color"] = "gold";
        json["translate"] = "block.minecraft.ominous_banner";
        displayJ->set("Name", String(nlohmann::to_string(json)));
        tagJ->set("display", displayJ);
      } else {
        auto blockEntityTag = Compound();
        blockEntityTag->set("id", String("minecraft:banner"));

        auto patternsB = tagB->listTag("Patterns");
        if (patternsB) {
          auto patternsJ = List<Tag::Type::Compound>();
          for (auto const &it : *patternsB) {
            auto patternB = it->asCompound();
            if (!patternB) {
              continue;
            }
            auto colorB = patternB->int32("Color");
            auto patternStringB = patternB->string("Pattern");
            if (!colorB || !patternStringB) {
              continue;
            }
            auto patternJ = Compound();
            BannerColorCodeBedrock bccb = static_cast<BannerColorCodeBedrock>(*colorB);
            ColorCodeJava ccj = ColorCodeJavaFromBannerColorCodeBedrock(bccb);
            patternJ->set("Color", Int(static_cast<int32_t>(ccj)));
            patternJ->set("Pattern", String(*patternStringB));

            patternsJ->push_back(patternJ);
          }
          blockEntityTag->set("Patterns", patternsJ);
        }

        tagJ->set("BlockEntityTag", blockEntityTag);
      }
      itemJ.set("tag", tagJ);
    }
    return nameJ;
  }

  static std::string Bed(std::string const &name, CompoundTag const &itemB, CompoundTag &itemJ, Context &ctx) {
    auto damage = itemB.int16("Damage", 0);
    ColorCodeJava ccj = static_cast<ColorCodeJava>(damage);
    return "minecraft:" + JavaNameFromColorCodeJava(ccj) + "_bed";
  }

  static std::string Book(std::string const &name, CompoundTag const &itemB, CompoundTag &itemJ, Context &ctx) {
    using namespace std;
    auto tagB = itemB.compoundTag("tag");
    if (!tagB) {
      return name;
    }
    auto tagJ = Compound();
    auto pagesB = tagB->listTag("pages");
    if (pagesB) {
      auto pagesJ = List<Tag::Type::String>();
      for (auto const &pageB : *pagesB) {
        auto const *c = pageB->asCompound();
        if (!c) {
          continue;
        }
        auto text = c->string("text");
        if (!text) {
          continue;
        }
        if (name == "minecraft:written_book") {
          nlohmann::json json;
          json["text"] = *text;
          string jsonString = nlohmann::to_string(json);
          pagesJ->push_back(String(jsonString));
        } else {
          pagesJ->push_back(String(*text));
        }
      }
      tagJ->set("pages", pagesJ);
    }
    auto author = tagB->string("author");
    if (author) {
      tagJ->set("author", String(*author));
    }
    auto title = tagB->string("title");
    if (title) {
      tagJ->set("title", String(*title));
    }
    itemJ.set("tag", tagJ);
    return name;
  }

  static std::string Bucket(std::string const &name, CompoundTag const &itemB, CompoundTag &itemJ, Context &ctx) {
    auto damage = itemB.int16("Damage", 0);
    std::string prefix = "";
    switch (damage) {
    case 1:
      prefix = "milk_";
      break;
    case 2:
      prefix = "cod_";
      break;
    case 3:
      prefix = "salmon_";
      break;
    case 5:
      prefix = "pufferfish_";
      break;
    case 10:
      prefix = "lava_";
      break;
    case 4: {
      prefix = "tropical_fish_";
      auto tagB = itemB.compoundTag("tag");
      if (tagB) {
        TropicalFish tf = TropicalFish::FromBedrockBucketTag(*tagB);
        auto tagJ = Compound();
        tagJ->set("BucketVariantTag", Int(tf.toJavaVariant()));
        itemJ["tag"] = tagJ;
      }
      break;
    }
    }
    return "minecraft:" + prefix + "bucket";
  }

  static std::string Crossbow(std::string const &name, CompoundTag const &itemB, CompoundTag &itemJ, Context &ctx) {
    if (auto tagB = itemB.compoundTag("tag"); tagB) {
      auto tagJ = Compound();
      auto chargedProjectiles = List<Tag::Type::Compound>();
      if (auto chargedItem = tagB->compoundTag("chargedItem"); chargedItem) {
        if (auto projectileJ = From(*chargedItem, ctx); projectileJ) {
          chargedProjectiles->push_back(projectileJ);
        }
        tagJ->set("ChargedProjectiles", chargedProjectiles);
        tagJ->set("Charged", Bool(!chargedProjectiles->empty()));
        itemJ["tag"] = tagJ;
      }
    }
    return name;
  }

  static std::string FireworkRocket(std::string const &name, CompoundTag const &itemB, CompoundTag &itemJ, Context &ctx) {
    auto tagB = itemB.compoundTag("tag");
    if (tagB) {
      auto tagJ = Compound();
      auto fireworksB = tagB->compoundTag("Fireworks");
      if (fireworksB) {
        FireworksData data = FireworksData::FromBedrock(*fireworksB);
        auto fireworksJ = data.toJavaCompoundTag();
        tagJ->set("Fireworks", fireworksJ);
      }
      itemJ.set("tag", tagJ);
    }
    return "minecraft:firework_rocket";
  }

  static std::string FireworkStar(std::string const &name, CompoundTag const &itemB, CompoundTag &itemJ, Context &ctx) {
    auto tagB = itemB.compoundTag("tag");
    if (tagB) {
      tagB->erase("customColor");
      auto tagJ = Compound();
      auto fireworksB = tagB->compoundTag("FireworksItem");
      if (fireworksB) {
        FireworksExplosion explosion = FireworksExplosion::FromBedrock(*fireworksB);
        auto explosionJ = explosion.toJavaCompoundTag();
        tagJ->set("Explosion", explosionJ);
        itemJ["tag"] = tagJ;
      }
    }
    return "minecraft:firework_star";
  }

  static std::string GoatHorn(std::string const &name, CompoundTag const &itemB, CompoundTag &itemJ, Context &ctx) {
    auto damage = itemB.int16("Damage", 0);
    auto instrument = GoatHorn::JavaInstrumentFromBedrockDamage(damage);
    auto tagJ = Compound();
    tagJ->set("instrument", String(instrument));
    itemJ["tag"] = tagJ;
    return name;
  }

  static std::string LegacyBannerPattern(std::string const &name, CompoundTag const &itemB, CompoundTag &itemJ, Context &ctx) {
    auto damage = itemB.int16("Damage", 0);
    std::string prefix;
    switch (damage) {
    case 2:
      prefix = "flower_";
      break;
    case 1:
      prefix = "creeper_";
      break;
    case 6:
      prefix = "piglin_";
      break;
    case 3:
      prefix = "mojang_";
      break;
    case 0:
    default:
      prefix = "skull_";
      break;
    }
    return Ns() + prefix + "banner_pattern";
  }

  static std::string LegacyBoat(std::string const &name, CompoundTag const &itemB, CompoundTag &itemJ, Context &ctx) {
    auto damage = itemB.int16("Damage", 0);
    std::string type = "oak";
    switch (damage) {
    case 1:
      type = "spruce";
      break;
    case 2:
      type = "birch";
      break;
    case 3:
      type = "jungle";
      break;
    case 4:
      type = "acacia";
      break;
    case 5:
      type = "dark_oak";
      break;
    case 0:
    default:
      type = "oak";
      break;
    }
    return "minecraft:" + type + "_boat";
  }

  static std::string LegacyDye(std::string const &name, CompoundTag const &itemB, CompoundTag &itemJ, Context &ctx) {
    auto damage = itemB.int16("Damage", 0);
    switch (damage) {
    case 1:
      return Ns() + "red_dye";
    case 2:
      return Ns() + "green_dye";
    case 3:
      return Ns() + "cocoa_beans";
    case 4:
      return Ns() + "lapis_lazuli";
    case 5:
      return Ns() + "purple_dye";
    case 6:
      return Ns() + "cyan_dye";
    case 7:
      return Ns() + "light_gray_dye";
    case 8:
      return Ns() + "gray_dye";
    case 9:
      return Ns() + "pink_dye";
    case 10:
      return Ns() + "lime_dye";
    case 11:
      return Ns() + "yellow_dye";
    case 12:
      return Ns() + "light_blue_dye";
    case 13:
      return Ns() + "magenta_dye";
    case 14:
      return Ns() + "orange_dye";
    case 15:
      return Ns() + "bone_meal";
    case 16:
      return Ns() + "black_dye";
    case 17:
      return Ns() + "brown_dye";
    case 18:
      return Ns() + "blue_dye";
    case 19:
      return Ns() + "white_dye";
    case 0:
    default:
      return Ns() + "ink_sac";
    }
  }

  static std::string LegacySpawnEgg(std::string const &name, CompoundTag const &itemB, CompoundTag &itemJ, Context &ctx) {
    auto damage = itemB.int16("Damage", 0);
    std::string prefix;
    switch (damage) {
    case 122:
      prefix = "bee_";
      break;
    case 10:
      prefix = "chicken_";
      break;
    case 16:
      prefix = "mooshroom_";
      break;
    case 28:
      prefix = "polar_bear_";
      break;
    case 11:
      prefix = "cow_";
      break;
    case 12:
      prefix = "pig_";
      break;
    case 13:
      prefix = "sheep_";
      break;
    case 14:
      prefix = "wolf_";
      break;
    case 22:
      prefix = "ocelot_";
      break;
    case 75:
      prefix = "cat_";
      break;
    case 30:
      prefix = "parrot_";
      break;
    case 19:
      prefix = "bat_";
      break;
    case 74:
      prefix = "turtle_";
      break;
    case 25:
      prefix = "mule_";
      break;
    case 18:
      prefix = "rabbit_";
      break;
    case 29:
      prefix = "llama_";
      break;
    case 23:
      prefix = "horse_";
      break;
    case 24:
      prefix = "donkey_";
      break;
    case 26:
      prefix = "skeleton_horse_";
      break;
    case 27:
      prefix = "zombie_horse_";
      break;
    case 121:
      prefix = "fox_";
      break;
    case 113:
      prefix = "panda_";
      break;
    case 31:
      prefix = "dolphin_";
      break;
    case 108:
      prefix = "pufferfish_";
      break;
    case 109:
      prefix = "salmon_";
      break;
    case 112:
      prefix = "cod_";
      break;
    case 38:
      prefix = "enderman_";
      break;
    case 33:
      prefix = "creeper_";
      break;
    case 39:
      prefix = "silverfish_";
      break;
    case 34:
      prefix = "skeleton_";
      break;
    case 45:
      prefix = "witch_";
      break;
    case 37:
      prefix = "slime_";
      break;
    case 48:
      prefix = "wither_skeleton_";
      break;
    case 46:
      prefix = "stray_";
      break;
    case 35:
      prefix = "spider_";
      break;
    case 32:
      prefix = "zombie_";
      break;
    case 50:
      prefix = "elder_guardian_";
      break;
    case 49:
      prefix = "guardian_";
      break;
    case 40:
      prefix = "cave_spider_";
      break;
    case 17:
      prefix = "squid_";
      break;
    case 110:
      prefix = "drowned_";
      break;
    case 47:
      prefix = "husk_";
      break;
    case 42:
      prefix = "magma_cube_";
      break;
    case 55:
      prefix = "endermite_";
      break;
    case 118:
      prefix = "wandering_trader_";
      break;
    case 127:
      prefix = "piglin_brute_";
      break;
    case 125:
      prefix = "strider_";
      break;
    case 124:
      prefix = "hoglin_";
      break;
    case 123:
      prefix = "piglin_";
      break;
    case 126:
      prefix = "zoglin_";
      break;
    case 41:
      prefix = "ghast_";
      break;
    case 43:
      prefix = "blaze_";
      break;
    case 58:
      prefix = "phantom_";
      break;
    case 105:
      prefix = "vex_";
      break;
    case 57:
      prefix = "vindicator_";
      break;
    case 54:
      prefix = "shulker_";
      break;
    case 59:
      prefix = "ravager_";
      break;
    case 114:
      prefix = "pillager_";
      break;
    case 36:
      prefix = "zombified_piglin_";
      break;
    case 111:
      prefix = "tropical_fish_";
      break;
    case 115:
      prefix = "villager_";
      break;
    case 104:
      prefix = "evoker_";
      break;
    case 116:
      prefix = "zombie_villager_";
      break;
    }
    return "minecraft:" + prefix + "spawn_egg";
  }

  static std::string Map(std::string const &name, CompoundTag const &itemB, CompoundTag &itemJ, Context &ctx) {
    auto tagB = itemB.compoundTag("tag");
    if (!tagB) {
      return name;
    }
    auto tagJ = Compound();
    auto uuid = tagB->int64("map_uuid");
    if (!uuid) {
      return name;
    }

    auto damage = itemB.int16("Damage", 0);
    std::string translate;
    std::optional<int32_t> mapColor;
    if (damage == 3) {
      translate = "filled_map.monument";
      mapColor = 3830373;
    } else if (damage == 4) {
      translate = "filled_map.mansion";
      mapColor = 5393476;
    } else if (damage == 5) {
      translate = "filled_map.buried_treasure";
    }
    if (!translate.empty()) {
      auto displayJ = Compound();
      nlohmann::json json;
      json["translate"] = translate;
      displayJ->set("Name", String(nlohmann::to_string(json)));

      if (mapColor) {
        displayJ->set("MapColor", Int(*mapColor));
      }

      tagJ->set("display", displayJ);
    }

    auto info = ctx.mapFromUuid(*uuid);
    if (info) {
      tagJ->set("map", Int(info->fNumber));
      ctx.markMapUuidAsUsed(*uuid);

      if (damage != 0 && !info->fDecorations.empty()) {
        auto decorationsJ = List<Tag::Type::Compound>();
        for (MapInfo::Decoration const &decoration : info->fDecorations) {
          decorationsJ->push_back(decoration.toCompoundTag());
        }
        tagJ->set("Decorations", decorationsJ);
      }
    }

    itemJ.set("tag", tagJ);
    return name;
  }

  static std::string Potion(std::string const &name, CompoundTag const &itemB, CompoundTag &itemJ, Context &ctx) {
    auto damage = itemB.int16("Damage", 0);
    auto potionName = je2be::Potion::JavaPotionTypeFromBedrock(damage);
    auto tagJ = Compound();
    tagJ->set("Potion", String(potionName));
    itemJ.set("tag", tagJ);
    return name;
  }

  static std::string Skull(std::string const &name, CompoundTag const &itemB, CompoundTag &itemJ, Context &ctx) {
    auto damage = itemB.int16("Damage", 0);
    SkullType st = static_cast<SkullType>(damage);
    return "minecraft:" + JavaNameFromSkullType(st);
  }

  static std::string SuspiciousStew(std::string const &name, CompoundTag const &itemB, CompoundTag &itemJ, Context &ctx) {
    auto damage = itemB.int16("Damage", 0);
    Effect::SuspiciousStewEffect sse = Effect::JavaEffectFromBedrockSuspiciousStew(damage);
    auto effects = List<Tag::Type::Compound>();
    auto effect = Compound();
    effect->set("EffectId", Byte(sse.fEffectId));
    effect->set("EffectDuration", Int(sse.fDuration));
    effects->push_back(effect);
    auto tag = Compound();
    tag->set("Effects", effects);
    itemJ["tag"] = tag;
    return name;
  }

  static std::string TropicalFishBucket(std::string const &name, CompoundTag const &itemB, CompoundTag &itemJ, Context &ctx) {
    auto tagB = itemB.compoundTag("tag");
    if (tagB) {
      TropicalFish tf = TropicalFish::FromBedrockBucketTag(*tagB);
      auto tagJ = Compound();
      tagJ->set("BucketVariantTag", Int(tf.toJavaVariant()));
      itemJ["tag"] = tagJ;
    }
    return name;
  }
#pragma endregion

#pragma region Converter generators
  static Converter Rename(std::string name) {
    return [name](std::string const &, CompoundTag const &itemB, CompoundTag &itemJ, Context &ctx) {
      return "minecraft:" + name;
    };
  }
#pragma endregion

  static std::string Ns() {
    return "minecraft:";
  }

  static std::unordered_map<std::string, Converter> *CreateTable() {
    using namespace std;
    auto *ret = new unordered_map<string, Converter>();

#define E(__name, __conv)                                \
  assert(ret->find("minecraft:" #__name) == ret->end()); \
  ret->insert(make_pair("minecraft:" #__name, __conv))

    E(writable_book, Book);
    E(written_book, Book);
    E(fish, Rename("cod"));
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
    E(sign, Rename("oak_sign"));              // legacy
    E(darkoak_sign, Rename("dark_oak_sign")); // legacy
    E(bed, Bed);
    E(frame, Rename("item_frame"));
    E(glow_frame, Rename("glow_item_frame"));
    E(carrotonastick, Rename("carrot_on_a_stick")); // legacy
    E(boat, LegacyBoat);                            // legacy
    E(wooden_door, Rename("oak_door"));
    E(turtle_shell_piece, Rename("scute")); // legacy
    E(bucket, Bucket);
    E(dye, LegacyDye);                                                   // legacy
    E(spawn_egg, LegacySpawnEgg);                                        // legacy
    E(fireball, Rename("fire_charge"));                                  // legacy
    E(zombified_pigman_spawn_egg, Rename("zombified_piglin_spawn_egg")); // legacy
    E(zombie_pigman_spawn_egg, Rename("zombified_piglin_spawn_egg"));
    E(netherbrick, Rename("nether_brick"));              // legacy
    E(netherstar, Rename("nether_star"));                // legacy
    E(horsearmorleather, Rename("leather_horse_armor")); // legacy
    E(horsearmoriron, Rename("iron_horse_armor"));       // legacy
    E(horsearmorgold, Rename("golden_horse_armor"));     // legacy
    E(horsearmordiamond, Rename("diamond_horse_armor")); // legacy
    E(empty_map, Rename("map"));
    E(chorus_fruit_popped, Rename("popped_chorus_fruit")); // legacy
    E(banner_pattern, LegacyBannerPattern);                // legacy
    E(appleenchanted, Rename("enchanted_golden_apple"));   // legacy
    E(melon, Rename("melon_slice"));                       // legacy
    E(muttoncooked, Rename("cooked_mutton"));              // legacy
    E(muttonraw, Rename("mutton"));                        // legacy
    E(cooked_fish, Rename("cooked_cod"));                  // legacy
    E(clownfish, Rename("tropical_fish"));                 // legacy
    E(tropical_fish_bucket, TropicalFishBucket);
    E(arrow, Arrow);
    E(totem, Rename("totem_of_undying")); // legacy
    E(suspicious_stew, SuspiciousStew);
    E(axolotl_bucket, AxolotlBucket);
    E(crossbow, Crossbow);

    // 1.19
    E(frog_spawn, Rename("frogspawn"));
    E(goat_horn, GoatHorn);
#undef E
    return ret;
  }
};

} // namespace je2be::toje
