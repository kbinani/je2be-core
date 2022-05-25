#pragma once

namespace je2be::toje {

class BlockEntity {
private:
  BlockEntity() = delete;
  using Result = BlockEntityConvertResult;

  using Converter = std::function<std::optional<Result>(Pos3i const &, mcfile::be::Block const &, CompoundTag const &, mcfile::je::Block const &, Context &ctx)>;

public:
  static std::optional<Result> FromBlockAndBlockEntity(Pos3i const &pos, mcfile::be::Block const &block, CompoundTag const &tag, mcfile::je::Block const &blockJ, Context &ctx) {
    using namespace std;
    static unique_ptr<unordered_map<string, Converter> const> const sTable(CreateTable());
    auto found = sTable->find(block.fName);
    if (found == sTable->end()) {
      return nullopt;
    }
    auto result = found->second(pos, block, tag, blockJ, ctx);
    if (result && result->fTileEntity) {
      if (!result->fTileEntity->string("CustomName")) {
        auto customName = tag.string("CustomName");
        if (customName) {
          nlohmann::json json;
          json["text"] = *customName;
          result->fTileEntity->set("CustomName", String(nlohmann::to_string(json)));
        }
      }
    }
    return result;
  }

#pragma region Dedicated converters
  static std::optional<Result> Banner(Pos3i const &pos, mcfile::be::Block const &block, CompoundTag const &tag, mcfile::je::Block const &blockJ, Context &ctx) {
    using namespace std;
    auto base = tag.int32("Base", 0);
    BannerColorCodeBedrock bccb = static_cast<BannerColorCodeBedrock>(base);
    auto colorNameJ = JavaNameFromColorCodeJava(ColorCodeJavaFromBannerColorCodeBedrock(bccb));
    string name;
    if (block.fName == "minecraft:standing_banner") {
      name = colorNameJ + "_banner";
    } else {
      name = colorNameJ + "_wall_banner";
    }
    auto te = EmptyShortName("banner", pos);
    auto type = tag.int32("Type", 0);
    if (type == 1) {
      // Illager Banner
      te->set("CustomName", String(R"({"color":"gold","translate":"block.minecraft.ominous_banner"})"));
      te->set("Patterns", Banner::OminousBannerPatterns());
    } else {
      auto patternsB = tag.listTag("Patterns");
      if (patternsB) {
        auto patternsJ = List<Tag::Type::Compound>();
        for (auto const &pB : *patternsB) {
          CompoundTag const *c = pB->asCompound();
          if (!c) {
            continue;
          }
          auto pColorB = c->int32("Color");
          auto pPatternB = c->string("Pattern");
          if (!pColorB || !pPatternB) {
            continue;
          }
          auto pJ = Compound();
          pJ->set("Color", Int(static_cast<int32_t>(ColorCodeJavaFromBannerColorCodeBedrock(static_cast<BannerColorCodeBedrock>(*pColorB)))));
          pJ->set("Pattern", String(*pPatternB));
          patternsJ->push_back(pJ);
        }
        te->set("Patterns", patternsJ);
      }
    }
    Result r;
    r.fBlock = BlockShortName(name, blockJ.fProperties);
    r.fTileEntity = te;
    return r;
  }

  static std::optional<Result> Beacon(Pos3i const &pos, mcfile::be::Block const &block, CompoundTag const &tagB, mcfile::je::Block const &blockJ, Context &ctx) {
    auto t = EmptyShortName("beacon", pos);
    CopyIntValues(tagB, *t, {{"primary", "Primary", -1}, {"secondary", "Secondary", -1}});
    // NOTE: "Levels" need to be defined by terrain around the beacon.
    // See also beacon.hpp
    Result r;
    r.fTileEntity = t;
    return r;
  }

  static std::optional<Result> Bed(Pos3i const &pos, mcfile::be::Block const &block, CompoundTag const &tagB, mcfile::je::Block const &blockJ, Context &ctx) {
    auto color = tagB.byte("color", 0);
    ColorCodeJava ccj = static_cast<ColorCodeJava>(color);
    auto name = JavaNameFromColorCodeJava(ccj);
    Result r;
    r.fBlock = BlockShortName(name + "_bed", blockJ.fProperties);
    r.fTileEntity = EmptyShortName("bed", pos);
    return r;
  }

  static std::optional<Result> Beehive(Pos3i const &pos, mcfile::be::Block const &block, CompoundTag const &tagB, mcfile::je::Block const &blockJ, Context &ctx) {
    auto te = EmptyShortName("beehive", pos);
    auto occupants = tagB.listTag("Occupants");
    if (occupants) {
      auto bees = List<Tag::Type::Compound>();
      for (auto const &it : *occupants) {
        auto beeB = it->asCompound();
        if (!beeB) {
          continue;
        }
        auto result = Entity::From(*beeB, ctx);
        if (result) {
          bees->push_back(result->fEntity);
        }
      }
      te->set("Bees", bees);
    }
    Result r;
    r.fTileEntity = te;
    return r;
  }

  static std::optional<Result> BrewingStand(Pos3i const &pos, mcfile::be::Block const &block, CompoundTag const &tagB, mcfile::je::Block const &blockJ, Context &ctx) {
    using namespace std;
    auto t = EmptyShortName("brewing_stand", pos);
    auto itemsB = ContainerItems(tagB, "Items", ctx);
    if (itemsB) {
      uint8_t mapping[5] = {3, 0, 1, 2, 4};
      map<int, shared_ptr<CompoundTag>> items;
      for (int i = 0; i < 5 && i < itemsB->size(); i++) {
        auto itemTag = itemsB->at(i);
        CompoundTag const *item = itemTag->asCompound();
        if (!item) {
          continue;
        }
        auto slot = item->byte("Slot");
        if (!slot) {
          continue;
        }
        if (5 <= *slot) {
          continue;
        }
        auto newSlot = mapping[*slot];
        shared_ptr<CompoundTag> copy = item->copy();
        copy->set("Slot", Byte(newSlot));
        items[newSlot] = copy;
      }
      auto itemsJ = List<Tag::Type::Compound>();
      for (auto it : items) {
        itemsJ->push_back(it.second);
      }
      t->set("Items", itemsJ);
    }
    CopyShortValues(tagB, *t, {{"CookTime", "BrewTime", 0}});
    auto fuelAmount = tagB.int16("FuelAmount", 0);
    t->set("Fuel", Byte(fuelAmount));
    Result r;
    r.fTileEntity = t;
    return r;
  }

  static std::optional<Result> Campfire(Pos3i const &pos, mcfile::be::Block const &block, CompoundTag const &tagB, mcfile::je::Block const &blockJ, Context &ctx) {
    using namespace std;
    auto items = List<Tag::Type::Compound>();
    vector<int> times;
    vector<int> totalTimes;
    for (int i = 0; i < 4; i++) {
      auto itemTag = tagB.compoundTag("Item" + to_string(i + 1));
      bool itemAdded = false;
      if (itemTag) {
        auto item = Item::From(*itemTag, ctx);
        if (item) {
          item->set("Slot", Byte(i));
          items->push_back(item);
          itemAdded = true;
        }
      }
      auto time = tagB.int32("ItemTime" + to_string(i + 1), 0);
      times.push_back(time);
      totalTimes.push_back((itemAdded || time > 0) ? 600 : 0);
    }
    auto timesTag = make_shared<IntArrayTag>(times);
    auto totalTimesTag = make_shared<IntArrayTag>(totalTimes);
    auto t = EmptyShortName("campfire", pos);
    t->set("Items", items);
    t->set("CookingTimes", timesTag);
    t->set("CookingTotalTimes", totalTimesTag);

    Result r;
    r.fTileEntity = t;
    return r;
  }

  static std::optional<Result> Chest(Pos3i const &pos, mcfile::be::Block const &block, CompoundTag const &tagB, mcfile::je::Block const &blockJ, Context &ctx) {
    using namespace std;
    auto px = tagB.int32("pairx");
    auto pz = tagB.int32("pairz");
    bool pairlead = tagB.boolean("pairlead", false);
    string type = "single";

    Result r;

    if (px && pz) {
      auto facingDirectionA = block.fStates->int32("facing_direction", 0);
      Facing6 f6 = Facing6FromBedrockFacingDirectionA(facingDirectionA);
      Pos3i vec = Pos3iFromFacing6(f6);
      Pos2i d2(vec.fX, vec.fZ);
      Pos2i pos2d(pos.fX, pos.fZ);
      Pos2i pair(*px, *pz);

      if (pair + Right90(d2) == pos2d) {
        type = "right";
      } else if (pair + Left90(d2) == pos2d) {
        type = "left";
      }
      if ((type == "right" && !pairlead) || (type == "left" && pairlead)) {
        // pairlead = true side of chest occupies upper half of the large chest in BE.
        // On the other hand, in JE, type = right side of chest always occupies upper half of the large chest.
        // Therefore, it is needed to swap "Items" between two chests in this situation.
        r.fTakeItemsFrom = Pos3i(*px, pos.fY, *pz);
      }
    }
    map<string, string> p(blockJ.fProperties);
    p["type"] = type;
    r.fBlock = BlockFullName(blockJ.fName, p);
    auto te = EmptyFullName(block.fName, pos);
    if (auto st = LootTable::BedrockToJava(tagB, *te); st == LootTable::State::NoLootTable && !r.fTakeItemsFrom) {
      auto items = ContainerItems(tagB, "Items", ctx);
      if (items) {
        te->set("Items", items);
      }
    }
    r.fTileEntity = te;
    return r;
  }

  static std::optional<Result> CommandBlock(Pos3i const &pos, mcfile::be::Block const &block, CompoundTag const &tagB, mcfile::je::Block const &blockJ, Context &ctx) {
    auto t = EmptyShortName("command_block", pos);

    if (auto commandB = tagB.string("Command"); commandB) {
      auto commandJ = je2be::command::Command::TranspileBedrockToJava(*commandB);
      t->set("Command", String(commandJ));
    }
    CopyIntValues(tagB, *t, {{"SuccessCount"}});
    CopyBoolValues(tagB, *t, {{"auto"}, {"powered"}, {"conditionMet"}, {"TrackOutput"}});
    CopyLongValues(tagB, *t, {{"LastExecution"}});

    t->set("UpdateLastExecution", Bool(true));

    auto customName = tagB.string("CustomName", "");
    if (customName == "") {
      customName = "@";
    }
    nlohmann::json json;
    json["text"] = customName;
    t->set("CustomName", String(nlohmann::to_string(json)));

    Result r;
    r.fTileEntity = t;
    return r;
  }

  static std::optional<Result> Comparator(Pos3i const &pos, mcfile::be::Block const &block, CompoundTag const &tagB, mcfile::je::Block const &blockJ, Context &ctx) {
    auto t = EmptyShortName("comparator", pos);
    CopyIntValues(tagB, *t, {{"OutputSignal", "OutputSignal", 0}});
    Result r;
    r.fTileEntity = t;
    return r;
  }

  static std::optional<Result> FlowerPot(Pos3i const &pos, mcfile::be::Block const &block, CompoundTag const &tagB, mcfile::je::Block const &blockJ, Context &ctx) {
    using namespace std;
    auto plantBlock = tagB.compoundTag("PlantBlock");
    if (!plantBlock) {
      return nullopt;
    }
    auto s = plantBlock->compoundTag("states");
    if (!s) {
      return nullopt;
    }
    auto fullName = plantBlock->string("name");
    if (!fullName) {
      return nullopt;
    }
    if (!fullName->starts_with("minecraft:")) {
      return nullopt;
    }
    string name = fullName->substr(10);
    string suffix;
    if (name == "sapling") {
      auto type = s->string("sapling_type");
      if (!type) {
        return nullopt;
      }
      suffix = *type + "_sapling";
    } else if (name == "red_flower") {
      auto type = s->string("flower_type");
      if (!type) {
        return nullopt;
      }
      auto rf = RedFlowerFromBedrockName(*type);
      if (!rf) {
        return nullopt;
      }
      suffix = JavaNameFromRedFlower(*rf);
    } else if (name == "yellow_flower") {
      suffix = "dandelion";
    } else if (name == "deadbush") {
      suffix = "dead_bush";
    } else if (name == "tallgrass") {
      auto type = s->string("tall_grass_type");
      if (!type) {
        return nullopt;
      }
      suffix = *type;
    } else if (name == "azalea") {
      suffix = "azalea_bush";
    } else if (name == "flowering_azalea") {
      suffix = "flowering_azalea_bush";
    } else {
      suffix = name;
    }
    Result r;
    r.fBlock = BlockShortName("potted_" + suffix);
    return r;
  }

  static std::optional<Result> Furnace(Pos3i const &pos, mcfile::be::Block const &block, CompoundTag const &tagB, mcfile::je::Block const &blockJ, Context &ctx) {
    using namespace std;
    string name = strings::LTrim(block.fName.substr(10), "lit_");
    auto te = EmptyShortName(name, pos);
    CopyShortValues(tagB, *te, {{"BurnDuration", "BurnTime"}, {"CookTime"}, {"BurnTime", "CookTimeTotal"}});
    te->set("RecipesUsed", Compound());
    auto items = ContainerItems(tagB, "Items", ctx);
    if (items) {
      te->set("Items", items);
    }
    Result r;
    r.fTileEntity = te;
    return r;
  }

  static std::optional<Result> Hopper(Pos3i const &pos, mcfile::be::Block const &block, CompoundTag const &tagB, mcfile::je::Block const &blockJ, Context &ctx) {
    auto t = EmptyShortName("hopper", pos);
    auto items = ContainerItems(tagB, "Items", ctx);
    if (items) {
      t->set("Items", items);
    }
    CopyIntValues(tagB, *t, {{"TransferCooldown", "TransferCooldown", 0}});
    Result r;
    r.fTileEntity = t;
    return r;
  }

  static std::optional<Result> Jukebox(Pos3i const &pos, mcfile::be::Block const &block, CompoundTag const &tag, mcfile::je::Block const &blockJ, Context &ctx) {
    using namespace std;
    auto record = tag.compoundTag("RecordItem");
    map<string, string> p(blockJ.fProperties);
    p["has_record"] = ToString(record != nullptr);
    auto te = EmptyShortName("jukebox", pos);
    if (record) {
      auto itemJ = Item::From(*record, ctx);
      if (itemJ) {
        te->set("RecordItem", itemJ);
      }
    }
    Result r;
    r.fBlock = BlockShortName("jukebox", p);
    r.fTileEntity = te;
    return r;
  }

  static std::optional<Result> Lectern(Pos3i const &pos, mcfile::be::Block const &block, CompoundTag const &tag, mcfile::je::Block const &blockJ, Context &ctx) {
    using namespace std;
    auto te = EmptyShortName("lectern", pos);
    map<string, string> p(blockJ.fProperties);
    auto page = tag.int32("page");
    auto bookB = tag.compoundTag("book");
    shared_ptr<CompoundTag> bookJ;
    if (bookB) {
      bookJ = Item::From(*bookB, ctx);
    }
    p["has_book"] = ToString(bookJ != nullptr);
    Result r;
    r.fBlock = BlockFullName(blockJ.fName, p);
    if (page) {
      te->set("Page", Int(*page));
    }
    if (bookJ) {
      te->set("Book", bookJ);
    }
    r.fTileEntity = te;
    return r;
  }

  static std::optional<Result> MobSpawner(Pos3i const &pos, mcfile::be::Block const &block, CompoundTag const &tag, mcfile::je::Block const &blockJ, Context &ctx) {
    using namespace std;
    auto t = EmptyShortName("mob_spawner", pos);

    CopyShortValues(tag, *t, {{"Delay"}, {"MaxNearbyEntities"}, {"MaxSpawnDelay"}, {"MinSpawnDelay"}, {"RequiredPlayerRange"}, {"SpawnCount"}, {"SpawnRange"}});

    auto entity = tag.string("EntityIdentifier");
    if (entity) {
      auto entityTag = Compound();
      entityTag->set("id", String(*entity));
      auto spawnData = Compound();
      spawnData->set("entity", entityTag);
      t->set("SpawnData", spawnData);

      auto potentials = List<Tag::Type::Compound>();
      auto potential = Compound();
      potential->set("data", spawnData->clone());
      potential->set("weight", Int(1));
      potentials->push_back(potential);
      t->set("SpawnPotentials", potentials);
    }

    Result r;
    r.fTileEntity = t;
    return r;
  }

  static std::optional<Result> Noteblock(Pos3i const &pos, mcfile::be::Block const &block, CompoundTag const &tag, mcfile::je::Block const &blockJ, Context &ctx) {
    using namespace std;
    auto note = tag.byte("note");
    map<string, string> p(blockJ.fProperties);
    if (note) {
      p["note"] = to_string(*note);
    }
    Result r;
    r.fBlock = BlockFullName(blockJ.fName, p);
    return r;
  }

  static std::optional<Result> SameNameEmpty(Pos3i const &pos, mcfile::be::Block const &block, CompoundTag const &tag, mcfile::je::Block const &blockJ, Context &ctx) {
    Result r;
    r.fTileEntity = EmptyFullName(block.fName, pos);
    return r;
  }

  static std::optional<Result> ShulkerBox(Pos3i const &pos, mcfile::be::Block const &block, CompoundTag const &tag, mcfile::je::Block const &blockJ, Context &ctx) {
    using namespace std;
    auto facing = tag.byte("facing", 0);
    auto f6 = Facing6FromBedrockFacingDirectionA(facing);
    map<string, string> p(blockJ.fProperties);
    p["facing"] = JavaNameFromFacing6(f6);
    auto te = EmptyShortName("shulker_box", pos);
    auto items = ContainerItems(tag, "Items", ctx);
    if (items) {
      te->set("Items", items);
    }
    Result r;
    r.fBlock = BlockFullName(blockJ.fName, p);
    r.fTileEntity = te;
    return r;
  }

  static std::optional<Result> Sign(Pos3i const &pos, mcfile::be::Block const &block, CompoundTag const &tag, mcfile::je::Block const &blockJ, Context &ctx) {
    using namespace std;
    auto te = EmptyShortName("sign", pos);
    CopyBoolValues(tag, *te, {{"IgnoreLighting", "GlowingText", false}});
    auto color = tag.int32("SignTextColor");
    if (color) {
      Rgba rgba = Rgba::FromRGB(*color);
      ColorCodeJava jcc = SignColor::MostSimilarColor(rgba);
      te->set("Color", String(JavaNameFromColorCodeJava(jcc)));
    }
    auto text = tag.string("Text", "");
    vector<string> lines = mcfile::String::Split(text, '\n');
    for (int i = 0; i < 4; i++) {
      string key = "Text" + to_string(i + 1);
      string line = i < lines.size() ? lines[i] : "";
      nlohmann::json json;
      json["text"] = line;
      te->set(key, String(nlohmann::to_string(json)));
    }
    Result r;
    r.fTileEntity = te;
    return r;
  }

  static std::optional<Result> Skull(Pos3i const &pos, mcfile::be::Block const &block, CompoundTag const &tag, mcfile::je::Block const &blockJ, Context &ctx) {
    using namespace std;

    SkullType type = static_cast<SkullType>(tag.byte("SkullType", 0));
    string skullName = JavaNameFromSkullType(type);

    map<string, string> p;

    int facingDirection = block.fStates->int32("facing_direction", 1);
    Facing6 f = Facing6FromBedrockFacingDirectionA(facingDirection);
    bool floorPlaced = f == Facing6::Up || f == Facing6::Down;
    if (floorPlaced) {
      int rotation = (int)std::round(tag.float32("Rotation", 0) + 0.5f);
      int rot = rotation * 16 / 360;
      p["rotation"] = to_string(rot);
    } else {
      skullName = strings::Replace(skullName, "_head", "_wall_head");
      skullName = strings::Replace(skullName, "_skull", "_wall_skull");
      p["facing"] = JavaNameFromFacing6(f);
    }
    Result r;
    r.fBlock = BlockShortName(skullName, p);
    auto te = EmptyShortName("skull", pos);
    r.fTileEntity = te;
    return r;
  }

  static std::optional<Result> StructureBlock(Pos3i const &pos, mcfile::be::Block const &block, CompoundTag const &tag, mcfile::je::Block const &blockJ, Context &ctx) {
    auto t = EmptyShortName("structure_block", pos);

    t->set("showair", Bool(false));

    CopyBoolValues(tag, *t, {{"ignoreEntities"}, {"isPowered", "powered"}, {"showBoundingBox", "showboundingbox"}});
    CopyFloatValues(tag, *t, {{"integrity"}});
    CopyStringValues(tag, *t, {{"structureName", "name"}, {"dataField", "metadata"}});
    CopyIntValues(tag, *t, {
                               {"xStructureOffset", "posX"},
                               {"yStructureOffset", "posY"},
                               {"zStructureOffset", "posZ"},
                               {"xStructureSize", "sizeX"},
                               {"yStructureSize", "sizeY"},
                               {"zStructureSize", "sizeZ"},
                           });
    CopyLongValues(tag, *t, {{"seed"}});

    // "NONE", "LEFT_RIGHT" (displayed as "<- ->"), "FRONT_BACK" (displayed as "^v")
    auto mirrorB = tag.byte("mirror", 0);
    std::string mirrorJ = "NONE";
    switch (mirrorB) {
    case 1:
      mirrorJ = "LEFT_RIGHT";
      break;
    case 2:
      mirrorJ = "FRONT_BACK";
      break;
    case 0:
    default:
      mirrorJ = "NONE";
      break;
    }
    t->set("mirror", String(mirrorJ));

    // "LOAD", "SAVE", "CORNER"
    auto dataB = tag.int32("data", 1);
    std::string mode = "LOAD";
    switch (dataB) {
    case 1:
      mode = "SAVE";
      break;
    case 3:
      mode = "CORNER";
      break;
    case 2:
    default:
      mode = "LOAD";
      break;
    }
    t->set("mode", String(mode));

    // "NONE" (displayed as "0"), "CLOCKWISE_90" (displayed as "90"), "CLOCKWISE_180" (displayed as "180"), "COUNTERCLOCKWISE_90" (displayed as "270")
    auto rotationB = tag.byte("rotation", 0);
    std::string rotationJ = "NONE";
    switch (rotationB) {
    case 1:
      rotationJ = "CLOCKWISE_90";
      break;
    case 2:
      rotationJ = "CLOCKWISE_180";
      break;
    case 3:
      rotationJ = "COUNTERCLOCKWISE_90";
      break;
    case 0:
    default:
      rotationJ = "NONE";
      break;
    }
    t->set("rotation", String(rotationJ));

    Result r;
    r.fTileEntity = t;
    return r;
  }
#pragma endregion

#pragma region Converter generators
  static Converter AnyStorage(std::string const &id) {
    return [id](Pos3i const &pos, mcfile::be::Block const &block, CompoundTag const &tag, mcfile::je::Block const &blockJ, Context &ctx) {
      auto te = EmptyFullName("minecraft:" + id, pos);
      auto items = ContainerItems(tag, "Items", ctx);
      if (items) {
        te->set("Items", items);
      }
      Result r;
      r.fTileEntity = te;
      return r;
    };
  }

  static Converter NamedEmpty(std::string id) {
    return [id](Pos3i const &pos, mcfile::be::Block const &block, CompoundTag const &tag, mcfile::je::Block const &blockJ, Context &ctx) {
      Result r;
      r.fTileEntity = EmptyShortName(id, pos);
      return r;
    };
  }
#pragma endregion

#pragma region Utilities
  static std::shared_ptr<mcfile::je::Block const> BlockShortName(std::string const &name, std::map<std::string, std::string> props = {}) {
    return std::make_shared<mcfile::je::Block const>("minecraft:" + name, props);
  }

  static std::shared_ptr<mcfile::je::Block const> BlockFullName(std::string const &name, std::map<std::string, std::string> props = {}) {
    return std::make_shared<mcfile::je::Block const>(name, props);
  }

  static ListTagPtr ContainerItems(CompoundTag const &parent, std::string const &key, Context &ctx) {
    using namespace std;
    auto tag = parent.listTag(key);
    if (!tag) {
      return nullptr;
    }
    auto ret = List<Tag::Type::Compound>();
    for (auto const &it : *tag) {
      CompoundTag const *c = it->asCompound();
      if (!c) {
        continue;
      }
      auto converted = Item::From(*c, ctx);
      if (!converted) {
        continue;
      }
      auto slot = c->byte("Slot");
      if (!slot) {
        continue;
      }
      converted->set("Slot", Byte(*slot));
      ret->push_back(converted);
    }
    return ret;
  }

  static CompoundTagPtr EmptyFullName(std::string const &id, Pos3i const &pos) {
    auto tag = Compound();
    tag->set("id", String(id));
    tag->set("x", Int(pos.fX));
    tag->set("y", Int(pos.fY));
    tag->set("z", Int(pos.fZ));
    tag->set("keepPacked", Bool(false));
    return tag;
  }

  static CompoundTagPtr EmptyShortName(std::string const &id, Pos3i const &pos) {
    return EmptyFullName("minecraft:" + id, pos);
  }

  static std::string ToString(bool b) {
    return b ? "true" : "false";
  }
#pragma endregion

  static std::unordered_map<std::string, Converter> *CreateTable() {
    using namespace std;
    auto *t = new unordered_map<string, Converter>();
#define E(__name, __conv)                            \
  assert(t->find("minecraft:" #__name) == t->end()); \
  t->insert(make_pair("minecraft:" #__name, __conv))

    E(flower_pot, FlowerPot);
    E(skull, Skull);
    E(bed, Bed);
    E(standing_banner, Banner);
    E(wall_banner, Banner);
    E(jukebox, Jukebox);
    E(shulker_box, ShulkerBox);
    E(undyed_shulker_box, ShulkerBox);
    E(noteblock, Noteblock);
    E(chest, Chest);
    E(trapped_chest, Chest);
    E(lectern, Lectern);
    E(wall_sign, Sign);
    E(standing_sign, Sign);
    E(acacia_wall_sign, Sign);
    E(birch_wall_sign, Sign);
    E(crimson_wall_sign, Sign);
    E(jungle_wall_sign, Sign);
    E(spruce_wall_sign, Sign);
    E(warped_wall_sign, Sign);
    E(darkoak_wall_sign, Sign);
    E(darkoak_standing_sign, Sign);
    E(acacia_standing_sign, Sign);
    E(birch_standing_sign, Sign);
    E(crimson_standing_sign, Sign);
    E(jungle_standing_sign, Sign);
    E(spruce_standing_sign, Sign);
    E(warped_standing_sign, Sign);
    E(ender_chest, SameNameEmpty);
    E(enchanting_table, SameNameEmpty);
    E(barrel, AnyStorage("barrel"));
    E(bell, SameNameEmpty);
    E(conduit, SameNameEmpty);
    E(campfire, Campfire);
    E(soul_campfire, Campfire);

    E(furnace, Furnace);
    E(lit_furnace, Furnace);
    E(blast_furnace, Furnace);
    E(lit_blast_furnace, Furnace);
    E(smoker, Furnace);
    E(lit_smoker, Furnace);

    E(dispenser, AnyStorage("dispenser"));
    E(powered_comparator, Comparator);
    E(unpowered_comparator, Comparator);
    E(dropper, AnyStorage("dropper"));
    E(brewing_stand, BrewingStand);
    E(hopper, Hopper);
    E(daylight_detector, NamedEmpty("daylight_detector"));
    E(daylight_detector_inverted, NamedEmpty("daylight_detector"));
    E(end_portal, SameNameEmpty);
    E(beacon, Beacon);
    E(mob_spawner, MobSpawner);
    E(command_block, CommandBlock);
    E(chain_command_block, CommandBlock);
    E(repeating_command_block, CommandBlock);
    E(structure_block, StructureBlock);
    E(beehive, Beehive);
    E(bee_nest, Beehive);

    E(sculk_sensor, SameNameEmpty);
    E(sculk_shrieker, SameNameEmpty);
#undef E
    return t;
  }
};

} // namespace je2be::toje
