#pragma once

namespace j2b {
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

  static std::shared_ptr<mcfile::nbt::CompoundTag> EmptyItem() {
    auto armor = std::make_shared<mcfile::nbt::CompoundTag>();
    armor->set("Count", props::Byte(0));
    armor->set("Damage", props::Short(0));
    armor->set("Name", props::String(""));
    armor->set("WasPickedUp", props::Bool(false));
    return armor;
  }

  std::shared_ptr<mcfile::nbt::CompoundTag> toBucketTag() {
    using namespace mcfile::nbt;
    using namespace std;
    using namespace props;

    auto ret = std::make_shared<mcfile::nbt::CompoundTag>();

    auto armors = make_shared<ListTag>(Tag::TAG_Compound);
    for (int i = 0; i < 4; i++) {
      auto armor = EmptyItem();
      armors->push_back(armor);
    }
    ret->set("Armor", armors);

    auto attributes = EntityAttributes::Mob("minecraft:axolotl");
    if (attributes) {
      ret->set("Attributes", attributes);
    }
    string age = fAge < 0 ? "Baby" : "Adult";
    string bodyId = "item.axolotl" + age + "BodySingle.name";
    ret->set("BodyID", String(bodyId));

    string colorId = "item.axolotlColor" + Cap(fVariant.colorName()) + ".name";
    ret->set("ColorID", String(colorId));

    auto mainhand = make_shared<ListTag>(Tag::TAG_Compound);
    mainhand->push_back(EmptyItem());
    ret->set("Mainhand", mainhand);

    auto offhand = make_shared<ListTag>(Tag::TAG_Compound);
    offhand->push_back(EmptyItem());
    ret->set("Offhand", offhand);

    ret->set("Variant", Int(fVariant.fBedrockRawValue));

    auto definitions = make_shared<mcfile::nbt::ListTag>(Tag::TAG_String);
    definitions->push_back(String("+minecraft:axolotl"));
    definitions->push_back(String("+"));
    if (fAge < 0) {
      definitions->push_back(String("+axolotl_baby"));
    } else {
      definitions->push_back(String("+axolotl_adult"));
    }
    definitions->push_back(String("+axolotl_" + fVariant.colorName()));
    definitions->push_back(String("-axolotl_on_land"));
    definitions->push_back(String("-axolotl_dried"));
    definitions->push_back(String("+axolotl_in_water"));
    ret->set("definitions", definitions);
    ret->set("identifier", String("minecraft:axolotl"));

    ret->set("Air", Short(300));
    ret->set("AppendCustomName", Bool(true));
    ret->set("AttackTime", Short(0));
    ret->set("BodyRot", Float(0));
    ret->set("BreedCooldown", Int(0));
    ret->set("Chested", Bool(false));
    ret->set("Color", Byte(0));
    ret->set("Color2", Byte(0));
    ret->set("Dead", Bool(false));
    ret->set("DeadTime", Short(0));
    ret->set("FallDistance", Float(0));
    ret->set("Fire", Short(0));
    ret->set("HurtTime", Short(0));
    ret->set("InLove", Int(0));
    ret->set("Invulnerable", Bool(false));
    ret->set("IsAngry", Bool(false));
    ret->set("IsAutonomous", Bool(false));
    ret->set("IsBaby", Bool(fAge < 0));
    ret->set("IsEating", Bool(false));
    ret->set("IsGliding", Bool(false));
    ret->set("IsGlobal", Bool(false));
    ret->set("IsIllagerCaptain", Bool(false));
    ret->set("IsOrphaned", Bool(false));
    ret->set("IsOutOfControl", Bool(false));
    ret->set("IsPregnant", Bool(false));
    ret->set("IsRoaring", Bool(false));
    ret->set("IsScared", Bool(false));
    ret->set("IsStunned", Bool(false));
    ret->set("IsSwimming", Bool(false));
    ret->set("IsTamed", Bool(false));
    ret->set("IsTrusting", Bool(false));
    ret->set("LastDimensionId", Int(0));
    ret->set("LeasherID", Long(-1));
    ret->set("LootDropped", Bool(false));
    ret->set("LoveCause", Long(0));
    ret->set("MarkVariant", Int(0));
    ret->set("Motion", Vec(0, 0, 0).toListTag());
    ret->set("NaturalSpawn", Bool(false));
    ret->set("OnGround", Bool(false));
    ret->set("OwnerNew", Long(-1));
    ret->set("Persistent", Bool(true));
    ret->set("PortalCooldown", Int(0));
    ret->set("Pos", Vec(0, 0, 0).toListTag());
    ret->set("Rotation", Rotation(0, 0).toListTag());
    ret->set("Saddled", Bool(false));
    ret->set("Sheared", Bool(false));
    ret->set("ShowBottom", Bool(false));
    ret->set("Sitting", Bool(false));
    ret->set("SkinID", Int(0));
    ret->set("Strength", Int(0));
    ret->set("StrengthMax", Int(0));
    ret->set("Surface", Bool(0));
    ret->set("TargetID", Long(-1));
    ret->set("TradeExperience", Int(0));
    ret->set("TradeTier", Int(0));
    ret->set("boundX", Int(0));
    ret->set("boundY", Int(0));
    ret->set("boundZ", Int(0));
    ret->set("canPickupItems", Bool(false));
    ret->set("hasBoundOrigin", Bool(false));
    ret->set("hasSetCanPickupItems", Bool(true));
    ret->set("limitedLife", Bool(false));

    //TODO: make UniqueID stable / reproducible
    ret->set("UniqueID", Long(NextUuid()));

    return ret;
  }

  static int64_t NextUuid() {
    std::random_device r;
    std::seed_seq seed2{r(), r(), r(), r(), r(), r(), r(), r()};
    std::mt19937_64 e2(seed2);
    return e2();
  }

  static Variant VariantFromJava(int32_t javaVariant) {
    static const int variantMapping[5] = {0, 3, 2, 1, 4};
    int32_t v = variantMapping[Clamp(javaVariant, 0, 4)];
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
} // namespace j2b
