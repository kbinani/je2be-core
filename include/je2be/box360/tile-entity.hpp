#pragma once

namespace je2be::box360 {

class TileEntity {
  TileEntity() = delete;

public:
  struct Result {
    CompoundTagPtr fTileEntity;
    std::shared_ptr<mcfile::je::Block const> fBlock;
  };

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

  using Converter = std::function<std::optional<Result>(CompoundTag const &in, mcfile::je::Block const &block, CompoundTagPtr &out)>;

  static std::unordered_map<std::string, Converter> const &GetTable() {
    static std::unique_ptr<std::unordered_map<std::string, Converter> const> const sTable(CreateTable());
    return *sTable;
  }

  static std::unordered_map<std::string, Converter> const *CreateTable() {
    auto ret = new std::unordered_map<std::string, Converter>();

#define E(__name, __conv)                   \
  assert(ret->find(#__name) == ret->end()); \
  ret->insert(std::make_pair(#__name, __conv))

    E(skull, Skull);
    E(banner, Banner);
    E(bed, Bed);
#undef E
    return ret;
  }

#pragma region Dedicated_Converters
  static std::optional<Result> Banner(CompoundTag const &in, mcfile::je::Block const &block, CompoundTagPtr &out) {
    using namespace std;
    auto base = static_cast<BannerColorCodeBedrock>(in.int32("Base", 0));
    string color = JavaNameFromColorCodeJava(ColorCodeJavaFromBannerColorCodeBedrock(base));
    string name;
    mcfile::nbt::PrintAsJson(cout, in, {.fTypeHint = true});
    cout << block.fName << endl;
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
};

} // namespace je2be::box360
