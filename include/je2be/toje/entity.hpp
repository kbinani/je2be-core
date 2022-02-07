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

    auto itemRotationB = blockEntityB.float32("ItemRotation");
    if (itemRotationB) {
      t["ItemRotation"] = Byte(static_cast<int8_t>(std::roundf(*itemRotationB / 45.0f)));
    }

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

  struct Result {
    Uuid fUuid;
    std::shared_ptr<CompoundTag> fEntity;
  };

  static std::optional<Result> From(CompoundTag const &entityB, ChunkContext &ctx) {
    auto id = entityB.string("identifier");
    if (!id) {
      return std::nullopt;
    }
    auto uid = entityB.int64("UniqueID");
    if (!uid) {
      return std::nullopt;
    }
    int64_t v = *uid;
    Uuid uuid = Uuid::GenWithU64Seed(*(uint64_t *)&v);

    auto const *table = GetTable();
    auto found = table->find(*id);
    if (found == table->end()) {
      return std::nullopt;
    }
    auto e = found->second(*id, entityB, ctx);
    if (!e) {
      return std::nullopt;
    }
    e->set("UUID", uuid.toIntArrayTag());
    Passengers(uuid, entityB, *e, ctx);

    Result r;
    r.fUuid = uuid;
    r.fEntity = e;
    return r;
  }

  using Converter = std::function<std::shared_ptr<CompoundTag>(std::string const &id, CompoundTag const &eneityB, ChunkContext &ctx)>;
  using Namer = std::function<std::string(std::string const &nameB, CompoundTag const &entityB)>;
  using Behavior = std::function<void(CompoundTag const &entityB, CompoundTag &entityJ, ChunkContext &ctx)>;

  struct C {
    template <class... Arg>
    C(Namer namer, Converter base, Arg... behaviors) : fNamer(namer), fBase(base), fBehaviors(std::initializer_list<Behavior>{behaviors...}) {}

    std::shared_ptr<CompoundTag> operator()(std::string const &id, CompoundTag const &entityB, ChunkContext &ctx) const {
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
  static Namer Rename(std::string name) {
    return [name](std::string const &nameB, CompoundTag const &entityB) {
      return "minecraft:" + name;
    };
  }

  static std::string Same(std::string const &nameB, CompoundTag const &entityB) {
    return nameB;
  }
#pragma endregion

#pragma region Dedicated Behaviors
  static void ArmorStand(CompoundTag const &b, CompoundTag &j, ChunkContext &ctx) {
    j.erase("ArmorDropChances");
    j.erase("HandDropChances");
    j["DisabledSlots"] = props::Int(false);
    j["Invisible"] = props::Bool(false);
    j["NoBasePlate"] = props::Bool(false);
    j["Small"] = props::Bool(false);

    bool showArms = false;
    auto poseB = b.compoundTag("Pose");
    if (poseB) {
      auto poseIndexB = poseB->int32("PoseIndex");
      if (poseIndexB) {
        auto pose = ArmorStand::JavaPoseFromBedrockPoseIndex(*poseIndexB);
        auto poseJ = pose->toCompoundTag();
        if (!poseJ->empty()) {
          j["Pose"] = poseJ;
        }
        showArms = true;
      }
    }
    j["ShowArms"] = props::Bool(showArms);
  }

  static void Bat(CompoundTag const &b, CompoundTag &j, ChunkContext &ctx) {
    CopyBoolValues(b, j, {{"BatFlags"}});
  }

  static void Bee(CompoundTag const &b, CompoundTag &j, ChunkContext &ctx) {
    j["CannotEnterHiveTicks"] = props::Int(0);
    j["CropsGrownSincePollination"] = props::Int(0);
    auto hasNectar = HasDefinition(b, "+has_nectar");
    j["HasNectar"] = props::Bool(hasNectar);
    /*
    after stunged a player, bee has these definitions:
    "+minecraft:bee",
    "+",
    "+bee_adult",
    "+shelter_detection",
    "+normal_attack",
    "-track_attacker",
    "-take_nearest_target",
    "-look_for_food",
    "-angry_bee",
    "-find_hive",
    "+countdown_to_perish"
    */
    j["HasStung"] = props::Bool(false);
  }

  static void Boat(CompoundTag const &b, CompoundTag &j, ChunkContext &ctx) {
    auto variant = b.int32("Variant", 0);
    auto type = Boat::JavaTypeFromBedrockVariant(variant);
    j["Type"] = props::String(type);

    auto rotB = props::GetRotation(b, "Rotation");
    je2be::Rotation rotJ(Rotation::ClampDegreesBetweenMinus180And180(rotB->fYaw - 90), rotB->fPitch);
    j["Rotation"] = rotJ.toListTag();
  }

  static void Chicken(CompoundTag const &b, CompoundTag &j, ChunkContext &ctx) {
    auto entries = b.listTag("entries");
    if (entries) {
      for (auto const &it : *entries) {
        auto c = it->asCompound();
        if (!c) {
          continue;
        }
        auto spawnTimer = c->int32("SpawnTimer");
        if (!spawnTimer) {
          continue;
        }
        j["EggLayTime"] = props::Int(*spawnTimer);
        break;
      }
    }
  }

  static void Creeper(CompoundTag const &b, CompoundTag &j, ChunkContext &ctx) {
    using namespace props;
    j["ExplosionRadius"] = Byte(3);
    j["Fuse"] = Short(30);
    j["ignited"] = Bool(false);
    if (HasDefinition(b, "+minecraft:charged_creeper")) {
      j["powered"] = Bool(true);
    }
  }

  static void Enderman(CompoundTag const &b, CompoundTag &j, ChunkContext &ctx) {
    auto carriedBlockTagB = b.compoundTag("carriedBlock");
    if (carriedBlockTagB) {
      auto carriedBlockB = mcfile::be::Block::FromCompound(*carriedBlockTagB);
      if (carriedBlockB) {
        auto carriedBlockJ = BlockData::From(*carriedBlockB);
        if (carriedBlockJ) {
          auto carriedBlockTagJ = std::make_shared<CompoundTag>();
          carriedBlockTagJ->set("Name", props::String(carriedBlockJ->fName));
          j["carriedBlockState"] = carriedBlockTagJ;
        }
      }
    }
  }

  static void Fox(CompoundTag const &b, CompoundTag &j, ChunkContext &ctx) {
    auto variant = b.int32("Variant", 0);
    std::string type;
    if (variant == 1) {
      type = "snow";
    } else {
      type = "red";
    }
    j["Type"] = props::String(type);

    j["Sleeping"] = props::Bool(HasDefinition(b, "+minecraft:fox_ambient_sleep"));
    j["Crouching"] = props::Bool(false);

    auto trusted = std::make_shared<ListTag>(Tag::Type::IntArray);
    auto trustedPlayers = b.int32("TrustedPlayersAmount", 0);
    if (trustedPlayers > 0) {
      for (int i = 0; i < trustedPlayers; i++) {
        auto uuidB = b.int64("TrustedPlayer" + std::to_string(i));
        if (!uuidB) {
          continue;
        }
        Uuid uuidJ = Uuid::GenWithI64Seed(*uuidB);
        trusted->push_back(uuidJ.toIntArrayTag());
      }
    }
    j["Trusted"] = trusted;
  }

  static void Horse(CompoundTag const &b, CompoundTag &j, ChunkContext &ctx) {
    auto armorDropChances = std::make_shared<ListTag>(Tag::Type::Float);
    armorDropChances->push_back(props::Float(0.085));
    armorDropChances->push_back(props::Float(0.085));
    armorDropChances->push_back(props::Float(0));
    armorDropChances->push_back(props::Float(0.085));
    j["ArmorDropChances"] = armorDropChances;

    int32_t variantB = b.int32("Variant", 0);
    int32_t markVariantB = b.int32("MarkVariant", 0);
    uint32_t uVariantB = *(uint32_t *)&variantB;
    uint32_t uMarkVariantB = *(uint32_t *)&markVariantB;
    uint32_t uVariantJ = (0xf & uVariantB) | ((0xf & uMarkVariantB) << 8);
    j["Variant"] = props::Int(*(int32_t *)&uVariantJ);
  }

  static void Item(CompoundTag const &b, CompoundTag &j, ChunkContext &ctx) {
    j["PickupDelay"] = props::Short(0);
    auto itemB = b.compoundTag("Item");
    if (itemB) {
      auto itemJ = toje::Item::From(*itemB, ctx.fCtx);
      if (itemJ) {
        j["Item"] = itemJ;
      }
    }
    CopyShortValues(b, j, {{"Age"}, {"Health"}});
  }

  static void Parrot(CompoundTag const &b, CompoundTag &j, ChunkContext &ctx) {
    CopyIntValues(b, j, {{"Variant"}});
  }

  static void Sheep(CompoundTag const &b, CompoundTag &j, ChunkContext &ctx) {
    CopyBoolValues(b, j, {{"Sheared"}});
    CopyByteValues(b, j, {{"Color"}});
  }

  static void Skeleton(CompoundTag const &b, CompoundTag &j, ChunkContext &ctx) {
    j["StrayConversionTime"] = props::Int(-1);
  }

  static void Slime(CompoundTag const &b, CompoundTag &j, ChunkContext &ctx) {
    auto sizeB = b.byte("Size", 1);
    j["Size"] = props::Int(sizeB - 1);
  }

  static void Zombie(CompoundTag const &b, CompoundTag &j, ChunkContext &ctx) {
    j["DrownedConversionTime"] = props::Int(-1);
    j["CanBreakDoors"] = props::Bool(false);
    j["InWaterTime"] = props::Int(-1);
  }
#pragma endregion

#pragma region Behaviors
  static void AbsorptionAmount(CompoundTag const &b, CompoundTag &j, ChunkContext &ctx) {
    j["AbsorptionAmount"] = props::Float(0);
  }

  static void Age(CompoundTag const &b, CompoundTag &j, ChunkContext &ctx) {
    CopyIntValues(b, j, {{"Age", "Age", 0}});
  }

  static void Air(CompoundTag const &b, CompoundTag &j, ChunkContext &ctx) {
    CopyShortValues(b, j, {{"Air", "Air", 300}});
  }

  static void AngerTime(CompoundTag const &b, CompoundTag &j, ChunkContext &ctx) {
    j["AngerTime"] = props::Int(0);
  }

  static void ArmorItems(CompoundTag const &b, CompoundTag &j, ChunkContext &ctx) {
    auto armorsB = b.listTag("Armor");
    auto armorsJ = std::make_shared<ListTag>(Tag::Type::Compound);
    auto chances = std::make_shared<ListTag>(Tag::Type::Float);
    if (armorsB) {
      std::vector<std::shared_ptr<CompoundTag>> armors;
      for (auto const &it : *armorsB) {
        auto armorB = it->asCompound();
        std::shared_ptr<CompoundTag> armorJ;
        if (armorB) {
          armorJ = Item::From(*armorB, ctx.fCtx);
        }
        if (!armorJ) {
          armorJ = std::make_shared<CompoundTag>();
        }
        armors.push_back(armorJ);
        chances->push_back(props::Float(0.085));
      }
      if (armors.size() == 4) {
        armorsJ->push_back(armors[3]);
        armorsJ->push_back(armors[2]);
        armorsJ->push_back(armors[1]);
        armorsJ->push_back(armors[0]);
      }
    }
    j["ArmorItems"] = armorsJ;
    j["ArmorDropChances"] = chances;
  }

  static void Brain(CompoundTag const &b, CompoundTag &j, ChunkContext &ctx) {
    auto memories = std::make_shared<CompoundTag>();
    auto brain = std::make_shared<CompoundTag>();
    brain->set("memories", memories);
    j["Brain"] = brain;
  }

  static void Bred(CompoundTag const &b, CompoundTag &j, ChunkContext &ctx) {
    j["Bred"] = props::Bool(false);
  }

  static void CanPickUpLoot(CompoundTag const &b, CompoundTag &j, ChunkContext &ctx) {
    CopyBoolValues(b, j, {{"canPickupItems", "CanPickUpLoot"}});
  }

  static void ChestedHorse(CompoundTag const &b, CompoundTag &j, ChunkContext &ctx) {
    CopyBoolValues(b, j, {{"Chested", "ChestedHorse", false}});
  }

  static void CustomName(CompoundTag const &b, CompoundTag &j, ChunkContext &ctx) {
    auto name = b.string("CustomName");
    if (name) {
      nlohmann::json json;
      json["text"] = *name;
      j["CustomName"] = props::String(nlohmann::to_string(json));
    }
    j["PersistenceRequired"] = props::Bool(!!name);
  }

  static void DeathTime(CompoundTag const &b, CompoundTag &j, ChunkContext &ctx) {
    CopyShortValues(b, j, {{"DeathTime"}});
  }

  static void EatingHaystack(CompoundTag const &b, CompoundTag &j, ChunkContext &ctx) {
    j["EatingHaystack"] = props::Bool(false);
  }

  static void FallDistance(CompoundTag const &b, CompoundTag &j, ChunkContext &ctx) {
    CopyFloatValues(b, j, {{"FallDistance"}});
  }

  static void FallFlying(CompoundTag const &b, CompoundTag &j, ChunkContext &ctx) {
    j["FallFlying"] = props::Bool(false);
  }

  static void Fire(CompoundTag const &b, CompoundTag &j, ChunkContext &ctx) {
    j["Fire"] = props::Short(-1);
  }

  static void FromBucket(CompoundTag const &b, CompoundTag &j, ChunkContext &ctx) {
    auto persistent = b.boolean("Persistent", false);
    j["FromBucket"] = props::Bool(persistent);
  }

  static void HandItems(CompoundTag const &b, CompoundTag &j, ChunkContext &ctx) {
    auto itemsJ = std::make_shared<ListTag>(Tag::Type::Compound);
    auto chances = std::make_shared<ListTag>(Tag::Type::Float);
    for (std::string key : {"Mainhand", "Offhand"}) {
      auto listB = b.listTag(key);
      std::shared_ptr<CompoundTag> itemJ;
      if (listB && !listB->empty()) {
        auto c = listB->at(0)->asCompound();
        if (c) {
          itemJ = Item::From(*c, ctx.fCtx);
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

  static void Health(CompoundTag const &b, CompoundTag &j, ChunkContext &ctx) {
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

  static void HopperMinecart(CompoundTag const &b, CompoundTag &j, ChunkContext &ctx) {
    auto enabled = HasDefinition(b, "+minecraft:hopper_active");
    j["Enabled"] = props::Bool(enabled);

    j["TransferCooldown"] = props::Int(0);
  }

  static void HurtByTimestamp(CompoundTag const &b, CompoundTag &j, ChunkContext &ctx) {
    j["HurtByTimestamp"] = props::Int(0);
  }

  static void HurtTime(CompoundTag const &b, CompoundTag &j, ChunkContext &ctx) {
    CopyShortValues(b, j, {{"HurtTime"}});
  }

  static void InLove(CompoundTag const &b, CompoundTag &j, ChunkContext &ctx) {
    CopyIntValues(b, j, {{"InLove", "InLove", 0}});
  }

  static void Invulnerable(CompoundTag const &b, CompoundTag &j, ChunkContext &ctx) {
    CopyBoolValues(b, j, {{"Invulnerable"}});
  }

  static void IsBaby(CompoundTag const &b, CompoundTag &j, ChunkContext &ctx) {
    CopyBoolValues(b, j, {{"IsBaby"}});
  }

  static void Items(CompoundTag const &b, CompoundTag &j, ChunkContext &ctx) {
    auto chestItems = b.listTag("ChestItems");
    auto items = std::make_shared<ListTag>(Tag::Type::Compound);
    if (chestItems) {
      std::shared_ptr<CompoundTag> saddle;
      for (auto const &it : *chestItems) {
        auto itemB = it->asCompound();
        if (!itemB) {
          continue;
        }
        auto itemJ = Item::From(*itemB, ctx.fCtx);
        if (!itemJ) {
          continue;
        }
        auto slot = itemJ->byte("Slot");
        if (!slot) {
          continue;
        }
        if (*slot == 0) {
          itemJ->erase("Slot");
          saddle = itemJ;
        } else {
          itemJ->set("Slot", props::Byte(*slot + 1));
          items->push_back(itemJ);
        }
      }

      if (saddle) {
        j["SaddleItem"] = saddle;
      }
    }
    j["Items"] = items;
  }

  static void NoGravity(CompoundTag const &b, CompoundTag &j, ChunkContext &ctx) {
    j["NoGravity"] = props::Bool(true);
  }

  static void StorageMinecart(CompoundTag const &b, CompoundTag &j, ChunkContext &ctx) {
    auto itemsB = b.listTag("ChestItems");
    if (itemsB) {
      auto itemsJ = std::make_shared<ListTag>(Tag::Type::Compound);
      for (auto const &it : *itemsB) {
        auto itemB = it->asCompound();
        if (!itemB) {
          continue;
        }
        auto nameB = itemB->string("Name");
        if (!nameB) {
          continue;
        }
        auto itemJ = Item::From(*itemB, ctx.fCtx);
        if (!itemJ) {
          continue;
        }
        itemsJ->push_back(itemJ);
      }
      j["Items"] = itemsJ;
    }

    auto posB = props::GetPos3f(b, "Pos");
    auto onGround = b.boolean("OnGround");
    if (posB && onGround) {
      Pos3d posJ = posB->toD();
      if (*onGround) {
        // JE: GroundLevel
        // BE: GroundLevel + 0.35
        posJ.fY = posB->fY - 0.35;
      } else {
        // JE: GroundLevel + 0.0625
        // BE: GroundLevel + 0.5
        posJ.fY = posB->fY - 0.5 + 0.0625;
      }
      j["Pos"] = posJ.toListTag();
    }
  }

  static void Tame(CompoundTag const &b, CompoundTag &j, ChunkContext &ctx) {
    CopyBoolValues(b, j, {{"IsTamed", "Tame", false}});
  }

  static void Temper(CompoundTag const &b, CompoundTag &j, ChunkContext &ctx) {
    CopyIntValues(b, j, {{"Temper", "Temper", 0}});
  }

  static void LeftHanded(CompoundTag const &b, CompoundTag &j, ChunkContext &ctx) {
    j["LeftHanded"] = props::Bool(false);
  }

  static void OnGround(CompoundTag const &b, CompoundTag &j, ChunkContext &ctx) {
    CopyBoolValues(b, j, {{"OnGround"}});
  }

  static void Owner(CompoundTag const &b, CompoundTag &j, ChunkContext &ctx) {
    auto ownerNew = b.int64("OwnerNew");
    if (!ownerNew) {
      return;
    }
    Uuid uuid = Uuid::GenWithI64Seed(*ownerNew);
    j["Owner"] = uuid.toIntArrayTag();
  }

  static void Painting(CompoundTag const &b, CompoundTag &j, ChunkContext &ctx) {
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

  static void PortalCooldown(CompoundTag const &b, CompoundTag &j, ChunkContext &ctx) {
    CopyIntValues(b, j, {{"PortalCooldown"}});
  }

  static void Pos(CompoundTag const &b, CompoundTag &j, ChunkContext &ctx) {
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
    double x = posB->at(0)->asFloat()->fValue;
    double y = posB->at(1)->asFloat()->fValue;
    double z = posB->at(2)->asFloat()->fValue;
    Pos3d posJ(x, y, z);
    j["Pos"] = posJ.toListTag();
  }

  static void Rotation(CompoundTag const &b, CompoundTag &j, ChunkContext &ctx) {
    auto rotB = b.listTag("Rotation");
    if (!rotB) {
      return;
    }
    if (rotB->size() != 2) {
      return;
    }
    if (rotB->fType != Tag::Type::Float) {
      return;
    }
    j["Rotation"] = rotB->clone();
  }

  static void Saddle(CompoundTag const &b, CompoundTag &j, ChunkContext &ctx) {
    j["Saddle"] = props::Bool(HasDefinitionWithPrefixAndSuffix(b, "+minecraft:", "_saddled"));
  }

  static void ShowBottom(CompoundTag const &b, CompoundTag &j, ChunkContext &ctx) {
    CopyBoolValues(b, j, {{"ShowBottom"}});
  }

  static void Sitting(CompoundTag const &b, CompoundTag &j, ChunkContext &ctx) {
    CopyBoolValues(b, j, {{"Sitting", "Sitting", false}});
  }
#pragma endregion

#pragma region Converters
  static std::shared_ptr<CompoundTag> Animal(std::string const &id, CompoundTag const &b, ChunkContext &ctx) {
    auto ret = LivingEntity(id, b, ctx);
    CompoundTag &j = *ret;
    Age(b, j, ctx);
    InLove(b, j, ctx);
    return ret;
  }

  static std::shared_ptr<CompoundTag> Base(std::string const &id, CompoundTag const &b, ChunkContext &ctx) {
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
    return ret;
  }

  static std::shared_ptr<CompoundTag> LivingEntity(std::string const &id, CompoundTag const &b, ChunkContext &ctx) {
    auto ret = Base(id, b, ctx);
    CompoundTag &j = *ret;
    AbsorptionAmount(b, j, ctx);
    ArmorItems(b, j, ctx);
    Brain(b, j, ctx);
    CanPickUpLoot(b, j, ctx);
    CustomName(b, j, ctx);
    DeathTime(b, j, ctx);
    FallFlying(b, j, ctx);
    HandItems(b, j, ctx);
    Health(b, j, ctx);
    HurtByTimestamp(b, j, ctx);
    HurtTime(b, j, ctx);
    LeftHanded(b, j, ctx);
    return ret;
  }
#pragma endregion

#pragma region Utilities
  static bool HasDefinition(CompoundTag const &entityB, std::string const &definitionToSearch) {
    auto definitions = entityB.listTag("definitions");
    if (!definitions) {
      return false;
    }
    for (auto const &it : *definitions) {
      auto definition = it->asString();
      if (!definition) {
        continue;
      }
      if (definition->fValue == definitionToSearch) {
        return true;
      }
    }
    return false;
  }

  static bool HasDefinitionWithPrefixAndSuffix(CompoundTag const &entityB, std::string const &prefix, std::string const &suffix) {
    auto definitions = entityB.listTag("definitions");
    if (!definitions) {
      return false;
    }
    for (auto const &it : *definitions) {
      auto definition = it->asString();
      if (!definition) {
        continue;
      }
      if (!prefix.empty() && !definition->fValue.starts_with(prefix)) {
        continue;
      }
      if (!suffix.empty() && !definition->fValue.ends_with(suffix)) {
        continue;
      }
      return true;
    }
    return false;
  }

  static void Passengers(Uuid const &uid, CompoundTag const &b, CompoundTag &j, ChunkContext &ctx) {
    auto links = b.listTag("LinksTag");
    if (!links) {
      return;
    }
    for (size_t index = 0; index < links->size(); index++) {
      auto link = links->at(index)->asCompound();
      if (!link) {
        continue;
      }
      auto id = link->int64("entityID");
      if (!id) {
        continue;
      }
      Uuid passengerUid = Uuid::GenWithI64Seed(*id);
      ctx.fPassengers[uid].insert(std::make_pair(index, passengerUid));
    }
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

    E(skeleton, C(Same, LivingEntity, Skeleton));
    E(creeper, C(Same, LivingEntity, Creeper));
    E(spider, C(Same, LivingEntity));
    E(bat, C(Same, LivingEntity, Bat));
    E(painting, C(Same, Base, Painting));
    E(zombie, C(Same, LivingEntity, IsBaby, Zombie));
    E(chicken, C(Same, Animal, Chicken));
    E(item, C(Same, Base, Entity::Item));
    E(armor_stand, C(Same, Base, AbsorptionAmount, ArmorItems, Brain, DeathTime, FallFlying, HandItems, Health, HurtByTimestamp, HurtTime, ArmorStand));
    E(ender_crystal, C(Rename("end_crystal"), Base, ShowBottom));
    E(chest_minecart, C(Same, Base, StorageMinecart));
    E(hopper_minecart, C(Same, Base, StorageMinecart, HopperMinecart));
    E(boat, C(Same, Base, Boat));
    E(slime, C(Same, LivingEntity, Slime));
    E(salmon, C(Same, LivingEntity, FromBucket));
    E(parrot, C(Same, Animal, Owner, Sitting, Parrot));
    E(enderman, C(Same, LivingEntity, AngerTime, Enderman));
    E(zombie_pigman, C(Rename("zombified_piglin"), LivingEntity, AngerTime, IsBaby, Zombie));
    E(bee, C(Same, Animal, AngerTime, NoGravity, Bee));
    E(blaze, C(Same, LivingEntity));
    E(cow, C(Same, Animal));
    E(elder_guardian, C(Same, LivingEntity));
    E(cod, C(Same, LivingEntity, FromBucket));
    E(fox, C(Same, Animal, Sitting, Fox));
    E(pig, C(Same, Animal, Saddle));
    E(zoglin, C(Same, LivingEntity));
    E(horse, C(Same, Animal, Bred, EatingHaystack, Tame, Temper, Horse));
    E(husk, C(Same, LivingEntity, IsBaby, Zombie));
    E(sheep, C(Same, Animal, Sheep));
    E(cave_spider, C(Same, LivingEntity));
    E(donkey, C(Same, Animal, Bred, ChestedHorse, EatingHaystack, Items, Tame, Temper));

#undef E
    return ret;
  }
};

} // namespace je2be::toje
