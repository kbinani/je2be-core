#pragma once

namespace je2be::tobe {

class EnchantData {
public:
  static std::shared_ptr<CompoundTag> From(CompoundTag const &item) {
    using namespace je2be::nbt;
    using namespace std;
    auto tag = Compound();
    auto name = item.string("id");
    if (!name) {
      return nullptr;
    }
    auto lvl = item.int16("lvl");
    if (!lvl) {
      return nullptr;
    }
    int16_t id = Enchantments::BedrockEnchantmentIdFromJava(*name);
    tag->set("id", Short(id));
    tag->set("lvl", Short(*lvl));
    return tag;
  }

private:
  EnchantData() = delete;
};

} // namespace je2be::tobe
