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

  using Converter = std::function<std::optional<Result>(Pos3i const &, mcfile::be::Block const &, mcfile::nbt::CompoundTag const &)>;

public:
  static std::optional<Result> FromBlockAndBlockEntity(Pos3i const &pos, mcfile::be::Block const &block, mcfile::nbt::CompoundTag const &tag) {
    using namespace std;
    static unique_ptr<unordered_map<string, Converter> const> const sTable(CreateTable());
    auto found = sTable->find(block.fName);
    if (found == sTable->end()) {
      return nullopt;
    }
    return found->second(pos, block, tag);
  }

  static std::optional<Result> FlowerPot(Pos3i const &pos, mcfile::be::Block const &block, mcfile::nbt::CompoundTag const &tagB) {
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

  static std::optional<Result> Skull(Pos3i const &pos, mcfile::be::Block const &block, mcfile::nbt::CompoundTag const &tag) {
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
    auto te = make_shared<CompoundTag>();
    te->set("id", String("minecraft:skull"));
    te->set("keepPacked", Bool(false));
    Pos(*te, pos);
    r.fTileEntity = te;
    return r;
  }

  static std::unordered_map<std::string, Converter> *CreateTable() {
    using namespace std;
    auto *t = new unordered_map<string, Converter>();
#define E(__name, __conv)                            \
  assert(t->find("minecraft:" #__name) == t->end()); \
  t->insert(make_pair("minecraft:" #__name, __conv))

    E(flower_pot, FlowerPot);
    E(skull, Skull);

#undef E
    return t;
  }

  static void Pos(mcfile::nbt::CompoundTag &tag, Pos3i const &pos) {
    tag.set("x", props::Int(pos.fX));
    tag.set("y", props::Int(pos.fY));
    tag.set("z", props::Int(pos.fZ));
  }

  static std::shared_ptr<mcfile::je::Block const> Block(std::string const &name, std::map<std::string, std::string> props = {}) {
    return std::make_shared<mcfile::je::Block const>("minecraft:" + name, props);
  }
};

} // namespace je2be::toje
