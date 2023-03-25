#pragma once

#include "entity/_entity-attributes.hpp"

namespace je2be::tobe {

class Axolotl {
public:
  struct Variant {
    i32 const fBedrockRawValue;

    explicit Variant(i32 bedrockRawValue) : fBedrockRawValue(bedrockRawValue) {
    }

    std::u8string definition() const {
      auto name = colorName();
      return u8"+axolotl_" + name;
    }

    std::u8string colorId() const {
      return u8"item.axolotlColor" + Cap(colorName()) + u8".name";
    }

  private:
    std::u8string colorName() const {
      switch (fBedrockRawValue) {
      case 1:
        return u8"cyan";
      case 2:
        return u8"gold";
      case 3:
        return u8"wild";
      case 4:
        return u8"blue";
      case 0:
      default:
        return u8"lucy";
      }
    }
  };

  Axolotl(i32 age, float health, i32 javaVariant)
      : fAge(age), fHealth(health), fVariant(VariantFromJava(javaVariant)) {
  }

  CompoundTagPtr toBucketTag() {
    using namespace std;

    auto ret = Compound();

    auto attributes = EntityAttributes::Mob(u8"minecraft:axolotl", fHealth);
    if (attributes) {
      ret->set(u8"Attributes", attributes->toBedrockListTag());
    }
    u8string age = fAge < 0 ? u8"Baby" : u8"Adult";
    u8string bodyId = u8"item.axolotl" + age + u8"BodySingle.name";
    ret->set(u8"BodyID", String(bodyId));

    ret->set(u8"ColorID", String(fVariant.colorId()));
    ret->set(u8"Variant", Int(fVariant.fBedrockRawValue));

    auto definitions = List<Tag::Type::String>();
    definitions->push_back(String(u8"+minecraft:axolotl"));
    definitions->push_back(String(u8"+"));
    if (fAge < 0) {
      definitions->push_back(String(u8"+axolotl_baby"));
    } else {
      definitions->push_back(String(u8"+axolotl_adult"));
    }
    definitions->push_back(String(fVariant.definition()));
    definitions->push_back(String(u8"-axolotl_on_land"));
    definitions->push_back(String(u8"-axolotl_dried"));
    definitions->push_back(String(u8"+axolotl_in_water"));
    ret->set(u8"definitions", definitions);
    ret->set(u8"identifier", String(u8"minecraft:axolotl"));

    ret->set(u8"AppendCustomName", Bool(true));
    ret->set(u8"IsBaby", Bool(fAge < 0));
    ret->set(u8"MarkVariant", Int(0));

    return ret;
  }

  static Variant VariantFromJava(i32 javaVariant) {
    static const int variantMapping[5] = {0, 3, 2, 1, 4};
    i32 v = variantMapping[std::clamp(javaVariant, 0, 4)];
    return Variant(v);
  }

private:
  static std::u8string Cap(std::u8string const &s) {
    if (s.empty()) {
      return s;
    }
    return std::u8string(1, (char8_t)std::toupper(s[0])) + s.substr(1);
  }

private:
  i32 fAge;
  float fHealth;
  Variant fVariant;
};

} // namespace je2be::tobe
