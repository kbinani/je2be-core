#include "java/_item.hpp"

#include <je2be/strings.hpp>

#include "_dimension-ext.hpp"
#include "_namespace.hpp"
#include "_nbt-ext.hpp"
#include "_optional.hpp"
#include "_props.hpp"
#include "entity/_tropical-fish.hpp"
#include "enums/_banner-color-code-bedrock.hpp"
#include "enums/_color-code-java.hpp"
#include "enums/_effect.hpp"
#include "enums/_skull-type.hpp"
#include "item/_banner.hpp"
#include "item/_fireworks-explosion.hpp"
#include "item/_fireworks.hpp"
#include "item/_goat-horn.hpp"
#include "item/_map-type.hpp"
#include "item/_potion.hpp"
#include "item/_tipped-arrow-potion.hpp"
#include "java/_axolotl.hpp"
#include "java/_block-data.hpp"
#include "java/_components.hpp"
#include "java/_context.hpp"
#include "java/_enchant-data.hpp"
#include "java/_entity.hpp"
#include "java/_tile-entity.hpp"
#include "java/_world-data.hpp"

#include <atomic>

namespace je2be::java {

class Item::Impl {
private:
  using Converter = std::function<CompoundTagPtr(std::u8string const &, CompoundTag const &, Context &ctx, int dataVersion)>;
  using Block = mcfile::je::Block;

public:
  static CompoundTagPtr From(CompoundTagPtr const &item, Context &ctx, int sourceDataVersion) {
    auto result = Convert(item, ctx, sourceDataVersion);
    if (result) {
      return Post(result, *item, ctx, sourceDataVersion);
    } else {
      return nullptr;
    }
  }

  static i8 GetSkullTypeFromBlockName(std::u8string_view const &name) {
    i8 type = 0;
    std::u8string n(Namespace::Remove(name));
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
    armor->set(u8"Name", u8"");
    armor->set(u8"WasPickedUp", Bool(false));
    return armor;
  }

private:
  static CompoundTagPtr Convert(CompoundTagPtr const &in, Context &ctx, int dataVersion) {
    using namespace std;

    auto const &blockItemMapping = BlockItemConverterTable();
    auto const &itemMapping = ItemConverterTable();
#if !defined(NDEBUG)
    static atomic<bool> sChecked(false);
    if (!sChecked.exchange(true)) {
      unordered_set<u8string> count;
      for (auto const &it : blockItemMapping) {
        count.insert(it.first);
      }
      for (auto const &it : itemMapping) {
        assert(count.count(it.first) == 0);
      }
    }
#endif

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
      auto n = mcfile::je::Flatten::Item(*idTag, &damage, dataVersion);
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

    if (auto foundItem = itemMapping.find(Namespace::Remove(name)); foundItem == itemMapping.end()) {
      CompoundTagPtr result;
      if (auto foundBlock = blockItemMapping.find(Namespace::Remove(name)); foundBlock != blockItemMapping.end()) {
        result = foundBlock->second(name, *item, ctx, dataVersion);
      } else {
        result = DefaultBlockItem(name, *item, ctx, dataVersion);
      }
      if (!result) {
        return nullptr;
      }
      if (!result->compoundTag(u8"Block")) {
        auto block = Block::FromName(name, dataVersion);
        if (auto blockData = BlockData::From(block, nullptr, {.fItem = true}); blockData) {
          result->set(u8"Block", blockData);
        }
      }
      return result;
    } else {
      return foundItem->second(name, *item, ctx, dataVersion);
    }
  }

  static std::unordered_map<std::u8string, Converter> *CreateBlockItemConverterTable() {
    using namespace std;
    auto table = new unordered_map<u8string, Converter>();
#define E(__name, __func)                  \
  assert(table->count(u8"" #__name) == 0); \
  table->try_emplace(u8"" #__name, __func)

    E(brown_mushroom_block, MushroomBlock(u8"minecraft:brown_mushroom_block", 14));
    E(red_mushroom_block, MushroomBlock(u8"minecraft:red_mushroom_block", 14));
    E(mushroom_stem, MushroomBlock(u8"minecraft:brown_mushroom_block", 15));
    E(torch, AnyTorch);
    E(soul_torch, AnyTorch);
    E(redstone_torch, AnyTorch);
    E(frogspawn, Rename(u8"frog_spawn"));
    E(turtle_egg, DefaultItem);
    E(conduit, DefaultItem);
    E(beacon, DefaultItem);
#undef E
    return table;
  }

  static std::unordered_map<std::u8string, Converter> const &BlockItemConverterTable() {
    using namespace std;
    static unique_ptr<unordered_map<u8string, Converter> const> sTable(CreateBlockItemConverterTable());
    return *sTable;
  }

  static std::unordered_map<std::u8string, Converter> *CreateItemConverterTable() {
    using namespace std;
    auto table = new unordered_map<u8string, Converter>();

#define E(__name, __func)                  \
  assert(table->count(u8"" #__name) == 0); \
  table->try_emplace(u8"" #__name, __func)

    E(furnace_minecart, Rename(u8"minecart")); // furnace minecart does not exist in bedrock
    E(tropical_fish_bucket, TropicalFishBucket);

    E(map, Subtype(u8"empty_map", 2));
    E(firework_rocket, FireworkRocket);
    E(firework_star, FireworkStar);

    // "13", "cat", "blocks", "chirp", "far", "mall", "mellohi", "stal", "strad", "ward", "11", "wait", "otherside", "pigstep"
    E(music_disc_chirp, DefaultItem);
    E(music_disc_stal, DefaultItem);
    E(music_disc_pigstep, DefaultItem);
    E(music_disc_13, DefaultItem);
    E(music_disc_11, DefaultItem);
    E(music_disc_cat, DefaultItem);
    E(music_disc_blocks, DefaultItem);
    E(music_disc_far, DefaultItem);
    E(music_disc_mall, DefaultItem);
    E(music_disc_mellohi, DefaultItem);
    E(music_disc_strad, DefaultItem);
    E(music_disc_ward, DefaultItem);
    E(music_disc_wait, DefaultItem);
    E(music_disc_otherside, DefaultItem);
    E(music_disc_5, DefaultItem);
    E(music_disc_relic, DefaultItem);

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

    // Java: nether_bricks (block), nether_brick (item)
    // Bedrock: nether_brick (block), netherbrick (item)
    E(nether_brick, Rename(u8"netherbrick"));

    E(spawn_egg, LegacySpawnEgg); // legacy
    E(frog_spawn_egg, DefaultItem);
    E(panda_spawn_egg, DefaultItem);
    E(zoglin_spawn_egg, DefaultItem);
    E(horse_spawn_egg, DefaultItem);
    E(zombie_horse_spawn_egg, DefaultItem);
    E(spider_spawn_egg, DefaultItem);
    E(sniffer_spawn_egg, DefaultItem);
    E(polar_bear_spawn_egg, DefaultItem);
    E(camel_spawn_egg, DefaultItem);
    E(allay_spawn_egg, DefaultItem);
    E(phantom_spawn_egg, DefaultItem);
    E(slime_spawn_egg, DefaultItem);
    E(bat_spawn_egg, DefaultItem);
    E(bee_spawn_egg, DefaultItem);
    E(blaze_spawn_egg, DefaultItem);
    E(cat_spawn_egg, DefaultItem);
    E(cave_spider_spawn_egg, DefaultItem);
    E(chicken_spawn_egg, DefaultItem);
    E(cod_spawn_egg, DefaultItem);
    E(cow_spawn_egg, DefaultItem);
    E(pillager_spawn_egg, DefaultItem);
    E(creeper_spawn_egg, DefaultItem);
    E(trader_llama_spawn_egg, DefaultItem);
    E(dolphin_spawn_egg, DefaultItem);
    E(donkey_spawn_egg, DefaultItem);
    E(drowned_spawn_egg, DefaultItem);
    E(elder_guardian_spawn_egg, DefaultItem);
    E(enderman_spawn_egg, DefaultItem);
    E(endermite_spawn_egg, DefaultItem);
    E(evoker_spawn_egg, DefaultItem);
    E(fox_spawn_egg, DefaultItem);
    E(ghast_spawn_egg, DefaultItem);
    E(guardian_spawn_egg, DefaultItem);
    E(hoglin_spawn_egg, DefaultItem);
    E(husk_spawn_egg, DefaultItem);
    E(llama_spawn_egg, DefaultItem);
    E(magma_cube_spawn_egg, DefaultItem);
    E(mooshroom_spawn_egg, DefaultItem);
    E(mule_spawn_egg, DefaultItem);
    E(wandering_trader_spawn_egg, DefaultItem);
    E(ocelot_spawn_egg, DefaultItem);
    E(parrot_spawn_egg, DefaultItem);
    E(pig_spawn_egg, DefaultItem);
    E(piglin_spawn_egg, DefaultItem);
    E(piglin_brute_spawn_egg, DefaultItem);
    E(pufferfish_spawn_egg, DefaultItem);
    E(rabbit_spawn_egg, DefaultItem);
    E(ravager_spawn_egg, DefaultItem);
    E(salmon_spawn_egg, DefaultItem);
    E(vex_spawn_egg, DefaultItem);
    E(sheep_spawn_egg, DefaultItem);
    E(shulker_spawn_egg, DefaultItem);
    E(silverfish_spawn_egg, DefaultItem);
    E(skeleton_spawn_egg, DefaultItem);
    E(skeleton_horse_spawn_egg, DefaultItem);
    E(squid_spawn_egg, DefaultItem);
    E(stray_spawn_egg, DefaultItem);
    E(strider_spawn_egg, DefaultItem);
    E(tropical_fish_spawn_egg, DefaultItem);
    E(turtle_spawn_egg, DefaultItem);
    E(villager_spawn_egg, DefaultItem);
    E(vindicator_spawn_egg, DefaultItem);
    E(witch_spawn_egg, DefaultItem);
    E(wither_skeleton_spawn_egg, DefaultItem);
    E(wolf_spawn_egg, DefaultItem);
    E(zombie_spawn_egg, DefaultItem);
    E(zombie_villager_spawn_egg, DefaultItem);
    E(iron_golem_spawn_egg, DefaultItem);
    E(warden_spawn_egg, DefaultItem);
    E(tadpole_spawn_egg, DefaultItem);
    E(zombified_piglin_spawn_egg, Rename(u8"zombie_pigman_spawn_egg"));
    E(ender_dragon_spawn_egg, DefaultItem);
    E(glow_squid_spawn_egg, DefaultItem);
    E(axolotl_spawn_egg, DefaultItem);
    E(goat_spawn_egg, DefaultItem);
    E(snow_golem_spawn_egg, DefaultItem);

    E(flower_banner_pattern, DefaultItem);
    E(mojang_banner_pattern, DefaultItem);
    E(piglin_banner_pattern, DefaultItem);
    E(creeper_banner_pattern, DefaultItem);
    E(skull_banner_pattern, DefaultItem);
    E(globe_banner_pattern, DefaultItem);

    E(minecart, DefaultItem);
    E(chest_minecart, DefaultItem);
    E(salmon_bucket, FishBucket(u8"salmon"));
    E(golden_carrot, DefaultItem);
    E(saddle, DefaultItem);
    E(bowl, DefaultItem);
    E(magenta_dye, DefaultItem);
    E(ink_sac, DefaultItem);
    E(stone_hoe, DefaultItem);
    E(carrot_on_a_stick, DefaultItem);
    E(cooked_cod, DefaultItem);
    E(netherite_ingot, DefaultItem);
    E(leather_horse_armor, DefaultItem);
    E(warped_fungus_on_a_stick, DefaultItem);
    E(tnt_minecart, DefaultItem);
    E(light_blue_dye, DefaultItem);
    E(dragon_breath, DefaultItem);
    E(hopper_minecart, DefaultItem);
    E(gunpowder, DefaultItem);
    E(elytra, DefaultItem);
    E(arrow, DefaultItem);
    E(egg, DefaultItem);
    E(leather, DefaultItem);
    E(iron_nugget, DefaultItem);
    E(gray_dye, DefaultItem);
    E(echo_shard, DefaultItem);
    E(diamond_pickaxe, DefaultItem);
    E(iron_ingot, DefaultItem);
    E(cod, DefaultItem);
    E(paper, DefaultItem);
    E(iron_sword, DefaultItem);
    E(diamond_horse_armor, DefaultItem);
    E(wheat_seeds, DefaultItem);
    E(scute, DefaultItem);
    E(brick, DefaultItem);
    E(coal, DefaultItem);
    E(prismarine_shard, DefaultItem);
    E(charcoal, DefaultItem);
    E(glowstone_dust, DefaultItem);
    E(stick, DefaultItem);
    E(diamond, DefaultItem);
    E(spectral_arrow, DefaultItem);
    E(lead, DefaultItem);
    E(gold_ingot, DefaultItem);
    E(wooden_pickaxe, DefaultItem);
    E(netherite_scrap, DefaultItem);
    E(book, DefaultItem);
    E(string, DefaultItem);
    E(iron_axe, DefaultItem);
    E(pink_dye, DefaultItem);
    E(feather, DefaultItem);
    E(dried_kelp, DefaultItem);
    E(wheat, DefaultItem);
    E(flint, DefaultItem);
    E(cyan_dye, DefaultItem);
    E(golden_axe, DefaultItem);
    E(bucket, DefaultItem);
    E(water_bucket, DefaultItem);
    E(lava_bucket, DefaultItem);
    E(snowball, DefaultItem);
    E(milk_bucket, DefaultItem);
    E(orange_dye, DefaultItem);
    E(pufferfish_bucket, FishBucket(u8"pufferfish"));
    E(cod_bucket, FishBucket(u8"cod"));
    E(clay_ball, DefaultItem);
    E(slime_ball, DefaultItem);
    E(cocoa_beans, DefaultItem);
    E(lapis_lazuli, DefaultItem);
    E(cauldron, DefaultItem);
    E(white_dye, DefaultItem);
    E(yellow_dye, DefaultItem);
    E(powder_snow_bucket, DefaultItem);
    E(golden_leggings, DefaultItem);
    E(lime_dye, DefaultItem);
    E(rabbit_hide, DefaultItem);
    E(light_gray_dye, DefaultItem);
    E(purple_dye, DefaultItem);
    E(blue_dye, DefaultItem);
    E(brown_dye, DefaultItem);
    E(green_dye, DefaultItem);
    E(red_dye, DefaultItem);
    E(black_dye, DefaultItem);
    E(bow, DefaultItem);
    E(golden_hoe, DefaultItem);
    E(bone_meal, DefaultItem);
    E(bone, DefaultItem);
    E(sugar, DefaultItem);
    E(flint_and_steel, DefaultItem);
    E(pumpkin_seeds, DefaultItem);
    E(melon_seeds, DefaultItem);
    E(ender_pearl, DefaultItem);
    E(blaze_rod, DefaultItem);
    E(glistering_melon_slice, DefaultItem);
    E(gold_nugget, DefaultItem);
    E(nether_wart, DefaultItem);
    E(ender_eye, DefaultItem);
    E(netherite_axe, DefaultItem);
    E(chainmail_helmet, DefaultItem);
    E(carrot, DefaultItem);
    E(campfire, Campfire);
    E(stone_shovel, DefaultItem);
    E(sweet_berries, DefaultItem);
    E(cooked_porkchop, DefaultItem);
    E(beetroot, DefaultItem);
    E(cake, DefaultItem);
    E(golden_pickaxe, DefaultItem);
    E(iron_leggings, DefaultItem);
    E(netherite_leggings, DefaultItem);
    E(glow_berries, DefaultItem);
    E(experience_bottle, DefaultItem);
    E(fire_charge, DefaultItem);
    E(emerald, DefaultItem);
    E(nether_star, DefaultItem);
    E(golden_boots, DefaultItem);
    E(quartz, DefaultItem);
    E(stone_axe, DefaultItem);
    E(prismarine_crystals, DefaultItem);
    E(iron_horse_armor, DefaultItem);
    E(golden_horse_armor, DefaultItem);
    E(shulker_shell, DefaultItem);
    E(chorus_fruit, DefaultItem);
    E(popped_chorus_fruit, DefaultItem);
    E(beetroot_seeds, DefaultItem);
    E(pufferfish, DefaultItem);
    E(porkchop, DefaultItem);
    E(reeds, DefaultItem);
    E(diamond_helmet, DefaultItem);
    E(disc_fragment_5, DefaultItem);
    E(nautilus_shell, DefaultItem);
    E(heart_of_the_sea, DefaultItem);
    E(clock, DefaultItem);
    E(wooden_axe, DefaultItem);
    E(amethyst_shard, DefaultItem);
    E(honeycomb, DefaultItem);
    E(apple, DefaultItem);
    E(mushroom_stew, DefaultItem);
    E(bread, DefaultItem);
    E(golden_apple, DefaultItem);
    E(enchanted_golden_apple, DefaultItem);
    E(salmon, DefaultItem);
    E(tropical_fish, DefaultItem);
    E(cooked_salmon, DefaultItem);
    E(shears, DefaultItem);
    E(cookie, DefaultItem);
    E(melon_slice, DefaultItem);
    E(beef, DefaultItem);
    E(cooked_beef, DefaultItem);
    E(chicken, DefaultItem);
    E(cooked_chicken, DefaultItem);
    E(rotten_flesh, DefaultItem);
    E(spider_eye, DefaultItem);
    E(potato, DefaultItem);
    E(baked_potato, DefaultItem);
    E(wooden_hoe, DefaultItem);
    E(poisonous_potato, DefaultItem);
    E(pumpkin_pie, DefaultItem);
    E(rabbit, DefaultItem);
    E(cooked_rabbit, DefaultItem);
    E(rabbit_stew, DefaultItem);
    E(mutton, DefaultItem);
    E(cooked_mutton, DefaultItem);
    E(enchanted_book, DefaultItem);
    E(beetroot_soup, DefaultItem);
    E(honey_bottle, DefaultItem);
    E(wooden_shovel, DefaultItem);
    E(stone_pickaxe, DefaultItem);
    E(golden_shovel, DefaultItem);
    E(iron_shovel, DefaultItem);
    E(netherite_helmet, DefaultItem);
    E(iron_pickaxe, DefaultItem);
    E(iron_hoe, DefaultItem);
    E(diamond_shovel, DefaultItem);
    E(diamond_axe, DefaultItem);
    E(diamond_hoe, DefaultItem);
    E(netherite_shovel, DefaultItem);
    E(netherite_pickaxe, DefaultItem);
    E(netherite_hoe, DefaultItem);
    E(compass, Compass);
    E(fishing_rod, DefaultItem);
    E(name_tag, DefaultItem);
    E(wooden_sword, DefaultItem);
    E(turtle_helmet, DefaultItem);
    E(stone_sword, DefaultItem);
    E(golden_chestplate, DefaultItem);
    E(golden_sword, DefaultItem);
    E(diamond_sword, DefaultItem);
    E(sugar_cane, DefaultItem);
    E(netherite_sword, DefaultItem);
    E(iron_helmet, DefaultItem);
    E(chainmail_chestplate, DefaultItem);
    E(chainmail_leggings, DefaultItem);
    E(chainmail_boots, DefaultItem);
    E(iron_chestplate, DefaultItem);
    E(iron_boots, DefaultItem);
    E(diamond_chestplate, DefaultItem);
    E(raw_iron, DefaultItem);
    E(diamond_leggings, DefaultItem);
    E(diamond_boots, DefaultItem);
    E(golden_helmet, DefaultItem);
    E(recovery_compass, DefaultItem);
    E(netherite_chestplate, DefaultItem);
    E(netherite_boots, DefaultItem);
    E(shield, DefaultItem);
    E(totem_of_undying, DefaultItem);
    E(trident, DefaultItem);
    E(ghast_tear, DefaultItem);
    E(flower_pot, DefaultItem);
    E(glass_bottle, DefaultItem);
    E(fermented_spider_eye, DefaultItem);
    E(blaze_powder, DefaultItem);
    E(magma_cream, DefaultItem);
    E(rabbit_foot, DefaultItem);
    E(phantom_membrane, DefaultItem);
    E(tadpole_bucket, FishBucket(u8"tadpole"));
    E(torchflower_seeds, DefaultItem);
    E(brush, DefaultItem);
    E(pitcher_pod, DefaultItem);
    E(glow_ink_sac, DefaultItem);
    E(repeater, DefaultItem);
    E(comparator, DefaultItem);
    E(soul_campfire, Campfire);
    E(kelp, DefaultItem);
    E(armor_stand, DefaultItem);
    E(raw_gold, DefaultItem);
    E(spyglass, DefaultItem);
    E(goat_horn, GoatHorn);
    E(nether_sprouts, DefaultItem);
    E(redstone, DefaultItem);
    E(copper_ingot, DefaultItem);
    E(end_crystal, DefaultItem);
    E(painting, DefaultItem);
    E(item_frame, Rename(u8"frame"));
    E(glow_item_frame, Rename(u8"glow_frame"));
    E(raw_copper, DefaultItem);
    E(chain, DefaultItem);

    E(oak_boat, DefaultItem);
    E(birch_boat, DefaultItem);
    E(spruce_boat, DefaultItem);
    E(acacia_boat, DefaultItem);
    E(jungle_boat, DefaultItem);
    E(dark_oak_boat, DefaultItem);
    E(mangrove_boat, DefaultItem);
    E(cherry_boat, DefaultItem);
    E(bamboo_raft, DefaultItem);

    E(oak_chest_boat, DefaultItem);
    E(birch_chest_boat, DefaultItem);
    E(spruce_chest_boat, DefaultItem);
    E(acacia_chest_boat, DefaultItem);
    E(jungle_chest_boat, DefaultItem);
    E(dark_oak_chest_boat, DefaultItem);
    E(mangrove_chest_boat, DefaultItem);
    E(cherry_chest_boat, DefaultItem);
    E(bamboo_chest_raft, DefaultItem);

    E(spire_armor_trim_smithing_template, DefaultItem);
    E(wild_armor_trim_smithing_template, DefaultItem);
    E(ward_armor_trim_smithing_template, DefaultItem);
    E(rib_armor_trim_smithing_template, DefaultItem);
    E(snout_armor_trim_smithing_template, DefaultItem);
    E(netherite_upgrade_smithing_template, DefaultItem);
    E(sentry_armor_trim_smithing_template, DefaultItem);
    E(dune_armor_trim_smithing_template, DefaultItem);
    E(coast_armor_trim_smithing_template, DefaultItem);
    E(eye_armor_trim_smithing_template, DefaultItem);
    E(vex_armor_trim_smithing_template, DefaultItem);
    E(tide_armor_trim_smithing_template, DefaultItem);
    E(host_armor_trim_smithing_template, DefaultItem);
    E(raiser_armor_trim_smithing_template, DefaultItem);
    E(wayfinder_armor_trim_smithing_template, DefaultItem);
    E(shaper_armor_trim_smithing_template, DefaultItem);
    E(silence_armor_trim_smithing_template, DefaultItem);

    E(pottery_shard_archer, Rename(u8"archer_pottery_sherd"));
    E(pottery_shard_prize, Rename(u8"prize_pottery_sherd"));
    E(pottery_shard_arms_up, Rename(u8"arms_up_pottery_sherd"));
    E(pottery_shard_skull, Rename(u8"skull_pottery_sherd"));

    E(archer_pottery_shard, Rename(u8"archer_pottery_sherd"));
    E(prize_pottery_shard, Rename(u8"prize_pottery_sherd"));
    E(arms_up_pottery_shard, Rename(u8"arms_up_pottery_sherd"));
    E(skull_pottery_shard, Rename(u8"skull_pottery_sherd"));

    E(archer_pottery_sherd, DefaultItem);
    E(skull_pottery_sherd, DefaultItem);
    E(friend_pottery_sherd, DefaultItem);
    E(burn_pottery_sherd, DefaultItem);
    E(snort_pottery_sherd, DefaultItem);
    E(angler_pottery_sherd, DefaultItem);
    E(sheaf_pottery_sherd, DefaultItem);
    E(plenty_pottery_sherd, DefaultItem);
    E(arms_up_pottery_sherd, DefaultItem);
    E(shelter_pottery_sherd, DefaultItem);
    E(danger_pottery_sherd, DefaultItem);
    E(brewer_pottery_sherd, DefaultItem);
    E(blade_pottery_sherd, DefaultItem);
    E(explorer_pottery_sherd, DefaultItem);
    E(heart_pottery_sherd, DefaultItem);
    E(heartbreak_pottery_sherd, DefaultItem);
    E(howl_pottery_sherd, DefaultItem);
    E(miner_pottery_sherd, DefaultItem);
    E(mourner_pottery_sherd, DefaultItem);
    E(prize_pottery_sherd, DefaultItem);

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
    E(bamboo_door, DefaultItem);
    E(cherry_door, DefaultItem);
    E(copper_door, DefaultItem);
    E(exposed_copper_door, DefaultItem);
    E(weathered_copper_door, DefaultItem);
    E(oxidized_copper_door, DefaultItem);
    E(waxed_copper_door, DefaultItem);
    E(waxed_exposed_copper_door, DefaultItem);
    E(waxed_weathered_copper_door, DefaultItem);
    E(waxed_oxidized_copper_door, DefaultItem);

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

    E(wolf_armor, WolfArmor);
    E(ominous_bottle, OminousBottle);
    E(turtle_scute, DefaultItem);
    E(armadillo_scute, DefaultItem);
    E(trial_key, DefaultItem);
    E(flow_banner_pattern, DefaultItem);
    E(guster_banner_pattern, DefaultItem);
    E(armadillo_spawn_egg, DefaultItem);
    E(breeze_spawn_egg, DefaultItem);
    E(bogged_spawn_egg, DefaultItem);
    E(mace, DefaultItem);
    E(bolt_armor_trim_smithing_template, DefaultItem);
    E(flow_armor_trim_smithing_template, DefaultItem);
    E(breeze_rod, DefaultItem);
    E(guster_pottery_sherd, DefaultItem);
    E(flow_pottery_sherd, DefaultItem);
    E(scrape_pottery_sherd, DefaultItem);

    E(shulker_box, ShulkerBox);
    E(black_shulker_box, ShulkerBox);
    E(red_shulker_box, ShulkerBox);
    E(green_shulker_box, ShulkerBox);
    E(brown_shulker_box, ShulkerBox);
    E(blue_shulker_box, ShulkerBox);
    E(purple_shulker_box, ShulkerBox);
    E(cyan_shulker_box, ShulkerBox);
    E(light_gray_shulker_box, ShulkerBox);
    E(gray_shulker_box, ShulkerBox);
    E(pink_shulker_box, ShulkerBox);
    E(lime_shulker_box, ShulkerBox);
    E(yellow_shulker_box, ShulkerBox);
    E(light_blue_shulker_box, ShulkerBox);
    E(magenta_shulker_box, ShulkerBox);
    E(orange_shulker_box, ShulkerBox);
    E(white_shulker_box, ShulkerBox);
    E(furnace, BlockContainer);
    E(smoker, BlockContainer);
    E(blast_furnace, BlockContainer);
    E(brewing_stand, BrewingStand);
    E(decorated_pot, DecoratedPot);
    E(chiseled_bookshelf, ChiseledBookshelf);
    E(chest, UnfindableBlockContainer);
    E(barrel, UnfindableBlockContainer);
    E(dispenser, BlockContainer);
    E(dropper, BlockContainer);
    E(hopper, SimpleContainer);
    E(trapped_chest, UnfindableBlockContainer);
    E(crafter, BlockContainer);

    E(music_disc_creator, DefaultItem);
    E(music_disc_creator_music_box, DefaultItem);
    E(music_disc_precipice, DefaultItem);
    E(ominous_trial_key, DefaultItem);
    E(wind_charge, DefaultItem);
#undef E
    return table;
  }

  static std::unordered_map<std::u8string, Converter> const &ItemConverterTable() {
    using namespace std;
    static unique_ptr<unordered_map<u8string, Converter> const> sTable(CreateItemConverterTable());
    return *sTable;
  }

  static CompoundTagPtr ChiseledBookshelf(std::u8string const &name, CompoundTag const &item, Context &ctx, int dataVersion) {
    auto ret = New(name, true);
    if (auto itemsJ = GetItems(item, ctx, dataVersion); itemsJ) {
      auto itemsB = List<Tag::Type::Compound>();
      for (int i = 0; i < 6; i++) {
        itemsB->push_back(Empty());
      }
      for (auto const &it : *itemsJ) {
        if (auto c = std::dynamic_pointer_cast<CompoundTag>(it); c) {
          if (auto slot = c->byte(u8"Slot"); slot) {
            if (0 <= *slot && *slot <= 5) {
              c->erase(u8"Slot");
              itemsB->fValue[*slot] = c;
            }
          }
        }
      }
      AppendTag(*ret, u8"Items", itemsB);
    }
    if (auto block = Block::FromName(name, dataVersion); block) {
      if (auto blockData = BlockData::From(block, nullptr, {.fItem = true}); blockData) {
        ret->set(u8"Block", blockData);
      }
    }
    return ret;
  }

  static CompoundTagPtr UnfindableBlockContainer(std::u8string const &name, CompoundTag const &item, Context &ctx, int dataVersion) {
    auto ret = New(name, true);
    ContainerItems(item, *ret, ctx, dataVersion);
    if (auto block = Block::FromName(name, dataVersion); block) {
      if (auto blockData = BlockData::From(block, nullptr, {.fItem = true}); blockData) {
        ret->set(u8"Block", blockData);
      }
    }
    if (ret->query(u8"tag/Items")->asList()) {
      AppendTag(*ret, u8"Findable", Bool(false));
    }
    return ret;
  }

  static CompoundTagPtr DecoratedPot(std::u8string const &name, CompoundTag const &item, Context &ctx, int dataVersion) {
    auto ret = New(name, true);
    if (auto items = GetItems(item, ctx, dataVersion); items) {
      for (auto const &it : *items) {
        auto c = std::dynamic_pointer_cast<CompoundTag>(it);
        if (!c) {
          continue;
        }
        auto slot = c->byte(u8"Slot");
        if (!slot) {
          continue;
        }
        c->erase(u8"Slot");
        if (*slot == 0) {
          AppendTag(*ret, u8"item", c);
          AppendTag(*ret, u8"animation", Bool(false));
        }
      }
    }
    if (auto block = Block::FromName(name, dataVersion); block) {
      if (auto blockData = BlockData::From(block, nullptr, {.fItem = true}); blockData) {
        ret->set(u8"Block", blockData);
      }
    }
    return ret;
  }

  static CompoundTagPtr BrewingStand(std::u8string const &name, CompoundTag const &item, Context &ctx, int dataVersion) {
    u8 const mapping[5] = {1, 2, 3, 0, 4};
    auto ret = New(name, true);
    auto tagB = Compound();
    if (auto items = GetItems(item, ctx, dataVersion); items) {
      auto itemsB = List<Tag::Type::Compound>();
      for (auto const &it : *items) {
        auto c = std::dynamic_pointer_cast<CompoundTag>(it);
        if (!c) {
          continue;
        }
        auto slot = c->byte(u8"Slot");
        if (!slot) {
          continue;
        }
        if (0 <= *slot && *slot <= 4) {
          auto i = mapping[*slot];
          c->set(u8"Slot", Byte(i));
          itemsB->push_back(c);
        }
      }
      tagB->set(u8"Items", itemsB);
    }
    if (auto blockEntityDataJ = Migrate<CompoundTag>(item, u8"block_entity_data", Depth::Tag, u8"BlockEntityTag"); blockEntityDataJ) {
      CopyShortValues(*blockEntityDataJ, *tagB, {{u8"BrewTime", u8"CookTime"}});
      if (auto fuelJ = blockEntityDataJ->byte(u8"Fuel"); fuelJ) {
        tagB->set(u8"FuelAmount", Short(*fuelJ));
        tagB->set(u8"FuelTotal", Short(20));
      }
    }
    if (!tagB->empty()) {
      ret->set(u8"tag", tagB);
    }
    return ret;
  }

  static CompoundTagPtr SimpleContainer(std::u8string const &name, CompoundTag const &item, Context &ctx, int dataVersion) {
    auto ret = New(name, true);
    ContainerItems(item, *ret, ctx, dataVersion);
    return ret;
  }

  static CompoundTagPtr BlockContainer(std::u8string const &name, CompoundTag const &item, Context &ctx, int dataVersion) {
    auto ret = New(name, true);
    ContainerItems(item, *ret, ctx, dataVersion);
    if (auto block = Block::FromName(name, dataVersion); block) {
      if (auto blockData = BlockData::From(block, nullptr, {.fItem = true}); blockData) {
        ret->set(u8"Block", blockData);
      }
    }
    return ret;
  }

  static CompoundTagPtr Campfire(std::u8string const &name, CompoundTag const &item, Context &ctx, int dataVersion) {
    auto ret = New(name, true);
    auto tagB = Compound();
    if (auto items = GetItems(item, ctx, dataVersion); items) {
      for (auto const &it : *items) {
        auto c = std::dynamic_pointer_cast<CompoundTag>(it);
        if (!c) {
          continue;
        }
        auto slot = c->byte(u8"Slot");
        if (!slot) {
          continue;
        }
        if (0 <= *slot && *slot <= 3) {
          c->erase(u8"Slot");
          tagB->set(u8"Item" + mcfile::String::ToString(*slot + 1), c);
        }
      }
    }
    if (auto cookingTimes = FallbackQuery(item, {u8"components/minecraft:block_entity_data/CookingTimes", u8"tag/BlockEntityTag/CookingTimes"})->asIntArray(); cookingTimes) {
      for (int i = 0; i < 4 && i < cookingTimes->fValue.size(); i++) {
        tagB->set(u8"ItemTime" + mcfile::String::ToString(i + 1), Int(cookingTimes->fValue[i]));
      }
    }
    if (!tagB->empty()) {
      ret->set(u8"tag", tagB);
    }
    return ret;
  }

  static CompoundTagPtr ShulkerBox(std::u8string const &name, CompoundTag const &item, Context &ctx, int dataVersion) {
    auto n = Namespace::Remove(name);
    if (n == u8"shulker_box") {
      n = u8"undyed_shulker_box";
    }
    auto ret = New(n);

    if (auto block = Block::FromName(name, dataVersion); block) {
      if (auto blockData = BlockData::From(block, nullptr, {.fItem = true}); blockData) {
        ret->set(u8"Block", blockData);
      }
    }

    if (auto container = item.query(u8"components/minecraft:container")->asList(); container) {
      auto itemsB = List<Tag::Type::Compound>();
      for (auto const &it : *container) {
        if (auto c = std::dynamic_pointer_cast<CompoundTag>(it); c) {
          if (auto itemJ = c->compoundTag(u8"item"); itemJ) {
            if (auto slotJ = c->int32(u8"slot"); slotJ) {
              if (auto itemB = Item::From(itemJ, ctx, dataVersion); itemB) {
                itemB->set(u8"Slot", Byte(*slotJ));
                itemsB->push_back(itemB);
              }
            }
          }
        }
      }
      auto tagB = Compound();
      tagB->set(u8"Items", itemsB);
      ret->set(u8"tag", tagB);
    } else if (auto itemsJ = item.query(u8"tag/BlockEntityTag/Items")->asList(); itemsJ) {
      auto itemsB = List<Tag::Type::Compound>();
      for (auto const &it : *itemsJ) {
        if (auto c = std::dynamic_pointer_cast<CompoundTag>(it); c) {
          if (auto slotJ = c->byte(u8"Slot"); slotJ) {
            if (auto itemB = Item::From(c, ctx, dataVersion); itemB) {
              itemB->set(u8"Slot", Byte(*slotJ));
              itemsB->push_back(itemB);
            }
          }
        }
      }
      auto tagB = Compound();
      tagB->set(u8"Items", itemsB);
      ret->set(u8"tag", tagB);
    }
    return ret;
  }

  static CompoundTagPtr Compass(std::u8string const &name, CompoundTag const &item, Context &ctx, int dataVersion) {
    struct Info {
      Info(Pos3i pos, mcfile::Dimension d) : fPos(pos), fDimension(d) {}
      Pos3i fPos;
      mcfile::Dimension fDimension;
    };
    std::optional<Info> info;
    if (auto target = item.query(u8"components/minecraft:lodestone_tracker/target")->asCompound(); target) {
      auto pos = props::GetPos3iFromIntArrayTag(*target, u8"pos");
      if (auto dimension = target->string(u8"dimension"); dimension) {
        if (auto d = DimensionFromJavaString(*dimension); d && pos) {
          info = Info(*pos, *d);
        }
      }
    } else if (auto tag = item.compoundTag(u8"tag"); tag) {
      auto pos = props::GetPos3i(*tag, u8"LodestonePos");
      if (auto dimension = tag->string(u8"LodestoneDimension"); dimension) {
        if (auto d = DimensionFromJavaString(*dimension); d && pos) {
          info = Info(*pos, *d);
        }
      }
    }
    if (info) {
      auto ret = New(u8"lodestone_compass");
      i32 tracker = ctx.fLodestones->get(info->fDimension, info->fPos);
      auto tagB = Compound();
      tagB->set(u8"trackingHandle", Int(tracker));
      ret->set(u8"tag", tagB);
      return ret;
    } else {
      return New(u8"compass");
    }
  }

  static CompoundTagPtr Sign(std::u8string const &name, CompoundTag const &item, Context &ctx, int dataVersion) {
    auto ret = DefaultBlockItem(name, item, ctx, dataVersion);
    if (!ret) {
      return nullptr;
    }
    ret->set(u8"Name", name);
    ret->erase(u8"Block");
    return ret;
  }

  static CompoundTagPtr AxolotlBucket(std::u8string const &name, CompoundTag const &item, Context const &, int dataVersion) {
    auto ret = New(u8"axolotl_bucket");
    ret->set(u8"Damage", Short(0));
    auto tg = FallbackQuery(item, {u8"components/minecraft:bucket_entity_data", u8"tag"})->asCompound();
    if (tg) {
      auto age = tg->int32(u8"Age", 0);
      auto health = tg->float32(u8"Health", 14);
      auto variant = tg->int32(u8"Variant", 0);
      auto axltl = Axolotl(age, health, variant);
      auto tag = axltl.toBedrockBucketTag();

      auto customName = GetCustomName(item);
      if (customName) {
        tag->set(u8"CustomName", *customName);
        tag->set(u8"CustomNameVisible", Bool(true));
      }

      ret->set(u8"tag", tag);
    }
    return ret;
  }

  static CompoundTagPtr TropicalFishBucket(std::u8string const &name, CompoundTag const &item, Context const &ctx, int dataVersion) {
    auto ret = New(u8"tropical_fish_bucket");
    ret->set(u8"Damage", Short(0));
    if (auto data = FallbackQuery(item, {u8"components/minecraft:bucket_entity_data", u8"tag"})->asCompound(); data) {
      if (auto variant = data->int32(u8"BucketVariantTag"); variant) {
        auto tf = TropicalFish::FromJavaVariant(*variant);
        auto tag = tf.toBedrockBucketTag(ctx.fUuids->randomEntityId(), data->float32(u8"Health"));
        ret->set(u8"tag", tag);
      }
    }
    return ret;
  }

  static Converter FishBucket(std::u8string const &type) {
    return [=](std::u8string const &name, CompoundTag const &item, Context const &ctx, int dataVersion) {
      auto ret = New(type + u8"_bucket");
      ret->set(u8"Damage", Short(0));
      if (auto data = FallbackQuery(item, {u8"components/minecraft:bucket_entity_data", u8"tag"})->asCompound(); data) {
        Entity::Rep rep(ctx.fUuids->randomEntityId());
        rep.fDefinitions.push_back(u8"+minecraft:" + type);
        rep.fDefinitions.push_back(u8"+");
        if (type == u8"salmon") {
          rep.fDefinitions.push_back(u8"+scale_normal");
        }
        auto tagB = rep.toCompoundTag();
        auto health = data->float32(u8"Health");
        auto attributes = EntityAttributes::Mob(Namespace::Add(type), health);
        if (attributes) {
          tagB->set(u8"Attributes", attributes->toBedrockListTag());
        }
        if (auto age = data->int32(u8"Age"); age) {
          tagB->set(u8"Age", Int(*age));
        }
        ret->set(u8"tag", tagB);
      }
      return ret;
    };
  }

  static CompoundTagPtr BooksAndQuill(std::u8string const &name, CompoundTag const &item, Context const &, int dataVersion) {
    using namespace std;

    auto tag = Compound();
    tag->insert({
        {u8"Name", String(name)},
        {u8"WasPickedUp", Bool(false)},
        {u8"Damage", Short(0)},
    });

    auto bookContent = FallbackQuery(item, {u8"components/minecraft:writable_book_content", u8"components/minecraft:written_book_content", u8"tag"})->asCompound();
    if (bookContent) {
      auto outTag = Compound();
      auto author = bookContent->stringTag(u8"author");
      if (author) {
        outTag->set(u8"author", author->fValue);
      }
      optional<u8string> title;
      if (auto titleCompound = bookContent->compoundTag(u8"title"); titleCompound) {
        if (auto raw = titleCompound->string(u8"raw"); raw) {
          title = *raw;
        }
      } else if (auto titleString = bookContent->string(u8"title"); titleString) {
        title = *titleString;
      }
      if (name == u8"minecraft:written_book") {
        outTag->set(u8"title", title ? *title : u8"");
        outTag->set(u8"generation", Int(0));
      }
      auto pages = bookContent->listTag(u8"pages");
      if (pages) {
        auto outPages = List<Tag::Type::Compound>();
        for (auto const &it : *pages) {
          std::u8string pageContent;
          if (auto pageCompound = it->asCompound(); pageCompound) {
            if (auto raw = pageCompound->string(u8"raw"); raw) {
              pageContent = *raw;
            }
          } else if (auto pageString = it->asString(); pageString) {
            pageContent = pageString->fValue;
          }
          u8string lineText;
          auto obj = props::ParseAsJson(pageContent);
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
            lineText = pageContent;
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

  static CompoundTagPtr LeatherArmor(std::u8string const &name, CompoundTag const &item, Context const &, int dataVersion) {
    auto tag = Compound();
    tag->insert({
        {u8"Name", String(name)},
        {u8"WasPickedUp", Bool(false)},
        {u8"Damage", Short(0)},
    });

    auto rgbJ = FallbackQuery(item, {u8"components/minecraft:dyed_color/rgb", u8"tag/display/color"})->asInt();
    if (rgbJ) {
      auto t = Compound();
      t->set(u8"customColor", Int(CustomColor(rgbJ->fValue)));
      tag->set(u8"tag", t);
    }
    return tag;
  }

  static i32 CustomColor(i32 rgb) {
    u32 v = 0xff000000 | *(u32 *)&rgb;
    return *(i32 *)&v;
  }

  static std::optional<std::tuple<int, CompoundTagPtr>> Map(std::u8string const &name, CompoundTag const &item, JavaEditionMap const &mapInfo) {
    auto ret = New(name, true);
    ret->set(u8"Damage", Short(0));

    auto mapId = Migrate<IntTag>(item, u8"map_id", Depth::Tag, u8"map");
    if (!mapId) {
      return std::nullopt;
    }

    auto scale = mapInfo.getScale(mapId->fValue);
    if (!scale) {
      return std::nullopt;
    }
    i64 uuid = Map::UUID(mapId->fValue, *scale);

    auto tag = Compound();
    tag->set(u8"map_uuid", Long(uuid));
    tag->set(u8"map_display_players", Byte(1));
    ret->set(u8"tag", tag);

    i16 type = 0;
    auto itemName = Migrate<StringTag>(item, u8"item_name", Depth::Display, u8"Name");
    if (itemName) {
      auto nameJson = props::ParseAsJson(itemName->fValue);
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
    ret->set(u8"Damage", Short(type));

    return make_tuple(mapId->fValue, ret);
  }

  static CompoundTagPtr AnyPotion(std::u8string const &name, CompoundTag const &item, Context const &, int dataVersion) {
    std::u8string itemName = name;
    std::optional<i16> type;
    if (auto potionContents = FallbackQuery(item, {u8"components/minecraft:potion_contents", u8"tag"})->asCompound(); potionContents) {
      if (auto potion = FallbackValue<std::u8string>(*potionContents, {u8"potion", u8"Potion"}); potion) {
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

  static CompoundTagPtr TippedArrow(std::u8string const &name, CompoundTag const &item, Context const &, int dataVersion) {
    auto tag = New(u8"arrow");
    std::optional<std::u8string> potion;
    if (auto components = item.compoundTag(u8"components"); components) {
      if (auto potionContents = components->compoundTag(u8"minecraft:potion_contents"); potionContents) {
        potion = potionContents->string(u8"potion");
      }
    } else if (auto legacyTag = item.compoundTag(u8"tag"); legacyTag) {
      potion = legacyTag->string(u8"Potion");
    }
    i16 type = 0;
    if (potion) {
      type = TippedArrowPotion::BedrockPotionType(*potion);
    }
    tag->set(u8"Damage", Short(type));
    return tag;
  }

  static CompoundTagPtr FireworkStar(std::u8string const &name, CompoundTag const &item, Context &ctx, int dataVersion) {
    auto data = Rename(u8"firework_star")(name, item, ctx, dataVersion);

    auto explosion = Migrate<CompoundTag>(item, u8"firework_explosion", Depth::Tag, u8"Explosion");
    if (explosion) {
      auto tag = Compound();

      auto e = FireworksExplosion::FromJava(*explosion);
      if (!e.fColors.empty()) {
        i8 damage = FireworksExplosion::GetBedrockColorCode(e.fColors[0]);
        data->set(u8"Damage", Short(damage));

        if (auto customColor = FireworksExplosion::BedrockCustomColorFromColorCode(damage); customColor) {
          tag->set(u8"customColor", Int(*customColor));
        }
      }

      tag->set(u8"FireworksItem", e.toBedrockCompoundTag());
      data->set(u8"tag", tag);
    }

    return data;
  }

  static CompoundTagPtr FireworkRocket(std::u8string const &name, CompoundTag const &item, Context const &, int dataVersion) {
    auto data = New(u8"firework_rocket");
    auto fireworks = Migrate<CompoundTag>(item, u8"fireworks", Depth::Tag, u8"Fireworks");
    if (fireworks) {
      auto fireworksData = FireworksData::FromJava(*fireworks);
      auto tag = Compound();
      tag->set(u8"Fireworks", fireworksData.toBedrockCompoundTag());
      data->set(u8"tag", tag);
    }
    return data;
  }

  static Converter Subtype(std::u8string const &newName, i16 damage) {
    return [=](std::u8string const &name, CompoundTag const &item, Context const &, int dataVersion) {
      auto tag = New(newName);
      tag->set(u8"Damage", Short(damage));
      return tag;
    };
  }

  static CompoundTagPtr AnyTorch(std::u8string const &name, CompoundTag const &item, Context const &, int dataVersion) {
    using namespace std;

    auto block = Block::FromName(name, dataVersion);
    auto blockData = BlockData::From(block, nullptr, {.fItem = true});

    auto states = Compound();
    states->set(u8"torch_facing_direction", u8"unknown");
    blockData->set(u8"states", states);

    auto tag = New(name, true);
    tag->set(u8"Damage", Short(0));
    tag->set(u8"Block", blockData);
    return tag;
  }

  static Converter Rename(std::u8string const &newName) {
    return [=](std::u8string const &name, CompoundTag const &item, Context const &, int dataVersion) {
      auto tag = New(newName);
      return tag;
    };
  }

  static CompoundTagPtr Skull(std::u8string const &name, CompoundTag const &item, Context const &, int dataVersion) {
    i8 type = GetSkullTypeFromBlockName(name);
    auto tag = New(u8"skull");
    tag->set(u8"Damage", Short(type));
    return tag;
  }

  static CompoundTagPtr LegacySpawnEgg(std::u8string const &name, CompoundTag const &j, Context &ctx, int dataVersion) {
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

  static CompoundTagPtr SuspiciousStew(std::u8string const &name, CompoundTag const &j, Context const &, int dataVersion) {
    auto b = New(u8"suspicious_stew");
    i16 damage = -1;
    if (auto effectsJ = Migrate<ListTag>(j, u8"suspicious_stew_effects", Depth::Tag, u8"effects"); effectsJ) {
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
    } else if (auto tagJ = j.compoundTag(u8"tag"); tagJ) {
      if (auto legacyEffectsJ = tagJ->listTag(u8"Effects"); legacyEffectsJ) {
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
    }
    if (damage >= 0) {
      b->set(u8"Damage", Short(damage));
    }
    return b;
  }

  static CompoundTagPtr Crossbow(std::u8string const &name, CompoundTag const &j, Context &ctx, int dataVersion) {
    auto b = New(u8"crossbow");

    auto chargedProjectiles = FallbackQuery(j, {u8"components/minecraft:charged_projectiles", u8"tag/ChargedProjectiles"})->asList();
    if (chargedProjectiles) {
      for (auto const &it : *chargedProjectiles) {
        auto projectileJ = std::dynamic_pointer_cast<CompoundTag>(it);
        if (!projectileJ) {
          continue;
        }
        auto projectileB = From(projectileJ, ctx, dataVersion);
        if (projectileB) {
          auto beTag = Compound();
          beTag->set(u8"chargedItem", projectileB);
          b->set(u8"tag", beTag);
          break;
        }
      }
    }
    return b;
  }

  static CompoundTagPtr Banner(std::u8string const &name, CompoundTag const &item, Context const &, int dataVersion) {
    auto colorName = strings::RemovePrefixAndSuffix(u8"minecraft:", name, u8"_banner");
    BannerColorCodeBedrock color = BannerColorCodeFromName(colorName);
    i16 damage = (i16)color;
    auto ret = New(u8"banner");
    ret->set(u8"Damage", Short(damage));

    auto patterns = Migrate<ListTag>(item, u8"banner_patterns", Depth::BlockEntityTag, u8"Patterns");
    auto itemName = Migrate<StringTag>(item, u8"item_name", Depth::Display, u8"Name");
    bool ominous = false;
    if (itemName) {
      auto json = props::ParseAsJson(itemName->fValue);
      if (json) {
        auto translate = json->find("translate");
        if (translate != json->end() && translate->is_string() && props::GetJsonStringValue(*translate) == u8"block.minecraft.ominous_banner") {
          ominous = true;
        }
      }
    }

    auto tag = Compound();
    if (ominous) {
      tag->set(u8"Type", Int(1));
      ret->set(u8"tag", tag);
    } else if (patterns) {
      auto bePatterns = List<Tag::Type::Compound>();
      for (auto const &it : *patterns) {
        auto c = it->asCompound();
        if (!c) {
          continue;
        }
        ColorCodeJava patternColor;
        if (auto patternColorString = c->string(u8"color"); patternColorString) {
          patternColor = ColorCodeJavaFromJavaName(*patternColorString);
        } else if (auto patternColorInt = c->int32(u8"Color"); patternColorInt) {
          patternColor = static_cast<ColorCodeJava>(*patternColorInt);
        } else {
          continue;
        }
        auto patternJ = FallbackPtr<StringTag>(*c, {u8"pattern", u8"Pattern"});
        if (!patternJ) {
          continue;
        }
        auto patternB = Banner::BedrockOrLegacyJavaPatternFromJava(patternJ->fValue);
        auto ptag = Compound();
        ptag->insert({
            {u8"Color", Int(static_cast<i32>(BannerColorCodeFromJava(patternColor)))},
            {u8"Pattern", String(Wrap(patternB, patternJ->fValue))},
        });
        bePatterns->push_back(ptag);
      }
      tag->set(u8"Patterns", bePatterns);
      tag->set(u8"Type", Int(0));
    } else {
      tag->set(u8"Type", Int(0));
    }
    ret->set(u8"tag", tag);

    return ret;
  }

  static CompoundTagPtr Bed(std::u8string const &name, CompoundTag const &item, Context const &, int dataVersion) {
    using namespace std;
    u8string colorName = strings::RemovePrefixAndSuffix(u8"minecraft:", name, u8"_bed");
    ColorCodeJava color = ColorCodeJavaFromJavaName(colorName);
    i16 damage = (i16)color;
    auto tag = New(u8"bed");
    tag->set(u8"Damage", Short(damage));
    return tag;
  }

  static Converter MushroomBlock(std::u8string const &name, int hugeMushroomBits) {
    return [=](std::u8string const &, CompoundTag const &item, Context const &, int dataVersion) -> CompoundTagPtr {
      using namespace std;

      auto block = Block::FromName(name, dataVersion);
      auto blockData = BlockData::From(block, nullptr, {.fItem = true});

      auto states = Compound();
      states->set(u8"huge_mushroom_bits", Int(hugeMushroomBits));
      blockData->set(u8"states", states);

      auto tag = New(name, true);
      tag->set(u8"Damage", Short(0));
      tag->set(u8"Block", blockData);
      return tag;
    };
  }

  static CompoundTagPtr GoatHorn(std::u8string const &name, CompoundTag const &item, Context const &, int dataVersion) {
    auto tagB = New(u8"goat_horn");
    i16 damage = 0;
    if (auto instrumentJ = FallbackQuery(item, {u8"components/minecraft:instrument", u8"tag/instrument"})->asString(); instrumentJ) {
      damage = GoatHorn::BedrockDamageFromJavaInstrument(instrumentJ->fValue);
    }
    tagB->set(u8"Damage", Short(damage));
    return tagB;
  }

  static CompoundTagPtr WolfArmor(std::u8string const &name, CompoundTag const &item, Context const &, int dataVersion) {
    auto itemB = New(u8"wolf_armor");
    if (auto rgbJ = item.query(u8"components/minecraft:dyed_color/rgb")->asInt(); rgbJ) {
      auto tagB = Compound();
      tagB->set(u8"customColor", Int(CustomColor(rgbJ->fValue)));
      itemB->set(u8"tag", tagB);
    }
    return itemB;
  }

  static CompoundTagPtr OminousBottle(std::u8string const &name, CompoundTag const &item, Context const &, int dataVersion) {
    auto itemB = New(u8"ominous_bottle");
    if (auto amplifier = item.query(u8"components/minecraft:ominous_bottle_amplifier")->asInt(); amplifier) {
      itemB->set(u8"Damage", Short(amplifier->fValue));
    }
    return itemB;
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

  static CompoundTagPtr DefaultBlockItem(std::u8string const &id, CompoundTag const &item, Context &ctx, int dataVersion) {
    using namespace std;

    auto block = Block::FromName(id, dataVersion);
    auto blockData = BlockData::From(block, nullptr, {.fItem = true});
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

  static CompoundTagPtr DefaultItem(std::u8string const &name, CompoundTag const &item, Context &ctx, int dataVersion) {
    auto ret = Compound();
    ret->insert({
        {u8"Name", String(name)},
        {u8"WasPickedUp", Bool(false)},
        {u8"Damage", Short(0)},
    });
    return ret;
  }

  static CompoundTagPtr Post(CompoundTagPtr const &itemB, CompoundTag const &itemJ, Context &ctx, int dataVersion) {
    using namespace std;

    auto nameB = itemB->string(u8"Name");

    auto count = Item::Count(itemJ);
    if (count) {
      itemB->set(u8"Count", Byte(*count));
    }

    auto componentsJ = itemJ.compoundTag(u8"components");
    auto legacyTagJ = itemJ.compoundTag(u8"tag");
    if (!componentsJ && !legacyTagJ) {
      return itemB;
    }

    auto tagB = itemB->compoundTag(u8"tag");
    if (!tagB) {
      tagB = Compound();
    }

    if (componentsJ) {
      CompoundTagPtr enchantments;
      if (auto storedEnchantments = componentsJ->compoundTag(u8"minecraft:stored_enchantments"); storedEnchantments) {
        enchantments = storedEnchantments;
      } else if (auto list = componentsJ->compoundTag(u8"minecraft:enchantments"); list) {
        enchantments = list;
      }
      if (enchantments) {
        if (auto levels = enchantments->compoundTag(u8"levels"); levels) {
          auto ench = List<Tag::Type::Compound>();
          for (auto const &it : *levels) {
            if (!it.second) {
              continue;
            }
            auto level = it.second->asInt();
            if (!level) {
              continue;
            }
            auto id = Enchantments::BedrockEnchantmentIdFromJava(it.first);
            auto be = Compound();
            be->set(u8"id", Short(id));
            be->set(u8"lvl", Short(level->fValue));
            ench->push_back(be);
          }
          if (!ench->empty()) {
            tagB->set(u8"ench", ench);
          }
        }
      }
    } else {
      ListTagPtr enchantments;
      if (auto storedEnchantments = itemJ.listTag(u8"StoredEnchantments"); storedEnchantments) {
        enchantments = storedEnchantments;
      } else if (auto list = itemJ.listTag(u8"Enchantments"); list) {
        enchantments = list;
      }
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
        if (!ench->empty()) {
          tagB->set(u8"ench", ench);
        }
      }
    }

    auto damage = Migrate<IntTag>(itemJ, u8"damage", Depth::Tag, u8"Damage");
    if (damage) {
      tagB->set(u8"Damage", Int(damage->fValue));
    }

    auto repairCost = Migrate<IntTag>(itemJ, u8"repair_cost", Depth::Tag, u8"RepairCost");
    if (repairCost) {
      tagB->set(u8"RepairCost", Int(repairCost->fValue));
    }

    CompoundTagPtr displayB = tagB->compoundTag(u8"display");

    auto name = GetCustomName(itemJ);
    if (name) {
      if (!displayB) {
        displayB = Compound();
      }
      displayB->set(u8"Name", *name);
    }

    ListTagPtr loreJ;
    if (componentsJ) {
      loreJ = componentsJ->listTag(u8"minecraft:lore");
    }
    if (!loreJ && legacyTagJ) {
      if (auto displayJ = legacyTagJ->compoundTag(u8"display"); displayJ) {
        loreJ = displayJ->listTag(u8"Lore");
      }
    }
    if (loreJ) {
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

    if (auto blockEntityTagJ = Migrate<CompoundTag>(itemJ, u8"block_entity_data", Depth::Tag, u8"BlockEntityTag"); blockEntityTagJ) {
      if (auto id = itemJ.string(u8"id"); id) {
        if (auto block = Block::FromName(*id, dataVersion); block) {
          Pos3i dummy(0, 0, 0);
          if (auto blockEntityTagB = TileEntity::FromBlockAndTileEntity(dummy, *block, blockEntityTagJ, ctx, dataVersion); blockEntityTagB) {
            static unordered_set<u8string> const sExclude({u8"Findable", u8"id", u8"isMovable", u8"x", u8"y", u8"z"});
            for (auto const &e : sExclude) {
              blockEntityTagB->erase(e);
            }
            if (nameB == u8"minecraft:banner" && tagB->int32(u8"Type") == 1) {
              // ominous_banner: "Type" property was already converted, and Base/Patterns can be omitted.
              blockEntityTagB->erase(u8"Type");
              blockEntityTagB->erase(u8"Base");
              blockEntityTagB->erase(u8"Patterns");
            }
            for (auto const &it : *blockEntityTagB) {
              if (tagB->find(it.first) == tagB->end()) {
                tagB->set(it.first, it.second);
              }
            }
            if (!itemB->compoundTag(u8"Block") && nameB != u8"minecraft:hopper" && nameB != u8"minecraft:campfire" && nameB != u8"minecraft:soul_campfire") {
              if (auto blockData = BlockData::From(block, nullptr, {.fItem = true}); blockData) {
                itemB->set(u8"Block", blockData);
              }
            }
          }
        }
      }
    }

    if (auto trimJ = Migrate<CompoundTag>(itemJ, u8"trim", Depth::Tag, u8"Trim"); trimJ) {
      auto materialJ = trimJ->string(u8"material");
      auto patternJ = trimJ->string(u8"pattern");
      if (materialJ && patternJ) {
        auto materialB = Namespace::Remove(*materialJ);
        auto patternB = Namespace::Remove(*patternJ);
        auto trimB = Compound();
        trimB->set(u8"Material", materialB);
        trimB->set(u8"Pattern", patternB);
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

  static void ContainerItems(CompoundTag const &itemJ, CompoundTag &itemB, Context &ctx, int dataVersion) {
    auto itemsB = GetItems(itemJ, ctx, dataVersion);
    if (!itemsB) {
      return;
    }
    AppendTag(itemB, u8"Items", itemsB);
  }

  static ListTagPtr GetItems(CompoundTag const &itemJ, Context &ctx, int dataVersion) {
    if (auto container = itemJ.query(u8"components/minecraft:container")->asList(); container) {
      auto itemsB = List<Tag::Type::Compound>();
      for (auto const &it : *container) {
        if (auto c = std::dynamic_pointer_cast<CompoundTag>(it); c) {
          if (auto iJ = c->compoundTag(u8"item"); iJ) {
            if (auto slotJ = c->int32(u8"slot"); slotJ) {
              if (auto itemB = Item::From(iJ, ctx, dataVersion); itemB) {
                itemB->set(u8"Slot", Byte(*slotJ));
                itemsB->push_back(itemB);
              }
            }
          }
        }
      }
      return itemsB;
    } else if (auto itemsJ = itemJ.query(u8"tag/BlockEntityTag/Items")->asList(); itemsJ) {
      auto itemsB = List<Tag::Type::Compound>();
      for (auto const &it : *itemsJ) {
        if (auto c = std::dynamic_pointer_cast<CompoundTag>(it); c) {
          if (auto slotJ = c->byte(u8"Slot"); slotJ) {
            if (auto itemB = Item::From(c, ctx, dataVersion); itemB) {
              itemB->set(u8"Slot", Byte(*slotJ));
              itemsB->push_back(itemB);
            }
          }
        }
      }
      return itemsB;
    } else {
      return nullptr;
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

CompoundTagPtr Item::From(CompoundTagPtr const &item, Context &ctx, int sourceDataVersion) {
  return Impl::From(item, ctx, sourceDataVersion);
}

i8 Item::GetSkullTypeFromBlockName(std::u8string_view const &name) {
  return Impl::GetSkullTypeFromBlockName(name);
}

CompoundTagPtr Item::Empty() {
  return Impl::Empty();
}

} // namespace je2be::java
