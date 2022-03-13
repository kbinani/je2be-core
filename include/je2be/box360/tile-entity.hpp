#pragma once

namespace je2be::box360 {

class TileEntity {
  TileEntity() = delete;

public:
  struct Result {
    CompoundTagPtr fTileEntity;
    std::shared_ptr<mcfile::je::Block const> fBlock;
  };

  static std::optional<Result> Convert(CompoundTag const &in, mcfile::je::Block const &block) {
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
    auto ret = found->second(in, block);
    return ret;
  }

  using Converter = std::function<std::optional<Result>(CompoundTag const &in, mcfile::je::Block const &block)>;

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
#undef E
    return ret;
  }

#pragma region Converters
  static std::optional<Result> Skull(CompoundTag const &in, mcfile::je::Block const &block) {
    // TODO:
    return std::nullopt;
  }
#pragma endregion
};

} // namespace je2be::box360
