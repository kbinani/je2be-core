#pragma once

#include "item/_enchantments.hpp"

namespace je2be::tobe {

class EnchantData {
public:
  static CompoundTagPtr From(CompoundTag const &item) {
    using namespace std;
    auto tag = Compound();
    auto name = item.string("id");
    if (!name) {
      auto i16Id = item.int16("id");
      if (!i16Id) {
        return nullptr;
      }
      auto n = Enchantments::JavaEnchantmentIdFromLegacyJava(*i16Id);
      if (!n) {
        return nullptr;
      }
      name = *n;
    }
    auto lvl = item.int16("lvl");
    if (!lvl) {
      return nullptr;
    }
    i16 id = Enchantments::BedrockEnchantmentIdFromJava(*name);
    tag->set("id", Short(id));
    tag->set("lvl", Short(*lvl));
    return tag;
  }

private:
  EnchantData() = delete;
};

} // namespace je2be::tobe
