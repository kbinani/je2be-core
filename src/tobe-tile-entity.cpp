#include "tobe/_tile-entity.hpp"

#include "_namespace.hpp"
#include "_nbt-ext.hpp"
#include "_optional.hpp"
#include "_props.hpp"
#include "_xxhash.hpp"
#include "color/_sign-color.hpp"
#include "command/_command.hpp"
#include "enums/_banner-color-code-bedrock.hpp"
#include "enums/_color-code-java.hpp"
#include "enums/_facing4.hpp"
#include "tile-entity/_loot-table.hpp"
#include "tobe/_context.hpp"
#include "tobe/_entity.hpp"
#include "tobe/_item.hpp"
#include "tobe/_versions.hpp"
#include "tobe/_world-data.hpp"

#include <numbers>
#include <variant>

namespace je2be::tobe {

class TileEntity::Impl {
private:
  using Block = mcfile::je::Block;
  using Converter = std::function<CompoundTagPtr(Pos3i const &, Block const &, CompoundTagPtr const &, Context const &ctx)>;

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
                                               Context const &ctx) {
    using namespace std;
    using namespace mcfile;

    auto const &table = Table();
    if (auto found = table.find(block.fId); found != table.end()) {
      return found->second(pos, block, tag, ctx);
    } else {
      return nullptr;
    }
  }

  static CompoundTagPtr FromBlock(Pos3i const &pos,
                                  mcfile::je::Block const &block,
                                  Context const &ctx) {
    using namespace std;
    using namespace mcfile;

    auto const &table = Table();
    if (auto found = table.find(block.fId); found != table.end()) {
      return found->second(pos, block, nullptr, ctx);
    } else {
      return nullptr;
    }
  }

  static bool IsStandaloneTileEntity(CompoundTagPtr const &tag) {
    auto id = tag->string("id");
    if (!id) {
      return false;
    }
    auto const &name = *id;
    if (name == "minecraft:mob_spawner") {
      return true;
    }
    return false;
  }

  static std::optional<std::tuple<CompoundTagPtr, std::string>> StandaloneTileEntityBlockdData(Pos3i pos, CompoundTagPtr const &tag) {
    auto id = tag->string("id");
    if (!id) {
      return std::nullopt;
    }
    auto const &name = *id;
    if (name == "minecraft:mob_spawner") {
      auto b = Compound();
      b->insert({
          {"name", String("minecraft:mob_spawner")},
          {"version", Int(kBlockDataVersion)},
          {"states", Compound()},
      });
      return std::make_tuple(b, "minecraft:mob_spawner");
    }
    return std::nullopt;
  }

  static CompoundTagPtr StandaloneTileEntityData(CompoundTagPtr const &tag) {
    auto id = tag->string("id");
    if (!id) {
      return nullptr;
    }
    auto const &name = *id;
    if (name == "minecraft:mob_spawner") {
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
#define E(__name, __func)                                                 \
  assert(table->find(mcfile::blocks::minecraft::__name) == table->end()); \
  table->insert(make_pair(mcfile::blocks::minecraft::__name, __func))
    E(chest, Chest);
    E(trapped_chest, Chest);

    auto sign = Sign("Sign");
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
    E(potted_poppy, PottedPlant("red_flower", {{"flower_type", "poppy"}}));
    E(potted_blue_orchid, PottedPlant("red_flower", {{"flower_type", "orchid"}}));
    E(potted_allium, PottedPlant("red_flower", {{"flower_type", "allium"}}));
    E(potted_azure_bluet, PottedPlant("red_flower", {{"flower_type", "houstonia"}}));
    E(potted_red_tulip, PottedPlant("red_flower", {{"flower_type", "tulip_red"}}));
    E(potted_orange_tulip, PottedPlant("red_flower", {{"flower_type", "tulip_orange"}}));
    E(potted_white_tulip, PottedPlant("red_flower", {{"flower_type", "tulip_white"}}));
    E(potted_pink_tulip, PottedPlant("red_flower", {{"flower_type", "tulip_pink"}}));
    E(potted_oxeye_daisy, PottedPlant("red_flower", {{"flower_type", "oxeye"}}));
    E(potted_cornflower, PottedPlant("red_flower", {{"flower_type", "cornflower"}}));
    E(potted_lily_of_the_valley, PottedPlant("red_flower", {{"flower_type", "lily_of_the_valley"}}));
    E(potted_dandelion, PottedPlant("yellow_flower", {}));
    E(potted_wither_rose, PottedPlant("wither_rose", {}));
    E(potted_crimson_fungus, PottedPlant("crimson_fungus", {}));
    E(potted_warped_fungus, PottedPlant("warped_fungus", {}));
    E(potted_dead_bush, PottedPlant("deadbush", {}));
    E(potted_red_mushroom, PottedPlant("red_mushroom", {}));
    E(potted_brown_mushroom, PottedPlant("brown_mushroom", {}));
    E(potted_fern, PottedPlant("tallgrass", {{"tall_grass_type", "fern"}}));
    E(potted_bamboo, PottedBamboo);
    E(potted_crimson_roots, PottedPlant("crimson_roots", {}));
    E(potted_warped_roots, PottedPlant("warped_roots", {}));
    E(potted_cactus, PottedPlant("cactus", {}));
    E(potted_azalea_bush, PottedPlant("azalea", {}));
    E(potted_flowering_azalea_bush, PottedPlant("flowering_azalea", {}));
    E(potted_mangrove_propagule, PottedPlant("mangrove_propagule", {{"hanging", false}, {"propagule_stage", i32(0)}}));
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

    E(barrel, AnyStorage("Barrel"));
    E(furnace, Furnace("Furnace"));
    E(brewing_stand, BrewingStand);
    E(blast_furnace, Furnace("BlastFurnace"));
    E(smoker, Furnace("Smoker"));
    E(hopper, Hopper);
    E(dropper, AnyStorage("Dropper"));
    E(dispenser, AnyStorage("Dispenser"));

    E(note_block, Note);
    E(jukebox, Jukebox);
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
    E(ender_chest, AnyStorage("EnderChest"));
    E(enchanting_table, EnchantingTable);
    E(bell, Bell);
    E(conduit, Conduit);
    E(campfire, Campfire);
    E(soul_campfire, Campfire);
    E(comparator, Comparator);
    E(daylight_detector, NamedEmpty("DaylightDetector"));
    E(structure_block, StructureBlock);

    E(sculk_sensor, NamedEmpty("SculkSensor"));
    E(sculk_shrieker, NamedEmpty("SculkShrieker"));
    E(sculk_catalyst, NamedEmpty("SculkCatalyst"));

    auto hangingSign = Sign("HangingSign");
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
    E(suspicious_sand, SuspiciousSand);
    E(jigsaw, Jigsaw);
    E(cherry_hanging_sign, hangingSign);
    E(cherry_wall_hanging_sign, hangingSign);
    E(cherry_sign, sign);
    E(cherry_wall_sign, sign);
#undef E
    return table;
  }

  static CompoundTagPtr Jigsaw(Pos3i const &pos, Block const &b, CompoundTagPtr const &c, Context const &ctx) {
    auto tag = New("JigsawBlock");

    if (c) {
      CopyStringValues(*c, *tag, {{"final_state"}, {"joint"}, {"target"}, {"pool", "target_pool"}, {"name"}});
    }

    Attach(c, pos, *tag);
    return tag;
  }

  static CompoundTagPtr SuspiciousSand(Pos3i const &pos, Block const &b, CompoundTagPtr const &c, Context const &ctx) {
    auto tag = New("SuspiciousSand");

    if (c) {
      if (LootTable::JavaToBedrock(*c, *tag) == LootTable::State::NoLootTable) {
        if (auto itemJ = c->compoundTag("item"); itemJ) {
          if (auto itemB = Item::From(itemJ, ctx); itemB) {
            tag->set("item", itemB);
          }
        }
      }
    }

    auto dusted = Wrap(strings::ToI32(b.property("dusted", "0")), 0);
    tag->set("brush_count", Int(dusted));

    Attach(c, pos, *tag);
    return tag;
  }

  static CompoundTagPtr New(std::string const &id) {
    auto tag = Compound();
    tag->set("isMovable", Bool(true));
    tag->set("id", String(id));
    return tag;
  }

  static CompoundTagPtr DecoratedPot(Pos3i const &pos, Block const &b, CompoundTagPtr const &c, Context const &ctx) {
    auto tag = Compound();
    auto shardsB = List<Tag::Type::String>();
    for (int i = 0; i < 4; i++) {
      shardsB->push_back(String(""));
    }
    if (c) {
      if (auto shardsJ = c->listTag("shards"); shardsJ) {
        std::string prefix = "minecraft:pottery_shard_";
        for (int i = 0; i < 4 && i < shardsJ->size(); i++) {
          auto const &shard = shardsJ->at(i);
          if (auto shardName = shard->asString(); shardName && shardName->fValue.starts_with(prefix)) {
            auto name = shardName->fValue.substr(prefix.size());
            shardsB->fValue[i] = String("minecraft:" + name + "_pottery_shard");
          }
        }
      }
    }
    tag->insert({{"isMovable", Bool(true)},
                 {"id", String("DecoratedPot")},
                 {"shards", shardsB}});
    Attach(c, pos, *tag);
    return tag;
  }

  static CompoundTagPtr ChiseledBookshelf(Pos3i const &pos, Block const &b, CompoundTagPtr const &c, Context const &ctx) {
    auto tag = Compound();
    auto items = GetItemsRemovingSlot(c, "Items", ctx, 6);
    tag->insert({{"isMovable", Bool(true)},
                 {"id", String("ChiseledBookshelf")},
                 {"Items", items}});
    Attach(c, pos, *tag);
    return tag;
  }

  static CompoundTagPtr StructureBlock(Pos3i const &pos, Block const &b, CompoundTagPtr const &c, Context const &ctx) {
    if (!c) {
      return nullptr;
    }
    auto t = Empty("StructureBlock", *c, pos);

    t->set("animationMode", Bool(false));
    t->set("animationSeconds", Float(0));
    t->set("dataField", String(""));
    t->set("includePlayers", Bool(false));
    t->set("redstoneSaveMode", Int(0));
    t->set("removeBlocks", Bool(false));

    CopyBoolValues(*c, *t, {{"ignoreEntities"}, {"powered", "isPowered"}, {"showboundingbox", "showBoundingBox"}});
    CopyFloatValues(*c, *t, {{"integrity"}});
    CopyStringValues(*c, *t, {{"name", "structureName"}, {"metadata", "dataField"}});
    CopyIntValues(*c, *t, {
                              {"posX", "xStructureOffset"},
                              {"posY", "yStructureOffset"},
                              {"posZ", "zStructureOffset"},
                              {"sizeX", "xStructureSize"},
                              {"sizeY", "yStructureSize"},
                              {"sizeZ", "zStructureSize"},
                          });
    CopyLongValues(*c, *t, {{"seed"}});

    // "NONE", "LEFT_RIGHT" (displayed as "<- ->"), "FRONT_BACK" (displayed as "^v")
    auto mirror = c->string("mirror", "NONE");
    i8 mirrorMode = 0; // NONE
    if (mirror == "LEFT_RIGHT") {
      mirrorMode = 1;
    } else if (mirror == "FRONT_BACK") {
      mirrorMode = 2;
    }
    t->set("mirror", Byte(mirrorMode));

    // "LOAD", "SAVE", "CORNER"
    auto mode = c->string("mode", "LOAD");
    int data = 2;
    if (mode == "LOAD") {
      data = 2;
    } else if (mode == "SAVE") {
      data = 1;
    } else if (mode == "CORNER") {
      data = 3;
    }
    t->set("data", Int(data));

    // "NONE" (displayed as "0"), "CLOCKWISE_90" (displayed as "90"), "CLOCKWISE_180" (displayed as "180"), "COUNTERCLOCKWISE_90" (displayed as "270")
    auto rotation = c->string("rotation", "NONE");
    i8 rot = 0;
    if (rotation == "NONE") {
      rot = 0;
    } else if (rotation == "CLOCKWISE_90") {
      rot = 1;
    } else if (rotation == "CLOCKWISE_180") {
      rot = 2;
    } else if (rotation == "COUNTERCLOCKWISE_90") {
      rot = 3;
    }
    t->set("rotation", Byte(rot));

    return t;
  }

  static Converter NamedEmpty(std::string id) {
    return [id](Pos3i const &pos, Block const &b, CompoundTagPtr const &c, Context const &ctx) -> CompoundTagPtr {
      if (!c) {
        return nullptr;
      }
      return Empty(id, *c, pos);
    };
  }

  static CompoundTagPtr Hopper(Pos3i const &pos, Block const &b, CompoundTagPtr const &c, Context const &ctx) {
    if (!c) {
      return nullptr;
    }
    auto t = AnyStorage("Hopper")(pos, b, c, ctx);
    CopyIntValues(*c, *t, {{"TransferCooldown", "TransferCooldown", 0}});
    return t;
  }

  static CompoundTagPtr Comparator(Pos3i const &pos, Block const &b, CompoundTagPtr const &c, Context const &ctx) {
    if (!c) {
      return nullptr;
    }
    auto t = Empty("Comparator", *c, pos);
    CopyIntValues(*c, *t, {{"OutputSignal", "OutputSignal", 0}});
    return t;
  }

  static CompoundTagPtr Campfire(Pos3i const &pos, Block const &b, CompoundTagPtr const &c, Context const &ctx) {
    using namespace std;
    if (!c) {
      return nullptr;
    }
    auto t = Empty("Campfire", *c, pos);
    auto timesTag = c->intArrayTag("CookingTimes");
    if (timesTag) {
      auto const &times = timesTag->value();
      for (int i = 0; i < 4 && i < times.size(); i++) {
        int time = times[i];
        t->set("ItemTime" + to_string(i + 1), Int(time));
      }
    }
    auto items = GetItems(c, "Items", ctx);
    if (items) {
      for (int i = 0; i < 4 && i < items->size(); i++) {
        auto item = items->at(i);
        if (item) {
          t->set("Item" + to_string(i + 1), item);
        }
      }
    }
    return t;
  }

  static CompoundTagPtr Conduit(Pos3i const &pos, Block const &b, CompoundTagPtr const &c, Context const &ctx) {
    if (!c) {
      return nullptr;
    }
    auto t = Empty("Conduit", *c, pos);
    t->set("Active", Bool(false));
    t->set("Target", Long(-1));
    return t;
  }

  static CompoundTagPtr Bell(Pos3i const &pos, Block const &b, CompoundTagPtr const &c, Context const &ctx) {
    if (!c) {
      return nullptr;
    }
    auto t = Empty("Bell", *c, pos);
    t->set("Direction", Int(0));
    t->set("Ringing", Bool(false));
    t->set("Ticks", Int(0));
    return t;
  }

  static CompoundTagPtr EnchantingTable(Pos3i const &pos, Block const &b, CompoundTagPtr const &c, Context const &ctx) {
    if (!c) {
      return nullptr;
    }
    auto t = Empty("EnchantTable", *c, pos);

    // "rott"
    // [-pi, pi] float, an angle of enchant table and player's position who placed the table.
    // Java doesn't store such data in tile entity, so generate rott based on its xyz position.
    XXHash32 xx(0);
    xx.add(&pos.fX, sizeof(pos.fX));
    xx.add(&pos.fY, sizeof(pos.fY));
    xx.add(&pos.fZ, sizeof(pos.fZ));
    u32 seed = xx.hash();
    std::mt19937 gen(seed);
    std::uniform_real_distribution<float> distribution(-std::numbers::pi, std::numbers::pi);
    float rott = distribution(gen);
    t->set("rott", Float(rott));

    return t;
  }

  static CompoundTagPtr EndGateway(Pos3i const &pos, Block const &b, CompoundTagPtr const &c, Context const &ctx) {
    using namespace std;
    if (!c) {
      return nullptr;
    }
    auto tag = Compound();
    tag->insert({
        {"id", String("EndGateway")},
        {"isMovable", Bool(true)},
    });

    auto age = c->int64("Age", 0);
    tag->set("Age", Int(age));

    auto exitPortal = c->compoundTag("ExitPortal");
    if (exitPortal) {
      auto x = exitPortal->int32("X");
      auto y = exitPortal->int32("Y");
      auto z = exitPortal->int32("Z");
      if (x && y && z) {
        auto ep = List<Tag::Type::Int>();
        ep->push_back(Int(*x));
        ep->push_back(Int(*y));
        ep->push_back(Int(*z));
        tag->set("ExitPortal", ep);
      }
    }
    Attach(c, pos, *tag);
    return tag;
  }

  static CompoundTagPtr MovingPiston(Pos3i const &pos, Block const &b, CompoundTagPtr const &c, Context const &ctx) {
    if (!c) {
      return nullptr;
    }
    auto id = c->string("id");
    // id = "j2b:MovingBlock" block entity was created in Converter::PreprocessChunk
    if (id != "j2b:MovingBlock") {
      return nullptr;
    }
    std::string newId = id->substr(4);
    c->set("id", String(newId));
    return c;
  }

  static CompoundTagPtr CommandBlock(Pos3i const &pos, Block const &b, CompoundTagPtr const &c, Context const &ctx) {
    if (!c) {
      return nullptr;
    }

    auto tag = Compound();
    auto powered = c->boolean("powered", false);
    auto automatic = c->boolean("auto", false);
    auto trackOutput = c->boolean("TrackOutput", true);
    auto successCount = c->int32("SuccessCount", 0);
    auto command = command::Command::TranspileJavaToBedrock(c->string("Command", ""));
    auto conditionMet = c->boolean("conditionMet", false);

    auto lastExecution = c->int64("LastExecution");
    if (lastExecution) {
      tag->set("LastExecution", Long(*lastExecution));
    }

    tag->insert({
        {"id", String("CommandBlock")},
        {"isMovable", Bool(true)},
        {"powered", Bool(powered)},
        {"auto", Bool(automatic)},
        {"TrackDelay", Int(0)},
        {"TrackOutput", Bool(trackOutput)},
        {"Version", Int(17)},
        {"SuccessCount", Int(successCount)},
        {"Command", String(command)},
        {"ExecuteOnFirstTick", Bool(false)},
        {"conditionMet", Bool(conditionMet)},
    });
    Attach(c, pos, *tag);
    auto customName = props::GetJson(*c, "CustomName");
    if (customName) {
      auto text = GetAsString(*customName, "text");
      if (text == "@") {
        tag->erase("CustomName");
      }
    }
    return tag;
  }

  static CompoundTagPtr Beehive(Pos3i const &pos, Block const &, CompoundTagPtr const &c, Context const &ctx) {
    using namespace std;

    auto tag = Compound();
    tag->insert({
        {"id", String("Beehive")},
        {"isMovable", Bool(true)},
    });

    shared_ptr<ListTag> bees;
    if (c) {
      bees = c->listTag("Bees");
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
        auto entityData = bee->compoundTag("EntityData");
        if (!entityData) {
          continue;
        }

        // UUIDs are not stored in EntityData, so create a unique id.
        XXHash h;
        h.update(&ctx.fWorldData.fDim, sizeof(ctx.fWorldData.fDim));
        h.update(&pos.fX, sizeof(pos.fX));
        h.update(&pos.fZ, sizeof(pos.fZ));
        i64 hash = h.digest();
        u32 lo = ((u32 *)&hash)[0];
        u32 hi = ((u32 *)&hash)[1];
        vector<i32> uuidSource = {*(i32 *)&lo, *(i32 *)&hi, pos.fY, index};
        auto uuid = make_shared<IntArrayTag>(uuidSource);
        entityData->set("UUID", uuid);

        auto converted = Entity::From(*entityData, ctx);
        if (!converted.fEntity) {
          continue;
        }
        auto saveData = converted.fEntity;

        auto outBee = Compound();
        outBee->set("SaveData", saveData);

        auto minOccupationTicks = bee->int32("MinOccupationTicks");
        auto ticksInHive = bee->int32("TicksInHive");
        if (minOccupationTicks && ticksInHive) {
          i32 ticksLeftToStay = std::max<i32>(0, *minOccupationTicks - *ticksInHive);
          outBee->set("TicksLeftToStay", Int(ticksLeftToStay));
        }

        outBee->set("ActorIdentifier", String("minecraft:bee<>"));

        occupants->push_back(outBee);
      }
      tag->set("Occupants", occupants);
    }
    tag->set("ShouldSpawnBees", Bool(!bees));

    Attach(c, pos, *tag);
    return tag;
  }

  struct PistonArm {
    explicit PistonArm(bool sticky) : sticky(sticky) {}

    CompoundTagPtr operator()(Pos3i const &pos, Block const &b, CompoundTagPtr const &c, Context const &ctx) const {
      if (!c) {
        return nullptr;
      }
      auto id = c->string("id");
      if (!id) {
        return nullptr;
      }
      if (*id == "j2b:PistonArm") {
        c->set("id", String("PistonArm"));
        return c;
      } else {
        auto ret = Compound();
        ret->set("id", String("PistonArm"));
        ret->set("isMovable", Bool(false));
        ret->set("LastProgress", Float(1));
        ret->set("NewState", Byte(2));
        ret->set("Progress", Float(1));
        ret->set("State", Byte(2));
        ret->set("Sticky", Bool(sticky));
        auto attachedBlocks = List<Tag::Type::Int>();
        auto breakBlocks = List<Tag::Type::Int>();
        ret->set("AttachedBlocks", attachedBlocks);
        ret->set("BreakBlocks", breakBlocks);
        Attach(c, pos, *ret);
        return ret;
      }
    }

    bool const sticky;
  };

  static CompoundTagPtr Lectern(Pos3i const &pos, Block const &b, CompoundTagPtr const &c, Context const &ctx) {
    using namespace std;

    auto tag = Compound();
    tag->insert({
        {"id", String("Lectern")},
        {"isMovable", Bool(true)},
    });
    shared_ptr<CompoundTag> book;
    if (c) {
      book = c->compoundTag("Book");
    }
    i32 totalPages = 0;
    if (book) {
      auto item = Item::From(book, ctx);
      if (item) {
        tag->set("book", item);
        auto pages = item->query("tag/pages")->asList();
        if (pages) {
          totalPages = pages->size();
        }
      }
    }
    tag->set("hasBook", Bool(book != nullptr));
    if (c) {
      auto page = c->int32("Page");
      if (page) {
        tag->set("page", Int(*page));
      }
    }
    if (totalPages > 0) {
      tag->set("totalPages", Int(totalPages));
    }
    Attach(c, pos, *tag);
    return tag;
  }

  static CompoundTagPtr Beacon(Pos3i const &pos, Block const &b, CompoundTagPtr const &c, Context const &ctx) {
    int primary = -1;
    int secondary = -1;
    if (c) {
      primary = c->int32("Primary", -1);
      secondary = c->int32("Secondary", -1);
    }

    auto tag = Compound();
    tag->insert({
        {"id", String("Beacon")},
        {"isMovable", Bool(true)},
        {"primary", Int(primary)},
        {"secondary", Int(secondary)},
    });
    Attach(c, pos, *tag);
    return tag;
  }

  static CompoundTagPtr EndPortal(Pos3i const &pos, Block const &b, CompoundTagPtr const &c, Context const &ctx) {
    auto tag = Compound();
    tag->insert({
        {"id", String("EndPortal")},
        {"isMovable", Bool(true)},
    });
    Attach(c, pos, *tag);
    return tag;
  }

  static CompoundTagPtr Cauldron(Pos3i const &pos, Block const &b, CompoundTagPtr const &c, Context const &ctx) {
    using namespace std;

    auto tag = Compound();
    tag->insert({{"id", String("Cauldron")}, {"isMovable", Bool(true)}, {"PotionId", Short(-1)}, {"PotionType", Short(-1)}});

    Attach(c, pos, *tag);
    return tag;
  }

  static CompoundTagPtr Jukebox(Pos3i const &pos, Block const &b, CompoundTagPtr const &c, Context const &ctx) {
    using namespace std;

    auto tag = Compound();
    tag->insert({
        {"id", String("Jukebox")},
        {"isMovable", Bool(true)},
    });

    shared_ptr<CompoundTag> recordItem;
    if (c) {
      recordItem = c->compoundTag("RecordItem");
    }
    if (recordItem) {
      auto beRecordItem = Item::From(recordItem, ctx);
      if (beRecordItem) {
        tag->set("RecordItem", beRecordItem);
      }
    }

    Attach(c, pos, *tag);
    return tag;
  }

  static CompoundTagPtr Note(Pos3i const &pos, Block const &b, CompoundTagPtr const &c, Context const &ctx) {
    using namespace std;

    auto note = strings::ToI32(b.property("note", "0"));
    if (!note) {
      return nullptr;
    }

    auto tag = Compound();
    tag->insert({
        {"id", String("Music")},
        {"isMovable", Bool(true)},
        {"note", Byte(*note)},
    });
    Attach(c, pos, *tag);
    return tag;
  }

  static CompoundTagPtr Spawner(CompoundTagPtr const &c) {
    if (!c) {
      return nullptr;
    }
    auto x = c->int32("x");
    auto y = c->int32("y");
    auto z = c->int32("z");
    if (!x || !y || !z) {
      return nullptr;
    }

    auto tag = Compound();

    std::string mob;
    auto spawnData = c->compoundTag("SpawnData");
    if (spawnData) {
      auto entity = spawnData->compoundTag("entity");
      if (entity) {
        auto id = entity->string("id");
        if (id) {
          mob = *id;
        }
      } else {
        auto id = spawnData->string("id");
        if (id) {
          mob = *id;
        }
      }
    }

    CopyShortValues(*c, *tag, {{"MaxNearbyEntities", "MaxNearbyEntities", 6}, {"MaxSpawnDelay", "MaxSpawnDelay", 800}, {"MinSpawnDelay", "MinSpawnDelay", 200}, {"RequiredPlayerRange", "RequiredPlayerRange", 16}, {"Delay", "Delay", 0}, {"SpawnCount", "SpawnCount", 4}, {"SpawnRange", "SpawnRange", 4}});

    tag->insert({
        {"x", Int(*x)},
        {"y", Int(*y)},
        {"z", Int(*z)},
        {"id", String("MobSpawner")},
        {"isMovable", Bool(true)},
    });
    if (!mob.empty()) {
      tag->set("EntityIdentifier", String(mob));
    }
    return tag;
  }

  static CompoundTagPtr BrewingStand(Pos3i const &pos, Block const &b, CompoundTagPtr const &c, Context const &ctx) {
    using namespace std;

    if (!c) {
      return nullptr;
    }

    auto tag = Compound();
    auto items = GetItems(c, "Items", ctx);
    if (!items) {
      return nullptr;
    }
    vector<shared_ptr<CompoundTag>> sorted(5);
    for (auto const &it : *items) {
      auto item = it->asCompound();
      if (!item) {
        continue;
      }
      auto slot = item->byte("Slot");
      if (!slot) {
        continue;
      }

      auto newItem = Compound();
      for (auto const &prop : *item) {
        if (prop.first == "Slot") {
          continue;
        }
        newItem->set(prop.first, prop.second);
      }
      if (*slot < 0 || 4 < *slot) {
        continue;
      }
      u8 const mapping[5] = {1, 2, 3, 0, 4};
      u8 newSlot = mapping[*slot];
      newItem->set("Slot", Byte(newSlot));
      sorted[newSlot] = newItem;
    }

    auto reordered = List<Tag::Type::Compound>();
    for (auto it : sorted) {
      if (!it) {
        continue;
      }
      reordered->push_back(it);
    }

    CopyShortValues(*c, *tag, {{"BrewTime", "CookTime", 0}});

    auto fuel = c->byte("Fuel", 0);
    tag->set("FuelAmount", Short(fuel));
    tag->set("FuelTotal", Short(20));

    tag->insert({
        {"Items", reordered},
        {"Findable", Bool(false)},
        {"id", String("BrewingStand")},
        {"isMovable", Bool(true)},
    });
    Attach(c, pos, *tag);
    return tag;
  }

  static Converter AnyStorage(std::string const &name) {
    return [=](Pos3i const &pos, Block const &b, CompoundTagPtr const &c, Context const &ctx) {
      using namespace std;

      auto tag = Compound();
      auto items = GetItems(c, "Items", ctx);
      if (items) {
        tag->set("Items", items);
      }

      tag->insert({
          {"Findable", Bool(false)},
          {"id", String(name)},
          {"isMovable", Bool(true)},
      });
      Attach(c, pos, *tag);
      return tag;
    };
  }

  static CompoundTagPtr Skull(Pos3i const &pos, Block const &b, CompoundTagPtr const &c, Context const &ctx) {
    using namespace std;
    using namespace mcfile::blocks;
    auto tag = Compound();
    auto const &name = b.fName;
    i8 type = Item::GetSkullTypeFromBlockName(name);
    auto rot = Wrap(strings::ToI32(b.property("rotation", "0")), 0);
    float rotation = Rotation::ClampDegreesBetweenMinus180And180(rot / 16.0f * 360.0f);
    tag->insert({
        {"id", String("Skull")},
        {"isMovable", Bool(true)},
        {"MouthMoving", Bool(false)},
        {"MouthTickCount", Int(0)},
        {"SkullType", Byte(type)},
        {"Rotation", Float(rotation)},
    });
    Attach(c, pos, *tag);
    return tag;
  }

  static CompoundTagPtr PottedBamboo(Pos3i const &pos, Block const &b, CompoundTagPtr const &c, Context const &ctx) {
    using namespace std;

    auto tag = Compound();
    auto plantBlock = Compound();
    auto states = Compound();
    states->insert({
        {"age_bit", Byte(0)},
        {"bamboo_leaf_size", String("no_leaves")},
        {"mamboo_stalk_thickness", String("thin")},
    });
    plantBlock->insert({
        {"states", states},
        {"name", String("minecraft:bamboo")},
        {"version", Int(kBlockDataVersion)},
    });
    tag->insert({
        {"id", String("FlowerPot")},
        {"isMovable", Bool(true)},
        {"PlantBlock", plantBlock},
    });
    Attach(c, pos, *tag);
    return tag;
  }

  static Converter PottedPlant(std::string const &name, std::map<std::string, std::variant<std::string, bool, i32>> const &properties) {
    return [=](Pos3i const &pos, Block const &b, CompoundTagPtr const &c, Context const &ctx) -> CompoundTagPtr {
      using namespace std;

      auto tag = Compound();
      auto plantBlock = Compound();
      auto states = Compound();
      for (auto const &p : properties) {
        if (p.second.index() == 0) {
          states->set(p.first, String(get<0>(p.second)));
        } else if (p.second.index() == 1) {
          states->set(p.first, Bool(get<1>(p.second)));
        } else if (p.second.index() == 2) {
          states->set(p.first, Int(get<2>(p.second)));
        } else {
          assert(false);
        }
      }
      plantBlock->insert({
          {"states", states},
          {"name", String("minecraft:" + name)},
          {"version", Int(kBlockDataVersion)},
      });
      tag->insert({
          {"id", String("FlowerPot")},
          {"isMovable", Bool(true)},
          {"PlantBlock", plantBlock},
      });
      Attach(c, pos, *tag);
      return tag;
    };
  }

  static CompoundTagPtr PottedSapling(Pos3i const &pos, Block const &b, CompoundTagPtr const &c, Context const &ctx) {
    using namespace std;

    auto tag = Compound();
    auto plantBlock = Compound();
    auto states = Compound();
    auto type = strings::Trim("minecraft:potted_", b.fName, "_sapling");
    states->insert({
        {"age_bit", Byte(0)},
        {"sapling_type", String(type)},
    });
    plantBlock->insert({
        {"states", states},
        {"name", String("minecraft:sapling")},
        {"version", Int(kBlockDataVersion)},
    });
    tag->insert({
        {"id", String("FlowerPot")},
        {"isMovable", Bool(true)},
        {"PlantBlock", plantBlock},
    });
    Attach(c, pos, *tag);
    return tag;
  }

  static CompoundTagPtr FlowerPot(Pos3i const &pos, Block const &b, CompoundTagPtr const &c, Context const &ctx) {
    using namespace std;
    if (!c) {
      return nullptr;
    }
    auto item = c->int32("Item"); // legacy: Java 1.7.10
    if (!item) {
      return nullptr;
    }
    auto data = c->int32("Data", 0);
    auto tag = Compound();

    auto states = Compound();
    auto plantBlock = Compound();
    plantBlock->set("version", Int(kBlockDataVersion));
    string name;

    switch (*item) {
    case 6: {
      name = "sapling";
      string type;
      switch (data) {
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
      states->insert({
          {"age_bit", Byte(0)},
          {"sapling_type", String(type)},
      });
      break;
    }
    case 31: {
      name = "tallgrass";
      string type;
      switch (data) {
      case 2:
        type = "fern";
        break;
      }
      states->set("tall_grass_type", String(type));
      break;
    }
    case 32:
      name = "deadbush";
      break;
    case 37:
      name = "yellow_flower";
      break;
    case 38: {
      name = "red_flower";
      string type;
      switch (data) {
      case 1:
        type = "orchid";
        break;
      case 2:
        type = "allium";
        break;
      case 3:
        type = "houstonia";
        break;
      case 4:
        type = "tulip_red";
        break;
      case 5:
        type = "tulip_orange";
        break;
      case 6:
        type = "tulip_white";
        break;
      case 7:
        type = "tulip_pink";
        break;
      case 8:
        type = "oxeye";
        break;
      case 0:
      default:
        type = "poppy";
        break;
      }
      states->set("flower_type", String(type));
      break;
    }
    case 39:
      name = "brown_mushroom";
      break;
    case 40:
      name = "red_mushroom";
      break;
    case 81:
      name = "cactus";
      break;
    default:
      return nullptr;
    }
    plantBlock->set("states", states);
    plantBlock->set("name", String("minecraft:" + name));

    tag->insert({
        {"id", String("FlowerPot")},
        {"isMovable", Bool(true)},
        {"PlantBlock", plantBlock},
    });
    Attach(c, pos, *tag);
    return tag;
  }

  static CompoundTagPtr Banner(Pos3i const &pos, Block const &b, CompoundTagPtr const &c, Context const &ctx) {
    using namespace std;
    auto tag = Compound();

    optional<nlohmann::json> customName;
    if (c) {
      customName = props::GetJson(*c, "CustomName");
    }
    i32 type = 0;
    if (customName) {
      auto color = GetAsString(*customName, "color");
      auto translate = GetAsString(*customName, "translate");
      if (color == "gold" && translate == "block.minecraft.ominous_banner") {
        type = 1; // Illager Banner
      }
    }

    auto patterns = GetList(c, "Patterns");
    auto patternsBedrock = List<Tag::Type::Compound>();
    if (patterns && type != 1) {
      for (auto const &pattern : *patterns) {
        auto p = pattern->asCompound();
        if (!p) {
          continue;
        }
        auto color = p->int32("Color");
        auto pat = p->string("Pattern");
        if (!color || !pat) {
          continue;
        }
        auto ptag = Compound();
        ptag->insert({
            {"Color", Int(static_cast<i32>(BannerColorCodeFromJava(static_cast<ColorCodeJava>(*color))))},
            {"Pattern", String(*pat)},
        });
        patternsBedrock->push_back(ptag);
      }
    }

    i32 base = BannerColor(b.fName);

    tag->insert({
        {"id", String("Banner")},
        {"isMovable", Bool(true)},
        {"Type", Int(type)},
        {"Base", Int(base)},
        {"Patterns", patternsBedrock},
    });
    Attach(c, pos, *tag);
    return tag;
  }

  static i32 BannerColor(std::string_view const &name) {
    std::string_view color = Namespace::Remove(name);
    auto suffix = color.rfind("_wall_banner");
    if (suffix != std::string::npos) {
      color = color.substr(0, suffix);
    }
    suffix = color.rfind("_banner");
    if (suffix != std::string::npos) {
      color = color.substr(0, suffix);
    }
    static std::unordered_map<std::string_view, i32> const mapping = {
        {"black", 0},
        {"red", 1},
        {"green", 2},
        {"brown", 3},
        {"blue", 4},
        {"purple", 5},
        {"cyan", 6},
        {"light_gray", 7},
        {"gray", 8},
        {"pink", 9},
        {"lime", 10},
        {"yellow", 11},
        {"light_blue", 12},
        {"magenta", 13},
        {"orange", 14},
        {"white", 15},
    };
    auto found = mapping.find(color);
    if (found == mapping.end()) {
      return 0;
    }
    return found->second;
  }

  static i8 BedColor(std::string_view const &name) {
    static std::unordered_map<std::string_view, i8> const mapping = {
        {"minecraft:white_bed", 0},
        {"minecraft:orange_bed", 1},
        {"minecraft:magenta_bed", 2},
        {"minecraft:light_blue_bed", 3},
        {"minecraft:yellow_bed", 4},
        {"minecraft:lime_bed", 5},
        {"minecraft:pink_bed", 6},
        {"minecraft:gray_bed", 7},
        {"minecraft:light_gray_bed", 8},
        {"minecraft:cyan_bed", 9},
        {"minecraft:purple_bed", 10},
        {"minecraft:blue_bed", 11},
        {"minecraft:brown_bed", 12},
        {"minecraft:green_bed", 13},
        {"minecraft:red_bed", 14},
        {"minecraft:black_bed", 15},
    };
    auto found = mapping.find(name);
    if (found == mapping.end()) {
      return 0;
    }
    return found->second;
  }

  static CompoundTagPtr Bed(Pos3i const &pos, Block const &b, CompoundTagPtr const &c, Context const &ctx) {
    auto tag = Compound();
    auto color = BedColor(b.fName);
    tag->insert({
        {"id", String("Bed")},
        {"color", Byte(color)},
        {"isMovable", Bool(true)},
    });
    Attach(c, pos, *tag);
    return tag;
  }

  static CompoundTagPtr ShulkerBox(Pos3i const &pos, Block const &b, CompoundTagPtr const &c, Context const &ctx) {
    auto facing = BlockData::GetFacingDirectionAFromFacing(b);
    auto tag = Compound();
    tag->insert({
        {"id", String("ShulkerBox")},
        {"facing", Byte((i8)facing)},
        {"Findable", Bool(false)},
        {"isMovable", Bool(true)},
    });
    auto items = GetItems(c, "Items", ctx);
    if (items) {
      tag->set("Items", items);
    }
    Attach(c, pos, *tag);
    return tag;
  }

  static std::optional<std::string> GetAsString(nlohmann::json const &obj, std::string const &key) {
    auto found = obj.find(key);
    if (found == obj.end()) {
      return std::nullopt;
    }
    return found->get<std::string>();
  }

  static std::optional<std::string> GetSignLine(CompoundTag const &in, std::string const &key) {
    auto json = props::GetJson(in, key);
    if (json) {
      std::string ret;
      auto text = json->find("text");
      if (text != json->end() && text->is_string()) {
        ret += text->get<std::string>();
      }
      auto extra = json->find("extra");
      if (extra != json->end() && extra->is_array()) {
        for (auto it = extra->begin(); it != extra->end(); it++) {
          auto t = it->find("text");
          if (t != it->end() && t->is_string()) {
            ret += t->get<std::string>();
          }
        }
      }
      return ret;
    } else {
      return in.string(key);
    }
  }

  static void Attach(CompoundTagPtr const &c, Pos3i const &pos, CompoundTag &tag) {
    tag.set("x", Int(pos.fX));
    tag.set("y", Int(pos.fY));
    tag.set("z", Int(pos.fZ));

    if (c) {
      auto customName = props::GetJson(*c, "CustomName");
      if (customName) {
        auto text = GetAsString(*customName, "text");
        if (text) {
          tag.set("CustomName", String(*text));
        }
      }
    }
  }

  static CompoundTagPtr Empty(std::string const &name, CompoundTag const &tagJ, Pos3i const &pos) {
    auto ret = Compound();
    ret->set("id", String(name));
    ret->set("x", Int(pos.fX));
    ret->set("y", Int(pos.fY));
    ret->set("z", Int(pos.fZ));

    auto customName = props::GetJson(tagJ, "CustomName");
    if (customName) {
      auto text = GetAsString(*customName, "text");
      if (text) {
        ret->set("CustomName", String(*text));
      }
    }
    return ret;
  }

  static ListTagPtr GetItemsRemovingSlot(CompoundTagPtr const &c, std::string const &name, Context const &ctx, size_t capacity) {
    auto itemsB = List<Tag::Type::Compound>();
    for (int i = 0; i < capacity; i++) {
      itemsB->push_back(Item::Empty());
    }
    auto items = GetItems(c, name, ctx);
    if (!items) {
      return itemsB;
    }
    for (auto &it : *items) {
      auto item = it->asCompound();
      if (!item) {
        continue;
      }
      auto slot = item->byte("Slot");
      if (!slot) {
        continue;
      }
      if (*slot < 0 || 5 < *slot) {
        continue;
      }
      auto copy = item->copy();
      copy->erase("Slot");
      itemsB->fValue[*slot] = copy;
    }
    return itemsB;
  }

  static ListTagPtr GetItems(CompoundTagPtr const &c, std::string const &name, Context const &ctx) {
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
      auto outItem = Item::From(inItem, ctx);
      if (!outItem) {
        continue;
      }

      auto count = inItem->byte("Count", 1);
      auto slot = inItem->byte("Slot", 0);
      outItem->set("Slot", Byte(slot));
      outItem->set("Count", Byte(count));

      tag->push_back(outItem);
    }
    return tag;
  }

  static ListTag const *GetList(CompoundTagPtr const &c, std::string const &name) {
    if (!c) {
      return nullptr;
    }
    auto found = c->find(name);
    if (found == c->end()) {
      return nullptr;
    }
    return found->second->asList();
  }

  static CompoundTagPtr Chest(Pos3i const &pos, mcfile::je::Block const &b, CompoundTagPtr const &comp, Context const &ctx) {
    using namespace std;

    auto type = b.property("type", "single");
    auto facing = b.property("facing", "north");
    optional<Pos2i> pairPos;
    Facing4 f4 = Facing4FromJavaName(facing);
    Pos2i direction = Pos2iFromFacing4(f4);
    if (type == "left") {
      pairPos = Pos2i(pos.fX, pos.fZ) + Right90(direction);
    } else if (type == "right") {
      pairPos = Pos2i(pos.fX, pos.fZ) + Left90(direction);
    }

    auto tag = Compound();
    if (comp) {
      if (auto st = LootTable::JavaToBedrock(*comp, *tag); st == LootTable::State::NoLootTable) {
        auto items = GetItems(comp, "Items", ctx);
        if (items) {
          tag->set("Items", items);
        }
      }
    }

    tag->insert({
        {"Findable", Bool(false)},
        {"id", String(string("Chest"))},
        {"isMovable", Bool(true)},
    });
    if (pairPos) {
      tag->set("pairlead", Bool(type == "right"));
      tag->set("pairx", Int(pairPos->fX));
      tag->set("pairz", Int(pairPos->fZ));
    }
    Attach(comp, pos, *tag);
    return tag;
  }

  static Converter Furnace(std::string id) {
    return [id](Pos3i const &pos, mcfile::je::Block const &b, CompoundTagPtr const &comp, Context const &ctx) {
      auto ret = AnyStorage(id)(pos, b, comp, ctx);
      if (comp) {
        auto burnTime = comp->int16("BurnTime");
        if (burnTime) {
          ret->set("BurnDuration", Short(*burnTime));
        }
        auto cookTime = comp->int16("CookTime");
        if (cookTime) {
          ret->set("CookTime", Short(*cookTime));
        }
        auto cookTimeTotal = comp->int16("CookTimeTotal");
        if (cookTimeTotal) {
          ret->set("BurnTime", Short(*cookTimeTotal));
        }
      }
      return ret;
    };
  }

  static Converter Sign(std::string id) {
    return [id](Pos3i const &pos, mcfile::je::Block const &b, CompoundTagPtr const &c, Context const &ctx) -> CompoundTagPtr {
      using namespace je2be::props;
      using namespace std;

      if (!c) {
        return nullptr;
      }

      auto color = Wrap<string>(c->string("Color"), "black");
      auto text1 = GetSignLine(*c, "Text1");
      auto text2 = GetSignLine(*c, "Text2");
      auto text3 = GetSignLine(*c, "Text3");
      auto text4 = GetSignLine(*c, "Text4");
      if (!text1 || !text2 || !text3 || !text4) {
        return nullptr;
      }
      Rgba signTextColor = SignColor::BedrockTexteColorFromJavaColorCode(ColorCodeJavaFromJavaName(color));
      bool glowing = c->boolean("GlowingText", false);
      string text = *text1 + "\x0a" + *text2 + "\x0a" + *text3 + "\x0a" + *text4;
      auto tag = Compound();
      tag->insert({
          {"id", String(id)},
          {"isMovable", Bool(true)},
          {"Text", String(text)},
          {"TextOwner", String("")},
          {"SignTextColor", Int(signTextColor.toARGB())},
          {"IgnoreLighting", Bool(glowing)},
          {"TextIgnoreLegacyBugResolved", Bool(true)},
      });
      Attach(c, pos, *tag);
      return tag;
    };
  }
};

bool TileEntity::IsTileEntity(mcfile::blocks::BlockId id) {
  return Impl::IsTileEntity(id);
}

CompoundTagPtr TileEntity::FromBlockAndTileEntity(Pos3i const &pos,
                                                  mcfile::je::Block const &block,
                                                  CompoundTagPtr const &tag,
                                                  Context const &ctx) {
  return Impl::FromBlockAndTileEntity(pos, block, tag, ctx);
}

CompoundTagPtr TileEntity::FromBlock(Pos3i const &pos,
                                     mcfile::je::Block const &block,
                                     Context const &ctx) {
  return Impl::FromBlock(pos, block, ctx);
}

bool TileEntity::IsStandaloneTileEntity(CompoundTagPtr const &tag) {
  return Impl::IsStandaloneTileEntity(tag);
}

std::optional<std::tuple<CompoundTagPtr, std::string>> TileEntity::StandaloneTileEntityBlockdData(Pos3i pos, CompoundTagPtr const &tag) {
  return Impl::StandaloneTileEntityBlockdData(pos, tag);
}

CompoundTagPtr TileEntity::StandaloneTileEntityData(CompoundTagPtr const &tag) {
  return Impl::StandaloneTileEntityData(tag);
}

} // namespace je2be::tobe
