#pragma once

namespace je2be::box360 {

class Entity {
  Entity() = delete;

public:
  struct Result {
    CompoundTagPtr fEntity;
  };

private:
  struct Ret {};
  using Converter = std::function<std::optional<Ret>(CompoundTag const &in, CompoundTagPtr &out, Context const &ctx)>;

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

    auto out = Default(in);
    if (!out) {
      return nullopt;
    }
    if (found == table.end()) {
      Result r;
      r.fEntity = out;
      return r;
    }

    auto converted = found->second(in, out, ctx);
    if (!converted) {
      return nullopt;
    }
    if (!out) {
      return nullopt;
    }

    Result r;
    r.fEntity = out;
    return r;
  }

private:
#pragma region Converters
  static std::optional<Ret> ItemFrame(CompoundTag const &in, CompoundTagPtr &out, Context const &ctx) {
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

    Ret r;
    return r;
  }

  static std::optional<Ret> Painting(CompoundTag const &in, CompoundTagPtr &out, Context const &ctx) {
    if (auto motive = in.string("Motive"); motive) {
      out->set("Motive", String(strings::Uncapitalize(*motive)));
    }
    Ret r;
    return r;
  }
#pragma endregion

  static CompoundTagPtr Default(CompoundTag const &in) {
    auto uuid = in.string("UUID");
    if (!uuid) {
      return nullptr;
    }
    if (!uuid->starts_with("ent") && uuid->size() != 35) {
      return nullptr;
    }
    uint8_t data[16];
    for (int i = 0; i < 16; i++) {
      auto sub = uuid->substr(3 + i * 2, 2);
      auto converted = strings::Toi(sub, 16);
      if (!converted) {
        return nullptr;
      }
      data[i] = 0xff & ((uint32_t)*converted);
    }
    Uuid u = Uuid::FromData(data);
    auto ret = in.copy();
    ret->set("UUID", u.toIntArrayTag());
    return ret;
  }

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
    E(painting, Painting);

#undef E
    return ret;
  }
};

} // namespace je2be::box360
