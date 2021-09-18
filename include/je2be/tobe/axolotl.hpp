#pragma once

namespace je2be::tobe {
class Axolotl {
public:
  struct Variant {
    int32_t const fBedrockRawValue;

    explicit Variant(int32_t bedrockRawValue) : fBedrockRawValue(bedrockRawValue) {
    }

    std::string definition() const {
      auto name = colorName();
      return "+axolotl_" + name;
    }

    std::string colorId() const {
      return "item.axolotlColor" + Cap(colorName()) + ".name";
    }

  private:
    std::string colorName() const {
      switch (fBedrockRawValue) {
      case 1:
        return "cyan";
      case 2:
        return "gold";
      case 3:
        return "wild";
      case 4:
        return "blue";
      case 0:
      default:
        return "lucy";
      }
    }
  };

  Axolotl(int32_t age, float health, int32_t javaVariant)
      : fAge(age), fHealth(health), fVariant(VariantFromJava(javaVariant)) {
  }

  std::shared_ptr<mcfile::nbt::CompoundTag> toBucketTag() {
    using namespace mcfile::nbt;
    using namespace std;
    using namespace props;

    auto ret = std::make_shared<mcfile::nbt::CompoundTag>();

    auto attributes = EntityAttributes::Mob("minecraft:axolotl");
    if (attributes) {
      attributes->health.updateCurrent(fHealth);
      ret->set("Attributes", attributes->toListTag());
    }
    string age = fAge < 0 ? "Baby" : "Adult";
    string bodyId = "item.axolotl" + age + "BodySingle.name";
    ret->set("BodyID", String(bodyId));

    ret->set("ColorID", String(fVariant.colorId()));
    ret->set("Variant", Int(fVariant.fBedrockRawValue));

    auto definitions = make_shared<mcfile::nbt::ListTag>(Tag::Type::String);
    definitions->push_back(String("+minecraft:axolotl"));
    definitions->push_back(String("+"));
    if (fAge < 0) {
      definitions->push_back(String("+axolotl_baby"));
    } else {
      definitions->push_back(String("+axolotl_adult"));
    }
    definitions->push_back(String(fVariant.definition()));
    definitions->push_back(String("-axolotl_on_land"));
    definitions->push_back(String("-axolotl_dried"));
    definitions->push_back(String("+axolotl_in_water"));
    ret->set("definitions", definitions);
    ret->set("identifier", String("minecraft:axolotl"));

    ret->set("AppendCustomName", Bool(true));
    ret->set("IsBaby", Bool(fAge < 0));
    ret->set("MarkVariant", Int(0));

    return ret;
  }

  static Variant VariantFromJava(int32_t javaVariant) {
    static const int variantMapping[5] = {0, 3, 2, 1, 4};
    int32_t v = variantMapping[std::clamp(javaVariant, 0, 4)];
    return Variant(v);
  }

private:
  static std::string Cap(std::string const &s) {
    if (s.empty()) {
      return s;
    }
    return std::string(1, (char)std::toupper(s[0])) + s.substr(1);
  }

private:
  int32_t fAge;
  float fHealth;
  Variant fVariant;
};
} // namespace je2be::tobe
