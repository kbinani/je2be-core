#pragma once

namespace je2be::box360 {

class TileEntity {
  TileEntity() = delete;

public:
  struct Result {
    CompoundTagPtr fTileEntity;
    std::shared_ptr<mcfile::je::Block const> fBlock;
  };

private:
  using Converter = std::function<std::optional<Result>(CompoundTag const &in, mcfile::je::Block const &block, CompoundTagPtr &out)>;

public:
  static std::optional<Result> Convert(CompoundTag const &in, mcfile::je::Block const &block, Pos3i const &pos) {
    using namespace std;
    auto rawId = in.string("id", "");
    if (!rawId.starts_with("minecraft:")) {
      return nullopt;
    }

    auto id = rawId.substr(10);
    auto const &table = GetTable();
    auto found = table.find(id);
    if (found == table.end()) {
      return nullopt;
    }
    auto out = Compound();
    out->set("x", Int(pos.fX));
    out->set("y", Int(pos.fY));
    out->set("z", Int(pos.fZ));
    out->set("keepPacked", Bool(false));
    out->set("id", String(rawId));

    auto ret = found->second(in, block, out);
    if (!ret) {
      return nullopt;
    }
    return ret;
  }

#pragma region Dedicated_Converters
  static std::optional<Result> Banner(CompoundTag const &in, mcfile::je::Block const &block, CompoundTagPtr &out) {
    using namespace std;
    auto base = static_cast<BannerColorCodeBedrock>(in.int32("Base", 0));
    string color = JavaNameFromColorCodeJava(ColorCodeJavaFromBannerColorCodeBedrock(base));
    string name;
    if (block.fName.find("wall") == std::string::npos) {
      name = color + "_banner";
    } else {
      name = color + "_wall_banner";
    }
    Result r;
    r.fTileEntity = out;
    r.fBlock = make_shared<mcfile::je::Block const>("minecraft:" + name, block.fProperties);
    return r;
  }

  static std::optional<Result> Bed(CompoundTag const &in, mcfile::je::Block const &block, CompoundTagPtr &out) {
    auto color = static_cast<ColorCodeJava>(in.int32("color", 0));
    std::string name = JavaNameFromColorCodeJava(color) + "_bed";
    Result r;
    r.fTileEntity = out;
    r.fBlock = std::make_shared<mcfile::je::Block const>("minecraft:" + name, block.fProperties);
    return r;
  }

  static std::optional<Result> Chest(CompoundTag const &in, mcfile::je::Block const &block, CompoundTagPtr &out) {
    if (block.fId == mcfile::blocks::minecraft::trapped_chest) {
      out->set("id", String("minecraft:trapped_chest"));
    }
    Items(in, out);
    Result r;
    r.fTileEntity = out;
    return r;
  }

  static std::optional<Result> EnderChest(CompoundTag const &in, mcfile::je::Block const &block, CompoundTagPtr &out) {
    out->set("id", String("minecraft:ender_chest"));
    Result r;
    r.fTileEntity = out;
    return r;
  }

  static std::optional<Result> FlowerPot(CompoundTag const &in, mcfile::je::Block const &block, CompoundTagPtr &out) {
    using namespace std;
    Result r;
    string name;
    auto data = in.int32("Data", 0);
    auto item = in.int32("Item", 0);
    switch (item) {
    case 6:
      switch (data) {
      case 1:
        name = "spruce_sapling";
        break;
      case 2:
        name = "birch_sapling";
        break;
      case 3:
        name = "jungle_sapling";
        break;
      case 4:
        name = "acacia_sapling";
        break;
      case 5:
        name = "dark_oak_sapling";
        break;
      case 0:
      default:
        name = "oak_sapling";
        break;
      }
      break;
    case 31:
      name = "fern"; // data = 2
      break;
    case 32:
      name = "dead_bush";
      break;
    case 37:
      name = "dandelion";
      break;
    case 38:
      switch (data) {
      case 1:
        name = "blue_orchid";
        break;
      case 2:
        name = "allium";
        break;
      case 3:
        name = "azure_bluet";
        break;
      case 4:
        name = "red_tulip";
        break;
      case 5:
        name = "orange_tulip";
        break;
      case 6:
        name = "white_tulip";
        break;
      case 7:
        name = "pink_tulip";
        break;
      case 8:
        name = "oxeye_daisy";
        break;
      case 0:
      default:
        name = "poppy";
        break;
      }
      break;
    case 39:
      name = "brown_mushroom";
      break;
    case 40:
      name = "red_mushroom";
      break;
    case 81:
      name = "cactus";
      break;
    }
    if (name.empty()) {
      return nullopt;
    }
    r.fBlock = make_shared<mcfile::je::Block const>("minecraft:potted_" + name);
    return r;
  }

  static std::optional<Result> Jukebox(CompoundTag const &in, mcfile::je::Block const &block, CompoundTagPtr &out) {
    if (auto recordItem = in.compoundTag("RecordItem"); recordItem) {
      if (auto converted = Item::Convert(*recordItem); converted) {
        out->set("RecordItem", converted);
      }
    }
    Result r;
    r.fTileEntity = out;
    return r;
  }

  static std::optional<Result> SameNameEmpty(CompoundTag const &in, mcfile::je::Block const &block, CompoundTagPtr &out) {
    Result r;
    r.fTileEntity = out;
    return r;
  }

  static std::optional<Result> ShulkerBox(CompoundTag const &in, mcfile::je::Block const &block, CompoundTagPtr &out) {
    Items(in, out);
    Result r;
    r.fTileEntity = out;
    return r;
  }

  static std::optional<Result> Sign(CompoundTag const &in, mcfile::je::Block const &block, CompoundTagPtr &out) {
    using namespace std;
    Result r;
    for (int i = 1; i <= 4; i++) {
      string key = "Text" + to_string(i);
      auto text = in.string(key);
      if (!text) {
        continue;
      }
      nlohmann::json obj;
      obj["text"] = *text;
      out->set(key, String(nlohmann::to_string(obj)));
    }
    r.fTileEntity = out;
    return r;
  }

  static std::optional<Result> Skull(CompoundTag const &in, mcfile::je::Block const &block, CompoundTagPtr &out) {
    using namespace std;
    auto type = static_cast<SkullType>(in.byte("SkullType", 0));
    auto skullName = JavaNameFromSkullType(type);
    auto rot = in.byte("Rot");

    map<string, string> props(block.fProperties);

    string name;
    if (block.fName.find("wall") == string::npos) {
      if (rot) {
        props["rotation"] = to_string(*rot);
      }
      name = skullName;
    } else {
      name = strings::Replace(strings::Replace(skullName, "_head", "_wall_head"), "_skull", "_wall_skull");
    }
    auto blockJ = make_shared<mcfile::je::Block const>("minecraft:" + name, props);
    Result r;
    r.fBlock = blockJ;
    r.fTileEntity = out;
    return r;
  }
#pragma endregion

private:
#pragma region Helpers
  static std::unordered_map<std::string, Converter> const *CreateTable() {
    auto ret = new std::unordered_map<std::string, Converter>();

#define E(__name, __conv)                   \
  assert(ret->find(#__name) == ret->end()); \
  ret->insert(std::make_pair(#__name, __conv))

    E(skull, Skull);
    E(banner, Banner);
    E(bed, Bed);
    E(chest, Chest);
    E(ender_Chest, EnderChest);
    E(shulker_box, ShulkerBox);
    E(flower_pot, FlowerPot);
    E(jukebox, Jukebox);
    E(sign, Sign);
    E(enchanting_table, SameNameEmpty);

#undef E
    return ret;
  }

  static std::unordered_map<std::string, Converter> const &GetTable() {
    static std::unique_ptr<std::unordered_map<std::string, Converter> const> const sTable(CreateTable());
    return *sTable;
  }

  static void Items(CompoundTag const &in, CompoundTagPtr &out) {
    auto items = in.listTag("Items");
    if (!items) {
      return;
    }
    auto itemsJ = List<Tag::Type::Compound>();
    for (auto it : *items) {
      if (auto c = it->asCompound(); c) {
        if (auto converted = Item::Convert(*c); converted) {
          itemsJ->push_back(converted);
        }
      }
    }
    out->set("Items", itemsJ);
  }
#pragma endregion
};

} // namespace je2be::box360
