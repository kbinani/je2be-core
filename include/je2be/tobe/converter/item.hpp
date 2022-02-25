#pragma once

namespace je2be::tobe {

class Item {
private:
  using ItemData = std::shared_ptr<CompoundTag>;
  using Converter = std::function<ItemData(std::string const &, CompoundTag const &, Context const &ctx)>;
  using Block = mcfile::je::Block;

public:
  static std::shared_ptr<CompoundTag> From(std::shared_ptr<CompoundTag> const &item, Context const &ctx) {
    using namespace std;

    static unique_ptr<unordered_map<string, Converter> const> const blockItemMapping(CreateBlockItemConverterTable());
    static unique_ptr<unordered_map<string, Converter> const> const itemMapping(CreateItemConverterTable());

    auto id = item->string("id");
    if (!id) {
      return nullptr;
    }
    string const &name = *id;

    if (name == "minecraft:filled_map") {
      auto ret = Map(name, *item, ctx.fMapInfo);
      if (ret) {
        auto [mapId, result] = *ret;
        ctx.fWorldData.addMap(mapId, item);
        return result;
      } else {
        return nullptr;
      }
    }

    if (IsItem(name)) {
      auto found = itemMapping->find(name);
      if (found == itemMapping->end()) {
        return DefaultItem(name, *item, ctx);
      } else {
        return found->second(name, *item, ctx);
      }
    } else {
      auto found = blockItemMapping->find(name);
      if (found == blockItemMapping->end()) {
        return DefaultBlockItem(name, *item, ctx);
      } else {
        return found->second(name, *item, ctx);
      }
    }
  }

  static int8_t GetSkullTypeFromBlockName(std::string const &name) {
    int8_t type = 0;
    std::string n = strings::LTrim(name, "minecraft:");
    n = strings::Remove(n, "_wall");
    auto st = SkullTypeFromJavaName(n);
    if (st) {
      type = static_cast<uint8_t>(*st);
    }
    return type;
  }

  static std::shared_ptr<CompoundTag> Empty() {
    auto armor = Compound();
    armor->set("Count", Byte(0));
    armor->set("Damage", Short(0));
    armor->set("Name", String(""));
    armor->set("WasPickedUp", Bool(false));
    return armor;
  }

private:
  static std::unordered_map<std::string, Converter> *CreateBlockItemConverterTable() {
    using namespace std;
    auto table = new unordered_map<string, Converter>();
#define E(__name, __func) table->insert(make_pair("minecraft:" __name, __func))
    E("brown_mushroom_block", MushroomBlock);
    E("red_mushroom_block", MushroomBlock);
    E("white_bed", Bed);
    E("orange_bed", Bed);
    E("magenta_bed", Bed);
    E("light_blue_bed", Bed);
    E("yellow_bed", Bed);
    E("lime_bed", Bed);
    E("pink_bed", Bed);
    E("gray_bed", Bed);
    E("light_gray_bed", Bed);
    E("cyan_bed", Bed);
    E("purple_bed", Bed);
    E("blue_bed", Bed);
    E("brown_bed", Bed);
    E("green_bed", Bed);
    E("red_bed", Bed);
    E("black_bed", Bed);
    E("white_banner", Banner);
    E("orange_banner", Banner);
    E("magenta_banner", Banner);
    E("light_blue_banner", Banner);
    E("yellow_banner", Banner);
    E("lime_banner", Banner);
    E("pink_banner", Banner);
    E("gray_banner", Banner);
    E("light_gray_banner", Banner);
    E("cyan_banner", Banner);
    E("purple_banner", Banner);
    E("blue_banner", Banner);
    E("brown_banner", Banner);
    E("green_banner", Banner);
    E("red_banner", Banner);
    E("black_banner", Banner);

    E("skeleton_skull", Skull);
    E("wither_skeleton_skull", Skull);
    E("player_head", Skull);
    E("zombie_head", Skull);
    E("creeper_head", Skull);
    E("dragon_head", Skull);

    E("item_frame", Rename("frame"));
    E("glow_item_frame", Rename("glow_frame"));
    E("repeater", DefaultItem);
    E("comparator", DefaultItem);

    E("iron_door", DefaultItem);
    E("oak_door", Rename("wooden_door"));
    E("spruce_door", DefaultItem);
    E("birch_door", DefaultItem);
    E("jungle_door", DefaultItem);
    E("acacia_door", DefaultItem);
    E("dark_oak_door", DefaultItem);
    E("crimson_door", DefaultItem);
    E("warped_door", DefaultItem);

    E("campfire", DefaultItem);
    E("soul_campfire", DefaultItem);
    E("chain", DefaultItem);

    E("torch", AnyTorch);
    E("soul_torch", AnyTorch);
    E("redstone_torch", AnyTorch);

    E("oak_sign", DefaultItem);
    E("birch_sign", DefaultItem);
    E("spruce_sign", DefaultItem);
    E("jungle_sign", DefaultItem);
    E("acacia_sign", DefaultItem);
    E("dark_oak_sign", DefaultItem);
    E("crimson_sign", DefaultItem);
    E("warped_sign", DefaultItem);
#undef E
    return table;
  }

  static std::unordered_map<std::string, Converter> *CreateItemConverterTable() {
    using namespace std;
    auto table = new unordered_map<string, Converter>();
#define E(__name, __func) table->insert(make_pair("minecraft:" __name, __func))
    E("oak_door", Rename("wooden_door"));
    E("furnace_minecart", Rename("minecart")); // furnace minecart does not exist in bedrock
    E("tropical_fish_bucket", TropicalFishBucket);
    E("zombified_piglin_spawn_egg", Rename("zombified_pigman_spawn_egg"));

    E("map", Subtype("empty_map", 2));
    E("firework_rocket", FireworkRocket);
    E("firework_star", FireworkStar);

    // "13", "cat", "blocks", "chirp", "far", "mall", "mellohi", "stal", "strad", "ward", "11", "wait", "otherside", "pigstep"
    // E("music_disc_pigstep", Same);

    E("cod", Rename("fish"));
    E("tipped_arrow", TippedArrow);
    E("potion", AnyPotion);
    E("splash_potion", AnyPotion);
    E("lingering_potion", AnyPotion);

    for (string type : {"helmet", "chestplate", "leggings", "boots"}) {
      table->insert(std::make_pair("minecraft:leather_" + type, LeatherArmor));
    }

    E("writable_book", BooksAndQuill);
    E("written_book", BooksAndQuill);

    E("axolotl_bucket", AxolotlBucket);

    E("suspicious_stew", SuspiciousStew);
    E("crossbow", Crossbow);
#undef E
    return table;
  }

  static ItemData AxolotlBucket(std::string const &name, CompoundTag const &item, Context const &) {
    auto ret = New("axolotl_bucket");
    ret->set("Damage", Short(0));
    auto tg = item.compoundTag("tag");
    if (tg) {
      auto age = tg->int32("Age", 0);
      auto health = tg->float32("Health", 14);
      auto variant = tg->int32("Variant", 0);
      auto axltl = Axolotl(age, health, variant);
      auto tag = axltl.toBucketTag();

      auto customName = GetCustomName(*tg);
      if (customName) {
        tag->set("CustomName", String(*customName));
        tag->set("CustomNameVisible", Bool(true));
      }

      ret->set("tag", tag);
    }
    return ret;
  }

  static ItemData TropicalFishBucket(std::string const &name, CompoundTag const &item, Context const &) {
    auto ret = New("tropical_fish_bucket");
    ret->set("Damage", Short(0));
    auto tg = item.compoundTag("tag");
    if (tg) {
      auto variant = tg->intTag("BucketVariantTag");
      if (variant) {
        auto tf = TropicalFish::FromJavaVariant(variant->fValue);
        auto tag = tf.toBedrockBucketTag();
        ret->set("tag", tag);
      }
    }
    return Post(ret, item);
  }

  static ItemData BooksAndQuill(std::string const &name, CompoundTag const &item, Context const &) {
    using namespace std;

    auto tag = Compound();
    tag->insert({
        {"Name", String(name)},
        {"WasPickedUp", Bool(false)},
        {"Damage", Short(0)},
    });

    auto tg = item.compoundTag("tag");
    if (tg) {
      auto outTag = Compound();
      auto author = tg->stringTag("author");
      if (author) {
        outTag->set("author", String(author->fValue));
      }
      auto title = tg->stringTag("title");
      if (title) {
        outTag->set("title", String(title->fValue));
      }
      if (title || author) {
        outTag->set("generation", Int(0));
      }
      auto pages = tg->listTag("pages");
      if (pages) {
        auto outPages = List<Tag::Type::Compound>();
        for (auto const &it : *pages) {
          auto line = it->asString();
          if (!line) {
            continue;
          }
          string lineText;
          auto obj = props::ParseAsJson(line->fValue);
          if (obj) {
            auto text = obj->find("text");
            auto extra = obj->find("extra");
            if (extra != obj->end() && extra->is_array()) {
              vector<string> lines;
              for (auto const &line : *extra) {
                auto t = line.find("text");
                if (t != line.end() && t->is_string()) {
                  string l = t->get<string>();
                  if (l == "\x0d\x0a") {
                    l = "";
                  }
                  lines.push_back(l);
                }
              }
              lineText = "";
              for (int i = 0; i < lines.size(); i++) {
                lineText += lines[i];
                if (i + 1 < lines.size()) {
                  lineText += "\x0d\x0a";
                }
              }
            } else if (text != obj->end()) {
              lineText = text->get<string>();
            }
          } else {
            lineText = line->fValue;
          }
          auto lineObj = Compound();
          lineObj->insert({
              {"photoname", String("")},
              {"text", String(lineText)},
          });
          outPages->push_back(lineObj);
        }
        outTag->set("pages", outPages);
      }
      tag->set("tag", outTag);
    }

    return Post(tag, item);
  }

  static ItemData LeatherArmor(std::string const &name, CompoundTag const &item, Context const &) {
    auto tag = Compound();
    tag->insert({
        {"Name", String(name)},
        {"WasPickedUp", Bool(false)},
        {"Damage", Short(0)},
    });

    auto customColor = item.query("tag/display/color")->asInt();
    if (customColor) {
      uint32_t v = 0xff000000 | *(uint32_t *)&customColor->fValue;
      auto t = Compound();
      t->set("customColor", Int(*(int32_t *)&v));
      tag->set("tag", t);
    }

    return Post(tag, item);
  }

  static std::optional<std::tuple<int, ItemData>> Map(std::string const &name, CompoundTag const &item, JavaEditionMap const &mapInfo) {
    auto ret = New(name, true);
    ret->set("Damage", Short(0));

    auto number = item.query("tag/map")->asInt();
    if (!number) {
      return std::nullopt;
    }
    int32_t mapId = number->fValue;

    auto scale = mapInfo.scale(mapId);
    if (!scale) {
      return std::nullopt;
    }
    int64_t uuid = Map::UUID(mapId, *scale);

    auto tag = Compound();
    tag->set("map_uuid", Long(uuid));
    tag->set("map_display_players", Byte(1));
    ret->set("tag", tag);

    int16_t type = 0;
    auto display = item.query("tag/display")->asCompound();
    if (display) {
      auto displayName = display->string("Name");
      if (displayName) {
        auto nameJson = props::ParseAsJson(*displayName);
        if (nameJson) {
          auto translate = nameJson->find("translate");
          if (translate != nameJson->end() && translate->is_string()) {
            auto translationKey = translate->get<std::string>();
            if (translationKey == "filled_map.buried_treasure") {
              type = 5;
            } else if (translationKey == "filled_map.mansion") {
              type = 4;
            } else if (translationKey == "filled_map.monument") {
              type = 3;
            }
          }
        }
      }
    }
    tag->set("map_name_index", Int(number->fValue));
    ret->set("Damage", Short(type));

    auto out = Post(ret, item);
    return make_tuple(mapId, out);
  }

  static ItemData AnyPotion(std::string const &name, CompoundTag const &item, Context const &) {
    auto tag = New(name, true);
    auto t = item.query("tag")->asCompound();
    int16_t type = 0;
    if (t) {
      auto potion = t->string("Potion", "");
      type = Potion::BedrockPotionTypeFromJava(potion);
    }
    tag->set("Damage", Short(type));
    return Post(tag, item);
  }

  static ItemData TippedArrow(std::string const &name, CompoundTag const &item, Context const &) {
    auto tag = New("arrow");
    auto t = item.query("tag")->asCompound();
    int16_t type = 0;
    if (t) {
      auto potion = t->string("Potion", "");
      type = TippedArrowPotion::BedrockPotionType(potion);
    }
    tag->set("Damage", Short(type));
    return Post(tag, item);
  }

  static ItemData FireworkStar(std::string const &name, CompoundTag const &item, Context const &ctx) {
    auto data = Rename("firework_star")(name, item, ctx);

    auto explosion = item.query("tag/Explosion")->asCompound();
    if (explosion) {
      auto tag = Compound();

      auto e = FireworksExplosion::FromJava(*explosion);
      if (!e.fColor.empty()) {
        int32_t customColor = e.fColor[0].toARGB();
        tag->set("customColor", Int(customColor));
      }
      tag->set("FireworksItem", e.toBedrockCompoundTag());
      data->set("tag", tag);
    }

    return data;
  }

  static ItemData FireworkRocket(std::string const &name, CompoundTag const &item, Context const &) {
    auto data = New("firework_rocket");
    auto fireworks = item.query("tag/Fireworks")->asCompound();
    if (fireworks) {
      auto fireworksData = FireworksData::FromJava(*fireworks);
      auto tag = Compound();
      tag->set("Fireworks", fireworksData.toBedrockCompoundTag());
      data->set("tag", tag);
    }
    return Post(data, item);
  }

  static Converter Subtype(std::string const &newName, int16_t damage) {
    return [=](std::string const &name, CompoundTag const &item, Context const &) {
      auto tag = New(newName);
      tag->set("Damage", Short(damage));
      return Post(tag, item);
    };
  }

  static std::unordered_set<std::string> *CreateItemNameList() {
    using namespace std;
    auto table = new unordered_set<string>();
#define E(__name) table->insert("minecraft:" __name)
    E("minecart");
    E("saddle");
    E("oak_boat");
    E("chest_minecart");
    E("furnace_minecart");
    E("carrot_on_a_stick");
    E("warped_fungus_on_a_stick");
    E("tnt_minecart");
    E("hopper_minecart");
    E("elytra");
    E("spruce_boat");
    E("birch_boat");
    E("jungle_boat");
    E("acacia_boat");
    E("dark_oak_boat");

    E("beacon");
    E("turtle_egg");
    E("conduit");
    E("scute");
    E("coal");
    E("charcoal");
    E("diamond");
    E("iron_ingot");
    E("gold_ingot");
    E("netherite_ingot");
    E("netherite_scrap");
    E("stick");
    E("bowl");
    E("string");
    E("feather");
    E("gunpowder");
    E("wheat_seeds");
    E("wheat");
    E("flint");
    E("bucket");
    E("water_bucket");
    E("lava_bucket");
    E("snowball");
    E("leather");
    E("milk_bucket");
    E("pufferfish_bucket");
    E("salmon_bucket");
    E("cod_bucket");
    E("tropical_fish_bucket");
    E("brick");
    E("clay_ball");
    E("paper");
    E("book");
    E("slime_ball");
    E("egg");
    E("glowstone_dust");
    E("ink_sac");
    E("cocoa_beans");
    E("lapis_lazuli");
    E("white_dye");
    E("orange_dye");
    E("magenta_dye");
    E("light_blue_dye");
    E("yellow_dye");
    E("lime_dye");
    E("pink_dye");
    E("gray_dye");
    E("light_gray_dye");
    E("cyan_dye");
    E("purple_dye");
    E("blue_dye");
    E("brown_dye");
    E("green_dye");
    E("red_dye");
    E("black_dye");
    E("bone_meal");
    E("bone");
    E("sugar");
    E("pumpkin_seeds");
    E("melon_seeds");
    E("ender_pearl");
    E("blaze_rod");
    E("gold_nugget");
    E("nether_wart");
    E("ender_eye");
    E("bat_spawn_egg");
    E("bee_spawn_egg");
    E("blaze_spawn_egg");
    E("cat_spawn_egg");
    E("cave_spider_spawn_egg");
    E("chicken_spawn_egg");
    E("cod_spawn_egg");
    E("cow_spawn_egg");
    E("creeper_spawn_egg");
    E("dolphin_spawn_egg");
    E("donkey_spawn_egg");
    E("drowned_spawn_egg");
    E("elder_guardian_spawn_egg");
    E("enderman_spawn_egg");
    E("endermite_spawn_egg");
    E("evoker_spawn_egg");
    E("fox_spawn_egg");
    E("ghast_spawn_egg");
    E("guardian_spawn_egg");
    E("hoglin_spawn_egg");
    E("horse_spawn_egg");
    E("husk_spawn_egg");
    E("llama_spawn_egg");
    E("magma_cube_spawn_egg");
    E("mooshroom_spawn_egg");
    E("mule_spawn_egg");
    E("ocelot_spawn_egg");
    E("panda_spawn_egg");
    E("parrot_spawn_egg");
    E("phantom_spawn_egg");
    E("pig_spawn_egg");
    E("piglin_spawn_egg");
    E("piglin_brute_spawn_egg");
    E("pillager_spawn_egg");
    E("polar_bear_spawn_egg");
    E("pufferfish_spawn_egg");
    E("rabbit_spawn_egg");
    E("ravager_spawn_egg");
    E("salmon_spawn_egg");
    E("sheep_spawn_egg");
    E("shulker_spawn_egg");
    E("silverfish_spawn_egg");
    E("skeleton_spawn_egg");
    E("skeleton_horse_spawn_egg");
    E("slime_spawn_egg");
    E("spider_spawn_egg");
    E("squid_spawn_egg");
    E("stray_spawn_egg");
    E("strider_spawn_egg");
    E("trader_llama_spawn_egg"); // there is no spawn egg for trader llama for
                                 // bedrock
    E("tropical_fish_spawn_egg");
    E("turtle_spawn_egg");
    E("vex_spawn_egg");
    E("villager_spawn_egg");
    E("vindicator_spawn_egg");
    E("wandering_trader_spawn_egg");
    E("witch_spawn_egg");
    E("wither_skeleton_spawn_egg");
    E("wolf_spawn_egg");
    E("zoglin_spawn_egg");
    E("zombie_spawn_egg");
    E("zombie_horse_spawn_egg");
    E("zombie_villager_spawn_egg");
    E("zombified_piglin_spawn_egg");
    E("experience_bottole");
    E("fire_charge");
    E("writable_book");
    E("written_book");
    E("emerald");
    E("map");
    E("nether_star");
    E("firework_rocket");
    E("firework_star");
    E("nether_brick");
    E("quartz");
    E("prismarine_shard");
    E("prismarine_crystals");
    E("rabbit_hide");
    E("iron_horse_armor");
    E("golden_horse_armor");
    E("diamond_horse_armor");
    E("leather_horse_armor");
    E("chorus_fruit");
    E("popped_chorus_fruit");
    E("beetroot_seeds");
    E("shulker_shell");
    E("iron_nugget");
    E("music_disc_13");
    E("music_disc_cat");
    E("music_disc_blocks");
    E("music_disc_chirp");
    E("music_disc_far");
    E("music_disc_mall");
    E("music_disc_mellohi");
    E("music_disc_stal");
    E("music_disc_strad");
    E("music_disc_ward");
    E("music_disc_11");
    E("music_disc_wait");
    E("music_disc_pigstep");
    E("nautilus_shell");
    E("heart_of_the_sea");
    E("flower_banner_pattern");
    E("creeper_banner_pattern");
    E("skull_banner_pattern");
    E("mojang_banner_pattern");
    E("globe_banner_pattern"); // there is no globe banner pattern for bedrock
    E("piglin_banner_pattern");
    E("honeycomb");

    E("apple");
    E("mushroom_stew");
    E("bread");
    E("porkchop");
    E("cooked_porkchop");
    E("golden_apple");
    E("enchanted_golden_apple");
    E("cod");
    E("salmon");
    E("tropical_fish");
    E("pufferfish");
    E("cooked_cod");
    E("cooked_salmon");
    E("cake");
    E("cookie");
    E("melon_slice");
    E("dried_kelp");
    E("beef");
    E("cooked_beef");
    E("chicken");
    E("cooked_chicken");
    E("rotten_flesh");
    E("spider_eye");
    E("carrot");
    E("potato");
    E("baked_potato");
    E("poisonous_potato");
    E("pumpkin_pie");
    E("rabbit");
    E("cooked_rabbit");
    E("rabbit_stew");
    E("mutton");
    E("cooked_mutton");
    E("beetroot");
    E("beetroot_soup");
    E("sweet_berries");
    E("honey_bottle");

    E("flint_and_steel");
    for (string type : {"wooden", "stone", "golden", "iron", "diamond", "netherite"}) {
      for (string tool : {"shovel", "pickaxe", "axe", "hoe"}) {
        table->insert("minecraft:" + type + "_" + tool);
      }
    }
    E("compass");
    E("fishing_rod");
    E("clock");
    E("shears");
    E("enchanted_book");
    E("lead");
    E("name_tag");

    E("turtle_helmet");
    E("bow");
    E("arrow");
    E("wooden_sword");
    E("stone_sword");
    E("golden_sword");
    E("iron_sword");
    E("diamond_sword");
    E("netherite_sword");
    for (string type : {"leather", "chainmail", "iron", "diamond", "golden", "netherite"}) {
      for (string item : {"helmet", "chestplate", "leggings", "boots"}) {
        table->insert("minecraft:" + type + "_" + item);
      }
    }
    E("spectral_arrow");
    E("tipped_arrow");
    E("shield");
    E("totem_of_undying");
    E("trident");
    E("crossbow");

    E("ghast_tear");
    E("potion");
    E("glass_bottle");
    E("fermented_spider_eye");
    E("blaze_powder");
    E("magma_cream");
    E("brewing_stand");
    E("cauldron");
    E("glistering_melon_slice");
    E("golden_carrot");
    E("rabbit_foot");
    E("dragon_breath");
    E("splash_potion");
    E("lingering_potion");
    E("phantom_membrane");
    E("axolotl_bucket");
    E("suspicious_stew");

    // listed with blocks, but not block item

    E("repeater");
    E("comparator");
    E("iron_door");
    E("oak_door");
    E("spruce_door");
    E("birch_door");
    E("jungle_door");
    E("acacia_door");
    E("dark_oak_door");
    E("crimson_door");
    E("warped_door");
    E("campfire");
    E("soul_campfire");
    E("hopper");
#undef E
    return table;
  }

  static bool IsItem(std::string const &name) {
    using namespace std;
    static unique_ptr<unordered_set<string> const> const list(CreateItemNameList());
    return list->find(name) != list->end();
  }

  static ItemData AnyTorch(std::string const &name, CompoundTag const &item, Context const &) {
    using namespace std;

    map<string, string> empty;
    auto block = make_shared<Block>(name, empty);
    auto blockData = BlockData::From(block);

    auto states = Compound();
    states->set("torch_facing_direction", String("unknown"));
    blockData->set("states", states);

    auto tag = New(name, true);
    tag->set("Damage", Short(0));
    tag->set("Block", blockData);
    return Post(tag, item);
  }

  static Converter Rename(std::string const &newName) {
    return [=](std::string const &name, CompoundTag const &item, Context const &) {
      auto tag = New(newName);
      return Post(tag, item);
    };
  }

  static ItemData Skull(std::string const &name, CompoundTag const &item, Context const &) {
    int8_t type = GetSkullTypeFromBlockName(name);
    auto tag = New("skull");
    tag->set("Damage", Short(type));
    return Post(tag, item);
  }

  static ItemData SuspiciousStew(std::string const &name, CompoundTag const &j, Context const &) {
    auto b = New("suspicious_stew");
    auto tagJ = j.compoundTag("tag");
    if (tagJ) {
      auto effectsJ = tagJ->listTag("Effects");
      if (effectsJ) {
        int16_t damage = -1;
        for (auto const &it : *effectsJ) {
          auto effectJ = it->asCompound();
          if (!effectJ) {
            continue;
          }
          auto id = effectJ->byte("EffectId");
          if (!id) {
            continue;
          }
          damage = Effect::BedrockSuspiciousStewFromJavaEffect(*id);
          break;
        }
        if (damage >= 0) {
          b->set("Damage", Short(damage));
        }
      }
    }
    return Post(b, j);
  }

  static ItemData Crossbow(std::string const &name, CompoundTag const &j, Context const &ctx) {
    auto b = New("crossbow");

    if (auto tagJ = j.compoundTag("tag"); tagJ) {
      auto charged = tagJ->boolean("Charged");
      auto chargedProjectiles = tagJ->listTag("ChargedProjectiles");
      if (charged && chargedProjectiles) {
        for (auto const &it : *chargedProjectiles) {
          auto projectileJ = std::dynamic_pointer_cast<CompoundTag>(it);
          if (!projectileJ) {
            continue;
          }
          std::unordered_map<int32_t, int8_t> m;
          auto projectileB = From(projectileJ, ctx);
          if (projectileB) {
            auto beTag = Compound();
            beTag->set("chargedItem", projectileB);
            b->set("tag", beTag);
            break;
          }
        }
      }
    }
    return Post(b, j);
  }

  static ItemData Banner(std::string const &name, CompoundTag const &item, Context const &) {
    auto colorName = strings::Trim("minecraft:", name, "_banner");
    BannerColorCodeBedrock color = BannerColorCodeFromName(colorName);
    int16_t damage = (int16_t)color;
    auto ret = New("banner");
    ret->set("Damage", Short(damage));

    auto patterns = item.query("tag/BlockEntityTag/Patterns")->asList();
    auto display = item.query("tag/display")->asCompound();
    bool ominous = false;
    if (display) {
      auto json = props::GetJson(*display, "Name");
      if (json) {
        auto translate = json->find("translate");
        if (translate != json->end() && translate->is_string() && translate->get<std::string>() == "block.minecraft.ominous_banner") {
          ominous = true;
        }
      }
    }

    if (ominous) {
      auto tag = Compound();
      tag->set("Type", Int(1));
      ret->set("tag", tag);
    } else if (patterns) {
      auto bePatterns = List<Tag::Type::Compound>();
      for (auto const &it : *patterns) {
        auto c = it->asCompound();
        if (!c) {
          continue;
        }
        auto patternColor = c->int32("Color");
        auto pat = c->string("Pattern");
        if (!patternColor || !pat) {
          continue;
        }
        auto ptag = Compound();
        ptag->insert({
            {"Color", Int(static_cast<int32_t>(BannerColorCodeFromJava(static_cast<ColorCodeJava>(*patternColor))))},
            {"Pattern", String(*pat)},
        });
        bePatterns->push_back(ptag);
      }
      auto tag = Compound();
      tag->set("Patterns", bePatterns);
      ret->set("tag", tag);
    }

    return Post(ret, item);
  }

  static ItemData Bed(std::string const &name, CompoundTag const &item, Context const &) {
    using namespace std;
    string colorName = strings::Trim("minecraft:", name, "_bed");
    ColorCodeJava color = ColorCodeJavaFromJavaName(colorName);
    int16_t damage = (int16_t)color;
    auto tag = New("bed");
    tag->set("Damage", Short(damage));
    return Post(tag, item);
  }

  static ItemData MushroomBlock(std::string const &name, CompoundTag const &item, Context const &) {
    using namespace std;

    map<string, string> empty;
    auto block = make_shared<Block>(name, empty);
    auto blockData = BlockData::From(block);

    auto states = Compound();
    states->set("huge_mushroom_bits", Int(14));
    blockData->set("states", states);

    auto tag = New(name, true);
    tag->set("Damage", Short(0));
    tag->set("Block", blockData);
    return Post(tag, item);
  }

  static ItemData New(std::string const &name, bool fullname = false) {
    std::string n;
    if (fullname) {
      n = name;
    } else {
      n = "minecraft:" + name;
    }
    auto tag = Compound();
    tag->insert({
        {"Name", String(n)},
        {"WasPickedUp", Bool(false)},
        {"Count", Byte(1)},
        {"Damage", Short(0)},
    });
    return tag;
  }

  Item() = delete;

  static ItemData DefaultBlockItem(std::string const &id, CompoundTag const &item, Context const &ctx) {
    using namespace std;

    map<string, string> p;
    auto block = make_shared<Block>(id, p);
    auto blockData = BlockData::From(block);
    assert(blockData);

    auto name = blockData->string("name");
    if (!name) {
      return nullptr;
    }

    auto ret = Compound();
    ret->insert({
        {"Name", String(ItemNameFromBlockName(*name))},
        {"WasPickedUp", Bool(false)},
        {"Damage", Short(0)},
    });

    ret->set("Block", blockData);

    if (auto tagJ = item.compoundTag("tag"); tagJ) {
      if (auto blockEntityTag = tagJ->compoundTag("BlockEntityTag"); blockEntityTag) {
        Pos3i dummy(0, 0, 0);
        auto tagB = ctx.fFromBlockAndTileEntity(dummy, *block, blockEntityTag, ctx);

        static std::unordered_set<std::string> const sExclude({"Findable", "id", "isMovable", "x", "y", "z"});
        for (auto const &e : sExclude) {
          tagB->erase(e);
        }

        if (auto displayJ = tagJ->compoundTag("display"); displayJ) {
          if (auto loreJ = displayJ->listTag("Lore"); loreJ) {
            auto loreB = List<Tag::Type::String>();
            for (auto const &item : *loreJ) {
              if (auto str = item->asString(); str) {
                if (str->fValue == "\"(+NBT)\"") {
                  loreB->push_back(String("(+DATA)"));
                } else {
                  loreB->push_back(String(str->fValue));
                }
              }
            }
            auto displayB = Compound();
            displayB->set("Lore", loreB);
            tagB->set("display", displayB);
          }
        }

        ret->set("tag", tagB);
      }
    }

    return Post(ret, item);
  }

  static ItemData DefaultItem(std::string const &name, CompoundTag const &item, Context const &) {
    auto ret = Compound();
    ret->insert({
        {"Name", String(name)},
        {"WasPickedUp", Bool(false)},
        {"Damage", Short(0)},
    });
    return Post(ret, item);
  }

  static ItemData Post(ItemData const &input, CompoundTag const &item) {
    using namespace std;

    CopyByteValues(item, *input, {{"Count"}});

    auto tag = item.compoundTag("tag");
    if (!tag) {
      return input;
    }

    shared_ptr<CompoundTag> beTag = input->compoundTag("tag");
    if (!beTag) {
      beTag = Compound();
    }

    auto storedEnchantments = tag->listTag("StoredEnchantments");
    if (storedEnchantments) {
      auto ench = List<Tag::Type::Compound>();
      for (auto const &e : *storedEnchantments) {
        auto c = e->asCompound();
        if (!c) {
          continue;
        }
        auto be = EnchantData::From(*c);
        if (!be) {
          continue;
        }
        ench->push_back(be);
      }
      beTag->set("ench", ench);
    } else {
      auto enchantments = tag->listTag("Enchantments");
      if (enchantments) {
        auto ench = List<Tag::Type::Compound>();
        for (auto const &e : *enchantments) {
          auto c = e->asCompound();
          if (!c) {
            continue;
          }
          auto be = EnchantData::From(*c);
          if (!be) {
            continue;
          }
          ench->push_back(be);
        }
        beTag->set("ench", ench);
      }
    }

    auto damage = tag->int32("Damage");
    if (damage) {
      beTag->set("Damage", Int(*damage));
    }

    auto repairCost = tag->int32("RepairCost");
    if (repairCost) {
      beTag->set("RepairCost", Int(*repairCost));
    }

    auto name = GetCustomName(*tag);
    if (name) {
      auto beDisplay = Compound();
      beDisplay->set("Name", String(*name));
      beTag->set("display", beDisplay);
    }

    if (!beTag->empty()) {
      input->set("tag", beTag);
    }

    return input;
  }

  static std::optional<std::string> GetCustomName(CompoundTag const &tag) {
    using namespace std;
    using namespace je2be::props;
    auto display = tag.compoundTag("display");
    if (!display) {
      return nullopt;
    }
    auto name = display->string("Name");
    if (!name) {
      return nullopt;
    }
    auto obj = ParseAsJson(*name);
    if (obj) {
      auto text = obj->find("text");
      if (text != obj->end() && text->is_string()) {
        return text->get<string>();
      } else {
        return nullopt;
      }
    } else {
      return strings::Trim("\"", *name, "\"");
    }
  }

  // converts block name (bedrock) to item name (bedrock)
  static std::string ItemNameFromBlockName(std::string const &name) {
    if (name == "minecraft:concretePowder") {
      return "minecraft:concrete_powder";
    }
    return name;
  }
};

} // namespace je2be::tobe
