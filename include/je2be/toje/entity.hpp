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

    std::map<size_t, Uuid> fPassengers;
    std::optional<int64_t> fLeasherId;
  };

  static std::optional<Result> From(CompoundTag const &entityB, Context &ctx) {
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

    Result r;

    e->set("UUID", uuid.toIntArrayTag());
    Passengers(uuid, entityB, r.fPassengers);

    auto leasherId = entityB.int64("LeasherID", -1);
    if (leasherId != -1) {
      r.fLeasherId = leasherId;
    }

    r.fUuid = uuid;
    r.fEntity = e;
    return r;
  }

  using Converter = std::function<std::shared_ptr<CompoundTag>(std::string const &id, CompoundTag const &eneityB, Context &ctx)>;
  using Namer = std::function<std::string(std::string const &nameB, CompoundTag const &entityB)>;
  using Behavior = std::function<void(CompoundTag const &entityB, CompoundTag &entityJ, Context &ctx)>;

  struct C {
    template <class... Arg>
    C(Namer namer, Converter base, Arg... behaviors) : fNamer(namer), fBase(base), fBehaviors(std::initializer_list<Behavior>{behaviors...}) {}

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
  static void ArmorStand(CompoundTag const &b, CompoundTag &j, Context &ctx) {
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

  static void Axolotl(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    auto variantB = b.int32("Variant", 0);
    j["Variant"] = props::Int(je2be::Axolotl::JavaVariantFromBedrockVariant(variantB));
  }

  static void Bat(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    CopyBoolValues(b, j, {{"BatFlags"}});
  }

  static void Bee(CompoundTag const &b, CompoundTag &j, Context &ctx) {
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

  static void Boat(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    auto variant = b.int32("Variant", 0);
    auto type = Boat::JavaTypeFromBedrockVariant(variant);
    j["Type"] = props::String(type);

    auto rotB = props::GetRotation(b, "Rotation");
    je2be::Rotation rotJ(Rotation::ClampDegreesBetweenMinus180And180(rotB->fYaw - 90), rotB->fPitch);
    j["Rotation"] = rotJ.toListTag();
  }

  static void Cat(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    auto variantB = b.int32("Variant", 8);
    int32_t catType = Cat::JavaCatTypeFromBedrockVariant(variantB);
    j["CatType"] = props::Int(catType);
  }

  static void ChestMinecart(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    if (auto st = LootTable::BedrockToJava(b, j); st == LootTable::State::HasLootTable) {
      j.erase("Items");
    }
  }

  static void Chicken(CompoundTag const &b, CompoundTag &j, Context &ctx) {
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
    j["IsChickenJockey"] = props::Bool(false);
  }

  static void Creeper(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    using namespace props;
    j["ExplosionRadius"] = Byte(3);
    j["Fuse"] = Short(30);
    j["ignited"] = Bool(false);
    if (HasDefinition(b, "+minecraft:charged_creeper")) {
      j["powered"] = Bool(true);
    }
  }

  static void Enderman(CompoundTag const &b, CompoundTag &j, Context &ctx) {
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

  static void Endermite(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    CopyIntValues(b, j, {{"Lifetime"}});
  }

  static void Evoker(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    j["SpellTicks"] = props::Int(0);
    /* {
      "Chested": 0, // byte
      "Color": 0, // byte
      "Color2": 0, // byte
      "FallDistance": 0, // float
      "Invulnerable": 0, // byte
      "IsAngry": 0, // byte
      "IsAutonomous": 0, // byte
      "IsBaby": 0, // byte
      "IsEating": 0, // byte
      "IsGliding": 0, // byte
      "IsGlobal": 0, // byte
      "IsIllagerCaptain": 0, // byte
      "IsOrphaned": 0, // byte
      "IsOutOfControl": 0, // byte
      "IsRoaring": 0, // byte
      "IsScared": 0, // byte
      "IsStunned": 0, // byte
      "IsSwimming": 0, // byte
      "IsTamed": 0, // byte
      "IsTrusting": 0, // byte
      "LastDimensionId": 0, // int
      "LeasherID": -1, // long
      "LootDropped": 0, // byte
      "MarkVariant": 0, // int
      "Motion": [
        0, // float
        0, // float
        0 // float
      ],
      "OnGround": 1, // byte
      "OwnerNew": -1644972474284, // long
      "PortalCooldown": 0, // int
      "Pos": [
        7, // float
        96, // float
        3 // float
      ],
      "Rotation": [
        0, // float
        0 // float
      ],
      "Saddled": 0, // byte
      "Sheared": 0, // byte
      "ShowBottom": 0, // byte
      "Sitting": 0, // byte
      "SkinID": 0, // int
      "Strength": 0, // int
      "StrengthMax": 0, // int
      "Tags": [
      ],
      "TradeExperience": 0, // int
      "TradeTier": 0, // int
      "UniqueID": -1649267441647, // long
      "Variant": 0, // int
      "canPickupItems": 1, // byte
      "definitions": [
      ],
      "hasSetCanPickupItems": 1, // byte
      "identifier": "minecraft:evocation_fang",
      "internalComponents": {
        "ActorLimitedLifetimeComponent": {
          "limitedLife": 12 // int
        }
      }
    */
  }

  static void Fox(CompoundTag const &b, CompoundTag &j, Context &ctx) {
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

  static void Ghast(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    j["ExplosionPower"] = props::Byte(1);
  }

  static void GlowSquid(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    j["DarkTicksRemaining"] = props::Int(0);
  }

  static void Goat(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    bool screamer = HasDefinition(b, "+goat_screamer") || HasDefinition(b, "+ram_screamer") || HasDefinition(b, "+interact_screamer");
    j["IsScreamingGoat"] = props::Bool(screamer);
  }

  static void Horse(CompoundTag const &b, CompoundTag &j, Context &ctx) {
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

  static void IronGolem(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    auto target = b.int64("TargetID", -1);
    if (target != -1) {
      auto angryAt = Uuid::GenWithI64Seed(target);
      j["AngryAt"] = angryAt.toIntArrayTag();
    }

    j["PlayerCreated"] = props::Bool(HasDefinition(b, "+minecraft:player_created"));
  }

  static void Item(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    j["PickupDelay"] = props::Short(0);
    auto itemB = b.compoundTag("Item");
    if (itemB) {
      auto itemJ = toje::Item::From(*itemB, ctx);
      if (itemJ) {
        j["Item"] = itemJ;
      }
    }
    CopyShortValues(b, j, {{"Age"}, {"Health"}});
  }

  static void Mooshroom(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    auto variant = b.int32("Variant", 0);
    std::string type = "red";
    if (variant == 1) {
      type = "brown";
    }
    j["Type"] = props::String(type);
  }

  static void Panda(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    auto genes = b.listTag("GeneArray");
    if (genes) {
      std::optional<int32_t> main;
      std::optional<int32_t> hidden;
      for (auto const &it : *genes) {
        auto gene = it->asCompound();
        if (!gene) {
          continue;
        }
        auto m = gene->int32("MainAllele");
        auto h = gene->int32("HiddenAllele");
        if (m && h) {
          main = m;
          hidden = h;
          break;
        }
      }
      if (main && hidden) {
        j["MainGene"] = props::String(Panda::JavaGeneNameFromGene(Panda::GeneFromBedrockAllele(*main)));
        j["HiddenGene"] = props::String(Panda::JavaGeneNameFromGene(Panda::GeneFromBedrockAllele(*hidden)));
      }
    }
  }

  static void Phantom(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    j["Size"] = props::Int(0);
  }

  static void PiglinBrute(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    auto homePos = props::GetPos3f(b, "HomePos");
    if (!homePos) {
      return;
    }
    auto homeDimensionId = b.int32("HomeDimensionId");
    if (!homeDimensionId) {
      return;
    }
    auto dim = DimensionFromBedrockDimension(*homeDimensionId);
    if (!dim) {
      return;
    }

    auto value = std::make_shared<CompoundTag>();
    value->set("dimension", props::String(JavaStringFromDimension(*dim)));
    std::vector<int32_t> pos;
    pos.push_back((int)round(homePos->fX));
    pos.push_back((int)round(homePos->fY));
    pos.push_back((int)round(homePos->fZ));
    auto posTag = std::make_shared<IntArrayTag>(pos);
    value->set("pos", posTag);

    auto homeTag = std::make_shared<CompoundTag>();
    homeTag->set("value", value);

    auto memories = std::make_shared<CompoundTag>();
    memories->set("minecraft:home", homeTag);

    auto brain = std::make_shared<CompoundTag>();
    brain->set("memories", memories);

    j["Brain"] = brain;
  }

  static void Pufferfish(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    int state = 0;
    if (HasDefinition(b, "+minecraft:half_puff_primary") || HasDefinition(b, "+minecraft:half_puff_secondary")) {
      state = 1;
    } else if (HasDefinition(b, "+minecraft:full_puff")) {
      state = 2;
    }
    j["PuffState"] = props::Int(state);
  }

  static void Rabbit(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    CopyIntValues(b, j, {{"MoreCarrotTicks"}});
    CopyIntValues(b, j, {{"Variant", "RabbitType", 0}});
  }

  static void Ravager(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    j["RoarTick"] = props::Int(0);
    j["StunTick"] = props::Int(0);
  }

  static void Sheep(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    CopyBoolValues(b, j, {{"Sheared"}});
    CopyByteValues(b, j, {{"Color"}});
  }

  static void Shulker(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    auto variant = b.int32("Variant", 16);
    j["Color"] = props::Byte(variant);
  }

  static void SkeletonHorse(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    // summon minecraft:skeleton_horse ~ ~ ~ minecraft:set_trap
    j["SkeletonTrap"] = props::Bool(HasDefinition(b, "+minecraft:skeleton_trap"));
    j["SkeletonTrapTime"] = props::Int(0);
  }

  static void SnowGolem(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    bool pumpkin = !HasDefinition(b, "+minecraft:snowman_sheared");
    j["Pumpkin"] = props::Bool(pumpkin);
  }

  static void TntMinecart(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    auto fuse = b.byte("Fuse", -1);
    j["TNTFuse"] = props::Int(fuse);
  }

  static void TropicalFish(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    auto tf = TropicalFish::FromBedrockBucketTag(b);
    j["Variant"] = props::Int(tf.toJavaVariant());
  }

  static void Turtle(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    auto homePos = props::GetPos3f(b, "HomePos");
    if (homePos) {
      j["HomePosX"] = props::Int(roundf(homePos->fX));
      j["HomePosY"] = props::Int(roundf(homePos->fY));
      j["HomePosZ"] = props::Int(roundf(homePos->fZ));
    }

    j["HasEgg"] = props::Bool(HasDefinition(b, "-minecraft:wants_to_lay_egg") || HasDefinition(b, "+minecraft:wants_to_lay_egg"));
  }

  static void Villager(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    CopyIntValues(b, j, {{"TradeExperience", "Xp", 0}});

    j.erase("InLove");

    auto dataJ = std::make_shared<CompoundTag>();

    VillagerProfession profession(static_cast<VillagerProfession::Variant>(0));
    auto variantB = b.int32("Variant", 0);
    if (variantB == 0) {
      // "Variant" of zombie villager is always 0
      auto definitions = b.listTag("definitions");
      if (definitions) {
        for (auto const &it : *definitions) {
          auto str = it->asString();
          if (!str) {
            continue;
          }
          std::string def = str->fValue;
          if (!def.starts_with("+")) {
            continue;
          }
          auto p = VillagerProfession::FromJavaProfession(def.substr(1));
          if (p) {
            profession = *p;
            break;
          }
        }
      }
    } else {
      auto professionVariant = static_cast<VillagerProfession::Variant>(variantB);
      profession = VillagerProfession(professionVariant);
    }
    dataJ->set("profession", props::String("minecraft:" + profession.string()));

    VillagerType::Variant variant = VillagerType::Plains;
    auto markVariantB = b.int32("MarkVariant", 0);
    if (markVariantB == 0) {
      /*
        Plains: "+villager_skin_1"
        Desert: "+villager_skin_4", "+desert_villager"
        Jungle: "+villager_skin_1", "+jungle_villager"
        Savanna: "+villager_skin_5", "+savanna_villager"
        Snow: "+villager_skin_3", "+snow_villager"
        Swamp: "+villager_skin_1", "+swamp_villager"
        Taiga: "+villager_skin_2"
      */
      if (HasDefinition(b, "+desert_villager")) {
        variant = VillagerType::Desert;
      } else if (HasDefinition(b, "+jungle_villager")) {
        variant = VillagerType::Jungle;
      } else if (HasDefinition(b, "+savanna_villager")) {
        variant = VillagerType::Savanna;
      } else if (HasDefinition(b, "+snow_villager")) {
        variant = VillagerType::Snow;
      } else if (HasDefinition(b, "+swamp_villager")) {
        variant = VillagerType::Swamp;
      } else if (HasDefinition(b, "+villager_skin_2")) {
        variant = VillagerType::Taiga;
      }
    } else {
      variant = static_cast<VillagerType::Variant>(markVariantB);
    }
    VillagerType type(variant);
    dataJ->set("type", props::String("minecraft:" + type.string()));

    auto tradeTier = b.int32("TradeTier", 0);
    dataJ->set("level", props::Int(tradeTier + 1));

    j["VillagerData"] = dataJ;
  }

  static void WanderingTrader(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    /*
      // spawned with spawn egg
      "entries": [
        {
          "SpawnTimer": -1, // int
          "StopSpawning": 1 // byte
        }
      ],
  */
    j["DespawnDelay"] = props::Int(0);
  }

  static void Wither(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    j["Invul"] = props::Int(0);
  }

  static void Zombie(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    j["DrownedConversionTime"] = props::Int(-1);
    j["CanBreakDoors"] = props::Bool(false);
    j["InWaterTime"] = props::Int(-1);
  }
#pragma endregion

#pragma region Behaviors
  static void AbsorptionAmount(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    j["AbsorptionAmount"] = props::Float(0);
  }

  static void Age(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    CopyIntValues(b, j, {{"Age", "Age", 0}});
  }

  static void Air(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    CopyShortValues(b, j, {{"Air", "Air", 300}});
  }

  static void AngerTime(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    j["AngerTime"] = props::Int(0);
  }

  static void ArmorItems(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    auto armorsB = b.listTag("Armor");
    auto armorsJ = std::make_shared<ListTag>(Tag::Type::Compound);
    auto chances = std::make_shared<ListTag>(Tag::Type::Float);
    if (armorsB) {
      std::vector<std::shared_ptr<CompoundTag>> armors;
      for (auto const &it : *armorsB) {
        auto armorB = it->asCompound();
        std::shared_ptr<CompoundTag> armorJ;
        if (armorB) {
          armorJ = Item::From(*armorB, ctx);
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

  static void AttackTick(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    auto attackTime = b.int16("AttackTime");
    if (attackTime) {
      j["AttackTick"] = props::Int(*attackTime);
    }
  }

  static void Brain(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    auto memories = std::make_shared<CompoundTag>();
    auto brain = std::make_shared<CompoundTag>();
    brain->set("memories", memories);
    j["Brain"] = brain;
  }

  static void Bred(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    j["Bred"] = props::Bool(false);
  }

  static void CanJoinRaid(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    j["CanJoinRaid"] = props::Bool(true);
  }

  static void CanPickUpLoot(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    CopyBoolValues(b, j, {{"canPickupItems", "CanPickUpLoot"}});
  }

  static void ChestedHorse(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    CopyBoolValues(b, j, {{"Chested", "ChestedHorse", false}});
  }

  static void CollarColor(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    auto owner = b.int64("OwnerNew", -1);
    if (owner == -1) {
      j["CollarColor"] = props::Byte(14);
    } else {
      CopyByteValues(b, j, {{"Color", "CollarColor"}});
    }
  }

  static void ConversionTime(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    j["ConversionTime"] = props::Int(-1);
  }

  static void CopyVariant(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    CopyIntValues(b, j, {{"Variant"}});
  }

  static void CustomName(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    auto name = b.string("CustomName");
    if (name) {
      nlohmann::json json;
      json["text"] = *name;
      j["CustomName"] = props::String(nlohmann::to_string(json));
    }
  }

  static void DeathTime(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    CopyShortValues(b, j, {{"DeathTime"}});
  }

  static void Debug(CompoundTag const &b, CompoundTag &j, Context &ctx) {
  }

  static void EatingHaystack(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    j["EatingHaystack"] = props::Bool(false);
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

  static void FoodLevel(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    j["FoodLevel"] = props::Byte(0);
  }

  static void FromBucket(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    CopyBoolValues(b, j, {{"Persistent", "FromBucket", false}});
    auto name = b.string("CustomName");
    j["PersistenceRequired"] = props::Bool(!!name);
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

  static void HopperMinecart(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    auto enabled = HasDefinition(b, "+minecraft:hopper_active");
    j["Enabled"] = props::Bool(enabled);

    j["TransferCooldown"] = props::Int(0);
  }

  static void HurtByTimestamp(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    j["HurtByTimestamp"] = props::Int(0);
  }

  static void HurtTime(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    CopyShortValues(b, j, {{"HurtTime"}});
  }

  static void InLove(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    CopyIntValues(b, j, {{"InLove", "InLove", 0}});
  }

  static void Inventory(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    //TODO:
    j["Inventory"] = std::make_shared<ListTag>(Tag::Type::Compound);
  }

  static void Invulnerable(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    CopyBoolValues(b, j, {{"Invulnerable"}});
  }

  static void IsBaby(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    CopyBoolValues(b, j, {{"IsBaby"}});
  }

  static void ItemsWithDecorItem(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    Items("DecorItem", b, j, ctx);

    auto armorsJ = j.listTag("ArmorItems");
    if (!armorsJ) {
      return;
    }
    if (armorsJ->size() < 3) {
      return;
    }
    armorsJ->fValue[2] = std::make_shared<CompoundTag>();
  }

  static void ItemsWithSaddleItem(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    Items("SaddleItem", b, j, ctx);
  }

  static void Minecart(CompoundTag const &b, CompoundTag &j, Context &ctx) {
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

  static void NoGravity(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    j["NoGravity"] = props::Bool(true);
  }

  static void Offers(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    std::shared_ptr<CompoundTag> offersB = b.compoundTag("Offers");
    if (!offersB) {
      offersB = b.compoundTag("persistingOffers");
    }
    if (!offersB) {
      return;
    }
    auto recipesB = offersB->listTag("Recipes");
    if (!recipesB) {
      return;
    }

    auto recipesJ = std::make_shared<ListTag>(Tag::Type::Compound);
    for (auto const &it : *recipesB) {
      auto recipeB = it->asCompound();
      if (!recipeB) {
        continue;
      }
      auto recipeJ = Recipe(*recipeB, ctx);
      if (!recipeJ) {
        continue;
      }
      recipesJ->push_back(recipeJ);
    }

    auto offersJ = std::make_shared<CompoundTag>();
    offersJ->set("Recipes", recipesJ);
    j["Offers"] = offersJ;
  }

  static void Size(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    auto sizeB = b.byte("Size", 1);
    j["Size"] = props::Int(sizeB - 1);
  }

  static void StorageMinecart(CompoundTag const &b, CompoundTag &j, Context &ctx) {
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
        auto itemJ = Item::From(*itemB, ctx);
        if (!itemJ) {
          continue;
        }
        itemsJ->push_back(itemJ);
      }
      j["Items"] = itemsJ;
    }
  }

  static void StrayConversionTime(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    j["StrayConversionTime"] = props::Int(-1);
  }

  static void Strength(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    CopyIntValues(b, j, {{"Strength"}});
  }

  static void Tame(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    CopyBoolValues(b, j, {{"IsTamed", "Tame", false}});
  }

  static void Temper(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    CopyIntValues(b, j, {{"Temper", "Temper", 0}});
  }

  static void LeftHanded(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    j["LeftHanded"] = props::Bool(false);
  }

  static void OnGround(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    CopyBoolValues(b, j, {{"OnGround"}});
  }

  static void Owner(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    auto ownerNew = b.int64("OwnerNew");
    if (!ownerNew) {
      return;
    }
    Uuid uuid = Uuid::GenWithI64Seed(*ownerNew);
    j["Owner"] = uuid.toIntArrayTag();
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

  static void PatrolLeader(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    CopyBoolValues(b, j, {{"IsIllagerCaptain", "PatrolLeader"}});
  }

  static void Patrolling(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    j["Patrolling"] = props::Bool(false);
  }

  static void PersistenceRequired(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    CopyBoolValues(b, j, {{"Persistent", "PersistenceRequired", false}});
  }

  static void PortalCooldown(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    CopyIntValues(b, j, {{"PortalCooldown"}});
  }

  static void Pos(CompoundTag const &b, CompoundTag &j, Context &ctx) {
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

  static void Rotation(CompoundTag const &b, CompoundTag &j, Context &ctx) {
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

  static void Saddle(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    j["Saddle"] = props::Bool(HasDefinitionWithPrefixAndSuffix(b, "+minecraft:", "_saddled"));
  }

  static void ShowBottom(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    CopyBoolValues(b, j, {{"ShowBottom"}});
  }

  static void Sitting(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    CopyBoolValues(b, j, {{"Sitting", "Sitting", false}});
  }

  static void Wave(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    /*
      uuid := b.int64("DwellingUniqueID")
      c := db.Get("VILLAGE_(uuid)_RAID")
      c => {
        "Raid": {
          "GroupNum": 2, // byte
          "NumGroups": 7, // byte
          "NumRaiders": 4, // byte
          "Raiders": [
            -1640677507038, // long
            -1640677507037, // long
            -1640677507036, // long
            -1640677507035 // long
          ],
          "SpawnFails": 0, // byte
          "SpawnX": 46, // float
          "SpawnY": 83, // float
          "SpawnZ": -26, // float
          "State": 3, // int
          "Ticks": 428, // long
          "TotalMaxHealth": 172 // float
        }
      }
      */
    j["Wave"] = props::Int(0);
  }
#pragma endregion

#pragma region Converters
  static std::shared_ptr<CompoundTag> Animal(std::string const &id, CompoundTag const &b, Context &ctx) {
    auto ret = LivingEntity(id, b, ctx);
    CompoundTag &j = *ret;
    Age(b, j, ctx);
    InLove(b, j, ctx);
    return ret;
  }

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
    return ret;
  }

  static std::shared_ptr<CompoundTag> LivingEntity(std::string const &id, CompoundTag const &b, Context &ctx) {
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
    PersistenceRequired(b, j, ctx);
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

  static void Items(std::string subItemKey, CompoundTag const &b, CompoundTag &j, Context &ctx) {
    auto chestItems = b.listTag("ChestItems");
    auto items = std::make_shared<ListTag>(Tag::Type::Compound);
    if (chestItems) {
      std::shared_ptr<CompoundTag> subItem;
      for (auto const &it : *chestItems) {
        auto itemB = it->asCompound();
        if (!itemB) {
          continue;
        }
        auto itemJ = Item::From(*itemB, ctx);
        if (!itemJ) {
          continue;
        }
        auto slot = itemJ->byte("Slot");
        if (!slot) {
          continue;
        }
        if (*slot == 0) {
          itemJ->erase("Slot");
          subItem = itemJ;
        } else {
          itemJ->set("Slot", props::Byte(*slot + 1));
          items->push_back(itemJ);
        }
      }

      if (subItem) {
        j[subItemKey] = subItem;
      }
    }
    j["Items"] = items;
  }

  static void Passengers(Uuid const &uid, CompoundTag const &b, std::map<size_t, Uuid> &passengers) {
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
      passengers[index] = passengerUid;
    }
  }

  static std::shared_ptr<CompoundTag> BuyItem(CompoundTag const &recipeB, std::string const &suffix, Context &ctx) {
    using namespace std;
    auto buy = recipeB.compoundTag("buy" + suffix);
    if (!buy) {
      return nullptr;
    }
    auto item = Item::From(*buy, ctx);
    if (!item) {
      return nullptr;
    }
    auto count = recipeB.int32("buyCount" + suffix);
    if (!count) {
      return nullptr;
    }
    item->set("Count", props::Byte(*count));
    return item;
  }

  static std::shared_ptr<CompoundTag> Recipe(CompoundTag const &recipeB, Context &ctx) {
    auto sellB = recipeB.compoundTag("sell");
    if (!sellB) {
      return nullptr;
    }
    auto sellJ = Item::From(*sellB, ctx);
    if (!sellJ) {
      return nullptr;
    }

    auto buyA = BuyItem(recipeB, "A", ctx);
    auto buyB = BuyItem(recipeB, "B", ctx);
    if (!buyA) {
      return nullptr;
    }

    auto ret = std::make_shared<CompoundTag>();

    ret->set("sell", sellJ);

    ret->set("buy", buyA);
    if (buyB) {
      ret->set("buyB", buyB);
    } else {
      auto air = std::make_shared<CompoundTag>();
      air->set("id", props::String("minecraft:air"));
      air->set("Count", props::Byte(1));
      ret->set("buyB", air);
    }

    ret->set("specialPrice", props::Int(0));

    CopyIntValues(recipeB, *ret, {{"demand"}, {"maxUses"}, {"uses"}, {"traderExp", "xp"}});
    CopyByteValues(recipeB, *ret, {{"rewardExp"}});
    CopyFloatValues(recipeB, *ret, {{"priceMultiplierA", "priceMultiplier"}, {"priceMultiplierB"}});

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

    E(skeleton, C(Same, LivingEntity, StrayConversionTime));
    E(stray, C(Same, LivingEntity));
    E(creeper, C(Same, LivingEntity, Creeper));
    E(spider, C(Same, LivingEntity));
    E(bat, C(Same, LivingEntity, Bat));
    E(painting, C(Same, Base, Painting));
    E(zombie, C(Same, LivingEntity, IsBaby, Zombie));
    E(chicken, C(Same, Animal, Chicken));
    E(item, C(Same, Base, Entity::Item));
    E(armor_stand, C(Same, Base, AbsorptionAmount, ArmorItems, Brain, DeathTime, FallFlying, HandItems, Health, HurtByTimestamp, HurtTime, ArmorStand));
    E(ender_crystal, C(Rename("end_crystal"), Base, ShowBottom));
    E(chest_minecart, C(Same, Base, Minecart, StorageMinecart, ChestMinecart));
    E(hopper_minecart, C(Same, Base, Minecart, StorageMinecart, HopperMinecart));
    E(boat, C(Same, Base, Boat));
    E(slime, C(Same, LivingEntity, Size));
    E(salmon, C(Same, LivingEntity, FromBucket));
    E(parrot, C(Same, Animal, Owner, Sitting, CopyVariant));
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
    E(donkey, C(Same, Animal, Bred, ChestedHorse, EatingHaystack, ItemsWithSaddleItem, Tame, Temper));
    E(drowned, C(Same, LivingEntity, IsBaby, Zombie));
    E(endermite, C(Same, LivingEntity, Endermite));
    E(evocation_illager, C(Rename("evoker"), LivingEntity, CanJoinRaid, PatrolLeader, Patrolling, Wave, Evoker));
    E(cat, C(Same, Animal, CollarColor, Sitting, Cat));
    E(guardian, C(Same, LivingEntity));
    E(llama, C(Same, Animal, Bred, ChestedHorse, EatingHaystack, ItemsWithDecorItem, Tame, CopyVariant, Strength));
    E(magma_cube, C(Same, LivingEntity, Size));
    E(mooshroom, C(Same, Animal, Mooshroom));
    E(mule, C(Same, Animal, Bred, ChestedHorse, EatingHaystack, ItemsWithSaddleItem, Tame, Temper));
    E(panda, C(Same, Animal, Panda));
    E(phantom, C(Same, LivingEntity, Phantom));
    E(ghast, C(Same, LivingEntity, Ghast));
    E(pillager, C(Same, LivingEntity, CanJoinRaid, Inventory, PatrolLeader, Patrolling, Wave));
    E(pufferfish, C(Same, LivingEntity, FromBucket, Pufferfish));
    E(rabbit, C(Same, Animal, Rabbit));
    E(ravager, C(Same, LivingEntity, AttackTick, CanJoinRaid, PatrolLeader, Patrolling, Wave, Ravager));
    E(silverfish, C(Same, LivingEntity));
    E(skeleton_horse, C(Same, Animal, Bred, EatingHaystack, Tame, Temper, SkeletonHorse));
    E(polar_bear, C(Same, Animal, AngerTime));
    E(glow_squid, C(Same, LivingEntity, GlowSquid));
    E(squid, C(Same, LivingEntity));
    E(strider, C(Same, Animal, Saddle));
    E(turtle, C(Same, Animal, Turtle));
    E(tropicalfish, C(Rename("tropical_fish"), LivingEntity, FromBucket, TropicalFish));
    E(minecart, C(Same, Base, Minecart));
    E(vex, C(Same, LivingEntity, NoGravity));
    E(villager_v2, C(Rename("villager"), Animal, FoodLevel, Inventory, Offers, Villager));
    E(wandering_trader, C(Same, LivingEntity, Age, Inventory, Offers, WanderingTrader));
    E(wolf, C(Same, Animal, AngerTime, CollarColor, Sitting));
    E(zombie_horse, C(Same, Animal, Bred, EatingHaystack, Tame, Temper));
    E(zombie_villager_v2, C(Rename("zombie_villager"), LivingEntity, IsBaby, ConversionTime, Offers, Zombie, Villager));
    E(snow_golem, C(Same, LivingEntity, SnowGolem));
    E(shulker, C(Same, LivingEntity, Shulker));
    E(wither_skeleton, C(Same, LivingEntity));
    E(witch, C(Same, LivingEntity, CanJoinRaid, PatrolLeader, Patrolling, Wave));
    E(iron_golem, C(Same, LivingEntity, AngerTime, IronGolem));
    E(tnt_minecart, C(Same, Base, Minecart, TntMinecart));
    E(goat, C(Same, Animal, Goat));
    E(axolotl, C(Same, Animal, FromBucket, Axolotl));
    E(wither, C(Same, LivingEntity, Wither));
    E(piglin, C(Same, LivingEntity, Inventory));
    E(piglin_brute, C(Same, LivingEntity, PiglinBrute));
    E(hoglin, C(Same, Animal));

#undef E
    return ret;
  }
};

} // namespace je2be::toje
