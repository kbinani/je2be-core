#pragma once

namespace je2be::toje {

class Entity {
public:
  static std::shared_ptr<CompoundTag> ItemFrameFromBedrock(mcfile::Dimension d, Pos3i pos, mcfile::be::Block const &blockJ, CompoundTag const &blockEntityB, Context &ctx) {
    using namespace props;
    auto ret = std::make_shared<CompoundTag>();
    CompoundTag &t = *ret;
    if (blockJ.fName == "minecraft:glow_frame") {
      t["id"] = String("minecraft:glow_item_frame");
    } else {
      t["id"] = String("minecraft:item_frame");
    }

    int32_t facingDirectionA = blockJ.fStates->int32("facing_direction", 0);
    Facing6 f6 = Facing6FromBedrockFacingDirectionA(facingDirectionA);
    je2be::Rotation rot(0, 0);
    switch (f6) {
    case Facing6::Up:
      rot.fPitch = -90;
      break;
    case Facing6::North:
      rot.fYaw = 180;
      break;
    case Facing6::East:
      rot.fYaw = 270;
      break;
    case Facing6::South:
      rot.fYaw = 0;
      break;
    case Facing6::West:
      rot.fYaw = 90;
      break;
    case Facing6::Down:
      rot.fPitch = 90;
      break;
    }
    t["Rotation"] = rot.toListTag();
    t["Facing"] = Byte(facingDirectionA);

    t["TileX"] = Int(pos.fX);
    t["TileY"] = Int(pos.fY);
    t["TileZ"] = Int(pos.fZ);

    Pos3i direction = Pos3iFromFacing6(f6);
    double thickness = 15.0 / 32.0;
    Pos3d position(pos.fX + 0.5 - direction.fX * thickness, pos.fY + 0.5 - direction.fY * thickness, pos.fZ + 0.5 - direction.fZ * thickness);
    t["Pos"] = position.toListTag();

    XXHash64 h(0);
    h.add(&d, sizeof(d));
    h.add(&position.fX, sizeof(position.fX));
    h.add(&position.fY, sizeof(position.fY));
    h.add(&position.fZ, sizeof(position.fZ));
    auto uuid = Uuid::GenWithU64Seed(h.hash());
    t["UUID"] = uuid.toIntArrayTag();

    auto itemB = blockEntityB.compoundTag("Item");
    if (itemB) {
      auto itemJ = Item::From(*itemB, ctx);
      if (itemJ) {
        t["Item"] = itemJ;
      }
    }

    auto itemRotationB = blockEntityB.float32("ItemRotation", 0);
    t["ItemRotation"] = Byte(static_cast<int8_t>(std::roundf(itemRotationB / 45.0f)));

    Pos3d motion(0, 0, 0);
    t["Motion"] = motion.toListTag();

    CopyFloatValues(blockEntityB, t, {{"ItemDropChance"}});

    t["Air"] = Short(300);
    t["FallDistance"] = Float(0);
    t["Fire"] = Short(-1);
    t["Fixed"] = Bool(false);
    t["Invisible"] = Bool(false);
    t["Invulnerable"] = Bool(false);
    t["OnGround"] = Bool(false);
    t["PortalCooldown"] = Int(0);

    return ret;
  }

  static std::shared_ptr<CompoundTag> From(CompoundTag const &entityB, Context &ctx) {
    auto id = entityB.string("identifier");
    if (!id) {
      return nullptr;
    }
    auto const *table = GetTable();
    auto found = table->find(*id);
    if (found == table->end()) {
      return nullptr;
    }
    return found->second(*id, entityB, ctx);
  }

  using Converter = std::function<std::shared_ptr<CompoundTag>(std::string const &id, CompoundTag const &eneityB, Context &ctx)>;
  using Namer = std::function<std::string(std::string const &nameB, CompoundTag const &entityB)>;
  using Behavior = std::function<void(CompoundTag const &entityB, CompoundTag &entityJ, Context &ctx)>;

  struct Convert {
    template <class... Arg>
    Convert(Namer namer, Converter base, Arg... behaviors) : fNamer(namer), fBase(base), fBehaviors(std::initializer_list<Behavior>{behaviors...}) {}

    std::shared_ptr<CompoundTag> operator()(std::string const &id, CompoundTag const &entityB, Context &ctx) const {
      auto name = fNamer(id, entityB);
      auto t = fBase(id, entityB, ctx);
      t->set("id", props::String(name));
      for (auto behavior : fBehaviors) {
        behavior(entityB, *t, ctx);
      }
      return t;
    }

    Namer fNamer;
    Converter fBase;
    std::vector<Behavior> fBehaviors;
  };

#pragma region Namers
  static std::string Same(std::string const &nameB, CompoundTag const &entityB) {
    return nameB;
  }
#pragma endregion

#pragma region Dedicated Behaviors
  static void Skeleton(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    j["StrayConversionTime"] = props::Int(-1);
  }

  static void Creeper(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    using namespace props;
    j["ExplosionRadius"] = Byte(3);
    j["Fuse"] = Short(30);
    j["ignited"] = Bool(false);
  }

  static void Bat(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    CopyBoolValues(b, j, {{"BatFlags"}});
  }
#pragma endregion

#pragma region Behaviors
  static void AbsorptionAmount(CompoundTag const &entityB, CompoundTag &entityJ, Context &) {
    entityJ["AbsorptionAmount"] = props::Float(0);
  }

  static void Air(CompoundTag const &entityB, CompoundTag &entityJ, Context &ctx) {
    CopyShortValues(entityB, entityJ, {{"Air", "Air", 300}});
  }

  static void ArmorItems(CompoundTag const &entityB, CompoundTag &entityJ, Context &ctx) {
    auto armorsB = entityB.listTag("Armor");
    auto armorsJ = std::make_shared<ListTag>(Tag::Type::Compound);
    auto chances = std::make_shared<ListTag>(Tag::Type::Float);
    if (armorsB) {
      for (auto const &it : *armorsB) {
        auto armorB = it->asCompound();
        std::shared_ptr<CompoundTag> armorJ;
        if (armorB) {
          armorJ = Item::From(*armorB, ctx);
        }
        if (!armorJ) {
          armorJ = std::make_shared<CompoundTag>();
        }
        armorsJ->push_back(armorJ);
        chances->push_back(props::Float(0.085));
      }
    }
    entityJ["ArmorItems"] = armorsJ;
    entityJ["ArmorDropChances"] = chances;
  }

  static void Brain(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    auto memories = std::make_shared<CompoundTag>();
    auto brain = std::make_shared<CompoundTag>();
    brain->set("memories", memories);
    j["Brain"] = brain;
  }

  static void CanPickUpLoot(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    CopyBoolValues(b, j, {{"canPickupItems", "CanPickUpLoot"}});
  }

  static void DeathTime(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    CopyShortValues(b, j, {{"DeathTime"}});
  }

  static void FallDistance(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    CopyFloatValues(b, j, {{"FallDistance"}});
  }

  static void FallFlying(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    j["FallFlying"] = props::Bool(false);
  }

  static void Fire(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    j["Fire"] = props::Short(-1);
  }

  static void HandItems(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    auto itemsJ = std::make_shared<ListTag>(Tag::Type::Compound);
    auto chances = std::make_shared<ListTag>(Tag::Type::Float);
    for (std::string key : {"Mainhand", "Offhand"}) {
      auto listB = b.listTag(key);
      std::shared_ptr<CompoundTag> itemJ;
      if (listB && !listB->empty()) {
        auto c = listB->at(0)->asCompound();
        if (c) {
          itemJ = Item::From(*c, ctx);
        }
      }
      if (!itemJ) {
        itemJ = std::make_shared<CompoundTag>();
      }
      itemsJ->push_back(itemJ);
      chances->push_back(props::Float(0.085));
    }
    j["HandItems"] = itemsJ;
    j["HandDropChances"] = chances;
  }

  static void Health(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    auto attributesB = b.listTag("Attributes");
    if (!attributesB) {
      return;
    }
    for (auto const &it : *attributesB) {
      auto c = it->asCompound();
      if (!c) {
        continue;
      }
      auto name = c->string("Name");
      if (name != "minecraft:health") {
        continue;
      }
      auto current = c->float32("Current");
      if (!current) {
        continue;
      }
      j["Health"] = props::Float(*current);
      return;
    }
  }

  static void HurtByTimestamp(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    j["HurtByTimestamp"] = props::Int(0);
  }

  static void HurtTime(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    CopyShortValues(b, j, {{"HurtTime"}});
  }

  static void Invulnerable(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    CopyBoolValues(b, j, {{"Invulnerable"}});
  }

  static void LeftHanded(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    j["LeftHanded"] = props::Bool(false);
  }

  static void OnGround(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    CopyBoolValues(b, j, {{"OnGround"}});
  }

  static void Painting(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    auto directionB = b.byte("Direction", 0);
    Facing4 direction = Facing4FromBedrockDirection(directionB);
    auto motiveB = b.string("Motive", "Aztec");
    Painting::Motive motive = Painting::MotiveFromBedrock(motiveB);
    auto motiveJ = Painting::JavaFromMotive(motive);
    auto posB = b.listTag("Pos");
    if (!posB) {
      return;
    }
    if (posB->size() != 3) {
      return;
    }
    if (posB->fType != Tag::Type::Float) {
      return;
    }
    float x = posB->at(0)->asFloat()->fValue;
    float y = posB->at(1)->asFloat()->fValue;
    float z = posB->at(2)->asFloat()->fValue;

    auto tile = Painting::JavaTilePosFromBedrockPos(Pos3f(x, y, z), direction, motive);
    if (!tile) {
      return;
    }

    j["Motive"] = props::String(motiveJ);
    j["Facing"] = props::Byte(directionB);
    j["TileX"] = props::Int(std::round(tile->fX));
    j["TileY"] = props::Int(std::round(tile->fY));
    j["TileZ"] = props::Int(std::round(tile->fZ));
  }

  static void PersistenceRequired(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    j["PersistenceRequired"] = props::Bool(false);
  }

  static void PortalCooldown(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    CopyIntValues(b, j, {{"PortalCooldown"}});
  }

  static void Pos(CompoundTag const &entityB, CompoundTag &entityJ, Context &ctx) {
    auto posB = entityB.listTag("Pos");
    if (!posB) {
      return;
    }
    if (posB->size() != 3) {
      return;
    }
    if (posB->fType != Tag::Type::Float) {
      return;
    }
    double x = posB->at(0)->asFloat()->fValue;
    double y = posB->at(1)->asFloat()->fValue;
    double z = posB->at(2)->asFloat()->fValue;
    Pos3d posJ(x, y, z);
    entityJ["Pos"] = posJ.toListTag();
  }

  static void Rotation(CompoundTag const &entityB, CompoundTag &entityJ, Context &ctx) {
    auto rotB = entityB.listTag("Rotation");
    if (!rotB) {
      return;
    }
    if (rotB->size() != 2) {
      return;
    }
    if (rotB->fType != Tag::Type::Float) {
      return;
    }
    entityJ["Rotation"] = rotB->clone();
  }

  static void UUID(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    auto id = b.int64("UniqueID");
    if (!id) {
      return;
    }
    int64_t v = *id;
    Uuid uuid = Uuid::GenWithU64Seed(*(uint64_t *)&v);
    j["UUID"] = uuid.toIntArrayTag();
  }
#pragma endregion

#pragma region Converters
  static std::shared_ptr<CompoundTag> Base(std::string const &id, CompoundTag const &b, Context &ctx) {
    auto ret = std::make_shared<CompoundTag>();
    CompoundTag &j = *ret;
    Air(b, j, ctx);
    OnGround(b, j, ctx);
    Pos(b, j, ctx);
    Rotation(b, j, ctx);
    PortalCooldown(b, j, ctx);
    FallDistance(b, j, ctx);
    Fire(b, j, ctx);
    Invulnerable(b, j, ctx);
    UUID(b, j, ctx);
    return ret;
  }

  static std::shared_ptr<CompoundTag> LivingEntity(std::string const &id, CompoundTag const &b, Context &ctx) {
    auto ret = Base(id, b, ctx);
    CompoundTag &j = *ret;
    AbsorptionAmount(b, j, ctx);
    ArmorItems(b, j, ctx);
    Brain(b, j, ctx);
    CanPickUpLoot(b, j, ctx);
    DeathTime(b, j, ctx);
    FallFlying(b, j, ctx);
    HandItems(b, j, ctx);
    Health(b, j, ctx);
    HurtByTimestamp(b, j, ctx);
    HurtTime(b, j, ctx);
    LeftHanded(b, j, ctx);
    PersistenceRequired(b, j, ctx);
    return ret;
  }
#pragma endregion

  static std::unordered_map<std::string, Converter> const *GetTable() {
    static std::unique_ptr<std::unordered_map<std::string, Converter> const> const sTable(CreateTable());
    return sTable.get();
  }

  static std::unordered_map<std::string, Converter> *CreateTable() {
    auto ret = new std::unordered_map<std::string, Converter>();

#define E(__name, __conv)                                \
  assert(ret->find("minecraft:" #__name) == ret->end()); \
  ret->insert(std::make_pair("minecraft:" #__name, __conv));

    E(skeleton, Convert(Same, LivingEntity, Skeleton));
    E(creeper, Convert(Same, LivingEntity, Creeper));
    E(spider, Convert(Same, LivingEntity));
    E(bat, Convert(Same, LivingEntity, Bat));
    E(painting, Convert(Same, Base, Painting));

#undef E
    return ret;
  }
};

} // namespace je2be::toje
