#pragma once

namespace je2be::box360 {

class Entity {
  Entity() = delete;

public:
  struct Result {
    CompoundTagPtr fEntity;
  };

private:
  using Converter = std::function<std::optional<Result>(CompoundTag const &in, Context const &ctx)>;

public:
  static std::optional<Result> Convert(CompoundTag const &in, Context const &ctx) {
    using namespace std;
    auto rawId = in.string("id");
    if (!rawId) {
      return nullopt;
    }
    if (!rawId->starts_with("minecraft:")) {
      return nullopt;
    }
    string id = rawId->substr(10);
    auto const &table = GetTable();
    auto found = table.find(id);
    if (found == table.end()) {
      return nullopt;
    }
    auto converted = found->second(in, ctx);
    if (!converted) {
      return nullopt;
    }
    if (!converted->fEntity) {
      return nullopt;
    }
    Result r;
    r.fEntity = converted->fEntity;
    return r;
  }

private:
#pragma region Converters
  static std::optional<Result> ItemFrame(CompoundTag const &in, Context const &ctx) {
    auto out = in.copy();

    int8_t facingB = in.byte("Facing", 0);
    int8_t facingJ = 3;
    switch (facingB) {
    case 1:
      facingJ = 4;
      break;
    case 2:
      facingJ = 2;
      break;
    case 3:
      facingJ = 5;
      break;
    case 0:
    default:
      facingJ = 3;
      break;
    }
    out->set("Facing", Byte(facingJ));

    if (auto item = in.compoundTag("Item"); item) {
      if (auto converted = Item::Convert(*item, ctx); converted) {
        out->set("Item", converted);
      }
    }

    Result r;
    r.fEntity = out;
    return r;
  }
#pragma endregion

  static std::unordered_map<std::string, Converter> const &GetTable() {
    using namespace std;
    static unique_ptr<unordered_map<string, Converter> const> const sTable(CreateTable());
    return *sTable;
  }

  static std::unordered_map<std::string, Converter> const *CreateTable() {
    auto ret = new std::unordered_map<std::string, Converter>();
#define E(__name, __conv)                   \
  assert(ret->find(#__name) == ret->end()); \
  ret->insert(std::make_pair(#__name, __conv))

    E(item_frame, ItemFrame);

#undef E
    return ret;
  }
};

} // namespace je2be::box360
