#include "java/_tile-entity.hpp"

#include "_namespace.hpp"
#include "_nbt-ext.hpp"
#include "_optional.hpp"
#include "_props.hpp"
#include "color/_sign-color.hpp"
#include "command/_command.hpp"
#include "enums/_banner-color-code-bedrock.hpp"
#include "enums/_color-code-java.hpp"
#include "enums/_facing4.hpp"
#include "item/_banner.hpp"
#include "java/_components.hpp"
#include "java/_context.hpp"
#include "java/_entity.hpp"
#include "java/_item.hpp"
#include "java/_versions.hpp"
#include "java/_world-data.hpp"
#include "tile-entity/_beacon.hpp"
#include "tile-entity/_loot-table.hpp"

#include <numbers>
#include <variant>

namespace je2be::java {

class TileEntity::Impl {
private:
  using Block = mcfile::je::Block;
  using Converter = std::function<CompoundTagPtr(Pos3i const &, Block const &, CompoundTagPtr const &, Context &ctx, DataVersion const &dataVersion)>;

  Impl() = delete;

public:
  static bool IsTileEntity(mcfile::blocks::BlockId id) {
    auto const &table = Table();
    auto found = table.find(id);
    return found != table.end();
  }

  static CompoundTagPtr FromBlockAndTileEntity(Pos3i const &pos,
                                               mcfile::je::Block const &block,
                                               CompoundTagPtr const &tag,
                                               Context &ctx,
                                               DataVersion const &dataVersion) {
    using namespace std;
    using namespace mcfile;

    auto const &table = Table();
    if (auto found = table.find(block.fId); found != table.end()) {
      return found->second(pos, block, tag, ctx, dataVersion);
    } else {
      return nullptr;
    }
  }

  static CompoundTagPtr FromBlock(Pos3i const &pos,
                                  mcfile::je::Block const &block,
                                  Context &ctx,
                                  DataVersion const &dataVersion) {
    using namespace std;
    using namespace mcfile;

    auto const &table = Table();
    if (auto found = table.find(block.fId); found != table.end()) {
      return found->second(pos, block, nullptr, ctx, dataVersion);
    } else {
      return nullptr;
    }
  }

  static bool IsStandaloneTileEntity(CompoundTagPtr const &tag) {
    auto id = tag->string(u8"id");
    if (!id) {
      return false;
    }
    auto const &name = *id;
    if (name == u8"minecraft:mob_spawner") {
      return true;
    }
    return false;
  }

  static std::optional<std::tuple<CompoundTagPtr, std::u8string>> StandaloneTileEntityBlockdData(Pos3i pos, CompoundTagPtr const &tag) {
    auto id = tag->string(u8"id");
    if (!id) {
      return std::nullopt;
    }
    auto const &name = *id;
    if (name == u8"minecraft:mob_spawner") {
      auto b = Compound();
      b->insert({
          {u8"name", String(u8"minecraft:mob_spawner")},
          {u8"version", Int(kBlockDataVersion)},
          {u8"states", Compound()},
      });
      return std::make_tuple(b, u8"minecraft:mob_spawner");
    }
    return std::nullopt;
  }

  static CompoundTagPtr StandaloneTileEntityData(CompoundTagPtr const &tag) {
    auto id = tag->string(u8"id");
    if (!id) {
      return nullptr;
    }
    auto const &name = *id;
    if (name == u8"minecraft:mob_spawner") {
      return Spawner(tag);
    }
    return nullptr;
  }

private:
  static std::unordered_map<mcfile::blocks::BlockId, Converter> const &Table() {
    using namespace std;
    static unique_ptr<unordered_map<mcfile::blocks::BlockId, Converter> const> const table(CreateTable());
    return *table;
  }

  static std::unordered_map<mcfile::blocks::BlockId, Converter> *CreateTable() {
    using namespace std;
    auto table = new unordered_map<mcfile::blocks::BlockId, Converter>();
#define E(__name, __func)                                       \
  assert(table->count(mcfile::blocks::minecraft::__name) == 0); \
  table->try_emplace(mcfile::blocks::minecraft::__name, __func)
    E(chest, Chest);
    E(trapped_chest, Chest);

    auto sign = Sign(u8"Sign");
    E(oak_sign, sign);
    E(spruce_sign, sign);
    E(birch_sign, sign);
    E(jungle_sign, sign);
    E(acacia_sign, sign);
    E(dark_oak_sign, sign);
    E(crimson_sign, sign);
    E(warped_sign, sign);
    E(mangrove_sign, sign);
    E(bamboo_sign, sign);

    E(oak_wall_sign, sign);
    E(spruce_wall_sign, sign);
    E(birch_wall_sign, sign);
    E(jungle_wall_sign, sign);
    E(acacia_wall_sign, sign);
    E(dark_oak_wall_sign, sign);
    E(crimson_wall_sign, sign);
    E(warped_wall_sign, sign);
    E(mangrove_wall_sign, sign);
    E(bamboo_wall_sign, sign);

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

    E(white_wall_banner, Banner);
    E(orange_wall_banner, Banner);
    E(magenta_wall_banner, Banner);
    E(light_blue_wall_banner, Banner);
    E(yellow_wall_banner, Banner);
    E(lime_wall_banner, Banner);
    E(pink_wall_banner, Banner);
    E(gray_wall_banner, Banner);
    E(light_gray_wall_banner, Banner);
    E(cyan_wall_banner, Banner);
    E(purple_wall_banner, Banner);
    E(blue_wall_banner, Banner);
    E(brown_wall_banner, Banner);
    E(green_wall_banner, Banner);
    E(red_wall_banner, Banner);
    E(black_wall_banner, Banner);

    E(potted_oak_sapling, PottedSapling);
    E(potted_spruce_sapling, PottedSapling);
    E(potted_birch_sapling, PottedSapling);
    E(potted_jungle_sapling, PottedSapling);
    E(potted_acacia_sapling, PottedSapling);
    E(potted_dark_oak_sapling, PottedSapling);
    E(potted_poppy, PottedPlant(u8"poppy", {}));
    E(potted_blue_orchid, PottedPlant(u8"blue_orchid", {}));
    E(potted_allium, PottedPlant(u8"allium", {}));
    E(potted_azure_bluet, PottedPlant(u8"azure_bluet", {}));
    E(potted_red_tulip, PottedPlant(u8"red_tulip", {}));
    E(potted_orange_tulip, PottedPlant(u8"orange_tulip", {}));
    E(potted_white_tulip, PottedPlant(u8"white_tulip", {}));
    E(potted_pink_tulip, PottedPlant(u8"pink_tulip", {}));
    E(potted_oxeye_daisy, PottedPlant(u8"oxeye_daisy", {}));
    E(potted_cornflower, PottedPlant(u8"cornflower", {}));
    E(potted_lily_of_the_valley, PottedPlant(u8"lily_of_the_valley", {}));
    E(potted_dandelion, PottedPlant(u8"dandelion", {}));
    E(potted_wither_rose, PottedPlant(u8"wither_rose", {}));
    E(potted_crimson_fungus, PottedPlant(u8"crimson_fungus", {}));
    E(potted_warped_fungus, PottedPlant(u8"warped_fungus", {}));
    E(potted_dead_bush, PottedPlant(u8"deadbush", {}));
    E(potted_red_mushroom, PottedPlant(u8"red_mushroom", {}));
    E(potted_brown_mushroom, PottedPlant(u8"brown_mushroom", {}));
    E(potted_fern, PottedPlant(u8"fern", {}));
    E(potted_bamboo, PottedBamboo);
    E(potted_crimson_roots, PottedPlant(u8"crimson_roots", {}));
    E(potted_warped_roots, PottedPlant(u8"warped_roots", {}));
    E(potted_cactus, PottedPlant(u8"cactus", {{u8"age", i32(0)}}));
    E(potted_azalea_bush, PottedPlant(u8"azalea", {}));
    E(potted_flowering_azalea_bush, PottedPlant(u8"flowering_azalea", {}));
    E(potted_mangrove_propagule, PottedPlant(u8"mangrove_propagule", {{u8"hanging", false}, {u8"propagule_stage", i32(0)}}));
    E(potted_cherry_sapling, PottedPlant(u8"cherry_sapling", {{u8"age_bit", i8(0)}}));
    E(potted_torchflower, PottedPlant(u8"torchflower", {}));
    E(flower_pot, FlowerPot);

    E(skeleton_skull, Skull);
    E(wither_skeleton_skull, Skull);
    E(player_head, Skull);
    E(zombie_head, Skull);
    E(creeper_head, Skull);
    E(dragon_head, Skull);
    E(piglin_head, Skull);
    E(skeleton_wall_skull, Skull);
    E(wither_skeleton_wall_skull, Skull);
    E(player_wall_head, Skull);
    E(zombie_wall_head, Skull);
    E(creeper_wall_head, Skull);
    E(dragon_wall_head, Skull);
    E(piglin_wall_head, Skull);

    E(barrel, AnyStorage(u8"Barrel", false));
    E(furnace, Furnace(u8"Furnace"));
    E(brewing_stand, BrewingStand);
    E(blast_furnace, Furnace(u8"BlastFurnace"));
    E(smoker, Furnace(u8"Smoker"));
    E(hopper, Hopper);
    E(dropper, AnyStorage(u8"Dropper", nullopt));
    E(dispenser, AnyStorage(u8"Dispenser", nullopt));

    E(note_block, Note);
    E(jukebox, Jukebox);
    E(cauldron, Cauldron);
    E(water_cauldron, Cauldron);
    E(lava_cauldron, Cauldron);
    E(powder_snow_cauldron, Cauldron);
    E(end_portal, EndPortal);
    E(beacon, Beacon);
    E(lectern, Lectern);
    E(bee_nest, Beehive);
    E(beehive, Beehive);
    E(command_block, CommandBlock);
    E(chain_command_block, CommandBlock);
    E(repeating_command_block, CommandBlock);

    E(moving_piston, MovingPiston);
    E(sticky_piston, PistonArm(true));
    E(piston, PistonArm(false));

    E(end_gateway, EndGateway);
    E(ender_chest, EnderChest);
    E(enchanting_table, EnchantingTable);
    E(bell, Bell);
    E(conduit, Conduit);
    E(campfire, Campfire);
    E(soul_campfire, Campfire);
    E(comparator, Comparator);
    E(daylight_detector, NamedEmpty(u8"DaylightDetector"));
    E(structure_block, StructureBlock);

    E(sculk_sensor, NamedEmpty(u8"SculkSensor"));
    E(sculk_shrieker, NamedEmpty(u8"SculkShrieker"));
    E(sculk_catalyst, NamedEmpty(u8"SculkCatalyst"));

    auto hangingSign = Sign(u8"HangingSign");
    E(acacia_hanging_sign, hangingSign);
    E(acacia_wall_hanging_sign, hangingSign);
    E(bamboo_hanging_sign, hangingSign);
    E(bamboo_wall_hanging_sign, hangingSign);
    E(birch_hanging_sign, hangingSign);
    E(birch_wall_hanging_sign, hangingSign);
    E(crimson_hanging_sign, hangingSign);
    E(crimson_wall_hanging_sign, hangingSign);
    E(dark_oak_hanging_sign, hangingSign);
    E(dark_oak_wall_hanging_sign, hangingSign);
    E(jungle_hanging_sign, hangingSign);
    E(jungle_wall_hanging_sign, hangingSign);
    E(mangrove_hanging_sign, hangingSign);
    E(mangrove_wall_hanging_sign, hangingSign);
    E(oak_hanging_sign, hangingSign);
    E(oak_wall_hanging_sign, hangingSign);
    E(spruce_hanging_sign, hangingSign);
    E(spruce_wall_hanging_sign, hangingSign);
    E(warped_hanging_sign, hangingSign);
    E(warped_wall_hanging_sign, hangingSign);

    E(chiseled_bookshelf, ChiseledBookshelf);
    E(decorated_pot, DecoratedPot);
    E(suspicious_sand, BrushableBlock);
    E(suspicious_gravel, BrushableBlock);
    E(jigsaw, Jigsaw);
    E(cherry_hanging_sign, hangingSign);
    E(cherry_wall_hanging_sign, hangingSign);
    E(cherry_sign, sign);
    E(cherry_wall_sign, sign);
    E(calibrated_sculk_sensor, NamedEmpty(u8"CalibratedSculkSensor"));
    E(lodestone, Lodestone);
    E(spore_blossom, NamedEmptyFromNull(u8"SporeBlossom"));

    E(crafter, Crafter);
    E(vault, Vault);
    E(trial_spawner, TrialSpawner);

    E(creaking_heart, CreakingHeart);
    E(pale_oak_sign, sign);
    E(pale_oak_wall_sign, sign);
    E(pale_oak_hanging_sign, hangingSign);
    E(pale_oak_wall_hanging_sign, hangingSign);
    E(potted_open_eyeblossom, PottedPlant(u8"open_eyeblossom", {}));
    E(potted_closed_eyeblossom, PottedPlant(u8"closed_eyeblossom", {}));
    E(potted_pale_oak_sapling, PottedPlant(u8"pale_oak_sapling", {{u8"age_bit", i8(0)}}));

    E(copper_chest, Chest);
    E(exposed_copper_chest, Chest);
    E(weathered_copper_chest, Chest);
    E(oxidized_copper_chest, Chest);
    E(waxed_copper_chest, Chest);
    E(waxed_exposed_copper_chest, Chest);
    E(waxed_weathered_copper_chest, Chest);
    E(waxed_oxidized_copper_chest, Chest);
    E(copper_golem_statue, CopperGolemStatue);
    E(exposed_copper_golem_statue, CopperGolemStatue);
    E(weathered_copper_golem_statue, CopperGolemStatue);
    E(oxidized_copper_golem_statue, CopperGolemStatue);
    E(waxed_copper_golem_statue, CopperGolemStatue);
    E(waxed_exposed_copper_golem_statue, CopperGolemStatue);
    E(waxed_weathered_copper_golem_statue, CopperGolemStatue);
    E(waxed_oxidized_copper_golem_statue, CopperGolemStatue);
#undef E
    return table;
  }

#pragma region Converters: B
  static CompoundTagPtr Banner(Pos3i const &pos, Block const &b, CompoundTagPtr const &c, Context &ctx, DataVersion const &dataVersion) {
    using namespace std;
    auto tag = Compound();

    i32 type = 0;
    if (auto customNameString = Migrate<StringTag>(c, u8"item_name", Depth::Root, u8"CustomName"); customNameString) {
      if (auto customName = props::ParseAsJson(customNameString->fValue); customName) {
        auto translate = GetAsString(*customName, "translate");
        if (translate == u8"block.minecraft.ominous_banner") {
          type = 1; // Illager Banner
        }
      }
    } else if (auto itemName = GetComponent<CompoundTag>(c, u8"item_name"); itemName) {
      if (auto translate = itemName->string(u8"translate"); translate == u8"block.minecraft.ominous_banner") {
        type = 1; // Illager Banner
      }
    }

    auto patternsJ = FallbackPtr<ListTag>(c, {u8"patterns", u8"Patterns"});
    auto patternsB = List<Tag::Type::Compound>();
    if (patternsJ && type != 1) {
      for (auto const &pattern : *patternsJ) {
        auto p = pattern->asCompound();
        if (!p) {
          continue;
        }
        ColorCodeJava color;
        if (auto patternColor = p->string(u8"color"); patternColor) {
          color = ColorCodeJavaFromJavaName(*patternColor);
        } else if (auto legacyPatternColor = p->int32(u8"Color"); legacyPatternColor) {
          color = static_cast<ColorCodeJava>(*legacyPatternColor);
        } else {
          continue;
        }
        auto patternJ = FallbackPtr<StringTag>(*p, {u8"pattern", u8"Pattern"});
        if (!patternJ) {
          continue;
        }
        auto patternB = Banner::BedrockOrLegacyJavaPatternFromJava(patternJ->fValue);
        auto ptag = Compound();
        ptag->insert({
            {u8"Color", Int(static_cast<i32>(BannerColorCodeFromJava(color)))},
            {u8"Pattern", String(Wrap(patternB, patternJ->fValue))},
        });
        patternsB->push_back(ptag);
      }
    }

    i32 base = BannerColor(b.fName);

    tag->insert({
        {u8"id", String(u8"Banner")},
        {u8"isMovable", Bool(true)},
        {u8"Type", Int(type)},
        {u8"Base", Int(base)},
    });
    if (!patternsB->empty()) {
      tag->set(u8"Patterns", patternsB);
    }
    Attach(c, pos, *tag);
    return tag;
  }

  static CompoundTagPtr Beacon(Pos3i const &pos, Block const &b, CompoundTagPtr const &c, Context &ctx, DataVersion const &dataVersion) {
    int primary = 0;
    int secondary = 0;
    if (c) {
      if (auto primaryJ = c->string(u8"primary_effect"); primaryJ) {
        primary = Beacon::BedrockEffectFromJava(*primaryJ);
      } else if (auto legacyPrimaryJ = c->int32(u8"Primary"); legacyPrimaryJ) {
        // <= 1.20.1
        primary = *legacyPrimaryJ;
      }

      if (auto secondaryJ = c->string(u8"secondary_effect"); secondaryJ) {
        secondary = Beacon::BedrockEffectFromJava(*secondaryJ);
      } else if (auto legacySecondaryJ = c->int32(u8"Secondary"); legacySecondaryJ) {
        // <= 1.20.1
        secondary = *legacySecondaryJ;
      }
    }

    auto tag = Compound();
    tag->insert({
        {u8"id", String(u8"Beacon")},
        {u8"isMovable", Bool(true)},
        {u8"primary", Int(primary)},
        {u8"secondary", Int(secondary)},
    });
    Attach(c, pos, *tag);
    return tag;
  }

  static CompoundTagPtr Bed(Pos3i const &pos, Block const &b, CompoundTagPtr const &c, Context &ctx, DataVersion const &dataVersion) {
    auto tag = Compound();
    auto color = BedColor(b.fName);
    tag->insert({
        {u8"id", String(u8"Bed")},
        {u8"color", Byte(color)},
        {u8"isMovable", Bool(true)},
    });
    Attach(c, pos, *tag);
    return tag;
  }

  static CompoundTagPtr Beehive(Pos3i const &pos, Block const &, CompoundTagPtr const &c, Context &ctx, DataVersion const &dataVersion) {
    using namespace std;

    auto tag = Compound();
    tag->insert({
        {u8"id", String(u8"Beehive")},
        {u8"isMovable", Bool(true)},
    });

    shared_ptr<ListTag> bees;
    if (c) {
      bees = FallbackPtr<ListTag>(*c, {u8"bees", u8"Bees"});
    }
    if (bees) {
      auto occupants = List<Tag::Type::Compound>();
      i32 index = 0;
      for (auto const &it : *bees) {
        index++;
        auto bee = dynamic_pointer_cast<CompoundTag>(it);
        if (!bee) {
          continue;
        }
        auto entityData = FallbackPtr<CompoundTag>(bee, {u8"entity_data", u8"EntityData"});
        if (!entityData) {
          continue;
        }

        // UUIDs are not stored in EntityData, so create a unique id.
        mcfile::XXHash<u64> h;
        h.update(&ctx.fWorldData.fDim, sizeof(ctx.fWorldData.fDim));
        h.update(&pos.fX, sizeof(pos.fX));
        h.update(&pos.fZ, sizeof(pos.fZ));
        u64 hash = h.digest();
        u32 lo = ((u32 *)&hash)[0];
        u32 hi = ((u32 *)&hash)[1];
        vector<i32> uuidSource = {*(i32 *)&lo, *(i32 *)&hi, pos.fY, index};
        auto uuid = make_shared<IntArrayTag>(uuidSource);
        entityData->set(u8"UUID", uuid);

        auto converted = Entity::From(*entityData, ctx, dataVersion, {});
        if (!converted.fEntity) {
          continue;
        }
        auto saveData = converted.fEntity;

        auto outBee = Compound();
        outBee->set(u8"SaveData", saveData);

        auto minOccupationTicks = bee->int32(u8"MinOccupationTicks");
        auto ticksInHive = bee->int32(u8"TicksInHive");
        if (minOccupationTicks && ticksInHive) {
          i32 ticksLeftToStay = std::max<i32>(0, *minOccupationTicks - *ticksInHive);
          outBee->set(u8"TicksLeftToStay", Int(ticksLeftToStay));
        }

        outBee->set(u8"ActorIdentifier", u8"minecraft:bee<>");

        occupants->push_back(outBee);
      }
      tag->set(u8"Occupants", occupants);
    }
    tag->set(u8"ShouldSpawnBees", Bool(false));

    Attach(c, pos, *tag);
    return tag;
  }

  static CompoundTagPtr Bell(Pos3i const &pos, Block const &b, CompoundTagPtr const &c, Context &ctx, DataVersion const &dataVersion) {
    if (!c) {
      return nullptr;
    }
    auto t = Empty(u8"Bell", *c, pos);
    t->set(u8"Direction", Int(255));
    t->set(u8"Ringing", Bool(false));
    t->set(u8"Ticks", Int(0));
    t->set(u8"isMovable", Bool(true));
    return t;
  }

  static CompoundTagPtr BrewingStand(Pos3i const &pos, Block const &b, CompoundTagPtr const &c, Context &ctx, DataVersion const &dataVersion) {
    using namespace std;

    if (!c) {
      return nullptr;
    }

    auto tag = Compound();
    auto items = GetItems(c, u8"Items", ctx, dataVersion, {.fConvertSlotTag = true});
    if (!items) {
      return nullptr;
    }
    array<shared_ptr<CompoundTag>, 5> sorted;
    for (auto const &it : *items) {
      auto item = it->asCompound();
      if (!item) {
        continue;
      }
      auto slot = item->byte(u8"Slot");
      if (!slot) {
        continue;
      }

      auto newItem = Compound();
      for (auto const &prop : *item) {
        if (prop.first == u8"Slot") {
          continue;
        }
        newItem->set(prop.first, prop.second);
      }
      if (*slot < 0 || 4 < *slot) {
        continue;
      }
      u8 const mapping[5] = {1, 2, 3, 0, 4};
      u8 newSlot = mapping[*slot];
      newItem->set(u8"Slot", Byte(newSlot));
      sorted[newSlot] = newItem;
    }

    auto reordered = List<Tag::Type::Compound>();
    for (auto it : sorted) {
      if (!it) {
        continue;
      }
      reordered->push_back(it);
    }

    CopyShortValues(*c, *tag, {{u8"BrewTime", u8"CookTime", 0}});

    auto fuel = c->byte(u8"Fuel", 0);
    tag->set(u8"FuelAmount", Short(fuel));
    tag->set(u8"FuelTotal", Short(fuel > 0 ? 20 : 0));

    tag->insert({
        {u8"Items", reordered},
        {u8"id", String(u8"BrewingStand")},
        {u8"isMovable", Bool(true)},
    });
    Attach(c, pos, *tag);
    return tag;
  }

  static CompoundTagPtr BrushableBlock(Pos3i const &pos, Block const &b, CompoundTagPtr const &c, Context &ctx, DataVersion const &dataVersion) {
    auto tag = New(u8"BrushableBlock");

    if (c) {
      if (LootTable::JavaToBedrock(*c, *tag) == LootTable::State::NoLootTable) {
        if (auto itemJ = c->compoundTag(u8"item"); itemJ) {
          if (auto itemB = Item::From(itemJ, ctx, dataVersion); itemB) {
            tag->set(u8"item", itemB);
          }
        }
      }
    }

    auto dusted = Wrap(strings::ToI32(b.property(u8"dusted", u8"0")), 0);
    tag->set(u8"brush_count", Int(dusted));

    tag->set(u8"type", std::u8string(b.fName));
    tag->set(u8"brush_direction", Byte(6));

    Attach(c, pos, *tag);
    return tag;
  }
#pragma endregion

#pragma region Converters: C
  static CompoundTagPtr Campfire(Pos3i const &pos, Block const &b, CompoundTagPtr const &c, Context &ctx, DataVersion const &dataVersion) {
    using namespace std;
    if (!c) {
      return nullptr;
    }
    auto t = Empty(u8"Campfire", *c, pos);
    auto timesTag = c->intArrayTag(u8"CookingTimes");
    if (timesTag) {
      auto const &times = timesTag->value();
      for (int i = 0; i < 4 && i < times.size(); i++) {
        int time = times[i];
        t->set(u8"ItemTime" + mcfile::String::ToString(i + 1), Int(time));
      }
    }
    auto items = GetItems(c, u8"Items", ctx, dataVersion, {.fConvertSlotTag = false});
    if (items) {
      for (int i = 0; i < 4 && i < items->size(); i++) {
        if (auto item = items->at(i); item) {
          t->set(u8"Item" + mcfile::String::ToString(i + 1), item);
        }
      }
    }
    t->set(u8"isMovable", Bool(true));
    return t;
  }

  static CompoundTagPtr Cauldron(Pos3i const &pos, Block const &b, CompoundTagPtr const &c, Context &ctx, DataVersion const &dataVersion) {
    using namespace std;

    auto tag = Compound();
    tag->insert({{u8"id", String(u8"Cauldron")},
                 {u8"isMovable", Bool(true)},
                 {u8"PotionId", Short(-1)},
                 {u8"PotionType", Short(-1)},
                 {u8"Items", List<Tag::Type::Compound>()}});

    Attach(c, pos, *tag);
    return tag;
  }

  static CompoundTagPtr Chest(Pos3i const &pos, mcfile::je::Block const &b, CompoundTagPtr const &comp, Context &ctx, DataVersion const &dataVersion) {
    using namespace std;

    auto type = b.property(u8"type", u8"single");
    auto facing = b.property(u8"facing", u8"north");
    optional<Pos2i> pairPos;
    Facing4 f4 = Facing4FromJavaName(facing);
    Pos2i direction = Pos2iFromFacing4(f4);
    if (type == u8"left") {
      pairPos = Pos2i(pos.fX, pos.fZ) + Right90(direction);
    } else if (type == u8"right") {
      pairPos = Pos2i(pos.fX, pos.fZ) + Left90(direction);
    }

    auto tag = Compound();
    if (comp) {
      tag->set(u8"Items", List<Tag::Type::Compound>());
      if (auto st = LootTable::JavaToBedrock(*comp, *tag); st == LootTable::State::NoLootTable) {
        auto items = GetItems(comp, u8"Items", ctx, dataVersion, {.fConvertSlotTag = true});
        if (items) {
          tag->set(u8"Items", items);
        }
      }
    }

    tag->insert({
        {u8"Findable", Bool(false)},
        {u8"id", String(u8"Chest")},
        {u8"isMovable", Bool(true)},
    });
    if (pairPos) {
      tag->set(u8"pairlead", Bool(type == u8"right"));
      tag->set(u8"pairx", Int(pairPos->fX));
      tag->set(u8"pairz", Int(pairPos->fZ));
    }
    Attach(comp, pos, *tag);
    return tag;
  }

  static CompoundTagPtr ChiseledBookshelf(Pos3i const &pos, Block const &b, CompoundTagPtr const &c, Context &ctx, DataVersion const &dataVersion) {
    auto tag = Compound();
    auto items = GetItemsRemovingSlot(c, u8"Items", ctx, 6, dataVersion);
    if (items) {
      bool empty = true;
      for (auto const &item : *items) {
        if (auto comp = item->asCompound(); comp) {
          if (auto count = Item::Count(*comp); count && *count > 0) {
            empty = false;
            break;
          }
        }
      }
      if (empty) {
        items.reset();
      }
    }
    tag->insert({
        {u8"isMovable", Bool(true)},
        {u8"id", String(u8"ChiseledBookshelf")},
    });
    if (items) {
      tag->set(u8"Items", items);
    }

    if (c) {
      CopyIntValues(*c, *tag, {{u8"last_interacted_slot", u8"LastInteractedSlot"}});
    }

    Attach(c, pos, *tag);
    return tag;
  }

  static CompoundTagPtr CommandBlock(Pos3i const &pos, Block const &b, CompoundTagPtr const &c, Context &ctx, DataVersion const &dataVersion) {
    if (!c) {
      return nullptr;
    }

    auto tag = Compound();
    auto powered = c->boolean(u8"powered", false);
    auto automatic = c->boolean(u8"auto", false);
    auto trackOutput = c->boolean(u8"TrackOutput", true);
    auto successCount = c->int32(u8"SuccessCount", 0);
    auto commandJ = props::GetTextComponent(c->string(u8"Command", u8""));
    auto commandB = command::Command::TranspileJavaToBedrock(commandJ);
    auto conditionMet = c->boolean(u8"conditionMet", false);

    auto lastExecution = c->int64(u8"LastExecution");
    if (lastExecution) {
      tag->set(u8"LastExecution", Long(*lastExecution));
    }

    tag->insert({
        {u8"id", String(u8"CommandBlock")},
        {u8"isMovable", Bool(true)},
        {u8"powered", Bool(powered)},
        {u8"auto", Bool(automatic)},
        {u8"TickDelay", Int(0)},
        {u8"TrackOutput", Bool(trackOutput)},
        {u8"Version", Int(34)},
        {u8"SuccessCount", Int(successCount)},
        {u8"Command", String(commandB)},
        {u8"ExecuteOnFirstTick", Bool(false)},
        {u8"conditionMet", Bool(conditionMet)},
    });
    Attach(c, pos, *tag);
    auto customNameJ = c->string(u8"CustomName");
    if (customNameJ) {
      auto text = props::GetTextComponent(*customNameJ);
      if (!text.empty()) {
        tag->set(u8"CustomName", text);
      }
    }
    return tag;
  }

  static CompoundTagPtr Comparator(Pos3i const &pos, Block const &b, CompoundTagPtr const &c, Context &ctx, DataVersion const &dataVersion) {
    if (!c) {
      return nullptr;
    }
    auto t = Empty(u8"Comparator", *c, pos);
    CopyIntValues(*c, *t, {{u8"OutputSignal", u8"OutputSignal", 0}});
    t->set(u8"isMovable", Bool(true));
    return t;
  }

  static CompoundTagPtr Conduit(Pos3i const &pos, Block const &b, CompoundTagPtr const &c, Context &ctx, DataVersion const &dataVersion) {
    if (!c) {
      return nullptr;
    }
    auto t = Empty(u8"Conduit", *c, pos);
    t->set(u8"Active", Bool(false));
    t->set(u8"Target", Long(-1));
    t->set(u8"isMovable", Bool(true));
    return t;
  }

  static CompoundTagPtr CopperGolemStatue(Pos3i const &pos, mcfile::je::Block const &b, CompoundTagPtr const &j, Context &ctx, DataVersion const &dataVersion) {
    auto tagB = Compound();
    auto actor = Compound();
    actor->set(u8"ActorIdentifier", String(u8"minecraft:copper_golem<>"));
    actor->set(u8"SaveData", Compound());
    tagB->set(u8"Actor", actor);
    tagB->set(u8"id", String(u8"CopperGolemStatue"));
    tagB->set(u8"isMovable", Bool(true));
    auto poseJ = b.property(u8"copper_golem_pose", u8"standing");
    int poseB = 0;
    if (poseJ == u8"sitting") {
      poseB = 1;
    } else if (poseJ == u8"running") {
      poseB = 2;
    } else if (poseJ == u8"star") {
      poseB = 3;
    }
    tagB->set(u8"Pose", Int(poseB));
    Attach(j, pos, *tagB);
    return tagB;
  }

  static CompoundTagPtr Crafter(Pos3i const &pos, Block const &b, CompoundTagPtr const &c, Context &ctx, DataVersion const &dataVersion) {
    auto tag = New(u8"Crafter");

    if (auto itemsB = GetItems(c, u8"Items", ctx, dataVersion, {}); itemsB) {
      tag->set(u8"Items", itemsB);
    }
    if (c) {
      u16 disabledSlotsB = 0;
      if (auto disabledSlotsJ = c->intArrayTag(u8"disabled_slots"); disabledSlotsJ) {
        for (auto index : disabledSlotsJ->fValue) {
          if (0 <= index && index <= 8) {
            disabledSlotsB |= u16(1) << index;
          }
        }
      }

      tag->set(u8"disabled_slots", Short(*(i16 *)&disabledSlotsB));
    }

    Attach(c, pos, *tag);
    return tag;
  }

  static CompoundTagPtr CreakingHeart(Pos3i const &pos, Block const &b, CompoundTagPtr const &c, Context &ctx, DataVersion const &dataVersion) {
    auto ret = New(u8"CreakingHeart");
    if (c) {
      if (auto id = c->intArrayTag(u8"creaking"); id) {
        if (auto uuid = Uuid::FromIntArray(*id); uuid) {
          auto uuidB = ctx.fUuids->toId(*uuid);
          ret->set(u8"SpawnedCreakingID", Long(uuidB));
        }
      }
    }
    Attach(c, pos, *ret);
    return ret;
  }
#pragma endregion

#pragma region Converters: D
  static CompoundTagPtr DecoratedPot(Pos3i const &pos, Block const &b, CompoundTagPtr const &c, Context &ctx, DataVersion const &dataVersion) {
    auto tag = Compound();
    auto sherdsB = List<Tag::Type::String>();
    bool empty = true;
    for (int i = 0; i < 4; i++) {
      sherdsB->push_back(String(u8""));
    }
    if (c) {
      auto sherdsJ = c->listTag(u8"sherds");
      if (!sherdsJ) {
        // < 1.20
        sherdsJ = c->listTag(u8"shards");
      }
      if (sherdsJ) {
        for (int i = 0; i < 4 && i < sherdsJ->size(); i++) {
          auto const &sherd = sherdsJ->at(i);
          if (auto sherdName = sherd->asString(); sherdName && !sherdName->fValue.empty()) {
            sherdsB->fValue[i] = String(sherdName->fValue);
            empty = false;
          }
        }
      }

      if (LootTable::JavaToBedrock(*c, *tag) == LootTable::State::NoLootTable) {
        if (auto itemJ = c->compoundTag(u8"item"); itemJ) {
          if (auto itemB = Item::From(itemJ, ctx, dataVersion); itemB) {
            tag->set(u8"item", itemB);
          } else {
            tag->set(u8"item", Item::Empty());
          }
        }
      }
    }
    tag->insert({{u8"isMovable", Bool(true)},
                 {u8"id", String(u8"DecoratedPot")},
                 {u8"animation", Byte(0)}});
    if (!empty) {
      tag->set(u8"sherds", sherdsB);
    }
    Attach(c, pos, *tag);
    return tag;
  }
#pragma endregion

#pragma region Converters: E
  static CompoundTagPtr EnchantingTable(Pos3i const &pos, Block const &b, CompoundTagPtr const &c, Context &ctx, DataVersion const &dataVersion) {
    if (!c) {
      return nullptr;
    }
    auto t = Empty(u8"EnchantTable", *c, pos);

    // "rott"
    // [-pi, pi] float, an angle of enchant table and player's position who placed the table.
    // Java doesn't store such data in tile entity, so generate rott based on its xyz position.
    mcfile::XXHash<u32> xx(0);
    xx.update(&pos.fX, sizeof(pos.fX));
    xx.update(&pos.fY, sizeof(pos.fY));
    xx.update(&pos.fZ, sizeof(pos.fZ));
    u32 seed = xx.digest();
    std::mt19937 gen(seed);
    std::uniform_real_distribution<float> distribution(-std::numbers::pi, std::numbers::pi);
    float rott = distribution(gen);
    t->set(u8"rott", Float(rott));

    return t;
  }

  static CompoundTagPtr EnderChest(Pos3i const &pos, Block const &b, CompoundTagPtr const &c, Context &ctx, DataVersion const &dataVersion) {
    auto ret = AnyStorage(u8"EnderChest", false)(pos, b, c, ctx, dataVersion);
    ret->set(u8"Items", List<Tag::Type::Compound>());
    return ret;
  }

  static CompoundTagPtr EndGateway(Pos3i const &pos, Block const &b, CompoundTagPtr const &c, Context &ctx, DataVersion const &dataVersion) {
    using namespace std;
    if (!c) {
      return nullptr;
    }
    auto tag = Compound();
    tag->insert({
        {u8"id", String(u8"EndGateway")},
        {u8"isMovable", Bool(true)},
    });

    auto age = c->int64(u8"Age", 0);
    tag->set(u8"Age", Int(age));

    auto exitPortal = c->compoundTag(u8"ExitPortal");
    if (exitPortal) {
      auto x = exitPortal->int32(u8"X");
      auto y = exitPortal->int32(u8"Y");
      auto z = exitPortal->int32(u8"Z");
      if (x && y && z) {
        auto ep = List<Tag::Type::Int>();
        ep->push_back(Int(*x));
        ep->push_back(Int(*y));
        ep->push_back(Int(*z));
        tag->set(u8"ExitPortal", ep);
      }
    }
    Attach(c, pos, *tag);
    return tag;
  }

  static CompoundTagPtr EndPortal(Pos3i const &pos, Block const &b, CompoundTagPtr const &c, Context &ctx, DataVersion const &dataVersion) {
    auto tag = Compound();
    tag->insert({
        {u8"id", String(u8"EndPortal")},
        {u8"isMovable", Bool(true)},
    });
    Attach(c, pos, *tag);
    return tag;
  }
#pragma endregion

#pragma region Converters: F
  static CompoundTagPtr FlowerPot(Pos3i const &pos, Block const &b, CompoundTagPtr const &c, Context &ctx, DataVersion const &dataVersion) {
    using namespace std;

    auto tag = Compound();
    tag->set(u8"id", u8"FlowerPot");
    tag->set(u8"isMovable", Bool(true));
    if (c) {
      // legacy: Java 1.7.10
      auto item = c->int32(u8"Item");
      if (!item) {
        return nullptr;
      }
      auto data = c->int32(u8"Data", 0);

      auto states = Compound();
      auto plantBlock = Compound();
      plantBlock->set(u8"version", Int(kBlockDataVersion));
      u8string name;

      switch (*item) {
      case 6: {
        u8string type;
        switch (data) {
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
        name = type + u8"_sapling";
        states->set(u8"age_bit", Byte(0));
        break;
      }
      case 31: {
        name = u8"fern";
        break;
      }
      case 32:
        name = u8"deadbush";
        break;
      case 37:
        name = u8"yellow_flower";
        break;
      case 38: {
        name = u8"poppy";
        switch (data) {
        case 1:
          name = u8"blue_orchid"; // orchid
          break;
        case 2:
          name = u8"allium";
          break;
        case 3:
          name = u8"azure_bluet"; // houstonia
          break;
        case 4:
          name = u8"red_tulip"; // tulip_red
          break;
        case 5:
          name = u8"orange_tulip"; // tulip_orange
          break;
        case 6:
          name = u8"white_tulip"; // tulip_white
          break;
        case 7:
          name = u8"pink_tulip"; // tulip_pink
          break;
        case 8:
          name = u8"oxeye_daisy"; // oxeye
          break;
        case 0:
        default:
          name = u8"poppy";
          break;
        }
        break;
      }
      case 39:
        name = u8"brown_mushroom";
        break;
      case 40:
        name = u8"red_mushroom";
        break;
      case 81:
        name = u8"cactus";
        states->set(u8"age", Int(0));
        break;
      default:
        return nullptr;
      }
      plantBlock->set(u8"states", states);
      plantBlock->set(u8"name", u8"minecraft:" + name);

      tag->set(u8"PlantBlock", plantBlock);
    }
    Attach(c, pos, *tag);
    return tag;
  }
#pragma endregion

#pragma region Converters: H
  static CompoundTagPtr Hopper(Pos3i const &pos, Block const &b, CompoundTagPtr const &c, Context &ctx, DataVersion const &dataVersion) {
    if (!c) {
      return nullptr;
    }
    auto t = AnyStorage(u8"Hopper", std::nullopt)(pos, b, c, ctx, dataVersion);
    CopyIntValues(*c, *t, {{u8"TransferCooldown", u8"TransferCooldown", 0}});
    return t;
  }
#pragma endregion

#pragma region Converters: J
  static CompoundTagPtr Jigsaw(Pos3i const &pos, Block const &b, CompoundTagPtr const &c, Context &ctx, DataVersion const &dataVersion) {
    auto tag = New(u8"JigsawBlock");

    if (c) {
      CopyStringValues(*c, *tag, {{u8"final_state"}, {u8"joint"}, {u8"target"}, {u8"pool", u8"target_pool"}, {u8"name"}});
    }

    Attach(c, pos, *tag);
    return tag;
  }

  static CompoundTagPtr Jukebox(Pos3i const &pos, Block const &b, CompoundTagPtr const &c, Context &ctx, DataVersion const &dataVersion) {
    using namespace std;

    auto tag = Compound();
    tag->insert({
        {u8"id", String(u8"Jukebox")},
        {u8"isMovable", Bool(true)},
    });

    shared_ptr<CompoundTag> recordItem;
    if (c) {
      recordItem = c->compoundTag(u8"RecordItem");
    }
    if (recordItem) {
      auto beRecordItem = Item::From(recordItem, ctx, dataVersion);
      if (beRecordItem) {
        tag->set(u8"RecordItem", beRecordItem);
      }
    }

    Attach(c, pos, *tag);
    return tag;
  }
#pragma endregion

#pragma region Converters: L
  static CompoundTagPtr Lectern(Pos3i const &pos, Block const &b, CompoundTagPtr const &c, Context &ctx, DataVersion const &dataVersion) {
    using namespace std;

    auto tag = Compound();
    tag->insert({
        {u8"id", String(u8"Lectern")},
        {u8"isMovable", Bool(true)},
    });
    shared_ptr<CompoundTag> book;
    if (c) {
      book = c->compoundTag(u8"Book");
    }
    if (book) {
      auto item = Item::From(book, ctx, dataVersion);
      if (item) {
        tag->set(u8"book", item);
        auto pages = item->query(u8"tag/pages")->asList();
        if (auto page = c->int32(u8"Page"); page && page >= 0) {
          tag->set(u8"page", Int(*page));
        }
        i32 totalPages;
        if (pages) {
          totalPages = pages->size();
        } else {
          totalPages = 0;
        }
        tag->set(u8"hasBook", Bool(true));
        tag->set(u8"totalPages", Int(totalPages));
      }
    }
    Attach(c, pos, *tag);
    return tag;
  }

  static CompoundTagPtr Lodestone(Pos3i const &pos, Block const &b, CompoundTagPtr const &c, Context &ctx, DataVersion const &dataVersion) {
    auto ret = New(u8"Lodestone");
    i32 tracker = ctx.fLodestones->get(ctx.fWorldData.fDim, pos);
    ret->set(u8"trackingHandle", Int(tracker));
    Attach(c, pos, *ret);
    return ret;
  }
#pragma endregion

#pragma region Converters: M
  static CompoundTagPtr MovingPiston(Pos3i const &pos, Block const &b, CompoundTagPtr const &c, Context &ctx, DataVersion const &dataVersion) {
    if (!c) {
      return nullptr;
    }
    auto id = c->string(u8"id");
    // id = "j2b:MovingBlock" block entity was created in Converter::PreprocessChunk
    if (id != u8"j2b:MovingBlock") {
      return nullptr;
    }
    std::u8string newId = id->substr(4);
    c->set(u8"id", newId);
    return c;
  }
#pragma endregion

#pragma region Converters: N
  static CompoundTagPtr Note(Pos3i const &pos, Block const &b, CompoundTagPtr const &c, Context &ctx, DataVersion const &dataVersion) {
    using namespace std;

    auto note = strings::ToI32(b.property(u8"note", u8"0"));
    if (!note) {
      return nullptr;
    }

    auto tag = Compound();
    tag->insert({
        {u8"id", String(u8"Music")},
        {u8"isMovable", Bool(true)},
        {u8"note", Byte(*note)},
    });
    Attach(c, pos, *tag);
    return tag;
  }
#pragma endregion

#pragma region Converters: P
  static CompoundTagPtr PottedBamboo(Pos3i const &pos, Block const &b, CompoundTagPtr const &c, Context &ctx, DataVersion const &dataVersion) {
    using namespace std;

    auto tag = Compound();
    auto plantBlock = Compound();
    auto states = Compound();
    states->insert({
        {u8"age_bit", Byte(0)},
        {u8"bamboo_leaf_size", String(u8"no_leaves")},
        {u8"bamboo_stalk_thickness", String(u8"thin")},
    });
    plantBlock->insert({
        {u8"states", states},
        {u8"name", String(u8"minecraft:bamboo")},
        {u8"version", Int(kBlockDataVersion)},
    });
    tag->insert({
        {u8"id", String(u8"FlowerPot")},
        {u8"isMovable", Bool(true)},
        {u8"PlantBlock", plantBlock},
    });
    Attach(c, pos, *tag);
    return tag;
  }

  static Converter PottedPlant(std::u8string const &name, std::map<std::u8string, std::variant<std::u8string, bool, i32, i8>> const &properties) {
    return [=](Pos3i const &pos, Block const &b, CompoundTagPtr const &c, Context &ctx, DataVersion const &dataVersion) -> CompoundTagPtr {
      using namespace std;

      auto tag = Compound();
      auto plantBlock = Compound();
      auto states = Compound();
      for (auto const &p : properties) {
        if (holds_alternative<u8string>(p.second)) {
          states->set(p.first, get<u8string>(p.second));
        } else if (holds_alternative<bool>(p.second)) {
          states->set(p.first, Bool(get<bool>(p.second)));
        } else if (holds_alternative<i32>(p.second)) {
          states->set(p.first, Int(get<i32>(p.second)));
        } else if (holds_alternative<i8>(p.second)) {
          states->set(p.first, Byte(get<i8>(p.second)));
        } else {
          assert(false);
        }
      }
      plantBlock->insert({
          {u8"states", states},
          {u8"name", String(u8"minecraft:" + name)},
          {u8"version", Int(kBlockDataVersion)},
      });
      tag->insert({
          {u8"id", String(u8"FlowerPot")},
          {u8"isMovable", Bool(true)},
          {u8"PlantBlock", plantBlock},
      });
      Attach(c, pos, *tag);
      return tag;
    };
  }

  static CompoundTagPtr PottedSapling(Pos3i const &pos, Block const &b, CompoundTagPtr const &c, Context &ctx, DataVersion const &dataVersion) {
    using namespace std;

    auto tag = Compound();
    auto plantBlock = Compound();
    auto states = Compound();
    auto type = strings::RemovePrefixAndSuffix(u8"minecraft:potted_", b.fName, u8"_sapling");
    states->set(u8"age_bit", Byte(0));
    plantBlock->insert({
        {u8"states", states},
        {u8"name", String(Namespace::Add(type + u8"_sapling"))},
        {u8"version", Int(kBlockDataVersion)},
    });
    tag->insert({
        {u8"id", String(u8"FlowerPot")},
        {u8"isMovable", Bool(true)},
        {u8"PlantBlock", plantBlock},
    });
    Attach(c, pos, *tag);
    return tag;
  }
#pragma endregion

#pragma region Converters: S
  static CompoundTagPtr ShulkerBox(Pos3i const &pos, Block const &b, CompoundTagPtr const &c, Context &ctx, DataVersion const &dataVersion) {
    auto facing = BlockData::GetFacingDirectionAFromFacing(b);
    auto tag = Compound();
    tag->insert({
        {u8"id", String(u8"ShulkerBox")},
        {u8"facing", Byte((i8)facing)},
        {u8"Findable", Bool(false)},
        {u8"isMovable", Bool(true)},
    });
    auto items = GetItems(c, u8"Items", ctx, dataVersion, {.fConvertSlotTag = true});
    if (items) {
      tag->set(u8"Items", items);
    }
    Attach(c, pos, *tag);
    return tag;
  }

  static CompoundTagPtr Skull(Pos3i const &pos, Block const &b, CompoundTagPtr const &c, Context &ctx, DataVersion const &dataVersion) {
    using namespace std;
    using namespace mcfile::blocks;
    auto tag = Compound();
    auto rot = Wrap(strings::ToI32(b.property(u8"rotation", u8"0")), 0);
    float rotation = Rotation::ClampDegreesBetweenMinus180And180(rot / 16.0f * 360.0f);
    tag->insert({
        {u8"id", String(u8"Skull")},
        {u8"isMovable", Bool(true)},
        {u8"DoingAnimation", Bool(false)},
        {u8"MouthTickCount", Int(0)},
        {u8"SkullType", Byte(-1)},
        {u8"Rotation", Float(rotation)},
    });
    Attach(c, pos, *tag);
    return tag;
  }

  static CompoundTagPtr StructureBlock(Pos3i const &pos, Block const &b, CompoundTagPtr const &c, Context &ctx, DataVersion const &dataVersion) {
    if (!c) {
      return nullptr;
    }
    auto t = Empty(u8"StructureBlock", *c, pos);

    t->set(u8"animationMode", Bool(false));
    t->set(u8"animationSeconds", Float(0));
    t->set(u8"dataField", u8"");
    t->set(u8"includePlayers", Bool(false));
    t->set(u8"redstoneSaveMode", Int(0));
    t->set(u8"removeBlocks", Bool(false));

    CopyBoolValues(*c, *t, {{u8"ignoreEntities"}, {u8"powered", u8"isPowered"}, {u8"showboundingbox", u8"showBoundingBox"}});
    CopyFloatValues(*c, *t, {{u8"integrity"}});
    CopyStringValues(*c, *t, {{u8"name", u8"structureName"}, {u8"metadata", u8"dataField"}});
    CopyIntValues(*c, *t, {
                              {u8"posX", u8"xStructureOffset"},
                              {u8"posY", u8"yStructureOffset"},
                              {u8"posZ", u8"zStructureOffset"},
                              {u8"sizeX", u8"xStructureSize"},
                              {u8"sizeY", u8"yStructureSize"},
                              {u8"sizeZ", u8"zStructureSize"},
                          });
    CopyLongValues(*c, *t, {{u8"seed"}});

    // "NONE", "LEFT_RIGHT" (displayed as "<- ->"), "FRONT_BACK" (displayed as "^v")
    auto mirror = c->string(u8"mirror", u8"NONE");
    i8 mirrorMode = 0; // NONE
    if (mirror == u8"LEFT_RIGHT") {
      mirrorMode = 1;
    } else if (mirror == u8"FRONT_BACK") {
      mirrorMode = 2;
    }
    t->set(u8"mirror", Byte(mirrorMode));

    // "LOAD", "SAVE", "CORNER"
    auto mode = c->string(u8"mode", u8"LOAD");
    int data;
    if (mode == u8"SAVE") {
      data = 1;
    } else if (mode == u8"CORNER") {
      data = 3;
    } else { // "LOAD"
      data = 2;
    }
    t->set(u8"data", Int(data));

    // "NONE" (displayed as "0"), "CLOCKWISE_90" (displayed as "90"), "CLOCKWISE_180" (displayed as "180"), "COUNTERCLOCKWISE_90" (displayed as "270")
    auto rotation = c->string(u8"rotation", u8"NONE");
    i8 rot = 0;
    if (rotation == u8"NONE") {
      rot = 0;
    } else if (rotation == u8"CLOCKWISE_90") {
      rot = 1;
    } else if (rotation == u8"CLOCKWISE_180") {
      rot = 2;
    } else if (rotation == u8"COUNTERCLOCKWISE_90") {
      rot = 3;
    }
    t->set(u8"rotation", Byte(rot));

    return t;
  }
#pragma endregion

#pragma region Converters: T
  static CompoundTagPtr TrialSpawner(Pos3i const &pos, Block const &b, CompoundTagPtr const &c, Context &ctx, DataVersion const &dataVersion) {
    auto ret = New(u8"TrialSpawner");
    if (c) {
      if (auto spawnDataJ = c->compoundTag(u8"spawn_data"); spawnDataJ) {
        auto spawnDataB = Compound();
        if (auto entityJ = spawnDataJ->compoundTag(u8"entity"); entityJ) {
          if (auto idJ = entityJ->string(u8"id"); idJ) {
            spawnDataB->set(u8"TypeId", *idJ);
            spawnDataB->set(u8"Weight", Int(1));
            ret->set(u8"spawn_data", spawnDataB);
          }
        }
      }
      for (auto const &key : {u8"registered_players", u8"current_mobs"}) {
        if (auto uuidListJ = c->listTag(key); uuidListJ) {
          auto uuidListB = ConvertUuidList(*uuidListJ, ctx);
          ret->set(key, uuidListB);
        }
      }
      bool hasNormalConfig = false;
      bool hasOminousConfig = false;
      if (auto config = c->compoundTag(u8"normal_config"); config) {
        ret->set(u8"normal_config", TrialSpawnerReplaceConfigLootTables(config));
        hasNormalConfig = true;
      }
      if (auto config = c->compoundTag(u8"ominous_config"); config) {
        ret->set(u8"ominous_config", TrialSpawnerReplaceConfigLootTables(config));
        hasOminousConfig = true;
      }
      if (hasNormalConfig || hasOminousConfig) {
        if (!hasNormalConfig) {
          ret->set(u8"normal_config", Compound());
        }
      } else {
        // trial spawner that was placed by player in creative mode
        ret->set(u8"spawn_range", Int(4));
        ret->set(u8"total_mobs", Float(6));
        ret->set(u8"total_mobs_added_per_player", Float(1));
        ret->set(u8"ticks_between_spawn", Int(20));
        ret->set(u8"simultaneous_mobs", Float(2));
        ret->set(u8"simultaneous_mobs_added_per_player", Float(1));
      }
      CopyLongValues(*c, *ret, {{u8"next_mob_spawns_at"}, {u8"cooldown_end_at"}});
      ret->set(u8"required_player_range", Int(14));
    }
    Attach(c, pos, *ret);
    return ret;
  }
#pragma endregion

#pragma region Converters: V
  static CompoundTagPtr Vault(Pos3i const &pos, Block const &b, CompoundTagPtr const &c, Context &ctx, DataVersion const &dataVersion) {
    auto ret = New(u8"Vault");
    if (c) {
      auto configB = Compound();
      configB->set(u8"activation_range", Float(4));
      configB->set(u8"deactivation_range", Float(4.5));
      configB->set(u8"loot_table", u8"loot_tables/chests/trial_chambers/reward.json");
      configB->set(u8"override_loot_table_to_display", u8"");
      if (auto configJ = c->compoundTag(u8"config"); configJ) {
        if (auto keyItemJ = configJ->compoundTag(u8"key_item"); keyItemJ) {
          if (auto keyItemB = Item::From(keyItemJ, ctx, dataVersion); keyItemB) {
            configB->set(u8"key_item", keyItemB);
          }
        }
      }

      auto dataB = Compound();
      if (auto serverDataJ = c->compoundTag(u8"server_data"); serverDataJ) {
        auto rewarededPlayersB = List<Tag::Type::Long>();
        if (auto rewarededPlayersJ = serverDataJ->listTag(u8"rewarded_players"); rewarededPlayersJ) {
          for (auto const &it : *rewarededPlayersJ) {
            auto v = it->asIntArray();
            if (!v) {
              continue;
            }
            if (auto idJ = Uuid::FromIntArray(*v); idJ) {
              auto idB = ctx.fUuids->toId(*idJ);
              rewarededPlayersB->push_back(Long(idB));
            }
          }
        }
        dataB->set(u8"rewarded_players", rewarededPlayersB);

        auto itemsToEjectB = List<Tag::Type::Compound>();
        if (auto itemsToEjectJ = serverDataJ->listTag(u8"items_to_eject"); itemsToEjectJ) {
          for (auto const &it : *itemsToEjectJ) {
            auto v = std::dynamic_pointer_cast<CompoundTag>(it);
            if (!v) {
              continue;
            }
            if (auto converted = Item::From(v, ctx, dataVersion); converted) {
              itemsToEjectB->push_back(converted);
            }
          }
        }
        dataB->set(u8"items_to_eject", itemsToEjectB);

        CopyLongValues(*serverDataJ, *dataB, {{u8"state_updating_resumes_at"}, {u8"total_ejections_needed"}});
      }
      if (auto sharedDataJ = c->compoundTag(u8"shared_data"); sharedDataJ) {
        if (auto displayItemJ = sharedDataJ->compoundTag(u8"display_item"); displayItemJ) {
          if (auto displayItemB = Item::From(displayItemJ, ctx, dataVersion); displayItemB) {
            dataB->set(u8"display_item", displayItemB);
          }
        }
      }
      ret->set(u8"config", configB);
      ret->set(u8"data", dataB);
    }
    Attach(c, pos, *ret);
    return ret;
  }
#pragma endregion

#pragma region Converter Generator
  static Converter AnyStorage(std::u8string const &name, std::optional<bool> findable) {
    return [=](Pos3i const &pos, Block const &b, CompoundTagPtr const &c, Context &ctx, DataVersion const &dataVersion) {
      using namespace std;

      auto tag = Compound();

      if (c) {
        if (LootTable::JavaToBedrock(*c, *tag) == LootTable::State::NoLootTable) {
          if (auto items = GetItems(c, u8"Items", ctx, dataVersion, {.fConvertSlotTag = true}); items) {
            tag->set(u8"Items", items);
          }
        }
      }

      tag->insert({
          {u8"id", String(name)},
          {u8"isMovable", Bool(true)},
      });
      if (findable) {
        tag->set(u8"Findable", Bool(*findable));
      }
      Attach(c, pos, *tag);
      return tag;
    };
  }

  static Converter Furnace(std::u8string id) {
    return [id](Pos3i const &pos, mcfile::je::Block const &b, CompoundTagPtr const &comp, Context &ctx, DataVersion const &dataVersion) {
      auto ret = AnyStorage(id, std::nullopt)(pos, b, comp, ctx, dataVersion);
      if (comp) {
        CopyShortValues(*comp, *ret, {{u8"lit_total_time", u8"BurnDuration"}, //
                                      {u8"BurnTime", u8"BurnDuration"},       //
                                      {u8"cooking_time_spent", u8"CookTime"}, //
                                      {u8"CookTime"},                         //
                                      {u8"lit_time_remaining", u8"BurnTime"}, //
                                      {u8"CookTimeTotal", u8"BurnTime"}});
      }
      return ret;
    };
  }

  static Converter NamedEmpty(std::u8string id) {
    return [id](Pos3i const &pos, Block const &b, CompoundTagPtr const &c, Context &ctx, DataVersion const &dataVersion) -> CompoundTagPtr {
      if (!c) {
        return nullptr;
      }
      return Empty(id, *c, pos);
    };
  }

  static Converter NamedEmptyFromNull(std::u8string id) {
    return [id](Pos3i const &pos, Block const &b, CompoundTagPtr const &c, Context &ctx, DataVersion const &dataVersion) -> CompoundTagPtr {
      return Empty(id, {}, pos);
    };
  }

  static Converter Sign(std::u8string id) {
    return [id](Pos3i const &pos, mcfile::je::Block const &b, CompoundTagPtr const &c, Context &ctx, DataVersion const &dataVersion) -> CompoundTagPtr {
      using namespace je2be::props;
      using namespace std;

      if (!c) {
        return nullptr;
      }

      auto tag = Compound();

      CompoundTagPtr frontTextJ, backTextJ;
      if (auto data = BlockEntityData(c); data) {
        frontTextJ = data->compoundTag(u8"front_text");
        backTextJ = data->compoundTag(u8"back_text");
      } else {
        frontTextJ = c->compoundTag(u8"front_text");
        backTextJ = c->compoundTag(u8"back_text");
      }
      if (frontTextJ || backTextJ) {
        if (frontTextJ) {
          auto frontTextB = SignText(*frontTextJ);
          tag->set(u8"FrontText", frontTextB);
        } else {
          tag->set(u8"FrontText", SignTextEmpty());
        }
        if (backTextJ) {
          auto backTextB = SignText(*backTextJ);
          tag->set(u8"BackText", backTextB);
        } else {
          tag->set(u8"BackText", SignTextEmpty());
        }
      } else {
        auto color = Wrap<u8string>(c->string(u8"Color"), u8"black");
        auto text1 = GetTextComponent(c->string(u8"Text1", u8""));
        auto text2 = GetTextComponent(c->string(u8"Text2", u8""));
        auto text3 = GetTextComponent(c->string(u8"Text3", u8""));
        auto text4 = GetTextComponent(c->string(u8"Text4", u8""));
        Rgba signTextColor = SignColor::BedrockTexteColorFromJavaColorCode(ColorCodeJavaFromJavaName(color));
        bool glowing = c->boolean(u8"GlowingText", false);
        u8string text = text1 + u8"\x0a" + text2 + u8"\x0a" + text3 + u8"\x0a" + text4;
        auto frontTextB = Compound();
        frontTextB->set(u8"Text", text);
        frontTextB->set(u8"TextOwner", u8"");
        frontTextB->set(u8"SignTextColor", Int(signTextColor.toARGB()));
        frontTextB->set(u8"IgnoreLighting", Bool(glowing));
        frontTextB->set(u8"HideGlowOutline", Bool(false));
        frontTextB->set(u8"PersistFormatting", Bool(false));
        tag->set(u8"FrontText", frontTextB);
        tag->set(u8"BackText", SignTextEmpty());
      }
      tag->insert({
          {u8"id", String(id)},
          {u8"isMovable", Bool(true)},
      });
      CopyBoolValues(*c, *tag, {{u8"is_waxed", u8"IsWaxed"}});
      Attach(c, pos, *tag);
      return tag;
    };
  }
#pragma endregion

#pragma region Utilities
  static void Attach(CompoundTagPtr const &c, Pos3i const &pos, CompoundTag &tag) {
    tag.set(u8"x", Int(pos.fX));
    tag.set(u8"y", Int(pos.fY));
    tag.set(u8"z", Int(pos.fZ));

    if (c) {
      auto customNameJ = c->string(u8"CustomName");
      if (customNameJ) {
        auto customNameB = props::GetTextComponent(*customNameJ);
        if (!customNameB.empty()) {
          tag.set(u8"CustomName", customNameB);
        }
      }
    }
  }

  static i32 BannerColor(std::u8string_view const &name) {
    std::u8string_view color = Namespace::Remove(name);
    auto suffix = color.rfind(u8"_wall_banner");
    if (suffix != std::u8string::npos) {
      color = color.substr(0, suffix);
    }
    suffix = color.rfind(u8"_banner");
    if (suffix != std::u8string::npos) {
      color = color.substr(0, suffix);
    }
    static std::unordered_map<std::u8string_view, i32> const mapping = {
        {u8"black", 0},
        {u8"red", 1},
        {u8"green", 2},
        {u8"brown", 3},
        {u8"blue", 4},
        {u8"purple", 5},
        {u8"cyan", 6},
        {u8"light_gray", 7},
        {u8"gray", 8},
        {u8"pink", 9},
        {u8"lime", 10},
        {u8"yellow", 11},
        {u8"light_blue", 12},
        {u8"magenta", 13},
        {u8"orange", 14},
        {u8"white", 15},
    };
    auto found = mapping.find(color);
    if (found == mapping.end()) {
      return 0;
    }
    return found->second;
  }

  static i8 BedColor(std::u8string_view const &name) {
    static std::unordered_map<std::u8string_view, i8> const mapping = {
        {u8"minecraft:white_bed", 0},
        {u8"minecraft:orange_bed", 1},
        {u8"minecraft:magenta_bed", 2},
        {u8"minecraft:light_blue_bed", 3},
        {u8"minecraft:yellow_bed", 4},
        {u8"minecraft:lime_bed", 5},
        {u8"minecraft:pink_bed", 6},
        {u8"minecraft:gray_bed", 7},
        {u8"minecraft:light_gray_bed", 8},
        {u8"minecraft:cyan_bed", 9},
        {u8"minecraft:purple_bed", 10},
        {u8"minecraft:blue_bed", 11},
        {u8"minecraft:brown_bed", 12},
        {u8"minecraft:green_bed", 13},
        {u8"minecraft:red_bed", 14},
        {u8"minecraft:black_bed", 15},
    };
    auto found = mapping.find(name);
    if (found == mapping.end()) {
      return 0;
    }
    return found->second;
  }

  static CompoundTagPtr BlockEntityData(CompoundTagPtr const &c) {
    if (!c) {
      return nullptr;
    }
    auto component = c->compoundTag(u8"components");
    if (!component) {
      return nullptr;
    }
    return component->compoundTag(u8"minecraft:block_entity_data");
  }

  static ListTagPtr ConvertUuidList(ListTag const &java, Context const &ctx) {
    auto bedrock = List<Tag::Type::Compound>();
    for (auto const &it : java) {
      if (auto uuidJ = it->asIntArray(); uuidJ) {
        if (auto uuid = Uuid::FromIntArray(*uuidJ); uuid) {
          auto uuidB = Compound();
          uuidB->set(u8"uuid", Long(ctx.fUuids->toId(*uuid)));
          bedrock->push_back(uuidB);
        }
      }
    }
    return bedrock;
  }

  static CompoundTagPtr Empty(std::u8string const &name, CompoundTag const &tagJ, Pos3i const &pos) {
    auto ret = Compound();
    ret->set(u8"id", name);
    ret->set(u8"x", Int(pos.fX));
    ret->set(u8"y", Int(pos.fY));
    ret->set(u8"z", Int(pos.fZ));
    ret->set(u8"isMovable", Bool(true));

    auto customName = props::GetJson(tagJ, u8"CustomName");
    if (customName) {
      auto text = GetAsString(*customName, "text");
      if (text) {
        ret->set(u8"CustomName", *text);
      }
    }
    return ret;
  }

  static std::optional<std::u8string> GetAsString(props::Json const &obj, std::string const &key) {
    auto found = obj.find(key);
    if (found == obj.end()) {
      return std::nullopt;
    }
    return props::GetJsonStringValue(*found);
  }

  struct GetItemsOption {
    bool fConvertSlotTag = true;
  };
  static ListTagPtr GetItems(CompoundTagPtr const &c, std::u8string const &name, Context &ctx, DataVersion const &dataVersion, GetItemsOption opt) {
    auto tag = List<Tag::Type::Compound>();
    auto list = GetList(c, name);
    if (list == nullptr) {
      return nullptr;
    }
    for (auto const &it : *list) {
      if (it->type() != Tag::Type::Compound) {
        continue;
      }
      auto inItem = std::dynamic_pointer_cast<CompoundTag>(it);
      auto outItem = Item::From(inItem, ctx, dataVersion);
      if (!outItem) {
        continue;
      }

      auto count = Item::Count(*inItem, 1);
      if (opt.fConvertSlotTag) {
        auto slot = inItem->byte(u8"Slot", 0);
        outItem->set(u8"Slot", Byte(slot));
      }
      outItem->set(u8"Count", Byte(count));

      tag->push_back(outItem);
    }
    return tag;
  }

  static ListTagPtr GetItemsRemovingSlot(CompoundTagPtr const &c, std::u8string const &name, Context &ctx, size_t capacity, DataVersion const &dataVersion) {
    auto itemsB = List<Tag::Type::Compound>();
    for (int i = 0; i < capacity; i++) {
      itemsB->push_back(Item::Empty());
    }
    auto items = GetItems(c, name, ctx, dataVersion, {.fConvertSlotTag = true});
    if (!items) {
      return itemsB;
    }
    for (auto &it : *items) {
      auto item = it->asCompound();
      if (!item) {
        continue;
      }
      auto slot = item->byte(u8"Slot");
      if (!slot) {
        continue;
      }
      if (*slot < 0 || 5 < *slot) {
        continue;
      }
      auto copy = item->copy();
      copy->erase(u8"Slot");
      itemsB->fValue[*slot] = copy;
    }
    return itemsB;
  }

  static ListTag const *GetList(CompoundTagPtr const &c, std::u8string const &name) {
    if (!c) {
      return nullptr;
    }
    auto found = c->find(name);
    if (found == c->end()) {
      return nullptr;
    }
    return found->second->asList();
  }

  static CompoundTagPtr New(std::u8string const &id) {
    auto tag = Compound();
    tag->set(u8"isMovable", Bool(true));
    tag->set(u8"id", id);
    return tag;
  }

  static CompoundTagPtr SignTextEmpty() {
    auto ret = Compound();
    ret->set(u8"HideGlowOutline", Bool(false));
    ret->set(u8"IgnoreLighting", Bool(false));
    ret->set(u8"PersistFormatting", Bool(true));
    ret->set(u8"SignTextColor", Int(-16777216));
    ret->set(u8"Text", u8"back");
    ret->set(u8"TextOwner", u8"");
    return ret;
  }

  static CompoundTagPtr SignText(CompoundTag const &input) {
    using namespace std;
    auto ret = Compound();
    u8string text;
    if (auto messages = input.listTag(u8"messages"); messages) {
      for (size_t i = 0; i < 4 && i < messages->size(); i++) {
        if (i > 0) {
          text += u8"\x0a";
        }
        auto line = messages->fValue[i];
        if (!line) {
          continue;
        }
        if (auto c = line->asCompound(); c) {
          text += c->string(u8"text", u8"");
        } else if (auto s = line->asString(); s) {
          text += props::GetTextComponent(strings::Unquote(s->fValue));
        }
      }
    }
    while (true) {
      auto t = strings::RemoveSuffix(strings::RemoveSuffix(text, u8"\x0d"), u8"\x0a");
      if (t == text) {
        break;
      }
      text = t;
    }
    ret->set(u8"Text", text);

    u8string color = input.string(u8"color", u8"black");
    Rgba signTextColor = SignColor::BedrockTexteColorFromJavaColorCode(ColorCodeJavaFromJavaName(color));
    ret->set(u8"SignTextColor", Int(signTextColor.toARGB()));

    bool glowing = input.boolean(u8"has_glowing_text", false);
    ret->set(u8"IgnoreLighting", Bool(glowing));

    ret->set(u8"TextOwner", u8"");
    ret->set(u8"HideGlowOutline", Bool(false));
    ret->set(u8"PersistFormatting", Bool(true));

    return ret;
  }

  static CompoundTagPtr Spawner(CompoundTagPtr const &c) {
    if (!c) {
      return nullptr;
    }
    auto x = c->int32(u8"x");
    auto y = c->int32(u8"y");
    auto z = c->int32(u8"z");
    if (!x || !y || !z) {
      return nullptr;
    }

    auto tag = Compound();

    std::u8string mob;
    auto spawnData = c->compoundTag(u8"SpawnData");
    if (spawnData) {
      auto entity = spawnData->compoundTag(u8"entity");
      if (entity) {
        auto id = entity->string(u8"id");
        if (id) {
          mob = *id;
        }
      } else {
        auto id = spawnData->string(u8"id");
        if (id) {
          mob = *id;
        }
      }
    }

    CopyShortValues(*c, *tag, {{u8"MaxNearbyEntities", u8"MaxNearbyEntities", 6}, {u8"MaxSpawnDelay", u8"MaxSpawnDelay", 800}, {u8"MinSpawnDelay", u8"MinSpawnDelay", 200}, {u8"RequiredPlayerRange", u8"RequiredPlayerRange", 16}, {u8"Delay", u8"Delay", 0}, {u8"SpawnCount", u8"SpawnCount", 4}, {u8"SpawnRange", u8"SpawnRange", 4}});

    tag->insert({
        {u8"x", Int(*x)},
        {u8"y", Int(*y)},
        {u8"z", Int(*z)},
        {u8"id", String(u8"MobSpawner")},
        {u8"isMovable", Bool(true)},
        {u8"DisplayEntityHeight", Float(1.8)},
        {u8"DisplayEntityScale", Float(1)},
        {u8"DisplayEntityWidth", Float(0.8)},
    });
    tag->set(u8"EntityIdentifier", mob);
    return tag;
  }

  static CompoundTagPtr TrialSpawnerReplaceConfigLootTables(CompoundTagPtr java) {
    auto bedrock = java->copy();
    if (auto dropJ = java->string(u8"items_to_drop_when_ominous"); dropJ) {
      if (auto dropB = LootTable::BedrockTableNameFromJava(*dropJ); dropB) {
        bedrock->set(u8"items_to_drop_when_ominous", *dropB);
      }
    }
    if (auto tablesJ = java->listTag(u8"loot_tables_to_eject"); tablesJ) {
      auto tablesB = List<Tag::Type::Compound>();
      for (auto const &it : *tablesJ) {
        if (auto tableJ = it->asCompound(); tableJ) {
          auto tableB = tableJ->copy();
          if (auto dataJ = tableJ->string(u8"data"); dataJ) {
            if (auto dataB = LootTable::BedrockTableNameFromJava(*dataJ); dataB) {
              tableB->set(u8"data", *dataB);
            }
          }
          tablesB->push_back(tableB);
        }
      }
      bedrock->set(u8"loot_tables_to_eject", tablesB);
    }
    return bedrock;
  }
#pragma endregion

  struct PistonArm {
    explicit PistonArm(bool sticky) : sticky(sticky) {}

    CompoundTagPtr operator()(Pos3i const &pos, Block const &b, CompoundTagPtr const &c, Context &ctx, DataVersion const &dataVersion) const {
      if (!c) {
        return nullptr;
      }
      auto id = c->string(u8"id");
      if (!id) {
        return nullptr;
      }
      if (*id == u8"j2b:PistonArm") {
        c->set(u8"id", u8"PistonArm");
        return c;
      } else {
        auto ret = Compound();
        ret->set(u8"id", u8"PistonArm");
        ret->set(u8"isMovable", Bool(false));
        ret->set(u8"LastProgress", Float(1));
        ret->set(u8"NewState", Byte(2));
        ret->set(u8"Progress", Float(1));
        ret->set(u8"State", Byte(2));
        ret->set(u8"Sticky", Bool(sticky));
        auto attachedBlocks = List<Tag::Type::Int>();
        auto breakBlocks = List<Tag::Type::Int>();
        ret->set(u8"AttachedBlocks", attachedBlocks);
        ret->set(u8"BreakBlocks", breakBlocks);
        Attach(c, pos, *ret);
        return ret;
      }
    }

    bool const sticky;
  };
};

bool TileEntity::IsTileEntity(mcfile::blocks::BlockId id) {
  return Impl::IsTileEntity(id);
}

CompoundTagPtr TileEntity::FromBlockAndTileEntity(Pos3i const &pos,
                                                  mcfile::je::Block const &block,
                                                  CompoundTagPtr const &tag,
                                                  Context &ctx,
                                                  DataVersion const &dataVersion) {
  return Impl::FromBlockAndTileEntity(pos, block, tag, ctx, dataVersion);
}

CompoundTagPtr TileEntity::FromBlock(Pos3i const &pos,
                                     mcfile::je::Block const &block,
                                     Context &ctx,
                                     DataVersion const &dataVersion) {
  return Impl::FromBlock(pos, block, ctx, dataVersion);
}

bool TileEntity::IsStandaloneTileEntity(CompoundTagPtr const &tag) {
  return Impl::IsStandaloneTileEntity(tag);
}

std::optional<std::tuple<CompoundTagPtr, std::u8string>> TileEntity::StandaloneTileEntityBlockdData(Pos3i pos, CompoundTagPtr const &tag) {
  return Impl::StandaloneTileEntityBlockdData(pos, tag);
}

CompoundTagPtr TileEntity::StandaloneTileEntityData(CompoundTagPtr const &tag) {
  return Impl::StandaloneTileEntityData(tag);
}

} // namespace je2be::java
