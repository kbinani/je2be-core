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
      suffix = BlockData::FlowerNameFromRedFlowerType(*type);
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

  static std::unordered_map<std::string, Converter> *CreateTable() {
    using namespace std;
    auto *t = new unordered_map<string, Converter>();
#define E(__name, __conv)                            \
  assert(t->find("minecraft:" #__name) == t->end()); \
  t->insert(make_pair("minecraft:" #__name, __conv))

    E(flower_pot, FlowerPot);

#undef E
    return t;
  }

  static std::shared_ptr<mcfile::je::Block const> Block(std::string const &name, std::map<std::string, std::string> props = {}) {
    return std::make_shared<mcfile::je::Block const>("minecraft:" + name, props);
  }
};

} // namespace je2be::toje
