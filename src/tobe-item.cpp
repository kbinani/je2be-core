#include "tobe/_item.hpp"

#include <je2be/strings.hpp>

#include "_namespace.hpp"
#include "_nbt-ext.hpp"
#include "_props.hpp"
#include "entity/_tropical-fish.hpp"
#include "enums/_banner-color-code-bedrock.hpp"
#include "enums/_color-code-java.hpp"
#include "enums/_effect.hpp"
#include "enums/_skull-type.hpp"
#include "item/_fireworks-explosion.hpp"
#include "item/_fireworks.hpp"
#include "item/_goat-horn.hpp"
#include "item/_map-type.hpp"
#include "item/_potion.hpp"
#include "item/_tipped-arrow-potion.hpp"
#include "tobe/_axolotl.hpp"
#include "tobe/_block-data.hpp"
#include "tobe/_context.hpp"
#include "tobe/_enchant-data.hpp"
#include "tobe/_tile-entity.hpp"
#include "tobe/_world-data.hpp"

namespace je2be::tobe {

class Item::Impl {
private:
  using Converter = std::function<CompoundTagPtr(std::u8string const &, CompoundTag const &, Context &ctx)>;
  using Block = mcfile::je::Block;

public:
  static CompoundTagPtr From(CompoundTagPtr const &item, Context &ctx) {
    auto result = Convert(item, ctx);
    if (result) {
      return Post(result, *item, ctx);
    } else {
      return nullptr;
    }
  }

  static i8 GetSkullTypeFromBlockName(std::u8string_view const &name) {
    i8 type = 0;
    std::u8string n = strings::RemovePrefix(name, u8"minecraft:");
    n = strings::Remove(n, u8"_wall");
    auto st = SkullTypeFromJavaName(n);
    if (st) {
      type = static_cast<u8>(*st);
    }
    return type;
  }

  static CompoundTagPtr Empty() {
    auto armor = Compound();
    armor->set(u8"Count", Byte(0));
    armor->set(u8"Damage", Short(0));
    armor->set(u8"Name", String(u8""));
    armor->set(u8"WasPickedUp", Bool(false));
    return armor;
  }

private:
  static CompoundTagPtr Convert(CompoundTagPtr const &in, Context &ctx) {
    using namespace std;

    static unique_ptr<unordered_map<u8string, Converter> const> const blockItemMapping(CreateBlockItemConverterTable());
    static unique_ptr<unordered_map<u8string, Converter> const> const itemMapping(CreateItemConverterTable());

    CompoundTagPtr item = in;
    u8string name;

    if (auto id = item->string(u8"id"); id) {
      name = *id;
      auto damageTag = in->int16(u8"Damage");
      if (damageTag) {
        i16 damage = *damageTag;
        auto n = mcfile::je::Flatten::Item(*id, &damage);
        if (n) {
          name = *n;
          item = in->copy();
          item->set(u8"Damage", Short(damage));
        }
      }
    } else {
      auto idTag = in->int16(u8"id");
      auto damageTag = in->int16(u8"Damage");
      if (!idTag || !damageTag) {
        return nullptr;
      }
      i16 damage = *damageTag;
      auto n = mcfile::je::Flatten::Item(*idTag, &damage);
      if (!n) {
        return nullptr;
      }
      name = *n;
      item = in->copy();
      item->set(u8"Damage", Short(damage));
    }

    if (name == u8"minecraft:filled_map") {
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
      auto found = itemMapping->find(Namespace::Remove(name));
      if (found == itemMapping->end()) {
        return DefaultItem(name, *item, ctx);
      } else {
        return found->second(name, *item, ctx);
      }
    } else {
      auto found = blockItemMapping->find(Namespace::Remove(name));
      if (found == blockItemMapping->end()) {
        return DefaultBlockItem(name, *item, ctx);
      } else {
        return found->second(name, *item, ctx);
      }
    }
  }

  static std::unordered_map<std::u8string, Converter> *CreateBlockItemConverterTable() {
    using namespace std;
    auto table = new unordered_map<u8string, Converter>();
#define E(__name, __func) table->insert(make_pair(u8"" #__name, __func))
    E(brown_mushroom_block, MushroomBlock);
    E(red_mushroom_block, MushroomBlock);
    E(white_bed, Bed);
    E(orange_bed, Bed);
    E(magenta_bed, Bed);
    E(light_blue_bed, Bed);
    E(yellow_bed, Bed);
    E(lime_bed, Bed);
    E(pink_bed, Bed);
    E(gray_bed, Bed);
    E(light_gray_bed, Bed);
    E(cyan_bed, Bed);
    E(purple_bed, Bed);
    E(blue_bed, Bed);
    E(brown_bed, Bed);
    E(green_bed, Bed);
    E(red_bed, Bed);
    E(black_bed, Bed);
    E(white_banner, Banner);
    E(orange_banner, Banner);
    E(magenta_banner, Banner);
    E(light_blue_banner, Banner);
    E(yellow_banner, Banner);
    E(lime_banner, Banner);
    E(pink_banner, Banner);
    E(gray_banner, Banner);
    E(light_gray_banner, Banner);
    E(cyan_banner, Banner);
    E(purple_banner, Banner);
    E(blue_banner, Banner);
    E(brown_banner, Banner);
    E(green_banner, Banner);
    E(red_banner, Banner);
    E(black_banner, Banner);

    E(skeleton_skull, Skull);
    E(wither_skeleton_skull, Skull);
    E(player_head, Skull);
    E(zombie_head, Skull);
    E(creeper_head, Skull);
    E(dragon_head, Skull);
    E(piglin_head, Skull);

    E(item_frame, Rename(u8"frame"));
    E(glow_item_frame, Rename(u8"glow_frame"));
    E(repeater, DefaultItem);
    E(comparator, DefaultItem);

    E(iron_door, DefaultItem);
    E(oak_door, Rename(u8"wooden_door"));
    E(spruce_door, DefaultItem);
    E(birch_door, DefaultItem);
    E(jungle_door, DefaultItem);
    E(acacia_door, DefaultItem);
    E(dark_oak_door, DefaultItem);
    E(crimson_door, DefaultItem);
    E(warped_door, DefaultItem);
    E(mangrove_door, DefaultItem);

    E(campfire, DefaultItem);
    E(soul_campfire, DefaultItem);
    E(chain, DefaultItem);

    E(torch, AnyTorch);
    E(soul_torch, AnyTorch);
    E(redstone_torch, AnyTorch);

    E(oak_sign, Sign);
    E(birch_sign, Sign);
    E(spruce_sign, Sign);
    E(jungle_sign, Sign);
    E(acacia_sign, Sign);
    E(dark_oak_sign, Sign);
    E(crimson_sign, Sign);
    E(warped_sign, Sign);
    E(mangrove_sign, Sign);
    E(bamboo_sign, Sign);
    E(cherry_sign, Sign);

    E(goat_horn, GoatHorn);
    E(frogspawn, Rename(u8"frog_spawn"));

    E(acacia_hanging_sign, Sign);
    E(bamboo_hanging_sign, Sign);
    E(birch_hanging_sign, Sign);
    E(crimson_hanging_sign, Sign);
    E(dark_oak_hanging_sign, Sign);
    E(jungle_hanging_sign, Sign);
    E(mangrove_hanging_sign, Sign);
    E(oak_hanging_sign, Sign);
    E(spruce_hanging_sign, Sign);
    E(warped_hanging_sign, Sign);
    E(cherry_hanging_sign, Sign);

    // Java: nether_bricks (block), nether_brick (item)
    // Bedrock: nether_brick (block), netherbrick (item)
    E(nether_brick, Rename(u8"nether_bricks"));
#undef E
    return table;
  }

  static std::unordered_map<std::u8string, Converter> *CreateItemConverterTable() {
    using namespace std;
    auto table = new unordered_map<u8string, Converter>();
#define E(__name, __func) table->insert(make_pair(u8"" #__name, __func))
    E(oak_door, Rename(u8"wooden_door"));
    E(furnace_minecart, Rename(u8"minecart")); // furnace minecart does not exist in bedrock
    E(tropical_fish_bucket, TropicalFishBucket);
    E(zombified_piglin_spawn_egg, Rename(u8"zombie_pigman_spawn_egg"));

    E(map, Subtype(u8"empty_map", 2));
    E(firework_rocket, FireworkRocket);
    E(firework_star, FireworkStar);

    // "13", "cat", "blocks", "chirp", "far", "mall", "mellohi", "stal", "strad", "ward", "11", "wait", "otherside", "pigstep"
    // E(u8"music_disc_pigstep", Same);

    E(cod, Rename(u8"fish"));
    E(tipped_arrow, TippedArrow);
    E(potion, AnyPotion);
    E(splash_potion, AnyPotion);
    E(lingering_potion, AnyPotion);

    for (u8string type : {u8"helmet", u8"chestplate", u8"leggings", u8"boots"}) {
      table->insert(std::make_pair(u8"leather_" + type, LeatherArmor));
    }

    E(writable_book, BooksAndQuill);
    E(written_book, BooksAndQuill);

    E(axolotl_bucket, AxolotlBucket);

    E(suspicious_stew, SuspiciousStew);
    E(crossbow, Crossbow);
    E(nether_brick, Rename(u8"netherbrick"));

    E(spawn_egg, LegacySpawnEgg); // legacy
    E(pottery_shard_archer, Rename(u8"archer_pottery_shard"));
    E(pottery_shard_prize, Rename(u8"prize_pottery_shard"));
    E(pottery_shard_arms_up, Rename(u8"arms_up_pottery_shard"));
    E(pottery_shard_skull, Rename(u8"skull_pottery_shard"));
#undef E
    return table;
  }

  static CompoundTagPtr Sign(std::u8string const &name, CompoundTag const &item, Context &ctx) {
    auto ret = DefaultBlockItem(name, item, ctx);
    if (!ret) {
      return nullptr;
    }
    ret->set(u8"Name", String(name));
    ret->erase(u8"Block");
    return ret;
  }

  static CompoundTagPtr AxolotlBucket(std::u8string const &name, CompoundTag const &item, Context const &) {
    auto ret = New(u8"axolotl_bucket");
    ret->set(u8"Damage", Short(0));
    auto tg = item.compoundTag(u8"tag");
    if (tg) {
      auto age = tg->int32(u8"Age", 0);
      auto health = tg->float32(u8"Health", 14);
      auto variant = tg->int32(u8"Variant", 0);
      auto axltl = Axolotl(age, health, variant);
      auto tag = axltl.toBucketTag();

      auto customName = GetCustomName(*tg);
      if (customName) {
        tag->set(u8"CustomName", String(*customName));
        tag->set(u8"CustomNameVisible", Bool(true));
      }

      ret->set(u8"tag", tag);
    }
    return ret;
  }

  static CompoundTagPtr TropicalFishBucket(std::u8string const &name, CompoundTag const &item, Context const &) {
    auto ret = New(u8"tropical_fish_bucket");
    ret->set(u8"Damage", Short(0));
    auto tg = item.compoundTag(u8"tag");
    if (tg) {
      auto variant = tg->intTag(u8"BucketVariantTag");
      if (variant) {
        auto tf = TropicalFish::FromJavaVariant(variant->fValue);
        auto tag = tf.toBedrockBucketTag();
        ret->set(u8"tag", tag);
      }
    }
    return ret;
  }

  static CompoundTagPtr BooksAndQuill(std::u8string const &name, CompoundTag const &item, Context const &) {
    using namespace std;

    auto tag = Compound();
    tag->insert({
        {u8"Name", String(name)},
        {u8"WasPickedUp", Bool(false)},
        {u8"Damage", Short(0)},
    });

    auto tg = item.compoundTag(u8"tag");
    if (tg) {
      auto outTag = Compound();
      auto author = tg->stringTag(u8"author");
      if (author) {
        outTag->set(u8"author", String(author->fValue));
      }
      auto title = tg->stringTag(u8"title");
      if (title) {
        outTag->set(u8"title", String(title->fValue));
      }
      if (title || author) {
        outTag->set(u8"generation", Int(0));
      }
      auto pages = tg->listTag(u8"pages");
      if (pages) {
        auto outPages = List<Tag::Type::Compound>();
        for (auto const &it : *pages) {
          auto page = it->asString();
          if (!page) {
            continue;
          }
          u8string lineText;
          auto obj = props::ParseAsJson(page->fValue);
          if (obj) {
            auto text = obj->find("text");
            auto extra = obj->find("extra");
            if (extra != obj->end() && extra->is_array()) {
              vector<u8string> lines;
              for (auto const &line : *extra) {
                auto t = line.find("text");
                if (t != line.end() && t->is_string()) {
                  u8string l = props::GetJsonStringValue(*t);
                  if (l == u8"\x0d\x0a") {
                    l = u8"";
                  }
                  lines.push_back(l);
                }
              }
              lineText = u8"";
              for (int i = 0; i < lines.size(); i++) {
                lineText += lines[i];
                if (i + 1 < lines.size()) {
                  lineText += u8"\x0d\x0a";
                }
              }
            } else if (text != obj->end()) {
              lineText = props::GetJsonStringValue(*text);
            }
          } else {
            lineText = page->fValue;
          }
          auto lineObj = Compound();
          lineObj->insert({
              {u8"photoname", String(u8"")},
              {u8"text", String(lineText)},
          });
          outPages->push_back(lineObj);
        }
        outTag->set(u8"pages", outPages);
      }
      tag->set(u8"tag", outTag);
    }
    return tag;
  }

  static CompoundTagPtr LeatherArmor(std::u8string const &name, CompoundTag const &item, Context const &) {
    auto tag = Compound();
    tag->insert({
        {u8"Name", String(name)},
        {u8"WasPickedUp", Bool(false)},
        {u8"Damage", Short(0)},
    });

    auto customColor = item.query(u8"tag/display/color")->asInt();
    if (customColor) {
      u32 v = 0xff000000 | *(u32 *)&customColor->fValue;
      auto t = Compound();
      t->set(u8"customColor", Int(*(i32 *)&v));
      tag->set(u8"tag", t);
    }
    return tag;
  }

  static std::optional<std::tuple<int, CompoundTagPtr>> Map(std::u8string const &name, CompoundTag const &item, JavaEditionMap const &mapInfo) {
    auto ret = New(name, true);
    ret->set(u8"Damage", Short(0));

    auto number = item.query(u8"tag/map")->asInt();
    if (!number) {
      return std::nullopt;
    }
    i32 mapId = number->fValue;

    auto scale = mapInfo.getScale(mapId);
    if (!scale) {
      return std::nullopt;
    }
    i64 uuid = Map::UUID(mapId, *scale);

    auto tag = Compound();
    tag->set(u8"map_uuid", Long(uuid));
    tag->set(u8"map_display_players", Byte(1));
    ret->set(u8"tag", tag);

    i16 type = 0;
    auto display = item.query(u8"tag/display")->asCompound();
    if (display) {
      auto displayName = display->string(u8"Name");
      if (displayName) {
        auto nameJson = props::ParseAsJson(*displayName);
        if (nameJson) {
          auto translate = nameJson->find("translate");
          if (translate != nameJson->end() && translate->is_string()) {
            auto translationKey = props::GetJsonStringValue(*translate);
            if (auto damage = MapType::BedrockDamageFromJavaTranslationKey(translationKey); damage) {
              type = *damage;
            }
          }
        }
      }
    }
    tag->set(u8"map_name_index", Int(number->fValue));
    ret->set(u8"Damage", Short(type));

    return make_tuple(mapId, ret);
  }

  static CompoundTagPtr AnyPotion(std::u8string const &name, CompoundTag const &item, Context const &) {
    std::u8string itemName = name;
    std::optional<i16> type;
    if (auto t = item.query(u8"tag")->asCompound(); t) {
      if (auto potion = t->string(u8"Potion"); potion) {
        type = Potion::BedrockPotionTypeFromJava(*potion);
      }
    }
    if (!type) {
      if (auto damage = item.int16(u8"Damage"); damage) {
        if (auto ret = Potion::JavaPotionTypeAndItemNameFromLegacyJavaDamage(*damage); ret) {
          itemName = ret->fItemName;
          type = Potion::BedrockPotionTypeFromJava(ret->fPotionType);
        }
      }
    }
    if (!type) {
      type = 0;
    }
    auto tag = New(itemName, true);
    tag->set(u8"Damage", Short(*type));
    return tag;
  }

  static CompoundTagPtr TippedArrow(std::u8string const &name, CompoundTag const &item, Context const &) {
    auto tag = New(u8"arrow");
    auto t = item.query(u8"tag")->asCompound();
    i16 type = 0;
    if (t) {
      auto potion = t->string(u8"Potion", u8"");
      type = TippedArrowPotion::BedrockPotionType(potion);
    }
    tag->set(u8"Damage", Short(type));
    return tag;
  }

  static CompoundTagPtr FireworkStar(std::u8string const &name, CompoundTag const &item, Context &ctx) {
    auto data = Rename(u8"firework_star")(name, item, ctx);

    auto explosion = item.query(u8"tag/Explosion")->asCompound();
    if (explosion) {
      auto tag = Compound();

      auto e = FireworksExplosion::FromJava(*explosion);
      if (!e.fColor.empty()) {
        i32 customColor = e.fColor[0].toARGB();
        tag->set(u8"customColor", Int(customColor));
      }
      tag->set(u8"FireworksItem", e.toBedrockCompoundTag());
      data->set(u8"tag", tag);
    }

    return data;
  }

  static CompoundTagPtr FireworkRocket(std::u8string const &name, CompoundTag const &item, Context const &) {
    auto data = New(u8"firework_rocket");
    auto fireworks = item.query(u8"tag/Fireworks")->asCompound();
    if (fireworks) {
      auto fireworksData = FireworksData::FromJava(*fireworks);
      auto tag = Compound();
      tag->set(u8"Fireworks", fireworksData.toBedrockCompoundTag());
      data->set(u8"tag", tag);
    }
    return data;
  }

  static Converter Subtype(std::u8string const &newName, i16 damage) {
    return [=](std::u8string const &name, CompoundTag const &item, Context const &) {
      auto tag = New(newName);
      tag->set(u8"Damage", Short(damage));
      return tag;
    };
  }

  static std::unordered_set<std::u8string> *CreateItemNameList() {
    using namespace std;
    auto table = new unordered_set<u8string>();
#define E(__name) table->insert(u8"" #__name)
    E(minecart);
    E(saddle);
    E(oak_boat);
    E(chest_minecart);
    E(furnace_minecart);
    E(carrot_on_a_stick);
    E(warped_fungus_on_a_stick);
    E(tnt_minecart);
    E(hopper_minecart);
    E(elytra);
    E(spruce_boat);
    E(birch_boat);
    E(jungle_boat);
    E(acacia_boat);
    E(dark_oak_boat);

    E(beacon);
    E(turtle_egg);
    E(conduit);
    E(scute);
    E(coal);
    E(charcoal);
    E(diamond);
    E(iron_ingot);
    E(gold_ingot);
    E(netherite_ingot);
    E(netherite_scrap);
    E(stick);
    E(bowl);
    E(string);
    E(feather);
    E(gunpowder);
    E(wheat_seeds);
    E(wheat);
    E(flint);
    E(bucket);
    E(water_bucket);
    E(lava_bucket);
    E(snowball);
    E(leather);
    E(milk_bucket);
    E(pufferfish_bucket);
    E(salmon_bucket);
    E(cod_bucket);
    E(tropical_fish_bucket);
    E(brick);
    E(clay_ball);
    E(paper);
    E(book);
    E(slime_ball);
    E(egg);
    E(glowstone_dust);
    E(ink_sac);
    E(cocoa_beans);
    E(lapis_lazuli);
    E(white_dye);
    E(orange_dye);
    E(magenta_dye);
    E(light_blue_dye);
    E(yellow_dye);
    E(lime_dye);
    E(pink_dye);
    E(gray_dye);
    E(light_gray_dye);
    E(cyan_dye);
    E(purple_dye);
    E(blue_dye);
    E(brown_dye);
    E(green_dye);
    E(red_dye);
    E(black_dye);
    E(bone_meal);
    E(bone);
    E(sugar);
    E(pumpkin_seeds);
    E(melon_seeds);
    E(ender_pearl);
    E(blaze_rod);
    E(gold_nugget);
    E(nether_wart);
    E(ender_eye);
    E(bat_spawn_egg);
    E(bee_spawn_egg);
    E(blaze_spawn_egg);
    E(cat_spawn_egg);
    E(cave_spider_spawn_egg);
    E(chicken_spawn_egg);
    E(cod_spawn_egg);
    E(cow_spawn_egg);
    E(creeper_spawn_egg);
    E(dolphin_spawn_egg);
    E(donkey_spawn_egg);
    E(drowned_spawn_egg);
    E(elder_guardian_spawn_egg);
    E(enderman_spawn_egg);
    E(endermite_spawn_egg);
    E(evoker_spawn_egg);
    E(fox_spawn_egg);
    E(ghast_spawn_egg);
    E(guardian_spawn_egg);
    E(hoglin_spawn_egg);
    E(horse_spawn_egg);
    E(husk_spawn_egg);
    E(llama_spawn_egg);
    E(magma_cube_spawn_egg);
    E(mooshroom_spawn_egg);
    E(mule_spawn_egg);
    E(ocelot_spawn_egg);
    E(panda_spawn_egg);
    E(parrot_spawn_egg);
    E(phantom_spawn_egg);
    E(pig_spawn_egg);
    E(piglin_spawn_egg);
    E(piglin_brute_spawn_egg);
    E(pillager_spawn_egg);
    E(polar_bear_spawn_egg);
    E(pufferfish_spawn_egg);
    E(rabbit_spawn_egg);
    E(ravager_spawn_egg);
    E(salmon_spawn_egg);
    E(sheep_spawn_egg);
    E(shulker_spawn_egg);
    E(silverfish_spawn_egg);
    E(skeleton_spawn_egg);
    E(skeleton_horse_spawn_egg);
    E(slime_spawn_egg);
    E(spider_spawn_egg);
    E(squid_spawn_egg);
    E(stray_spawn_egg);
    E(strider_spawn_egg);
    E(trader_llama_spawn_egg); // there is no spawn egg for trader llama for bedrock
    E(tropical_fish_spawn_egg);
    E(turtle_spawn_egg);
    E(vex_spawn_egg);
    E(villager_spawn_egg);
    E(vindicator_spawn_egg);
    E(wandering_trader_spawn_egg);
    E(witch_spawn_egg);
    E(wither_skeleton_spawn_egg);
    E(wolf_spawn_egg);
    E(zoglin_spawn_egg);
    E(zombie_spawn_egg);
    E(zombie_horse_spawn_egg);
    E(zombie_villager_spawn_egg);
    E(zombified_piglin_spawn_egg);
    E(experience_bottole);
    E(fire_charge);
    E(writable_book);
    E(written_book);
    E(emerald);
    E(map);
    E(nether_star);
    E(firework_rocket);
    E(firework_star);
    E(nether_brick);
    E(quartz);
    E(prismarine_shard);
    E(prismarine_crystals);
    E(rabbit_hide);
    E(iron_horse_armor);
    E(golden_horse_armor);
    E(diamond_horse_armor);
    E(leather_horse_armor);
    E(chorus_fruit);
    E(popped_chorus_fruit);
    E(beetroot_seeds);
    E(shulker_shell);
    E(iron_nugget);
    E(music_disc_13);
    E(music_disc_cat);
    E(music_disc_blocks);
    E(music_disc_chirp);
    E(music_disc_far);
    E(music_disc_mall);
    E(music_disc_mellohi);
    E(music_disc_stal);
    E(music_disc_strad);
    E(music_disc_ward);
    E(music_disc_11);
    E(music_disc_wait);
    E(music_disc_pigstep);
    E(music_disc_otherside);
    E(music_disc_5);
    E(disc_fragment_5);
    E(nautilus_shell);
    E(heart_of_the_sea);
    E(flower_banner_pattern);
    E(creeper_banner_pattern);
    E(skull_banner_pattern);
    E(mojang_banner_pattern);
    E(globe_banner_pattern); // there is no globe banner pattern for bedrock
    E(piglin_banner_pattern);
    E(honeycomb);

    E(apple);
    E(mushroom_stew);
    E(bread);
    E(porkchop);
    E(cooked_porkchop);
    E(golden_apple);
    E(enchanted_golden_apple);
    E(cod);
    E(salmon);
    E(tropical_fish);
    E(pufferfish);
    E(cooked_cod);
    E(cooked_salmon);
    E(cake);
    E(cookie);
    E(melon_slice);
    E(dried_kelp);
    E(beef);
    E(cooked_beef);
    E(chicken);
    E(cooked_chicken);
    E(rotten_flesh);
    E(spider_eye);
    E(carrot);
    E(potato);
    E(baked_potato);
    E(poisonous_potato);
    E(pumpkin_pie);
    E(rabbit);
    E(cooked_rabbit);
    E(rabbit_stew);
    E(mutton);
    E(cooked_mutton);
    E(beetroot);
    E(beetroot_soup);
    E(sweet_berries);
    E(honey_bottle);

    E(flint_and_steel);
    for (u8string type : {u8"wooden", u8"stone", u8"golden", u8"iron", u8"diamond", u8"netherite"}) {
      for (u8string tool : {u8"shovel", u8"pickaxe", u8"axe", u8"hoe"}) {
        table->insert(type + u8"_" + tool);
      }
    }
    E(compass);
    E(fishing_rod);
    E(clock);
    E(shears);
    E(enchanted_book);
    E(lead);
    E(name_tag);

    E(turtle_helmet);
    E(bow);
    E(arrow);
    E(wooden_sword);
    E(stone_sword);
    E(golden_sword);
    E(iron_sword);
    E(diamond_sword);
    E(netherite_sword);
    for (u8string type : {u8"leather", u8"chainmail", u8"iron", u8"diamond", u8"golden", u8"netherite"}) {
      for (u8string item : {u8"helmet", u8"chestplate", u8"leggings", u8"boots"}) {
        table->insert(type + u8"_" + item);
      }
    }
    E(spectral_arrow);
    E(tipped_arrow);
    E(shield);
    E(totem_of_undying);
    E(trident);
    E(crossbow);

    E(ghast_tear);
    E(potion);
    E(glass_bottle);
    E(fermented_spider_eye);
    E(blaze_powder);
    E(magma_cream);
    E(brewing_stand);
    E(cauldron);
    E(glistering_melon_slice);
    E(golden_carrot);
    E(rabbit_foot);
    E(dragon_breath);
    E(splash_potion);
    E(lingering_potion);
    E(phantom_membrane);
    E(axolotl_bucket);
    E(suspicious_stew);
    E(raw_iron);

    E(oak_chest_boat);
    E(birch_chest_boat);
    E(spruce_chest_boat);
    E(acacia_chest_boat);
    E(jungle_chest_boat);
    E(dark_oak_chest_boat);
    E(mangrove_chest_boat);
    E(warden_spawn_egg);
    E(tadpole_spawn_egg);
    E(frog_spawn_egg);
    E(tadpole_bucket);
    E(mangrove_boat);
    E(echo_shard);
    E(recovery_compass);
    E(spawn_egg); // legacy
    E(torchflower_seeds);
    E(pottery_shard_archer);
    E(pottery_shard_prize);
    E(pottery_shard_arms_up);
    E(pottery_shard_skull);
    E(sniffer_spawn_egg);
    E(camel_spawn_egg);
    E(brush);
    E(cherry_boat);
    E(cherry_chest_boat);
    E(netherite_upgrade_smithing_template);
    E(sentry_armor_trim_smithing_template);
    E(dune_armor_trim_smithing_template);
    E(coast_armor_trim_smithing_template);
    E(wild_armor_trim_smithing_template);
    E(ward_armor_trim_smithing_template);
    E(eye_armor_trim_smithing_template);
    E(vex_armor_trim_smithing_template);
    E(tide_armor_trim_smithing_template);
    E(snout_armor_trim_smithing_template);
    E(rib_armor_trim_smithing_template);
    E(spire_armor_trim_smithing_template);

    // listed with blocks, but not block item

    E(repeater);
    E(comparator);
    E(iron_door);
    E(oak_door);
    E(spruce_door);
    E(birch_door);
    E(jungle_door);
    E(acacia_door);
    E(dark_oak_door);
    E(crimson_door);
    E(warped_door);
    E(campfire);
    E(soul_campfire);
    E(hopper);
    E(flower_pot);
#undef E
    return table;
  }

  static bool IsItem(std::u8string const &name) {
    using namespace std;
    static unique_ptr<unordered_set<u8string> const> const list(CreateItemNameList());
    return list->find(Namespace::Remove(name)) != list->end();
  }

  static CompoundTagPtr AnyTorch(std::u8string const &name, CompoundTag const &item, Context const &) {
    using namespace std;

    auto block = make_shared<Block>(name);
    auto blockData = BlockData::From(block, nullptr);

    auto states = Compound();
    states->set(u8"torch_facing_direction", String(u8"unknown"));
    blockData->set(u8"states", states);

    auto tag = New(name, true);
    tag->set(u8"Damage", Short(0));
    tag->set(u8"Block", blockData);
    return tag;
  }

  static Converter Rename(std::u8string const &newName) {
    return [=](std::u8string const &name, CompoundTag const &item, Context const &) {
      auto tag = New(newName);
      return tag;
    };
  }

  static CompoundTagPtr Skull(std::u8string const &name, CompoundTag const &item, Context const &) {
    i8 type = GetSkullTypeFromBlockName(name);
    auto tag = New(u8"skull");
    tag->set(u8"Damage", Short(type));
    return tag;
  }

  static CompoundTagPtr LegacySpawnEgg(std::u8string const &name, CompoundTag const &j, Context &ctx) {
    std::u8string n = name;
    if (auto tag = j.compoundTag(u8"tag"); tag) {
      if (auto entityTag = tag->compoundTag(u8"EntityTag"); entityTag) {
        if (auto id = entityTag->string(u8"id"); id) {
          std::u8string entity = Namespace::Remove(*id);
          if (entity == u8"evocation_illager") {
            entity = u8"evoker";
          } else if (entity == u8"vindication_illager") {
            entity = u8"vindicator";
          }
          n = u8"minecraft:" + entity + u8"_spawn_egg";
        }
      }
    }
    return New(n, true);
  }

  static CompoundTagPtr SuspiciousStew(std::u8string const &name, CompoundTag const &j, Context const &) {
    auto b = New(u8"suspicious_stew");
    auto tagJ = j.compoundTag(u8"tag");
    if (tagJ) {
      i16 damage = -1;
      if (auto effectsJ = tagJ->listTag(u8"effects"); effectsJ) {
        // >= 1.20.2
        for (auto const &it : *effectsJ) {
          auto effectJ = it->asCompound();
          if (!effectJ) {
            continue;
          }
          auto idJ = effectJ->string(u8"id");
          if (!idJ) {
            continue;
          }
          if (auto legacyIdJ = Effect::LegacyIdFromNamespacedId(*idJ); legacyIdJ) {
            damage = Effect::BedrockSuspiciousStewFromJavaEffect(*legacyIdJ);
            break;
          }
        }
      } else if (auto legacyEffectsJ = tagJ->listTag(u8"Effects"); legacyEffectsJ) {
        // <= 1.20.1
        for (auto const &it : *legacyEffectsJ) {
          auto effectJ = it->asCompound();
          if (!effectJ) {
            continue;
          }
          auto id = effectJ->byte(u8"EffectId");
          if (!id) {
            continue;
          }
          damage = Effect::BedrockSuspiciousStewFromJavaEffect(*id);
          break;
        }
      }
      if (damage >= 0) {
        b->set(u8"Damage", Short(damage));
      }
    }
    return b;
  }

  static CompoundTagPtr Crossbow(std::u8string const &name, CompoundTag const &j, Context &ctx) {
    auto b = New(u8"crossbow");

    if (auto tagJ = j.compoundTag(u8"tag"); tagJ) {
      auto charged = tagJ->boolean(u8"Charged");
      auto chargedProjectiles = tagJ->listTag(u8"ChargedProjectiles");
      if (charged && chargedProjectiles) {
        for (auto const &it : *chargedProjectiles) {
          auto projectileJ = std::dynamic_pointer_cast<CompoundTag>(it);
          if (!projectileJ) {
            continue;
          }
          auto projectileB = From(projectileJ, ctx);
          if (projectileB) {
            auto beTag = Compound();
            beTag->set(u8"chargedItem", projectileB);
            b->set(u8"tag", beTag);
            break;
          }
        }
      }
    }
    return b;
  }

  static CompoundTagPtr Banner(std::u8string const &name, CompoundTag const &item, Context const &) {
    auto colorName = strings::RemovePrefixAndSuffix(u8"minecraft:", name, u8"_banner");
    BannerColorCodeBedrock color = BannerColorCodeFromName(colorName);
    i16 damage = (i16)color;
    auto ret = New(u8"banner");
    ret->set(u8"Damage", Short(damage));

    auto patterns = item.query(u8"tag/BlockEntityTag/Patterns")->asList();
    auto display = item.query(u8"tag/display")->asCompound();
    bool ominous = false;
    if (display) {
      auto json = props::GetJson(*display, u8"Name");
      if (json) {
        auto translate = json->find("translate");
        if (translate != json->end() && translate->is_string() && props::GetJsonStringValue(*translate) == u8"block.minecraft.ominous_banner") {
          ominous = true;
        }
      }
    }

    if (ominous) {
      auto tag = Compound();
      tag->set(u8"Type", Int(1));
      ret->set(u8"tag", tag);
    } else if (patterns) {
      auto bePatterns = List<Tag::Type::Compound>();
      for (auto const &it : *patterns) {
        auto c = it->asCompound();
        if (!c) {
          continue;
        }
        auto patternColor = c->int32(u8"Color");
        auto pat = c->string(u8"Pattern");
        if (!patternColor || !pat) {
          continue;
        }
        auto ptag = Compound();
        ptag->insert({
            {u8"Color", Int(static_cast<i32>(BannerColorCodeFromJava(static_cast<ColorCodeJava>(*patternColor))))},
            {u8"Pattern", String(*pat)},
        });
        bePatterns->push_back(ptag);
      }
      auto tag = Compound();
      tag->set(u8"Patterns", bePatterns);
      ret->set(u8"tag", tag);
    }

    return ret;
  }

  static CompoundTagPtr Bed(std::u8string const &name, CompoundTag const &item, Context const &) {
    using namespace std;
    u8string colorName = strings::RemovePrefixAndSuffix(u8"minecraft:", name, u8"_bed");
    ColorCodeJava color = ColorCodeJavaFromJavaName(colorName);
    i16 damage = (i16)color;
    auto tag = New(u8"bed");
    tag->set(u8"Damage", Short(damage));
    return tag;
  }

  static CompoundTagPtr MushroomBlock(std::u8string const &name, CompoundTag const &item, Context const &) {
    using namespace std;

    auto block = make_shared<Block>(name);
    auto blockData = BlockData::From(block, nullptr);

    auto states = Compound();
    states->set(u8"huge_mushroom_bits", Int(14));
    blockData->set(u8"states", states);

    auto tag = New(name, true);
    tag->set(u8"Damage", Short(0));
    tag->set(u8"Block", blockData);
    return tag;
  }

  static CompoundTagPtr GoatHorn(std::u8string const &name, CompoundTag const &item, Context const &) {
    auto tagB = New(u8"goat_horn");
    i16 damage = 0;
    if (auto tagJ = item.compoundTag(u8"tag"); tagJ) {
      if (auto instrumentJ = tagJ->string(u8"instrument"); instrumentJ) {
        damage = GoatHorn::BedrockDamageFromJavaInstrument(*instrumentJ);
      }
    }
    tagB->set(u8"Damage", Short(damage));
    return tagB;
  }

  static CompoundTagPtr New(std::u8string const &name, bool fullname = false) {
    std::u8string n;
    if (fullname) {
      n = name;
    } else {
      n = u8"minecraft:" + name;
    }
    auto tag = Compound();
    tag->insert({
        {u8"Name", String(n)},
        {u8"WasPickedUp", Bool(false)},
        {u8"Count", Byte(1)},
        {u8"Damage", Short(0)},
    });
    return tag;
  }

  Impl() = delete;

  static CompoundTagPtr DefaultBlockItem(std::u8string const &id, CompoundTag const &item, Context &ctx) {
    using namespace std;

    auto block = make_shared<Block>(id);
    auto blockData = BlockData::From(block, nullptr);
    assert(blockData);

    auto name = blockData->string(u8"name");
    if (!name) {
      return nullptr;
    }

    auto ret = Compound();
    ret->insert({
        {u8"Name", String(ItemNameFromBlockName(*name))},
        {u8"WasPickedUp", Bool(false)},
        {u8"Damage", Short(0)},
    });

    ret->set(u8"Block", blockData);

    return ret;
  }

  static CompoundTagPtr DefaultItem(std::u8string const &name, CompoundTag const &item, Context &ctx) {
    auto ret = Compound();
    ret->insert({
        {u8"Name", String(name)},
        {u8"WasPickedUp", Bool(false)},
        {u8"Damage", Short(0)},
    });
    return ret;
  }

  static CompoundTagPtr Post(CompoundTagPtr const &itemB, CompoundTag const &itemJ, Context &ctx) {
    using namespace std;

    CopyByteValues(itemJ, *itemB, {{u8"Count"}});

    auto tagJ = itemJ.compoundTag(u8"tag");
    if (!tagJ) {
      return itemB;
    }

    auto tagB = itemB->compoundTag(u8"tag");
    if (!tagB) {
      tagB = Compound();
    }

    auto storedEnchantments = tagJ->listTag(u8"StoredEnchantments");
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
      tagB->set(u8"ench", ench);
    } else {
      auto enchantments = tagJ->listTag(u8"Enchantments");
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
        tagB->set(u8"ench", ench);
      }
    }

    auto damage = tagJ->int32(u8"Damage");
    if (damage) {
      tagB->set(u8"Damage", Int(*damage));
    }

    auto repairCost = tagJ->int32(u8"RepairCost");
    if (repairCost) {
      tagB->set(u8"RepairCost", Int(*repairCost));
    }

    CompoundTagPtr displayB = tagB->compoundTag(u8"display");

    auto name = GetCustomName(*tagJ);
    if (name) {
      if (!displayB) {
        displayB = Compound();
      }
      displayB->set(u8"Name", String(*name));
    }

    if (auto displayJ = tagJ->compoundTag(u8"display"); displayJ) {
      if (auto loreJ = displayJ->listTag(u8"Lore"); loreJ) {
        auto loreB = List<Tag::Type::String>();
        for (auto const &item : *loreJ) {
          if (auto str = item->asString(); str) {
            if (str->fValue == u8"\"(+NBT)\"") {
              loreB->push_back(String(u8"(+DATA)"));
            } else {
              loreB->push_back(String(str->fValue));
            }
          }
        }
        if (!displayB) {
          displayB = Compound();
        }
        displayB->set(u8"Lore", loreB);
      }
    }

    if (auto blockEntityTagJ = tagJ->compoundTag(u8"BlockEntityTag"); blockEntityTagJ) {
      if (auto id = itemJ.string(u8"id"); id) {
        auto block = std::make_shared<Block>(*id);

        Pos3i dummy(0, 0, 0);
        if (auto blockEntityTagB = TileEntity::FromBlockAndTileEntity(dummy, *block, blockEntityTagJ, ctx); blockEntityTagB) {
          static unordered_set<u8string> const sExclude({u8"Findable", u8"id", u8"isMovable", u8"x", u8"y", u8"z"});
          for (auto const &e : sExclude) {
            blockEntityTagB->erase(e);
          }
          for (auto const &it : *blockEntityTagB) {
            if (tagB->find(it.first) == tagB->end()) {
              tagB->set(it.first, it.second);
            }
          }
        }
      }
    }

    if (auto trimJ = tagJ->compoundTag(u8"Trim"); trimJ) {
      auto materialJ = trimJ->string(u8"material");
      auto patternJ = trimJ->string(u8"pattern");
      if (materialJ && patternJ) {
        auto materialB = Namespace::Remove(*materialJ);
        auto patternB = Namespace::Remove(*patternJ);
        auto trimB = Compound();
        trimB->set(u8"Material", String(materialB));
        trimB->set(u8"Pattern", String(patternB));
        tagB->set(u8"Trim", trimB);
      }
    }

    if (displayB) {
      tagB->set(u8"display", displayB);
    }

    if (!tagB->empty()) {
      itemB->set(u8"tag", tagB);
    }

    return itemB;
  }

  static std::optional<std::u8string> GetCustomName(CompoundTag const &tag) {
    using namespace std;
    using namespace je2be::props;
    auto display = tag.compoundTag(u8"display");
    if (!display) {
      return nullopt;
    }
    auto name = display->string(u8"Name");
    if (!name) {
      return nullopt;
    }
    auto obj = ParseAsJson(*name);
    if (obj) {
      auto text = obj->find("text");
      if (text != obj->end() && text->is_string()) {
        return props::GetJsonStringValue(*text);
      } else {
        return nullopt;
      }
    } else {
      return strings::RemovePrefixAndSuffix(u8"\"", *name, u8"\"");
    }
  }

  // converts block name (bedrock) to item name (bedrock)
  static std::u8string ItemNameFromBlockName(std::u8string const &name) {
    if (name == u8"minecraft:concretePowder") {
      return u8"minecraft:concrete_powder";
    }
    return name;
  }
};

CompoundTagPtr Item::From(CompoundTagPtr const &item, Context &ctx) {
  return Impl::From(item, ctx);
}

i8 Item::GetSkullTypeFromBlockName(std::u8string_view const &name) {
  return Impl::GetSkullTypeFromBlockName(name);
}

CompoundTagPtr Item::Empty() {
  return Impl::Empty();
}

} // namespace je2be::tobe
