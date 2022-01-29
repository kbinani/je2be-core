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
  static std::optional<Result> FromBlockAndBlockEntity(Pos3i const &pos, mcfile::be::Block const &block, mcfile::nbt::CompoundTag const &tag, mcfile::je::Block const &inout) {
    using namespace std;
    static unique_ptr<unordered_map<string, Converter> const> const sTable(CreateTable());
    auto found = sTable->find(block.fName);
    if (found == sTable->end()) {
      return nullopt;
    }
    return found->second(pos, block, tag, inout);
  }

#pragma region Converters
  static std::optional<Result> Banner(Pos3i const &pos, mcfile::be::Block const &block, mcfile::nbt::CompoundTag const &tag, mcfile::je::Block const &inout) {
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
    auto te = Empty("banner", pos);
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
    r.fBlock = Block(name, inout.fProperties);
    r.fTileEntity = te;
    return r;
  }

  static std::optional<Result> Bed(Pos3i const &pos, mcfile::be::Block const &block, mcfile::nbt::CompoundTag const &tagB, mcfile::je::Block const &inout) {
    auto color = tagB.byte("color", 0);
    ColorCodeJava ccj = static_cast<ColorCodeJava>(color);
    auto name = JavaNameFromColorCodeJava(ccj);
    Result r;
    r.fBlock = Block(name + "_bed", inout.fProperties);
    r.fTileEntity = Empty("bed", pos);
    return r;
  }

  static std::optional<Result> FlowerPot(Pos3i const &pos, mcfile::be::Block const &block, mcfile::nbt::CompoundTag const &tagB, mcfile::je::Block const &inout) {
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
    r.fBlock = Block("potted_" + suffix);
    return r;
  }

  static std::optional<Result> Jukebox(Pos3i const &pos, mcfile::be::Block const &block, mcfile::nbt::CompoundTag const &tag, mcfile::je::Block const &inout) {
    using namespace std;
    auto record = tag.compoundTag("RecordItem");
    map<string, string> p(inout.fProperties);
    p["has_record"] = ToString(record != nullptr);
    auto te = Empty("jukebox", pos);
    if (record) {
      auto itemJ = Item::From(*record);
      if (itemJ) {
        te->set("RecordItem", itemJ);
      }
    }
    Result r;
    r.fBlock = Block("jukebox", p);
    r.fTileEntity = te;
    return r;
  }

  static std::optional<Result> Skull(Pos3i const &pos, mcfile::be::Block const &block, mcfile::nbt::CompoundTag const &tag, mcfile::je::Block const &inout) {
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
    r.fBlock = Block(skullName, p);
    auto te = Empty("skull", pos);
    r.fTileEntity = te;
    return r;
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

#undef E
    return t;
  }

  static std::shared_ptr<mcfile::nbt::CompoundTag> BannerPattern(int32_t color, std::string const &pattern) {
    auto c = std::make_shared<mcfile::nbt::CompoundTag>();
    c->set("Color", props::Int(color));
    c->set("Pattern", props::String(pattern));
    return c;
  }

  static std::string ToString(bool b) {
    return b ? "true" : "false";
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

  static std::shared_ptr<mcfile::nbt::CompoundTag> Empty(std::string const &id, Pos3i const &pos) {
    auto tag = std::make_shared<mcfile::nbt::CompoundTag>();
    tag->set("id", props::String("minecraft:" + id));
    tag->set("x", props::Int(pos.fX));
    tag->set("y", props::Int(pos.fY));
    tag->set("z", props::Int(pos.fZ));
    tag->set("keepPacked", props::Bool(false));
    return tag;
  }

  static std::shared_ptr<mcfile::je::Block const> Block(std::string const &name, std::map<std::string, std::string> props = {}) {
    return std::make_shared<mcfile::je::Block const>("minecraft:" + name, props);
  }
};

} // namespace je2be::toje
