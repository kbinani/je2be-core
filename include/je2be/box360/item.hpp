#pragma once

namespace je2be::box360 {

class Item {
  Item() = delete;

  using Converter = std::function<CompoundTagPtr(CompoundTag const &in, CompoundTagPtr &out, int16_t damage)>;

public:
  static CompoundTagPtr Convert(CompoundTag const &in) {
    using namespace std;
    auto rawId = in.string("id");
    if (!rawId) {
      return nullptr;
    }
    if (!rawId->starts_with("minecraft:")) {
      return nullptr;
    }
    auto id = rawId->substr(10);
    auto const &table = GetTable();
    auto found = table.find(id);
    if (found == table.end()) {
      return nullptr;
    }
    auto damage = in.int16("Damage", 0);

    auto out = Compound();
    out->set("id", String(*rawId));
    CopyByteValues(in, *out, {{"Count"}, {"Slot"}});

    return found->second(in, out, damage);
  }

private:
#pragma region Converter
  static CompoundTagPtr Same(CompoundTag const &in, CompoundTagPtr &out, int16_t damage) {
    return out;
  }

  static CompoundTagPtr Stone(CompoundTag const &in, CompoundTagPtr &out, int16_t damage) {
    std::string name = "stone";
    switch (damage) {
    case 0:
    default:
      break;
    }
    out->set("id", String("minecraft:" + name));
    return out;
  }
#pragma endregion

#pragma region Converter_Generator
  static Converter Rename(std::string const &name) {
    return [name](CompoundTag const &in, CompoundTagPtr &out, int16_t damage) {
      out->set("id", String("minecraft:" + name));
      return out;
    };
  }
#pragma endregion

  static std::unordered_map<std::string, Converter> const &GetTable() {
    static std::unique_ptr<std::unordered_map<std::string, Converter> const> const sTable(CreateTable());
    return *sTable;
  }

  static std::unordered_map<std::string, Converter> const *CreateTable() {
    using namespace std;
    auto ret = new unordered_map<string, Converter>();
#define E(__name, __conv)                   \
  assert(ret->find(#__name) == ret->end()); \
  ret->insert(std::make_pair(#__name, __conv))

    E(comparator, Same);
    E(daylight_detector, Same);
    E(stone, Stone);

#undef E
    return ret;
  }
};

} // namespace je2be::box360
