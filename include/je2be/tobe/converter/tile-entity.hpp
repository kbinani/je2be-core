#pragma once

namespace je2be::tobe {

class TileEntity {
public:
  using TileEntityData = std::shared_ptr<mcfile::nbt::CompoundTag>;

private:
  using CompoundTag = mcfile::nbt::CompoundTag;
  using Block = mcfile::je::Block;
  using Converter = std::function<TileEntityData(Pos3i const &, Block const &, std::shared_ptr<CompoundTag> const &, JavaEditionMap const &, WorldData &)>;

public:
  static bool IsTileEntity(mcfile::blocks::BlockId id) {
    auto const &table = Table();
    auto found = table.find(id);
    return found != table.end();
  }

  static std::shared_ptr<mcfile::nbt::CompoundTag> FromBlockAndTileEntity(Pos3i const &pos,
                                                                          mcfile::je::Block const &block,
                                                                          std::shared_ptr<mcfile::nbt::CompoundTag> const &tag,
                                                                          JavaEditionMap const &mapInfo,
                                                                          WorldData &wd) {
    using namespace std;
    using namespace mcfile;
    using namespace mcfile::nbt;

    auto const &table = Table();
    return table.at(block.fId)(pos, block, tag, mapInfo, wd);
  }

  static std::shared_ptr<mcfile::nbt::CompoundTag> FromBlock(Pos3i const &pos,
                                                             mcfile::je::Block const &block,
                                                             JavaEditionMap const &mapInfo,
                                                             WorldData &wd) {
    using namespace std;
    using namespace mcfile;
    using namespace mcfile::nbt;

    auto const &table = Table();
    return table.at(block.fId)(pos, block, nullptr, mapInfo, wd);
  }

  static bool IsStandaloneTileEntity(std::shared_ptr<CompoundTag> const &tag) {
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

  static std::optional<std::tuple<std::shared_ptr<mcfile::nbt::CompoundTag>, std::string>> StandaloneTileEntityBlockdData(Pos3i pos, std::shared_ptr<CompoundTag> const &tag) {
    auto id = tag->string("id");
    if (!id) {
      return std::nullopt;
    }
    auto const &name = *id;
    if (name == "minecraft:mob_spawner") {
      auto b = std::make_shared<CompoundTag>();
      b->insert({
          {"name", props::String("minecraft:mob_spawner")},
          {"version", props::Int(kBlockDataVersion)},
          {"states", std::make_shared<CompoundTag>()},
      });
      return std::make_tuple(b, "minecraft:mob_spawner");
    }
    return std::nullopt;
  }

  static std::shared_ptr<mcfile::nbt::CompoundTag> StandaloneTileEntityData(std::shared_ptr<CompoundTag> const &tag) {
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
  TileEntity() = delete;

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
    E(oak_sign, Sign);
    E(spruce_sign, Sign);
    E(birch_sign, Sign);
    E(jungle_sign, Sign);
    E(acacia_sign, Sign);
    E(dark_oak_sign, Sign);
    E(crimson_sign, Sign);
    E(warped_sign, Sign);
    E(oak_wall_sign, Sign);
    E(spruce_wall_sign, Sign);
    E(birch_wall_sign, Sign);
    E(jungle_wall_sign, Sign);
    E(acacia_wall_sign, Sign);
    E(dark_oak_wall_sign, Sign);
    E(crimson_wall_sign, Sign);
    E(warped_wall_sign, Sign);

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

    E(skeleton_skull, Skull);
    E(wither_skeleton_skull, Skull);
    E(player_head, Skull);
    E(zombie_head, Skull);
    E(creeper_head, Skull);
    E(dragon_head, Skull);
    E(skeleton_wall_skull, Skull);
    E(wither_skeleton_wall_skull, Skull);
    E(player_wall_head, Skull);
    E(zombie_wall_head, Skull);
    E(creeper_wall_head, Skull);
    E(dragon_wall_head, Skull);

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
#undef E
    return table;
  }

  static TileEntityData StructureBlock(Pos3i const &pos, Block const &b, std::shared_ptr<CompoundTag> const &c, JavaEditionMap const &mapInfo, WorldData &wd) {
    using namespace props;
    if (!c) {
      return nullptr;
    }
    auto t = Empty("StructureBlock", *c, pos);

    t->set("animationMode", props::Bool(false));
    t->set("animationSeconds", Float(0));
    t->set("dataField", String(""));
    t->set("includePlayers", Bool(false));
    t->set("redstoneSaveMode", Int(0));
    t->set("removeBlocks", Bool(false));

    CopyBoolValues(*c, *t, {{"ignoreEntities", "ignoreEntities"}, {"powered", "isPowered"}, {"showboundingbox", "showBoundingBox"}});
    CopyFloatValues(*c, *t, {{"integrity", "integrity"}});
    CopyStringValues(*c, *t, {{"name", "structureName"}, {"metadata", "dataField"}});
    CopyIntValues(*c, *t, {
                              {"posX", "xStructureOffset"},
                              {"posY", "yStructureOffset"},
                              {"posZ", "zStructureOffset"},
                              {"sizeX", "xStructureSize"},
                              {"sizeY", "yStructureSize"},
                              {"sizeZ", "zStructureSize"},
                          });
    CopyLongValues(*c, *t, {{"seed", "seed"}});

    // "NONE", "LEFT_RIGHT" (displayed as "<- ->"), "FRONT_BACK" (displayed as "ª«")
    auto mirror = c->string("mirror", "NONE");
    int8_t mirrorMode = 0; // NONE
    if (mirror == "LEFT_RIGHT") {
      mirrorMode = 1;
    } else if (mirror == "FRONT_BACK") {
      mirrorMode = 2;
    }
    t->set("mirror", props::Byte(mirrorMode));

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
    int8_t rot = 0;
    if (rotation == "NONE") {
      rot = 0;
    } else if (rotation == "CLOCKWISE_90") {
      rot = 1;
    } else if (rotation == "CLOCKWISE_180") {
      rot = 2;
    } else if (rotation == "COUNTERCLOCKWISE_90") {
      rot = 3;
    }
    t->set("rotation", props::Byte(rot));

    return t;
  }

  static void CopyBoolValues(mcfile::nbt::CompoundTag const &src, mcfile::nbt::CompoundTag &dest, std::initializer_list<std::pair<std::string, std::string>> keys) {
    for (auto const &it : keys) {
      auto value = src.byte(it.first);
      if (value) {
        dest.set(it.second, props::Byte(*value));
      }
    }
  }

  static void CopyFloatValues(mcfile::nbt::CompoundTag const &src, mcfile::nbt::CompoundTag &dest, std::initializer_list<std::pair<std::string, std::string>> keys) {
    for (auto const &it : keys) {
      auto value = src.float32(it.first);
      if (value) {
        dest.set(it.second, props::Float(*value));
      }
    }
  }

  static void CopyIntValues(mcfile::nbt::CompoundTag const &src, mcfile::nbt::CompoundTag &dest, std::initializer_list<std::pair<std::string, std::string>> keys) {
    for (auto const &it : keys) {
      auto value = src.int32(it.first);
      if (value) {
        dest.set(it.second, props::Int(*value));
      }
    }
  }

  static void CopyLongValues(mcfile::nbt::CompoundTag const &src, mcfile::nbt::CompoundTag &dest, std::initializer_list<std::pair<std::string, std::string>> keys) {
    for (auto const &it : keys) {
      auto value = src.int64(it.first);
      if (value) {
        dest.set(it.second, props::Long(*value));
      }
    }
  }

  static void CopyStringValues(mcfile::nbt::CompoundTag const &src, mcfile::nbt::CompoundTag &dest, std::initializer_list<std::pair<std::string, std::string>> keys) {
    for (auto const &it : keys) {
      auto value = src.string(it.first);
      if (value) {
        dest.set(it.second, props::String(*value));
      }
    }
  }

  static Converter NamedEmpty(std::string id) {
    return [id](Pos3i const &pos, Block const &b, std::shared_ptr<CompoundTag> const &c, JavaEditionMap const &mapInfo, WorldData &wd) -> TileEntityData {
      if (!c) {
        return nullptr;
      }
      return Empty(id, *c, pos);
    };
  }

  static TileEntityData Hopper(Pos3i const &pos, Block const &b, std::shared_ptr<CompoundTag> const &c, JavaEditionMap const &mapInfo, WorldData &wd) {
    if (!c) {
      return nullptr;
    }
    auto t = AnyStorage("Hopper")(pos, b, c, mapInfo, wd);
    auto transferCooldown = c->int32("TransferCooldown", 0);
    t->set("TransferCooldown", props::Int(transferCooldown));
    return t;
  }

  static TileEntityData Comparator(Pos3i const &pos, Block const &b, std::shared_ptr<CompoundTag> const &c, JavaEditionMap const &mapInfo, WorldData &wd) {
    if (!c) {
      return nullptr;
    }
    auto t = Empty("Comparator", *c, pos);
    auto outputSignal = c->int32("OutputSignal", 0);
    t->set("OutputSignal", props::Int(outputSignal));
    return t;
  }

  static TileEntityData Campfire(Pos3i const &pos, Block const &b, std::shared_ptr<CompoundTag> const &c, JavaEditionMap const &mapInfo, WorldData &wd) {
    using namespace std;
    using namespace props;
    using namespace mcfile::nbt;
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
    auto items = GetItems(c, "Items", mapInfo, wd);
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

  static TileEntityData Conduit(Pos3i const &pos, Block const &b, std::shared_ptr<CompoundTag> const &c, JavaEditionMap const &mapInfo, WorldData &wd) {
    using namespace props;
    if (!c) {
      return nullptr;
    }
    auto t = Empty("Conduit", *c, pos);
    t->set("Active", Bool(false));
    t->set("Target", Long(-1));
    return t;
  }

  static TileEntityData Bell(Pos3i const &pos, Block const &b, std::shared_ptr<CompoundTag> const &c, JavaEditionMap const &mapInfo, WorldData &wd) {
    using namespace props;
    if (!c) {
      return nullptr;
    }
    auto t = Empty("Bell", *c, pos);
    t->set("Direction", Int(0));
    t->set("Ringing", Bool(false));
    t->set("Ticks", Int(0));
    return t;
  }

  static TileEntityData EnchantingTable(Pos3i const &pos, Block const &b, std::shared_ptr<CompoundTag> const &c, JavaEditionMap const &mapInfo, WorldData &wd) {
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
    uint32_t seed = xx.hash();
    std::mt19937 gen(seed);
    std::uniform_real_distribution<float> distribution(-std::numbers::pi, std::numbers::pi);
    float rott = distribution(gen);
    t->set("rott", props::Float(rott));

    return t;
  }

  static TileEntityData EndGateway(Pos3i const &pos, Block const &b, std::shared_ptr<CompoundTag> const &c, JavaEditionMap const &mapInfo, WorldData &wd) {
    using namespace std;
    using namespace mcfile::nbt;
    using namespace props;
    if (!c) {
      return nullptr;
    }
    auto tag = make_shared<CompoundTag>();
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
        auto ep = make_shared<ListTag>(Tag::Type::Int);
        ep->push_back(Int(*x));
        ep->push_back(Int(*y));
        ep->push_back(Int(*z));
        tag->set("ExitPortal", ep);
      }
    }
    Attach(c, pos, *tag);
    return tag;
  }

  static TileEntityData MovingPiston(Pos3i const &pos, Block const &b, std::shared_ptr<CompoundTag> const &c, JavaEditionMap const &mapInfo, WorldData &wd) {
    if (!c) {
      return nullptr;
    }
    auto id = c->string("id");
    // id = "j2b:MovingBlock" block entity was created in Converter::PreprocessChunk
    if (id != "j2b:MovingBlock") {
      return nullptr;
    }
    std::string newId = id->substr(4);
    c->set("id", props::String(newId));
    return c;
  }

  static TileEntityData CommandBlock(Pos3i const &pos, Block const &b, std::shared_ptr<CompoundTag> const &c, JavaEditionMap const &mapInfo, WorldData &wd) {
    using namespace props;
    using namespace mcfile::nbt;

    if (!c) {
      return nullptr;
    }

    auto tag = std::make_shared<CompoundTag>();
    auto powered = c->boolean("powered", false);
    auto automatic = c->boolean("auto", false);
    auto trackOutput = c->boolean("TrackOutput", true);
    auto successCount = c->int32("SuccessCount", 0);
    auto command = Command::Transpile(c->string("Command", ""));
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
    auto customName = GetJson(*c, "CustomName");
    if (customName) {
      auto text = GetAsString(*customName, "text");
      if (text == "@") {
        tag->erase("CustomName");
      }
    }
    return tag;
  }

  static TileEntityData Beehive(Pos3i const &pos, Block const &b, std::shared_ptr<CompoundTag> const &c, JavaEditionMap const &mapInfo, WorldData &wd) {
    using namespace props;
    using namespace mcfile::nbt;
    using namespace std;

    auto tag = make_shared<CompoundTag>();
    tag->insert({
        {"id", String("Beehive")},
        {"isMovable", Bool(true)},
    });

    shared_ptr<ListTag> bees;
    if (c) {
      bees = c->listTag("Bees");
    }
    if (bees) {
      auto occupants = make_shared<ListTag>(Tag::Type::Compound);
      int32_t index = 0;
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
        h.update(&wd.fDim, sizeof(wd.fDim));
        h.update(&pos.fX, sizeof(pos.fX));
        h.update(&pos.fZ, sizeof(pos.fZ));
        int64_t hash = h.digest();
        uint32_t a = ((uint32_t *)&hash)[0];
        uint32_t b = ((uint32_t *)&hash)[1];
        vector<int32_t> uuidSource = {*(int32_t *)&a, *(int32_t *)&b, pos.fY, index};
        auto uuid = make_shared<IntArrayTag>(uuidSource);
        entityData->set("UUID", uuid);

        auto converted = Entity::From(*entityData, mapInfo, wd);
        if (converted.empty()) {
          continue;
        }
        auto saveData = converted[0];

        auto outBee = make_shared<CompoundTag>();
        outBee->set("SaveData", saveData);

        auto minOccupationTicks = bee->int32("MinOccupationTicks");
        auto ticksInHive = bee->int32("TicksInHive");
        if (minOccupationTicks && ticksInHive) {
          int32_t ticksLeftToStay = std::max<int32_t>(0, *minOccupationTicks - *ticksInHive);
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

    TileEntityData operator()(Pos3i const &pos, Block const &b, std::shared_ptr<CompoundTag> const &c, JavaEditionMap const &mapInfo, WorldData &wd) const {
      using namespace props;
      using namespace mcfile::nbt;

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
        auto ret = std::make_shared<CompoundTag>();
        ret->set("id", String("PistonArm"));
        ret->set("isMovable", Bool(false));
        ret->set("LastProgress", Float(1));
        ret->set("NewState", Byte(2));
        ret->set("Progress", Float(1));
        ret->set("State", Byte(2));
        ret->set("Sticky", Bool(sticky));
        auto attachedBlocks = std::make_shared<ListTag>(Tag::Type::Int);
        auto breakBlocks = std::make_shared<ListTag>(Tag::Type::Int);
        ret->set("AttachedBlocks", attachedBlocks);
        ret->set("BreakBlocks", breakBlocks);
        Attach(c, pos, *ret);
        return ret;
      }
    }

    bool const sticky;
  };

  static TileEntityData Lectern(Pos3i const &pos, Block const &b, std::shared_ptr<CompoundTag> const &c, JavaEditionMap const &mapInfo, WorldData &wd) {
    using namespace props;
    using namespace std;
    using namespace mcfile::nbt;

    auto tag = make_shared<CompoundTag>();
    tag->insert({
        {"id", String("Lectern")},
        {"isMovable", Bool(true)},
    });
    shared_ptr<CompoundTag> book;
    if (c) {
      book = c->compoundTag("Book");
    }
    int32_t totalPages = 0;
    if (book) {
      auto item = Item::From(book, mapInfo, wd);
      if (item) {
        tag->set("book", item);
        auto pages = item->query("tag/pages")->asList();
        if (pages) {
          totalPages = pages->size();
        }
      }
    }
    tag->set("hasBook", Bool(book != nullptr));
    auto page = c->int32("Page");
    if (page) {
      tag->set("page", Int(*page));
    }
    if (totalPages > 0) {
      tag->set("totalPages", Int(totalPages));
    }
    Attach(c, pos, *tag);
    return tag;
  }

  static TileEntityData Beacon(Pos3i const &pos, Block const &b, std::shared_ptr<CompoundTag> const &c, JavaEditionMap const &mapInfo, WorldData &wd) {
    using namespace props;

    int primary = -1;
    int secondary = -1;
    if (c) {
      primary = c->int32("Primary", -1);
      secondary = c->int32("Secondary", -1);
    }

    auto tag = std::make_shared<CompoundTag>();
    tag->insert({
        {"id", String("Beacon")},
        {"isMovable", Bool(true)},
        {"primary", Int(primary)},
        {"secondary", Int(secondary)},
    });
    Attach(c, pos, *tag);
    return tag;
  }

  static TileEntityData EndPortal(Pos3i const &pos, Block const &b, std::shared_ptr<CompoundTag> const &c, JavaEditionMap const &mapInfo, WorldData &wd) {
    using namespace props;

    auto tag = std::make_shared<CompoundTag>();
    tag->insert({
        {"id", String("EndPortal")},
        {"isMovable", Bool(true)},
    });
    Attach(c, pos, *tag);
    return tag;
  }

  static TileEntityData Cauldron(Pos3i const &pos, Block const &b, std::shared_ptr<CompoundTag> const &c, JavaEditionMap const &mapInfo, WorldData &wd) {
    using namespace props;
    using namespace mcfile::nbt;
    using namespace std;

    auto tag = std::make_shared<CompoundTag>();
    tag->insert({{"id", String("Cauldron")}, {"isMovable", Bool(true)}, {"PotionId", Short(-1)}, {"PotionType", Short(-1)}});

    Attach(c, pos, *tag);
    return tag;
  }

  static TileEntityData Jukebox(Pos3i const &pos, Block const &b, std::shared_ptr<CompoundTag> const &c, JavaEditionMap const &mapInfo, WorldData &wd) {
    using namespace props;
    using namespace mcfile::nbt;
    using namespace std;

    auto tag = std::make_shared<CompoundTag>();
    tag->insert({
        {"id", String("Jukebox")},
        {"isMovable", Bool(true)},
    });

    shared_ptr<CompoundTag> recordItem;
    if (c) {
      recordItem = c->compoundTag("RecordItem");
    }
    if (recordItem) {
      auto beRecordItem = Item::From(recordItem, mapInfo, wd);
      if (beRecordItem) {
        tag->set("RecordItem", beRecordItem);
      }
    }

    Attach(c, pos, *tag);
    return tag;
  }

  static TileEntityData Note(Pos3i const &pos, Block const &b, std::shared_ptr<CompoundTag> const &c, JavaEditionMap const &, WorldData &) {
    using namespace props;
    using namespace mcfile::nbt;
    using namespace std;

    auto note = strings::Toi(b.property("note", "0"));
    if (!note) {
      return nullptr;
    }

    auto tag = std::make_shared<CompoundTag>();
    tag->insert({
        {"id", String("Music")},
        {"isMovable", Bool(true)},
        {"note", Byte(*note)},
    });
    Attach(c, pos, *tag);
    return tag;
  }

  static TileEntityData Spawner(std::shared_ptr<CompoundTag> const &c) {
    using namespace props;

    auto x = c->int32("x");
    auto y = c->int32("y");
    auto z = c->int32("z");
    if (!x || !y || !z) {
      return nullptr;
    }

    auto tag = std::make_shared<CompoundTag>();

    auto delay = c->int16("Delay", 0);
    auto maxNearbyEntities = c->int16("MaxNearbyEntities", 6);
    auto maxSpawnDelay = c->int16("MaxSpawnDelay", 800);
    auto minSpawnDelay = c->int16("MinSpawnDelay", 200);
    auto requiredPlayerRange = c->int16("RequiredPlayerRange", 16);
    auto spawnCount = c->int16("SpawnCount", 4);
    auto spawnRange = c->int16("SpawnRange", 4);

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

    tag->insert({
        {"x", Int(*x)},
        {"y", Int(*y)},
        {"z", Int(*z)},
        {"Delay", Short(delay)},
        {"id", String("MobSpawner")},
        {"isMovable", Bool(true)},
        {"MaxNearbyEntities", Short(maxNearbyEntities)},
        {"MaxSpawnDelay", Short(maxSpawnDelay)},
        {"MinSpawnDelay", Short(minSpawnDelay)},
        {"RequiredPlayerRange", Short(requiredPlayerRange)},
        {"SpawnCount", Short(spawnCount)},
        {"SpawnRange", Short(spawnRange)},
    });
    if (!mob.empty()) {
      tag->set("EntityIdentifier", String(mob));
    }
    return tag;
  }

  static TileEntityData BrewingStand(Pos3i const &pos, Block const &b, std::shared_ptr<CompoundTag> const &c, JavaEditionMap const &mapInfo, WorldData &wd) {
    using namespace props;
    using namespace mcfile::nbt;
    using namespace std;

    if (!c) {
      return nullptr;
    }

    auto tag = std::make_shared<CompoundTag>();
    auto items = GetItems(c, "Items", mapInfo, wd);
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

      auto newItem = make_shared<CompoundTag>();
      for (auto const &prop : *item) {
        if (prop.first == "Slot") {
          continue;
        }
        newItem->set(prop.first, prop.second);
      }
      if (*slot < 0 || 4 < *slot) {
        continue;
      }
      uint8_t mapping[5] = {1, 2, 3, 0, 4};
      uint8_t newSlot = mapping[*slot];
      newItem->set("Slot", Byte(newSlot));
      sorted[newSlot] = newItem;
    }

    auto reordered = make_shared<ListTag>(Tag::Type::Compound);
    for (auto it : sorted) {
      if (!it) {
        continue;
      }
      reordered->push_back(it);
    }

    auto brewTime = c->int16("BrewTime", 0);
    tag->set("CookTime", Short(brewTime));

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
    return [=](Pos3i const &pos, Block const &b, std::shared_ptr<CompoundTag> const &c, JavaEditionMap const &mapInfo, WorldData &wd) {
      using namespace props;
      using namespace mcfile::nbt;
      using namespace std;

      auto tag = std::make_shared<CompoundTag>();
      auto items = GetItems(c, "Items", mapInfo, wd);

      tag->insert({
          {"Items", items},
          {"Findable", Bool(false)},
          {"id", String(name)},
          {"isMovable", Bool(true)},
      });
      Attach(c, pos, *tag);
      return tag;
    };
  }

  static TileEntityData Skull(Pos3i const &pos, Block const &b, std::shared_ptr<CompoundTag> const &c, JavaEditionMap const &, WorldData &) {
    using namespace props;
    using namespace mcfile::nbt;
    using namespace std;
    auto tag = make_shared<CompoundTag>();
    auto const &name = b.fName;
    int8_t type = Item::GetSkullTypeFromBlockName(name);
    auto rot = Wrap(strings::Toi(b.property("rotation", "0")), 0);
    float rotation = rot / 16.0f * 360.0f;
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

  static TileEntityData PottedBamboo(Pos3i const &pos, Block const &b, std::shared_ptr<CompoundTag> const &c, JavaEditionMap const &, WorldData &) {
    using namespace props;
    using namespace mcfile::nbt;
    using namespace std;

    auto tag = make_shared<CompoundTag>();
    auto plantBlock = make_shared<CompoundTag>();
    auto states = make_shared<CompoundTag>();
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

  static Converter PottedPlant(std::string const &name, std::map<std::string, std::string> const &properties) {
    return [=](Pos3i const &pos, Block const &b, std::shared_ptr<CompoundTag> const &c, JavaEditionMap const &, WorldData &) -> TileEntityData {
      using namespace props;
      using namespace mcfile::nbt;
      using namespace std;

      auto tag = make_shared<CompoundTag>();
      auto plantBlock = make_shared<CompoundTag>();
      auto states = make_shared<CompoundTag>();
      for (auto const &p : properties) {
        states->set(p.first, String(p.second));
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

  static TileEntityData PottedSapling(Pos3i const &pos, Block const &b, std::shared_ptr<CompoundTag> const &c, JavaEditionMap const &, WorldData &) {
    using namespace props;
    using namespace mcfile::nbt;
    using namespace std;

    auto tag = make_shared<CompoundTag>();
    auto plantBlock = make_shared<CompoundTag>();
    auto states = make_shared<CompoundTag>();
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

  static TileEntityData Banner(Pos3i const &pos, Block const &b, std::shared_ptr<CompoundTag> const &c, JavaEditionMap const &, WorldData &) {
    using namespace props;
    using namespace mcfile::nbt;
    using namespace std;
    auto tag = make_shared<CompoundTag>();

    optional<nlohmann::json> customName;
    if (c) {
      customName = GetJson(*c, "CustomName");
    }
    int32_t type = 0;
    if (customName) {
      auto color = GetAsString(*customName, "color");
      auto translate = GetAsString(*customName, "translate");
      if (color == "gold" && translate == "block.minecraft.ominous_banner") {
        type = 1; // Illager Banner
      }
    }

    auto patterns = GetList(c, "Patterns");
    auto patternsBedrock = make_shared<ListTag>(Tag::Type::Compound);
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
        auto ptag = make_shared<CompoundTag>();
        ptag->insert({
            {"Color", Int(static_cast<int32_t>(BannerColorCodeFromJava(static_cast<ColorCodeJava>(*color))))},
            {"Pattern", String(*pat)},
        });
        patternsBedrock->push_back(ptag);
      }
    }

    int32_t base = BannerColor(b.fName);

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

  static int32_t BannerColor(std::string const &name) {
    auto color = name.substr(10); // minecraft:
    auto suffix = color.rfind("_wall_banner");
    if (suffix != std::string::npos) {
      color = color.substr(0, suffix);
    }
    suffix = color.rfind("_banner");
    if (suffix != std::string::npos) {
      color = color.substr(0, suffix);
    }
    static std::unordered_map<std::string, int32_t> const mapping = {
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

  static int8_t BedColor(std::string const &name) {
    static std::unordered_map<std::string, int8_t> const mapping = {
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

  static TileEntityData Bed(Pos3i const &pos, Block const &b, std::shared_ptr<CompoundTag> const &c, JavaEditionMap const &, WorldData &) {
    using namespace props;
    auto tag = std::make_shared<CompoundTag>();
    auto color = BedColor(b.fName);
    tag->insert({
        {"id", String("Bed")},
        {"color", Byte(color)},
        {"isMovable", Bool(true)},
    });
    Attach(c, pos, *tag);
    return tag;
  }

  static TileEntityData ShulkerBox(Pos3i const &pos, Block const &b, std::shared_ptr<CompoundTag> const &c, JavaEditionMap const &mapInfo, WorldData &wd) {
    using namespace props;
    auto facing = BlockData::GetFacingDirectionAFromFacing(b);
    auto items = GetItems(c, "Items", mapInfo, wd);
    auto tag = std::make_shared<CompoundTag>();
    tag->insert({
        {"id", String("ShulkerBox")},
        {"facing", Byte((int8_t)facing)},
        {"Findable", Bool(false)},
        {"isMovable", Bool(true)},
        {"Items", items},
    });
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

  static std::string GetSignLine(nlohmann::json const &json) {
    std::string ret;
    auto text = json.find("text");
    if (text != json.end() && text->is_string()) {
      ret += text->get<std::string>();
    }
    auto extra = json.find("extra");
    if (extra != json.end() && extra->is_array()) {
      for (auto it = extra->begin(); it != extra->end(); it++) {
        auto t = it->find("text");
        if (t != it->end() && t->is_string()) {
          ret += t->get<std::string>();
        }
      }
    }
    return ret;
  }

  static TileEntityData Sign(Pos3i const &pos, mcfile::je::Block const &b, std::shared_ptr<CompoundTag> const &c, JavaEditionMap const &, WorldData &) {
    using namespace props;
    using namespace mcfile::nbt;
    using namespace std;

    if (!c) {
      return nullptr;
    }

    auto color = c->string("Color");
    auto text1 = GetJson(*c, "Text1");
    auto text2 = GetJson(*c, "Text2");
    auto text3 = GetJson(*c, "Text3");
    auto text4 = GetJson(*c, "Text4");
    if (!color || !text1 || !text2 || !text3 || !text4) {
      return nullptr;
    }
    Rgba signTextColor = SignColor::BedrockTexteColorFromJavaColorCode(ColorCodeJavaFromJavaName(*color));
    bool glowing = c->boolean("GlowingText", false);
    string text = GetSignLine(*text1) + "\x0a" + GetSignLine(*text2) + "\x0a" + GetSignLine(*text3) + "\x0a" + GetSignLine(*text4);
    auto tag = make_shared<CompoundTag>();
    tag->insert({
        {"id", String("Sign")},
        {"isMovable", Bool(true)},
        {"Text", String(text)},
        {"TextOwner", String("")},
        {"SignTextColor", Int(signTextColor.toARGB())},
        {"IgnoreLighting", Bool(glowing)},
        {"TextIgnoreLegacyBugResolved", Bool(true)},
    });
    Attach(c, pos, *tag);
    return tag;
  }

  [[deprecated]] static void Attach(std::shared_ptr<mcfile::nbt::CompoundTag> const &c, Pos3i const &pos, mcfile::nbt::CompoundTag &tag) {
    using namespace props;

    tag.set("x", Int(pos.fX));
    tag.set("y", Int(pos.fY));
    tag.set("z", Int(pos.fZ));

    if (c) {
      auto customName = GetJson(*c, "CustomName");
      if (customName) {
        auto text = GetAsString(*customName, "text");
        if (text) {
          tag.set("CustomName", String(*text));
        }
      }
    }
  }

  static std::shared_ptr<mcfile::nbt::CompoundTag> Empty(std::string const &name, mcfile::nbt::CompoundTag const &tagJ, Pos3i const &pos) {
    using namespace props;
    auto ret = std::make_shared<mcfile::nbt::CompoundTag>();
    ret->set("id", String(name));
    ret->set("x", Int(pos.fX));
    ret->set("y", Int(pos.fY));
    ret->set("z", Int(pos.fZ));

    auto customName = GetJson(tagJ, "CustomName");
    if (customName) {
      auto text = GetAsString(*customName, "text");
      if (text) {
        ret->set("CustomName", String(*text));
      }
    }
    return ret;
  }

  static std::shared_ptr<mcfile::nbt::ListTag> GetItems(std::shared_ptr<CompoundTag> const &c, std::string const &name, JavaEditionMap const &mapInfo, WorldData &wd) {
    auto tag = std::make_shared<mcfile::nbt::ListTag>(mcfile::nbt::Tag::Type::Compound);
    auto list = GetList(c, name);
    if (list == nullptr) {
      return tag;
    }
    for (auto const &it : *list) {
      if (it->type() != mcfile::nbt::Tag::Type::Compound) {
        continue;
      }
      auto inItem = std::dynamic_pointer_cast<CompoundTag>(it);
      auto outItem = Item::From(inItem, mapInfo, wd);
      if (!outItem) {
        continue;
      }

      auto count = inItem->byte("Count", 1);
      auto slot = inItem->byte("Slot", 0);
      outItem->set("Slot", props::Byte(slot));
      outItem->set("Count", props::Byte(count));

      tag->push_back(outItem);
    }
    return tag;
  }

  static mcfile::nbt::ListTag const *GetList(std::shared_ptr<CompoundTag> const &c, std::string const &name) {
    if (!c) {
      return nullptr;
    }
    auto found = c->find(name);
    if (found == c->end()) {
      return nullptr;
    }
    return found->second->asList();
  }

  static TileEntityData Chest(Pos3i const &pos, mcfile::je::Block const &b, std::shared_ptr<CompoundTag> const &comp, JavaEditionMap const &mapInfo, WorldData &wd) {
    using namespace props;
    using namespace mcfile::nbt;
    using namespace std;

    auto type = b.property("type", "single");
    auto facing = b.property("facing", "north");
    optional<pair<int, int>> pair;
    Facing4 f4 = Facing4FromJavaName(facing);
    Pos2i direction = Pos2iFromFacing4(f4);
    if (type == "left") {
      Pos2i pairPos = Pos2i(pos.fX, pos.fZ) + Right90(direction);
      pair = make_pair(pairPos.fX, pairPos.fZ);
    } else if (type == "right") {
      Pos2i pairPos = Pos2i(pos.fX, pos.fZ) + Left90(direction);
      pair = make_pair(pairPos.fX, pairPos.fZ);
    }

    auto tag = std::make_shared<CompoundTag>();
    auto items = GetItems(comp, "Items", mapInfo, wd);

    tag->insert({
        {"Items", items},
        {"Findable", Bool(false)},
        {"id", String(string("Chest"))},
        {"isMovable", Bool(true)},
    });
    if (pair) {
      tag->set("pairlead", Bool(true));
      tag->set("pairx", Int(pair->first));
      tag->set("pairz", Int(pair->second));
    }
    auto lootTable = comp->string("LootTable"); // "minecraft:chests/simple_dungeon"
    auto lootTableSeed = comp->int64("LootTableSeed");
    if (lootTable && lootTableSeed) {
      auto slash = lootTable->find('/');
      if (lootTable->starts_with("minecraft:") && slash != string::npos) {
        auto type = lootTable->substr(0, slash).substr(10);                        // "chests"
        string table = "loot_tables/" + type + lootTable->substr(slash) + ".json"; // "loot_tables/chests/simple_dungeon.json"
        tag->set("LootTable", String(table));
        tag->set("LootTableSeed", Int(SquashI64ToI32(*lootTableSeed)));
      }
    }
    Attach(comp, pos, *tag);
    return tag;
  }

  static Converter Furnace(std::string id) {
    return [id](Pos3i const &pos, mcfile::je::Block const &b, std::shared_ptr<CompoundTag> const &comp, JavaEditionMap const &mapInfo, WorldData &wd) {
      auto ret = AnyStorage(id)(pos, b, comp, mapInfo, wd);
      if (comp) {
        auto burnTime = comp->int16("BurnTime");
        if (burnTime) {
          ret->set("BurnDuration", props::Short(*burnTime));
        }
        auto cookTime = comp->int16("CookTime");
        if (cookTime) {
          ret->set("CookTime", props::Short(*cookTime));
        }
        auto cookTimeTotal = comp->int16("CookTimeTotal");
        if (cookTimeTotal) {
          ret->set("BurnTime", props::Short(*cookTimeTotal));
        }
      }
      return ret;
    };
  }
};

} // namespace je2be::tobe
