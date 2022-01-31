#pragma once

namespace je2be::toje {

class BlockEntity {
public:
  struct Result {
    std::shared_ptr<mcfile::nbt::CompoundTag> fTileEntity;
    std::shared_ptr<mcfile::je::Block const> fBlock;
  };

private:
  BlockEntity() = delete;

  using Converter = std::function<std::optional<Result>(Pos3i const &, mcfile::be::Block const &, mcfile::nbt::CompoundTag const &, mcfile::je::Block const &)>;

public:
  static std::optional<Result> FromBlockAndBlockEntity(Pos3i const &pos, mcfile::be::Block const &block, mcfile::nbt::CompoundTag const &tag, mcfile::je::Block const &blockJ) {
    using namespace std;
    static unique_ptr<unordered_map<string, Converter> const> const sTable(CreateTable());
    auto found = sTable->find(block.fName);
    if (found == sTable->end()) {
      return nullopt;
    }
    return found->second(pos, block, tag, blockJ);
  }

#pragma region Dedicated converters
  static std::optional<Result> Banner(Pos3i const &pos, mcfile::be::Block const &block, mcfile::nbt::CompoundTag const &tag, mcfile::je::Block const &blockJ) {
    using namespace std;
    using namespace mcfile::nbt;
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
      te->set("CustomName", props::String(R"({"color":"gold","translate":"block.minecraft.ominous_banner"})"));
      te->set("Patterns", OmniousBannerPatterns());
    } else {
      auto patternsB = tag.listTag("Patterns");
      if (patternsB) {
        auto patternsJ = make_shared<ListTag>(Tag::Type::Compound);
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
          auto pJ = make_shared<CompoundTag>();
          pJ->set("Color", props::Int(static_cast<int32_t>(ColorCodeJavaFromBannerColorCodeBedrock(static_cast<BannerColorCodeBedrock>(*pColorB)))));
          pJ->set("Pattern", props::String(*pPatternB));
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

  static std::optional<Result> Bed(Pos3i const &pos, mcfile::be::Block const &block, mcfile::nbt::CompoundTag const &tagB, mcfile::je::Block const &blockJ) {
    auto color = tagB.byte("color", 0);
    ColorCodeJava ccj = static_cast<ColorCodeJava>(color);
    auto name = JavaNameFromColorCodeJava(ccj);
    Result r;
    r.fBlock = BlockShortName(name + "_bed", blockJ.fProperties);
    r.fTileEntity = EmptyShortName("bed", pos);
    return r;
  }

  static std::optional<Result> Campfire(Pos3i const &pos, mcfile::be::Block const &block, mcfile::nbt::CompoundTag const &tagB, mcfile::je::Block const &blockJ) {
    using namespace std;
    using namespace mcfile::nbt;
    auto items = make_shared<ListTag>(Tag::Type::Compound);
    vector<int> times;
    vector<int> totalTimes;
    for (int i = 0; i < 4; i++) {
      auto itemTag = tagB.compoundTag("Item" + to_string(i + 1));
      bool itemAdded = false;
      if (itemTag) {
        auto item = Item::From(*itemTag);
        if (item) {
          item->set("Slot", props::Byte(i));
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

  static std::optional<Result> Chest(Pos3i const &pos, mcfile::be::Block const &block, mcfile::nbt::CompoundTag const &tagB, mcfile::je::Block const &blockJ) {
    using namespace std;
    auto px = tagB.int32("pairx");
    auto pz = tagB.int32("pairz");
    string type = "single";

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
    }
    map<string, string> p(blockJ.fProperties);
    p["type"] = type;
    Result r;
    r.fBlock = BlockFullName(blockJ.fName, p);
    auto te = EmptyFullName(block.fName, pos);
    auto items = ContainerItems(tagB, "Items");
    if (items && !items->empty()) {
      te->set("Items", items);
    }
    auto lootTable = tagB.string("LootTable");
    if (lootTable && lootTable->starts_with("loot_tables/") && lootTable->ends_with(".json")) {
      string name = strings::RTrim(lootTable->substr(12), ".json");
      te->set("LootTable", props::String("minecraft:" + name));
      auto lootTableSeed = tagB.int32("LootTableSeed");
      if (lootTableSeed) {
        te->set("LootTableSeed", props::Long(*lootTableSeed));
      }
    }
    r.fTileEntity = te;
    return r;
  }

  static std::optional<Result> Comparator(Pos3i const &pos, mcfile::be::Block const &block, mcfile::nbt::CompoundTag const &tagB, mcfile::je::Block const &blockJ) {
    auto t = EmptyShortName("comparator", pos);
    auto outputSignal = tagB.int32("OutputSignal", 0);
    t->set("OutputSignal", props::Int(outputSignal));
    Result r;
    r.fTileEntity = t;
    return r;
  }

  static std::optional<Result> FlowerPot(Pos3i const &pos, mcfile::be::Block const &block, mcfile::nbt::CompoundTag const &tagB, mcfile::je::Block const &blockJ) {
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

  static std::optional<Result> Furnace(Pos3i const &pos, mcfile::be::Block const &block, mcfile::nbt::CompoundTag const &tagB, mcfile::je::Block const &blockJ) {
    using namespace std;
    string name = strings::LTrim(block.fName.substr(10), "lit_");
    auto te = EmptyShortName(name, pos);
    auto burnDuration = tagB.int16("BurnDuration");
    if (burnDuration) {
      te->set("BurnTime", props::Short(*burnDuration));
    }
    auto cookTime = tagB.int16("CookTime");
    if (cookTime) {
      te->set("CookTime", props::Short(*cookTime));
    }
    auto burnTime = tagB.int16("BurnTime");
    if (burnTime) {
      te->set("CookTimeTotal", props::Short(*burnTime));
    }
    te->set("RecipesUsed", make_shared<mcfile::nbt::CompoundTag>());
    Result r;
    r.fTileEntity = te;
    return r;
  }

  static std::optional<Result> Jukebox(Pos3i const &pos, mcfile::be::Block const &block, mcfile::nbt::CompoundTag const &tag, mcfile::je::Block const &blockJ) {
    using namespace std;
    auto record = tag.compoundTag("RecordItem");
    map<string, string> p(blockJ.fProperties);
    p["has_record"] = ToString(record != nullptr);
    auto te = EmptyShortName("jukebox", pos);
    if (record) {
      auto itemJ = Item::From(*record);
      if (itemJ) {
        te->set("RecordItem", itemJ);
      }
    }
    Result r;
    r.fBlock = BlockShortName("jukebox", p);
    r.fTileEntity = te;
    return r;
  }

  static std::optional<Result> Lectern(Pos3i const &pos, mcfile::be::Block const &block, mcfile::nbt::CompoundTag const &tag, mcfile::je::Block const &blockJ) {
    using namespace std;
    auto te = EmptyShortName("lectern", pos);
    map<string, string> p(blockJ.fProperties);
    auto page = tag.int32("page");
    auto bookB = tag.compoundTag("book");
    shared_ptr<mcfile::nbt::CompoundTag> bookJ;
    if (bookB) {
      bookJ = Item::From(*bookB);
    }
    p["has_book"] = ToString(bookJ != nullptr);
    Result r;
    r.fBlock = BlockFullName(blockJ.fName, p);
    if (page) {
      te->set("Page", props::Int(*page));
    }
    if (bookJ) {
      te->set("Book", bookJ);
    }
    r.fTileEntity = te;
    return r;
  }

  static std::optional<Result> Noteblock(Pos3i const &pos, mcfile::be::Block const &block, mcfile::nbt::CompoundTag const &tag, mcfile::je::Block const &blockJ) {
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

  static std::optional<Result> SameNameEmpty(Pos3i const &pos, mcfile::be::Block const &block, mcfile::nbt::CompoundTag const &tag, mcfile::je::Block const &blockJ) {
    Result r;
    r.fTileEntity = EmptyFullName(block.fName, pos);
    return r;
  }

  static std::optional<Result> ShulkerBox(Pos3i const &pos, mcfile::be::Block const &block, mcfile::nbt::CompoundTag const &tag, mcfile::je::Block const &blockJ) {
    using namespace std;
    auto facing = tag.byte("facing", 0);
    auto f6 = Facing6FromBedrockFacingDirectionA(facing);
    map<string, string> p(blockJ.fProperties);
    p["facing"] = JavaNameFromFacing6(f6);
    auto te = EmptyShortName("shulker_box", pos);
    auto items = ContainerItems(tag, "Items");
    if (items && !items->empty()) {
      te->set("Items", items);
    }
    Result r;
    r.fBlock = BlockFullName(blockJ.fName, p);
    r.fTileEntity = te;
    return r;
  }

  static std::optional<Result> Sign(Pos3i const &pos, mcfile::be::Block const &block, mcfile::nbt::CompoundTag const &tag, mcfile::je::Block const &blockJ) {
    using namespace std;
    using namespace props;
    auto te = EmptyShortName("sign", pos);
    auto glowing = tag.boolean("IgnoreLighting", false);
    te->set("GlowingText", Bool(glowing));
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

  static std::optional<Result> Skull(Pos3i const &pos, mcfile::be::Block const &block, mcfile::nbt::CompoundTag const &tag, mcfile::je::Block const &blockJ) {
    using namespace std;
    using namespace mcfile::nbt;
    using namespace props;

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
#pragma endregion

#pragma region Converter generators
  static Converter AnyStorage(std::string const &id) {
    return [id](Pos3i const &pos, mcfile::be::Block const &block, mcfile::nbt::CompoundTag const &tag, mcfile::je::Block const &blockJ) {
      auto te = EmptyFullName("minecraft:" + id, pos);
      auto items = ContainerItems(tag, "Items");
      if (items && !items->empty()) {
        te->set("Items", items);
      }
      Result r;
      r.fTileEntity = te;
      return r;
    };
  }
#pragma endregion

#pragma region Utilities
  static std::shared_ptr<mcfile::nbt::CompoundTag> BannerPattern(int32_t color, std::string const &pattern) {
    auto c = std::make_shared<mcfile::nbt::CompoundTag>();
    c->set("Color", props::Int(color));
    c->set("Pattern", props::String(pattern));
    return c;
  }

  static std::shared_ptr<mcfile::je::Block const> BlockShortName(std::string const &name, std::map<std::string, std::string> props = {}) {
    return std::make_shared<mcfile::je::Block const>("minecraft:" + name, props);
  }

  static std::shared_ptr<mcfile::je::Block const> BlockFullName(std::string const &name, std::map<std::string, std::string> props = {}) {
    return std::make_shared<mcfile::je::Block const>(name, props);
  }

  static std::shared_ptr<mcfile::nbt::ListTag> ContainerItems(mcfile::nbt::CompoundTag const &parent, std::string const &key) {
    using namespace std;
    using namespace mcfile::nbt;
    auto tag = parent.listTag(key);
    if (!tag) {
      return nullptr;
    }
    auto ret = make_shared<ListTag>(Tag::Type::Compound);
    for (auto const &it : *tag) {
      CompoundTag const *c = it->asCompound();
      if (!c) {
        continue;
      }
      auto converted = Item::From(*c);
      if (!converted) {
        continue;
      }
      auto slot = c->byte("Slot");
      if (!slot) {
        continue;
      }
      converted->set("Slot", props::Byte(*slot));
      ret->push_back(converted);
    }
    return ret;
  }

  static std::shared_ptr<mcfile::nbt::CompoundTag> EmptyFullName(std::string const &id, Pos3i const &pos) {
    auto tag = std::make_shared<mcfile::nbt::CompoundTag>();
    tag->set("id", props::String(id));
    tag->set("x", props::Int(pos.fX));
    tag->set("y", props::Int(pos.fY));
    tag->set("z", props::Int(pos.fZ));
    tag->set("keepPacked", props::Bool(false));
    return tag;
  }

  static std::shared_ptr<mcfile::nbt::CompoundTag> EmptyShortName(std::string const &id, Pos3i const &pos) {
    return EmptyFullName("minecraft:" + id, pos);
  }

  static std::shared_ptr<mcfile::nbt::ListTag> OmniousBannerPatterns() {
    auto p = std::make_shared<mcfile::nbt::ListTag>(mcfile::nbt::Tag::Type::Compound);
    p->push_back(BannerPattern(9, "mr"));
    p->push_back(BannerPattern(8, "bs"));
    p->push_back(BannerPattern(7, "cs"));
    p->push_back(BannerPattern(8, "bo"));
    p->push_back(BannerPattern(15, "ms"));
    p->push_back(BannerPattern(8, "hh"));
    p->push_back(BannerPattern(8, "mc"));
    p->push_back(BannerPattern(15, "bo"));
    return p;
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

#undef E
    return t;
  }
};

} // namespace je2be::toje
