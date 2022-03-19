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
  static CompoundTagPtr Log(CompoundTag const &in, CompoundTagPtr &out, int16_t damage) {
    std::string name;
    switch (damage) {
    case 1:
      name = "spruce_log";
      break;
    case 2:
      name = "birch_log";
      break;
    case 3:
      name = "jungle_log";
      break;
    case 0:
    default:
      name = "oak_log";
      break;
    }
    out->set("id", String("minecraft:" + name));
    return out;
  }

  static CompoundTagPtr Planks(CompoundTag const &in, CompoundTagPtr &out, int16_t damage) {
    std::string name;
    switch (damage) {
    case 1:
      name = "spruce_planks";
      break;
    case 2:
      name = "birch_planks";
      break;
    case 3:
      name = "jungle_planks";
      break;
    case 4:
      name = "acacia_planks";
      break;
    case 5:
      name = "dark_oak_planks";
      break;
    case 0:
    default:
      name = "oak_planks";
      break;
    }
    out->set("id", String("minecraft:" + name));
    return out;
  }

  static CompoundTagPtr RedSandstone(CompoundTag const &in, CompoundTagPtr &out, int16_t damage) {
    std::string name = "";
    switch (damage) {
    case 1:
      name = "chiseled_red_sandstone";
      break;
    case 2:
      name = "cut_red_sandstone";
      break;
    case 0:
    default:
      name = "red_sandstone";
      break;
    }
    out->set("id", String("minecraft:" + name));
    return out;
  }

  static CompoundTagPtr Same(CompoundTag const &in, CompoundTagPtr &out, int16_t damage) {
    return out;
  }

  static CompoundTagPtr Sand(CompoundTag const &in, CompoundTagPtr &out, int16_t damage) {
    std::string name;
    switch (damage) {
    case 1:
      name = "red_sand";
      break;
    case 0:
    default:
      name = "sand";
      break;
    }
    out->set("id", String("minecraft:" + name));
    return out;
  }

  static CompoundTagPtr Sandstone(CompoundTag const &in, CompoundTagPtr &out, int16_t damage) {
    std::string name;
    switch (damage) {
    case 2:
      name = "cut_sandstone";
      break;
    case 1:
      name = "chiseled_sandstone";
      break;
    case 0:
    default:
      name = "sandstone";
      break;
    }
    out->set("id", String("minecraft:" + name));
    return out;
  }

  static CompoundTagPtr Stone(CompoundTag const &in, CompoundTagPtr &out, int16_t damage) {
    std::string name = "stone";
    switch (damage) {
    case 1:
      name = "granite";
      break;
    case 2:
      name = "polished_granite";
      break;
    case 3:
      name = "diorite";
      break;
    case 4:
      name = "polished_diorite";
      break;
    case 5:
      name = "andesite";
      break;
    case 6:
      name = "polished_andesite";
      break;
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
    E(red_sandstone, RedSandstone);
    E(sand, Sand);
    E(sandstone, Sandstone);
    E(log, Log);
    E(planks, Planks);

#undef E
    return ret;
  }
};

} // namespace je2be::box360
