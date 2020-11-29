#pragma once

namespace j2b {

class TileEntity {
public:
  using TileEntityData = std::shared_ptr<mcfile::nbt::CompoundTag>;

private:
  using CompoundTag = mcfile::nbt::CompoundTag;
  using Block = mcfile::Block;
  using Converter = std::function<TileEntityData(
      Pos const &, Block const &, std::shared_ptr<CompoundTag> const &,
      JavaEditionMap const &, DimensionDataFragment &)>;

public:
  static bool IsTileEntity(std::string const &name) {
    auto const &table = Table();
    auto found = table.find(name);
    return found != table.end();
  }

  static std::shared_ptr<mcfile::nbt::CompoundTag>
  From(Pos const &pos, mcfile::Block const &block,
       std::shared_ptr<mcfile::nbt::CompoundTag> const &tag,
       JavaEditionMap const &mapInfo, DimensionDataFragment &ddf) {
    using namespace std;
    using namespace mcfile;
    using namespace mcfile::nbt;

    string const &name = block.fName;
    auto const &table = Table();
    return table.at(name)(pos, block, tag, mapInfo, ddf);
  }

  static bool IsStandaloneTileEntity(std::shared_ptr<CompoundTag> const &tag) {
    auto id = tag->string("id");
    if (!id)
      return false;
    auto const &name = *id;
    if (name == "minecraft:mob_spawner") {
      return true;
    }
    return false;
  }

  static std::optional<
      std::tuple<Pos, std::shared_ptr<mcfile::nbt::CompoundTag>, std::string>>
  StandaloneTileEntityBlockdData(std::shared_ptr<CompoundTag> const &tag) {
    auto id = tag->string("id");
    if (!id)
      return std::nullopt;
    auto const &name = *id;
    if (name == "minecraft:mob_spawner") {
      auto b = std::make_shared<CompoundTag>();
      b->insert({
          {"name", props::String("minecraft:mob_spawner")},
          {"version", props::Int(BlockData::kBlockDataVersion)},
          {"states", std::make_shared<CompoundTag>()},
      });
      auto x = tag->int32("x");
      auto y = tag->int32("y");
      auto z = tag->int32("z");
      if (!x || !y || !z)
        return std::nullopt;
      Pos p(*x, *y, *z);
      return std::make_tuple(p, b, "minecraft:mob_spawner");
    }
    return std::nullopt;
  }

  static std::shared_ptr<mcfile::nbt::CompoundTag>
  StandaloneTileEntityData(std::shared_ptr<CompoundTag> const &tag) {
    auto id = tag->string("id");
    if (!id)
      return nullptr;
    auto const &name = *id;
    if (name == "minecraft:mob_spawner") {
      return Spawner(tag);
    }
    return nullptr;
  }

private:
  TileEntity() = delete;

  static std::unordered_map<std::string, Converter> const &Table() {
    using namespace std;
    static unique_ptr<unordered_map<string, Converter> const> const table(
        CreateTable());
    return *table;
  }

  static std::unordered_map<std::string, Converter> *CreateTable() {
    using namespace std;
    auto table = new unordered_map<string, Converter>();
#define E(__name, __func) table->insert(make_pair("minecraft:" __name, __func))
    E("chest", Chest);
    E("trapped_chest", Chest);
    E("oak_sign", Sign);
    E("spruce_sign", Sign);
    E("birch_sign", Sign);
    E("jungle_sign", Sign);
    E("acacia_sign", Sign);
    E("dark_oak_sign", Sign);
    E("crimson_sign", Sign);
    E("warped_sign", Sign);
    E("oak_wall_sign", Sign);
    E("spruce_wall_sign", Sign);
    E("birch_wall_sign", Sign);
    E("jungle_wall_sign", Sign);
    E("acacia_wall_sign", Sign);
    E("dark_oak_wall_sign", Sign);
    E("crimson_wall_sign", Sign);
    E("warped_wall_sign", Sign);

    E("shulker_box", ShulkerBox);
    E("black_shulker_box", ShulkerBox);
    E("red_shulker_box", ShulkerBox);
    E("green_shulker_box", ShulkerBox);
    E("brown_shulker_box", ShulkerBox);
    E("blue_shulker_box", ShulkerBox);
    E("purple_shulker_box", ShulkerBox);
    E("cyan_shulker_box", ShulkerBox);
    E("light_gray_shulker_box", ShulkerBox);
    E("gray_shulker_box", ShulkerBox);
    E("pink_shulker_box", ShulkerBox);
    E("lime_shulker_box", ShulkerBox);
    E("yellow_shulker_box", ShulkerBox);
    E("light_blue_shulker_box", ShulkerBox);
    E("magenta_shulker_box", ShulkerBox);
    E("orange_shulker_box", ShulkerBox);
    E("white_shulker_box", ShulkerBox);

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

    E("white_wall_banner", Banner);
    E("orange_wall_banner", Banner);
    E("magenta_wall_banner", Banner);
    E("light_blue_wall_banner", Banner);
    E("yellow_wall_banner", Banner);
    E("lime_wall_banner", Banner);
    E("pink_wall_banner", Banner);
    E("gray_wall_banner", Banner);
    E("light_gray_wall_banner", Banner);
    E("cyan_wall_banner", Banner);
    E("purple_wall_banner", Banner);
    E("blue_wall_banner", Banner);
    E("brown_wall_banner", Banner);
    E("green_wall_banner", Banner);
    E("red_wall_banner", Banner);
    E("black_wall_banner", Banner);

    E("potted_oak_sapling", PottedSapling);
    E("potted_spruce_sapling", PottedSapling);
    E("potted_birch_sapling", PottedSapling);
    E("potted_jungle_sapling", PottedSapling);
    E("potted_acacia_sapling", PottedSapling);
    E("potted_dark_oak_sapling", PottedSapling);
    E("potted_poppy", PottedPlant("red_flower", {{"flower_type", "poppy"}}));
    E("potted_blue_orchid",
      PottedPlant("red_flower", {{"flower_type", "orchid"}}));
    E("potted_allium", PottedPlant("red_flower", {{"flower_type", "allium"}}));
    E("potted_azure_bluet",
      PottedPlant("red_flower", {{"flower_type", "houstonia"}}));
    E("potted_red_tulip",
      PottedPlant("red_flower", {{"flower_type", "tulip_red"}}));
    E("potted_orange_tulip",
      PottedPlant("red_flower", {{"flower_type", "tulip_orange"}}));
    E("potted_white_tulip",
      PottedPlant("red_flower", {{"flower_type", "tulip_white"}}));
    E("potted_pink_tulip",
      PottedPlant("red_flower", {{"flower_type", "tulip_pink"}}));
    E("potted_oxeye_daisy",
      PottedPlant("red_flower", {{"flower_type", "oxeye"}}));
    E("potted_cornflower",
      PottedPlant("red_flower", {{"flower_type", "cornflower"}}));
    E("potted_lily_of_the_valley",
      PottedPlant("red_flower", {{"flower_type", "lily_of_the_valley"}}));
    E("potted_dandelion", PottedPlant("yellow_flower", {}));
    E("potted_wither_rose", PottedPlant("wither_rose", {}));
    E("potted_crimson_fungus", PottedPlant("crimson_fungus", {}));
    E("potted_warped_fungus", PottedPlant("warped_fungus", {}));
    E("potted_dead_bush", PottedPlant("deadbush", {}));
    E("potted_red_mushroom", PottedPlant("red_mushroom", {}));
    E("potted_brown_mushroom", PottedPlant("brown_mushroom", {}));
    E("potted_fern", PottedPlant("tallgrass", {{"tall_grass_type", "fern"}}));
    E("potted_bamboo", PottedBamboo);
    E("potted_crimson_roots", PottedPlant("crimson_roots", {}));
    E("potted_warped_roots", PottedPlant("warped_roots", {}));
    E("potted_cactus", PottedPlant("cactus", {}));

    E("skeleton_skull", Skull);
    E("wither_skeleton_skull", Skull);
    E("player_head", Skull);
    E("zombie_head", Skull);
    E("creeper_head", Skull);
    E("dragon_head", Skull);
    E("skeleton_wall_skull", Skull);
    E("wither_skeleton_wall_skull", Skull);
    E("player_wall_head", Skull);
    E("zombie_wall_head", Skull);
    E("creeper_wall_head", Skull);
    E("dragon_wall_head", Skull);

    E("barrel", AnyStorage("Barrel"));
    E("furnace", AnyStorage("Furnace"));
    E("brewing_stand", BrewingStand);
    E("blast_furnace", AnyStorage("BlastFurnace"));
    E("smoker", AnyStorage("Smoker"));
    E("hopper", AnyStorage("Hopper"));
    E("dropper", AnyStorage("Dropper"));
    E("dispenser", AnyStorage("Dispenser"));

    E("note_block", Note);
    E("jukebox", Jukebox);
    E("cauldron", Cauldron);
    E("end_portal", EndPortal);
    E("beacon", Beacon);
    E("lectern", Lectern);
    E("piston_head", PistonArm(false));
    E("sticky_piston_head", PistonArm(true));
    E("bee_nest", Beehive);
    E("beehive", Beehive);
#undef E
    return table;
  }

  static TileEntityData Beehive(Pos const &pos, Block const &b,
                                std::shared_ptr<CompoundTag> const &c,
                                JavaEditionMap const &mapInfo,
                                DimensionDataFragment &ddf) {
    using namespace props;
    using namespace mcfile::nbt;

    auto tag = std::make_shared<CompoundTag>();
    tag->insert({
        {"id", String("Beehive")},
        {"isMovable", Bool(true)},
    });

    auto bees = c->listTag("Bees");
    if (bees) {
      auto occupants = std::make_shared<ListTag>();
      occupants->fType = Tag::TAG_Compound;
      int32_t index = 0;
      for (auto const &it : *bees) {
        index++;
        auto bee = std::dynamic_pointer_cast<CompoundTag>(it);
        if (!bee) {
          continue;
        }
        auto entityData = bee->compoundTag("EntityData");
        if (!entityData) {
          continue;
        }

        // UUIDs are not stored in EntityData, so create a unique id.
        XXHash h;
        h.update(&ddf.fDim, sizeof(ddf.fDim));
        h.update(&pos.fX, sizeof(pos.fX));
        h.update(&pos.fZ, sizeof(pos.fZ));
        int64_t hash = h.digest();
        uint32_t a = ((uint32_t *)&hash)[0];
        uint32_t b = ((uint32_t *)&hash)[1];
        std::vector<int32_t> uuidSource = {*(int32_t *)&a, *(int32_t *)&b,
                                           pos.fY, index};
        auto uuid = std::make_shared<IntArrayTag>(uuidSource);
        entityData->set("UUID", uuid);

        auto converted = Entity::From(*entityData, mapInfo, ddf);
        if (converted.empty()) {
          continue;
        }
        auto saveData = converted[0];

        auto outBee = std::make_shared<CompoundTag>();
        outBee->set("SaveData", saveData);

        auto minOccupationTicks = bee->int32("MinOccupationTicks");
        auto ticksInHive = bee->int32("TicksInHive");
        if (minOccupationTicks && ticksInHive) {
          int32_t ticksLeftToStay =
              std::max<int32_t>(0, *minOccupationTicks - *ticksInHive);
          outBee->set("TicksLeftToStay", Int(ticksLeftToStay));
        }

        outBee->set("ActorIdentifier", String("minecraft:bee<>"));

        occupants->push_back(outBee);
      }
      tag->set("Occupants", occupants);
    }
    tag->set("ShouldSpawnBees", Bool(!bees));

    Attach(pos, *tag);
    return tag;
  }

  struct PistonArm {
    explicit PistonArm(bool sticky) : sticky(sticky) {}

    TileEntityData operator()(Pos const &pos, Block const &b,
                              std::shared_ptr<CompoundTag> const &c,
                              JavaEditionMap const &mapInfo,
                              DimensionDataFragment &ddf) const {
      using namespace props;
      using namespace mcfile::nbt;
      auto ret = std::make_shared<CompoundTag>();
      ret->set("id", String("PistonArm"));
      ret->set("isMovable", Bool(false));
      ret->set("LastProgress", Float(1));
      ret->set("NewState", Byte(2));
      ret->set("Progress", Float(1));
      ret->set("State", Byte(2));
      ret->set("Sticky", Bool(sticky));
      auto attachedBlocks = std::make_shared<ListTag>();
      attachedBlocks->fType = Tag::TAG_Compound;
      auto breakBlocks = std::make_shared<ListTag>();
      breakBlocks->fType = Tag::TAG_Compound;
      ret->set("AttachedBlocks", attachedBlocks);
      ret->set("BreakBlocks", breakBlocks);
      Attach(pos, *ret);
      return ret;
    }

    bool const sticky;
  };

  static TileEntityData Lectern(Pos const &pos, Block const &b,
                                std::shared_ptr<CompoundTag> const &c,
                                JavaEditionMap const &mapInfo,
                                DimensionDataFragment &ddf) {
    using namespace props;

    auto tag = std::make_shared<CompoundTag>();
    tag->insert({
        {"id", String("Lectern")},
        {"isMovable", Bool(true)},
    });
    auto book = c->compoundTag("Book");
    int32_t totalPages = 0;
    if (book) {
      auto item = Item::From(book, mapInfo, ddf);
      if (item) {
        tag->set("book", item);
        auto pages = item->query("tag/pages")->asList();
        if (pages) {
          totalPages = pages->size();
        }
      }
    }
    tag->set("hasBook", Bool(totalPages > 0));
    if (totalPages > 0) {
      auto page = c->int32("Page", 0);
      tag->set("page", Int(page));
      tag->set("totalPages", Int(totalPages));
    }
    Attach(pos, *tag);
    return tag;
  }

  static TileEntityData Beacon(Pos const &pos, Block const &b,
                               std::shared_ptr<CompoundTag> const &c,
                               JavaEditionMap const &mapInfo,
                               DimensionDataFragment &ddf) {
    using namespace props;

    auto primary = c->int32("Primary", -1);
    auto secondary = c->int32("Secondary", -1);

    auto tag = std::make_shared<CompoundTag>();
    tag->insert({
        {"id", String("Beacon")},
        {"isMovable", Bool(true)},
        {"primary", Int(primary)},
        {"secondary", Int(secondary)},
    });
    Attach(pos, *tag);
    return tag;
  }

  static TileEntityData EndPortal(Pos const &pos, Block const &b,
                                  std::shared_ptr<CompoundTag> const &c,
                                  JavaEditionMap const &mapInfo,
                                  DimensionDataFragment &ddf) {
    using namespace props;

    auto tag = std::make_shared<CompoundTag>();
    tag->insert({
        {"id", String("EndPortal")},
        {"isMovable", Bool(true)},
    });
    Attach(pos, *tag);
    return tag;
  }

  static TileEntityData Cauldron(Pos const &pos, Block const &b,
                                 std::shared_ptr<CompoundTag> const &c,
                                 JavaEditionMap const &mapInfo,
                                 DimensionDataFragment &ddf) {
    using namespace props;
    using namespace mcfile::nbt;
    using namespace std;

    auto tag = std::make_shared<CompoundTag>();
    tag->insert({{"id", String("Cauldron")},
                 {"isMovable", Bool(true)},
                 {"PotionId", Short(-1)},
                 {"PotionType", Short(-1)}});

    Attach(pos, *tag);
    return tag;
  }

  static TileEntityData Jukebox(Pos const &pos, Block const &b,
                                std::shared_ptr<CompoundTag> const &c,
                                JavaEditionMap const &mapInfo,
                                DimensionDataFragment &ddf) {
    using namespace props;
    using namespace mcfile::nbt;
    using namespace std;

    auto tag = std::make_shared<CompoundTag>();
    tag->insert({
        {"id", String("Jukebox")},
        {"isMovable", Bool(true)},
    });

    auto recordItem = c->compoundTag("RecordItem");
    if (recordItem) {
      auto beRecordItem = Item::From(recordItem, mapInfo, ddf);
      if (beRecordItem) {
        tag->set("RecordItem", beRecordItem);
      }
    }

    Attach(pos, *tag);
    return tag;
  }

  static TileEntityData Note(Pos const &pos, Block const &b,
                             std::shared_ptr<CompoundTag> const &c,
                             JavaEditionMap const &, DimensionDataFragment &) {
    using namespace props;
    using namespace mcfile::nbt;
    using namespace std;

    auto note = strings::Toi(b.property("note", "0"));
    if (!note)
      return nullptr;

    auto tag = std::make_shared<CompoundTag>();
    tag->insert({
        {"id", String("Music")},
        {"isMovable", Bool(true)},
        {"note", Byte(*note)},
    });
    Attach(pos, *tag);
    return tag;
  }

  static TileEntityData Spawner(std::shared_ptr<CompoundTag> const &c) {
    using namespace props;

    auto x = c->int32("x");
    auto y = c->int32("y");
    auto z = c->int32("z");
    if (!x || !y || !z)
      return nullptr;

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
      auto id = spawnData->string("id");
      if (id) {
        mob = *id;
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

  static TileEntityData BrewingStand(Pos const &pos, Block const &b,
                                     std::shared_ptr<CompoundTag> const &c,
                                     JavaEditionMap const &mapInfo,
                                     DimensionDataFragment &ddf) {
    using namespace props;
    using namespace mcfile::nbt;
    using namespace std;

    auto tag = std::make_shared<CompoundTag>();
    auto items = GetItems(c, "Items", mapInfo, ddf);
    vector<shared_ptr<CompoundTag>> sorted(5);
    for (auto const &it : *items) {
      auto item = it->asCompound();
      if (!item)
        continue;
      auto slot = item->byte("Slot");
      if (!slot)
        continue;

      auto newItem = make_shared<CompoundTag>();
      for (auto const &prop : *item) {
        if (prop.first == "Slot")
          continue;
        newItem->set(prop.first, prop.second);
      }
      if (*slot < 0 || 4 < *slot)
        continue;
      int mapping[5] = {1, 2, 3, 0, 4};
      int newSlot = mapping[*slot];
      newItem->set("Slot", Byte(newSlot));
      sorted[newSlot] = newItem;
    }

    auto reordered = make_shared<ListTag>();
    reordered->fType = Tag::TAG_Compound;
    for (auto it : sorted) {
      if (!it)
        continue;
      reordered->push_back(it);
    }

    tag->insert({
        {"Items", reordered},
        {"Findable", Bool(false)},
        {"id", String("BrewingStand")},
        {"isMovable", Bool(true)},
    });
    Attach(pos, *tag);
    return tag;
  }

  static Converter AnyStorage(std::string const &name) {
    return [=](Pos const &pos, Block const &b,
               std::shared_ptr<CompoundTag> const &c,
               JavaEditionMap const &mapInfo, DimensionDataFragment &ddf) {
      using namespace props;
      using namespace mcfile::nbt;
      using namespace std;

      auto tag = std::make_shared<CompoundTag>();
      auto items = GetItems(c, "Items", mapInfo, ddf);

      tag->insert({
          {"Items", items},
          {"Findable", Bool(false)},
          {"id", String(name)},
          {"isMovable", Bool(true)},
      });
      Attach(pos, *tag);
      return tag;
    };
  }

  static TileEntityData Skull(Pos const &pos, Block const &b,
                              std::shared_ptr<CompoundTag> const &c,
                              JavaEditionMap const &, DimensionDataFragment &) {
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
    Attach(pos, *tag);
    return tag;
  }

  static TileEntityData PottedBamboo(Pos const &pos, Block const &b,
                                     std::shared_ptr<CompoundTag> const &c,
                                     JavaEditionMap const &,
                                     DimensionDataFragment &) {
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
        {"version", Int(BlockData::kBlockDataVersion)},
    });
    tag->insert({
        {"id", String("FlowerPot")},
        {"isMovable", Bool(true)},
        {"PlantBlock", plantBlock},
    });
    Attach(pos, *tag);
    return tag;
  }

  static Converter
  PottedPlant(std::string const &name,
              std::map<std::string, std::string> const &properties) {
    return [=](Pos const &pos, Block const &b,
               std::shared_ptr<CompoundTag> const &c, JavaEditionMap const &,
               DimensionDataFragment &) -> TileEntityData {
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
          {"version", Int(BlockData::kBlockDataVersion)},
      });
      tag->insert({
          {"id", String("FlowerPot")},
          {"isMovable", Bool(true)},
          {"PlantBlock", plantBlock},
      });
      Attach(pos, *tag);
      return tag;
    };
  }

  static TileEntityData PottedSapling(Pos const &pos, Block const &b,
                                      std::shared_ptr<CompoundTag> const &c,
                                      JavaEditionMap const &,
                                      DimensionDataFragment &) {
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
        {"version", Int(BlockData::kBlockDataVersion)},
    });
    tag->insert({
        {"id", String("FlowerPot")},
        {"isMovable", Bool(true)},
        {"PlantBlock", plantBlock},
    });
    Attach(pos, *tag);
    return tag;
  }

  static TileEntityData Banner(Pos const &pos, Block const &b,
                               std::shared_ptr<CompoundTag> const &c,
                               JavaEditionMap const &,
                               DimensionDataFragment &) {
    using namespace props;
    using namespace mcfile::nbt;
    auto tag = std::make_shared<CompoundTag>();

    auto customName = GetJson(*c, "CustomName");
    int32_t type = 0;
    if (customName) {
      auto color = GetAsString(*customName, "color");
      auto translate = GetAsString(*customName, "translate");
      if (color == "gold" && translate == "block.minecraft.ominous_banner") {
        type = 1; // Illager Banner
      }
    }

    auto patterns = GetList(c, "Patterns");
    auto patternsBedrock = std::make_shared<ListTag>();
    patternsBedrock->fType = Tag::TAG_Compound;
    if (patterns && type != 1) {
      for (auto const &pattern : *patterns) {
        auto p = pattern->asCompound();
        if (!p)
          continue;
        auto color = p->int32("Color");
        auto pat = p->string("Pattern");
        if (!color || !pat)
          continue;
        auto ptag = std::make_shared<CompoundTag>();
        ptag->insert({
            {"Color", Int(BannerColorCodeFromJava(*color))},
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
    Attach(pos, *tag);
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
        {"black", 0},       {"red", 1},      {"green", 2},   {"brown", 3},
        {"blue", 4},        {"purple", 5},   {"cyan", 6},    {"light_gray", 7},
        {"gray", 8},        {"pink", 9},     {"lime", 10},   {"yellow", 11},
        {"light_blue", 12}, {"magenta", 13}, {"orange", 14}, {"white", 15},
    };
    auto found = mapping.find(color);
    if (found == mapping.end()) {
      return 0;
    }
    return found->second;
  }

  static int8_t BedColor(std::string const &name) {
    static std::unordered_map<std::string, int8_t> const mapping = {
        {"minecraft:white_bed", 0},      {"minecraft:orange_bed", 1},
        {"minecraft:magenta_bed", 2},    {"minecraft:light_blue_bed", 3},
        {"minecraft:yellow_bed", 4},     {"minecraft:lime_bed", 5},
        {"minecraft:pink_bed", 6},       {"minecraft:gray_bed", 7},
        {"minecraft:light_gray_bed", 8}, {"minecraft:cyan_bed", 9},
        {"minecraft:purple_bed", 10},    {"minecraft:blue_bed", 11},
        {"minecraft:brown_bed", 12},     {"minecraft:green_bed", 13},
        {"minecraft:red_bed", 14},       {"minecraft:black_bed", 15},
    };
    auto found = mapping.find(name);
    if (found == mapping.end()) {
      return 0;
    }
    return found->second;
  }

  static TileEntityData Bed(Pos const &pos, Block const &b,
                            std::shared_ptr<CompoundTag> const &c,
                            JavaEditionMap const &, DimensionDataFragment &) {
    using namespace props;
    auto tag = std::make_shared<CompoundTag>();
    auto color = BedColor(b.fName);
    tag->insert({
        {"id", String("Bed")},
        {"color", Byte(color)},
        {"isMovable", Bool(true)},
    });
    Attach(pos, *tag);
    return tag;
  }

  static TileEntityData ShulkerBox(Pos const &pos, Block const &b,
                                   std::shared_ptr<CompoundTag> const &c,
                                   JavaEditionMap const &mapInfo,
                                   DimensionDataFragment &ddf) {
    using namespace props;
    auto facing = BlockData::GetFacingDirectionFromFacingA(b);
    auto items = GetItems(c, "Items", mapInfo, ddf);
    auto tag = std::make_shared<CompoundTag>();
    tag->insert({
        {"id", String("ShulkerBox")},
        {"facing", Byte((int8_t)facing)},
        {"Findable", Bool(false)},
        {"isMovable", Bool(true)},
        {"Items", items},
    });
    Attach(pos, *tag);
    return tag;
  }

  static std::string ColorCode(std::string const &color) {
    if (color == "black") {
      return "§0";
    } else if (color == "red") {
      return "§4";
    } else if (color == "green") {
      return "§2";
    } else if (color == "brown") {
      return ""; // no matching color for brown
    } else if (color == "blue") {
      return "§1";
    } else if (color == "purple") {
      return "§5";
    } else if (color == "cyan") {
      return "§3";
    } else if (color == "light_gray") {
      return "§7";
    } else if (color == "gray") {
      return "§8";
    } else if (color == "pink") {
      return "§d"; // not best match. same as magenta
    } else if (color == "lime") {
      return "§a";
    } else if (color == "yellow") {
      return "§e";
    } else if (color == "light_blue") {
      return "§b";
    } else if (color == "magenta") {
      return "§d";
    } else if (color == "orange") {
      return "§6";
    } else if (color == "white") {
      return "§f";
    }
    return "";
  }

  static std::string GetAsString(nlohmann::json const &obj,
                                 std::string const &key) {
    auto found = obj.find(key);
    if (found == obj.end())
      return "";
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

  static TileEntityData Sign(Pos const &pos, mcfile::Block const &b,
                             std::shared_ptr<CompoundTag> const &c,
                             JavaEditionMap const &, DimensionDataFragment &) {
    using namespace props;
    using namespace mcfile::nbt;
    using namespace std;

    auto color = c->string("Color");
    auto text1 = GetJson(*c, "Text1");
    auto text2 = GetJson(*c, "Text2");
    auto text3 = GetJson(*c, "Text3");
    auto text4 = GetJson(*c, "Text4");
    if (!color || !text1 || !text2 || !text3 || !text4)
      return nullptr;
    string text = "";
    if (*color != "black") {
      text += ColorCode(*color);
    }
    text += GetSignLine(*text1) + "\x0a" + GetSignLine(*text2) + "\x0a" +
            GetSignLine(*text3) + "\x0a" + GetSignLine(*text4);
    auto tag = make_shared<CompoundTag>();
    tag->insert({
        {"id", String("Sign")},
        {"isMovable", Bool(true)},
        {"Text", String(text)},
        {"TextOwner", String("")},
    });
    Attach(pos, *tag);
    return tag;
  }

  static void Attach(Pos const &pos, mcfile::nbt::CompoundTag &tag) {
    tag.set("x", props::Int(pos.fX));
    tag.set("y", props::Int(pos.fY));
    tag.set("z", props::Int(pos.fZ));
  }

  static std::shared_ptr<mcfile::nbt::ListTag>
  GetItems(std::shared_ptr<CompoundTag> const &c, std::string const &name,
           JavaEditionMap const &mapInfo, DimensionDataFragment &ddf) {
    auto tag = std::make_shared<mcfile::nbt::ListTag>();
    tag->fType = mcfile::nbt::Tag::TAG_Compound;
    auto list = GetList(c, name);
    if (list == nullptr) {
      return tag;
    }
    for (auto const &it : *list) {
      if (it->id() != mcfile::nbt::Tag::TAG_Compound)
        continue;
      auto inItem = std::dynamic_pointer_cast<CompoundTag>(it);
      auto outItem = Item::From(inItem, mapInfo, ddf);
      if (!outItem)
        continue;

      auto count = inItem->byte("Count", 1);
      auto slot = inItem->byte("Slot", 0);
      outItem->set("Slot", props::Byte(slot));
      outItem->set("Count", props::Byte(count));

      tag->push_back(outItem);
    }
    return tag;
  }

  static mcfile::nbt::ListTag const *
  GetList(std::shared_ptr<CompoundTag> const &c, std::string const &name) {
    if (!c) {
      return nullptr;
    }
    auto found = c->find(name);
    if (found == c->end()) {
      return nullptr;
    }
    return found->second->asList();
  }

  static TileEntityData Chest(Pos const &pos, mcfile::Block const &b,
                              std::shared_ptr<CompoundTag> const &comp,
                              JavaEditionMap const &mapInfo,
                              DimensionDataFragment &ddf) {
    using namespace props;
    using namespace mcfile::nbt;
    using namespace std;

    auto type = b.property("type", "single");
    auto facing = b.property("facing", "north");
    optional<pair<int, int>> pair;
    if (type == "left" && facing == "north") {
      pair = make_pair(pos.fX + 1, pos.fZ);
    } else if (type == "right" && facing == "south") {
      pair = make_pair(pos.fX + 1, pos.fZ);
    } else if (type == "right" && facing == "west") {
      pair = make_pair(pos.fX, pos.fZ + 1);
    } else if (type == "left" && facing == "east") {
      pair = make_pair(pos.fX, pos.fZ + 1);
    }

    auto tag = std::make_shared<CompoundTag>();
    auto items = GetItems(comp, "Items", mapInfo, ddf);

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
    Attach(pos, *tag);
    return tag;
  }
};

} // namespace j2b
