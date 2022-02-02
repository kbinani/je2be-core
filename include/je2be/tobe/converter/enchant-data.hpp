#pragma once

namespace je2be::tobe {

class EnchantData {
public:
  static std::shared_ptr<CompoundTag> From(CompoundTag const &item) {
    using namespace props;
    using namespace std;
    auto tag = std::make_shared<CompoundTag>();
    auto name = item.string("id");
    if (!name) {
      return nullptr;
    }
    auto lvl = item.int16("lvl");
    if (!lvl) {
      return nullptr;
    }
    int16_t id = Enchantments::BedrockEnchantmentIdFromJava(*name);
    tag->set("id", props::Short(id));
    tag->set("lvl", props::Short(*lvl));
    return tag;
  }

private:
  EnchantData() = delete;
};

} // namespace je2be::tobe
