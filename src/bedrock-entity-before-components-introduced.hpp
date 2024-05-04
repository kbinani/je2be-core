#pragma once

namespace je2be::bedrock {

class BedrockEntityBeforeComponentsIntroduced {
  using LocalPlayerData = Entity::LocalPlayerData;
  using Result = Entity::Result;

public:
  static CompoundTagPtr ItemFrameFromBedrock(mcfile::Dimension d, Pos3i pos, mcfile::be::Block const &blockJ, CompoundTag const &blockEntityB, Context &ctx, int dataVersion) {
    auto ret = Compound();
    CompoundTag &t = *ret;
    if (blockJ.fName == u8"minecraft:glow_frame") {
      t[u8"id"] = String(u8"minecraft:glow_item_frame");
    } else {
      t[u8"id"] = String(u8"minecraft:item_frame");
    }

    i32 facingDirectionA = blockJ.fStates->int32(u8"facing_direction", 0);
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
    t[u8"Rotation"] = rot.toListTag();
    t[u8"Facing"] = Byte(facingDirectionA);

    t[u8"TileX"] = Int(pos.fX);
    t[u8"TileY"] = Int(pos.fY);
    t[u8"TileZ"] = Int(pos.fZ);

    Pos3i direction = Pos3iFromFacing6(f6);
    double thickness = 15.0 / 32.0;
    Pos3d position(pos.fX + 0.5 - direction.fX * thickness, pos.fY + 0.5 - direction.fY * thickness, pos.fZ + 0.5 - direction.fZ * thickness);
    t[u8"Pos"] = position.toListTag();

    mcfile::XXHash<u64> h(0);
    h.update(&d, sizeof(d));
    h.update(&position.fX, sizeof(position.fX));
    h.update(&position.fY, sizeof(position.fY));
    h.update(&position.fZ, sizeof(position.fZ));
    auto uuid = Uuid::GenWithU64Seed(h.digest());
    t[u8"UUID"] = uuid.toIntArrayTag();

    auto itemB = blockEntityB.compoundTag(u8"Item");
    if (itemB) {
      auto itemJ = Item::From(*itemB, ctx, dataVersion, {});
      if (itemJ) {
        t[u8"Item"] = itemJ;
      }
    }

    auto itemRotationB = blockEntityB.float32(u8"ItemRotation");
    if (itemRotationB) {
      t[u8"ItemRotation"] = Byte(static_cast<i8>(std::roundf(*itemRotationB / 45.0f)));
    }

    Pos3d motion(0, 0, 0);
    t[u8"Motion"] = motion.toListTag();

    CopyFloatValues(blockEntityB, t, {{u8"ItemDropChance"}});

    t[u8"Air"] = Short(300);
    t[u8"FallDistance"] = Float(0);
    t[u8"Fire"] = Short(-1);
    t[u8"Fixed"] = Bool(false);
    t[u8"Invisible"] = Bool(false);
    t[u8"Invulnerable"] = Bool(false);
    t[u8"OnGround"] = Bool(false);
    t[u8"PortalCooldown"] = Int(0);

    return ret;
  }

  static std::optional<Result> From(CompoundTag const &entityB, Context &ctx, int dataVersion) {
    auto id = entityB.string(u8"identifier");
    if (!id) {
      return std::nullopt;
    }
    auto uid = entityB.int64(u8"UniqueID");
    if (!uid) {
      return std::nullopt;
    }
    i64 v = *uid;
    Uuid uuid = Uuid::GenWithU64Seed(*(u64 *)&v);

    auto const *table = GetTable();
    std::u8string_view key(*id);
    auto found = table->find(Namespace::Remove(key));
    if (found == table->end()) {
      return std::nullopt;
    }

    auto e = found->second(*id, entityB, ctx, dataVersion);
    if (!e) {
      return std::nullopt;
    }
    e->set(u8"UUID", uuid.toIntArrayTag());

    if (ctx.setShoulderEntityIfItIs(*uid, e)) {
      return std::nullopt;
    }

    Result r;

    auto st = Passengers(uuid, entityB, ctx, r.fPassengers);
    if (st == PassengerStatus::ContainsLocalPlayer) {
      ctx.setRootVehicle(uuid);
    }

    auto leasherId = entityB.int64(u8"LeasherID", -1);
    if (leasherId != -1) {
      r.fLeasherId = leasherId;
    }

    r.fUuid = uuid;
    r.fEntity = e;
    return r;
  }

  using Converter = std::function<CompoundTagPtr(std::u8string const &id, CompoundTag const &eneityB, Context &ctx, int dataVersion)>;
  using Namer = std::function<std::u8string(std::u8string const &nameB, CompoundTag const &entityB)>;
  using Behavior = std::function<void(CompoundTag const &entityB, CompoundTag &entityJ, Context &ctx, int dataVersion)>;

  struct C {
    template <class... Arg>
    C(Namer namer, Converter base, Arg... behaviors) : fNamer(namer), fBase(base), fBehaviors(std::initializer_list<Behavior>{behaviors...}) {}

    CompoundTagPtr operator()(std::u8string const &id, CompoundTag const &entityB, Context &ctx, int dataVersion) const {
      auto name = fNamer(id, entityB);
      auto t = fBase(id, entityB, ctx, dataVersion);
      t->set(u8"id", name);
      for (auto behavior : fBehaviors) {
        behavior(entityB, *t, ctx, dataVersion);
      }
      return t;
    }

    Namer fNamer;
    Converter fBase;
    std::vector<Behavior> fBehaviors;
  };

#pragma region Namers
  static std::u8string LlamaName(std::u8string const &nameB, CompoundTag const &entityB) {
    if (HasDefinition(entityB, u8"+minecraft:llama_wandering_trader")) {
      // legacy
      return u8"minecraft:trader_llama";
    } else {
      return u8"minecraft:llama";
    }
  }

  static Namer Rename(std::u8string name) {
    return [name](std::u8string const &nameB, CompoundTag const &entityB) {
      return u8"minecraft:" + name;
    };
  }

  static std::u8string Same(std::u8string const &nameB, CompoundTag const &entityB) {
    return nameB;
  }
#pragma endregion

#pragma region Dedicated Behaviors
  static void Allay(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    if (auto ownerNew = b.int64(u8"OwnerNew"); ownerNew) {
      Uuid uuid;
      if (auto mapped = ctx.mapLocalPlayerId(*ownerNew); mapped) {
        uuid = *mapped;
      } else {
        uuid = Uuid::GenWithI64Seed(*ownerNew);
      }
      auto brain = Compound();
      auto memories = Compound();
      auto likedPlayer = Compound();
      likedPlayer->set(u8"value", uuid.toIntArrayTag());
      memories->set(u8"minecraft:liked_player", likedPlayer);
      brain->set(u8"memories", memories);
      j[u8"Brain"] = brain;
    }
    j[u8"PersistenceRequired"] = Bool(false);

    j[u8"CanDuplicate"] = Bool(true);
    CopyLongValues(b, j, {{u8"AllayDuplicationCooldown", u8"DuplicationCooldown"}});
  }

  static void ArmorStand(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    j.erase(u8"ArmorDropChances");
    j.erase(u8"HandDropChances");
    j[u8"DisabledSlots"] = Int(false);
    j[u8"Invisible"] = Bool(false);
    j[u8"NoBasePlate"] = Bool(false);
    j[u8"Small"] = Bool(false);

    bool showArms = false;
    auto poseB = b.compoundTag(u8"Pose");
    if (poseB) {
      auto poseIndexB = poseB->int32(u8"PoseIndex");
      if (poseIndexB) {
        auto pose = ArmorStand::JavaPoseFromBedrockPoseIndex(*poseIndexB);
        auto poseJ = pose->toCompoundTag();
        if (!poseJ->empty()) {
          j[u8"Pose"] = poseJ;
        }
        showArms = true;
      }
    }
    j[u8"ShowArms"] = Bool(showArms);
  }

  static void Axolotl(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    auto variantB = b.int32(u8"Variant", 0);
    j[u8"Variant"] = Int(je2be::Axolotl::JavaVariantFromBedrockVariant(variantB));
  }

  static void Bat(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    CopyBoolValues(b, j, {{u8"BatFlags"}});
  }

  static void Bee(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    j[u8"CannotEnterHiveTicks"] = Int(0);
    j[u8"CropsGrownSincePollination"] = Int(0);
    auto hasNectar = HasDefinition(b, u8"+has_nectar");
    j[u8"HasNectar"] = Bool(hasNectar);
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
    j[u8"HasStung"] = Bool(false);
  }

  static void Boat(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    auto variant = b.int32(u8"Variant", 0);
    auto type = Boat::JavaTypeFromBedrockVariant(variant);
    j[u8"Type"] = String(type);

    if (auto rotB = props::GetRotation(b, u8"Rotation"); rotB) {
      je2be::Rotation rotJ(Rotation::ClampDegreesBetweenMinus180And180(rotB->fYaw - 90), rotB->fPitch);
      j[u8"Rotation"] = rotJ.toListTag();
    }

    if (auto posB = props::GetPos3f(b, u8"Pos"); posB) {
      auto posJ = posB->toD();
      if (b.boolean(u8"OnGround", false)) {
        posJ.fY = round(posB->fY - 0.375);
      }
      j[u8"Pos"] = posJ.toListTag();
    }
  }

  static void Camel(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    j[u8"Temper"] = Int(0);
  }

  static void Cat(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    using Cat = je2be::Cat;

    auto variantB = b.int32(u8"Variant", 8);
    Cat::Type catType = Cat::CatTypeFromBedrockVariant(variantB);
    j[u8"variant"] = String(u8"minecraft:" + Cat::JavaVariantFromCatType(catType));
  }

  static void ChestMinecart(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    if (auto st = LootTable::BedrockToJava(b, j); st == LootTable::State::HasLootTable) {
      j.erase(u8"Items");
    }
  }

  static void Chicken(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    auto entries = b.listTag(u8"entries");
    if (entries) {
      for (auto const &it : *entries) {
        auto c = it->asCompound();
        if (!c) {
          continue;
        }
        auto spawnTimer = c->int32(u8"SpawnTimer");
        if (!spawnTimer) {
          continue;
        }
        j[u8"EggLayTime"] = Int(*spawnTimer);
        break;
      }
    }
    j[u8"IsChickenJockey"] = Bool(false);
  }

  static void Creeper(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    j[u8"ExplosionRadius"] = Byte(3);
    j[u8"Fuse"] = Short(30);
    j[u8"ignited"] = Bool(false);
    if (HasDefinition(b, u8"+minecraft:charged_creeper")) {
      j[u8"powered"] = Bool(true);
    }
  }

  static void EnderCrystal(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    auto x = b.int32(u8"BlockTargetX");
    auto y = b.int32(u8"BlockTargetY");
    auto z = b.int32(u8"BlockTargetZ");
    if (x && y && z) {
      auto beamTarget = Compound();
      beamTarget->set(u8"X", Int(*x));
      beamTarget->set(u8"Y", Int(*y));
      beamTarget->set(u8"Z", Int(*z));
      j[u8"BeamTarget"] = beamTarget;
    }
  }

  static void EnderDragon(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    auto deathTime = b.int32(u8"DeathTime", 0);
    j[u8"DragonDeathTime"] = Int(deathTime);
    if (deathTime > 0) {
      j[u8"DragonPhase"] = Int(9);
    } else {
      j[u8"DragonPhase"] = Int(0);
    }
    j[u8"PersistenceRequired"] = Bool(false);
  }

  static void Enderman(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    using namespace std;
    auto carriedBlockTagB = b.compoundTag(u8"carriedBlock");
    if (carriedBlockTagB) {
      auto carriedBlockB = mcfile::be::Block::FromCompound(*carriedBlockTagB);
      if (carriedBlockB) {
        auto carriedBlockJ = BlockData::From(*carriedBlockB, dataVersion);
        if (carriedBlockJ) {
          if (carriedBlockJ->fId == mcfile::blocks::minecraft::grass_block) {
            carriedBlockJ = carriedBlockJ->applying({{u8"snowy", u8"false"}});
          }
          j[u8"carriedBlockState"] = carriedBlockJ->toCompoundTag();
        }
      }
    }
  }

  static void Endermite(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    CopyIntValues(b, j, {{u8"Lifetime"}});
  }

  static void Evoker(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    j[u8"SpellTicks"] = Int(0);
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

  static void ExperienceOrb(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    auto value = b.int32(u8"experience value");
    if (value) {
      j[u8"Value"] = Short(*value);
    }
    CopyShortValues(b, j, {{u8"Age"}});
    j[u8"Health"] = Short(5);
    j[u8"Count"] = Int(1);
  }

  static void FallingBlock(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    if (auto fallingBlockB = b.compoundTag(u8"FallingBlock"); fallingBlockB) {
      if (auto blockB = mcfile::be::Block::FromCompound(*fallingBlockB); blockB) {
        if (auto blockJ = BlockData::From(*blockB, dataVersion); blockJ) {
          j[u8"BlockState"] = blockJ->toCompoundTag();
        }
      }
    }

    if (auto time = b.byte(u8"Time"); time) {
      i8 i = *time;
      u8 u = *(u8 *)&i;
      j[u8"Time"] = Int(u);
    }

    j[u8"CancelDrop"] = Bool(false);
  }

  static void Fox(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    auto variant = b.int32(u8"Variant", 0);
    std::u8string type;
    if (variant == 1) {
      type = u8"snow";
    } else {
      type = u8"red";
    }
    j[u8"Type"] = String(type);

    j[u8"Sleeping"] = Bool(HasDefinition(b, u8"+minecraft:fox_ambient_sleep"));
    j[u8"Crouching"] = Bool(false);

    auto trusted = List<Tag::Type::IntArray>();
    auto trustedPlayers = b.int32(u8"TrustedPlayersAmount", 0);
    if (trustedPlayers > 0) {
      for (int i = 0; i < trustedPlayers; i++) {
        auto uuidB = b.int64(u8"TrustedPlayer" + mcfile::String::ToString(i));
        if (!uuidB) {
          continue;
        }
        Uuid uuidJ;
        if (auto mapped = ctx.mapLocalPlayerId(*uuidB); mapped) {
          uuidJ = *mapped;
        } else {
          uuidJ = Uuid::GenWithI64Seed(*uuidB);
        }
        trusted->push_back(uuidJ.toIntArrayTag());
      }
    }
    j[u8"Trusted"] = trusted;
  }

  static void Frog(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    auto variantB = b.int32(u8"Variant", 0);
    auto variantJ = Frog::JavaVariantFromBedrockVariant(variantB);
    j[u8"variant"] = String(variantJ);
  }

  static void Ghast(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    j[u8"ExplosionPower"] = Byte(1);
  }

  static void GlowSquid(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    j[u8"DarkTicksRemaining"] = Int(0);
  }

  static void Goat(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    bool screamer = HasDefinition(b, u8"+goat_screamer") || HasDefinition(b, u8"+ram_screamer") || HasDefinition(b, u8"+interact_screamer");
    j[u8"IsScreamingGoat"] = Bool(screamer);

    auto hornCount = b.int32(u8"GoatHornCount", 0);
    j[u8"HasRightHorn"] = Bool(hornCount == 2); // Goat loses right horn first in BE.
    j[u8"HasLeftHorn"] = Bool(hornCount > 0);
  }

  static void Hoglin(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    if (HasDefinition(b, u8"-angry_hoglin")) {
      j[u8"CannotBeHunted"] = Bool(true);
    }
  }

  static void Horse(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    auto armorDropChances = List<Tag::Type::Float>();
    armorDropChances->push_back(Float(0.085));
    armorDropChances->push_back(Float(0.085));
    armorDropChances->push_back(Float(0));
    armorDropChances->push_back(Float(0.085));
    j[u8"ArmorDropChances"] = armorDropChances;

    i32 variantB = b.int32(u8"Variant", 0);
    i32 markVariantB = b.int32(u8"MarkVariant", 0);
    u32 uVariantB = *(u32 *)&variantB;
    u32 uMarkVariantB = *(u32 *)&markVariantB;
    u32 uVariantJ = (0xf & uVariantB) | ((0xf & uMarkVariantB) << 8);
    j[u8"Variant"] = Int(*(i32 *)&uVariantJ);

    if (auto chestItems = b.listTag(u8"ChestItems"); chestItems) {
      // NOTE: horse's b[u8"Chested"] is always false, so we cannot use ItemsWithSaddleItem function for horse.
      for (auto const &it : *chestItems) {
        auto itemB = it->asCompound();
        if (!itemB) {
          continue;
        }
        auto itemJ = Item::From(*itemB, ctx, dataVersion, {});
        if (!itemJ) {
          continue;
        }
        auto slotJ = itemJ->byte(u8"Slot");
        itemJ->erase(u8"Slot");
        if (slotJ == 0) {
          j[u8"SaddleItem"] = itemJ;
        } else if (slotJ == 1) {
          j[u8"ArmorItem"] = itemJ;
        }
      }
    }
  }

  static void IronGolem(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    auto target = b.int64(u8"TargetID", -1);
    if (target != -1) {
      auto angryAt = Uuid::GenWithI64Seed(target);
      j[u8"AngryAt"] = angryAt.toIntArrayTag();
    }

    j[u8"PlayerCreated"] = Bool(HasDefinition(b, u8"+minecraft:player_created"));
  }

  static void Item(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    j[u8"PickupDelay"] = Short(0);
    auto itemB = b.compoundTag(u8"Item");
    if (itemB) {
      auto itemJ = bedrock::Item::From(*itemB, ctx, dataVersion, {});
      if (itemJ) {
        j[u8"Item"] = itemJ;
      }
    }
    CopyShortValues(b, j, {{u8"Age"}, {u8"Health"}});
  }

  static void Llama(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    if (HasDefinition(b, u8"+minecraft:llama_wandering_trader")) {
      // Ignoring "IsTamed" property here, because it is always true for natural spawned wandering trader llama.
      auto tamed = b.int64(u8"OwnerNew", -1) != -1;
      j[u8"Tame"] = Bool(tamed);
    }
  }

  static void Mooshroom(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    auto variant = b.int32(u8"Variant", 0);
    std::u8string type = u8"red";
    if (variant == 1) {
      type = u8"brown";
    }
    j[u8"Type"] = String(type);
  }

  static void Ocelot(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    CopyBoolValues(b, j, {{u8"IsTrusting", u8"Trusting"}});
  }

  static void Panda(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    auto genes = b.listTag(u8"GeneArray");
    if (genes) {
      std::optional<i32> main;
      std::optional<i32> hidden;
      for (auto const &it : *genes) {
        auto gene = it->asCompound();
        if (!gene) {
          continue;
        }
        auto m = gene->int32(u8"MainAllele");
        auto h = gene->int32(u8"HiddenAllele");
        if (m && h) {
          main = m;
          hidden = h;
          break;
        }
      }
      if (main && hidden) {
        j[u8"MainGene"] = String(Panda::JavaGeneNameFromGene(Panda::GeneFromBedrockAllele(*main)));
        j[u8"HiddenGene"] = String(Panda::JavaGeneNameFromGene(Panda::GeneFromBedrockAllele(*hidden)));
      }
    }
  }

  static void Phantom(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    j[u8"Size"] = Int(0);
  }

  static void Piglin(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    if (HasDefinition(b, u8"+not_hunter")) {
      j[u8"CannotHunt"] = Bool(true);
    }
  }

  static void PiglinBrute(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    auto homePos = props::GetPos3f(b, u8"HomePos");
    if (!homePos) {
      return;
    }
    auto homeDimensionId = b.int32(u8"HomeDimensionId");
    if (!homeDimensionId) {
      return;
    }
    auto dim = DimensionFromBedrockDimension(*homeDimensionId);
    if (!dim) {
      return;
    }

    auto value = Compound();
    value->set(u8"dimension", JavaStringFromDimension(*dim));
    std::vector<i32> pos;
    pos.push_back((int)round(homePos->fX));
    pos.push_back((int)round(homePos->fY));
    pos.push_back((int)round(homePos->fZ));
    auto posTag = std::make_shared<IntArrayTag>(pos);
    value->set(u8"pos", posTag);

    auto homeTag = Compound();
    homeTag->set(u8"value", value);

    auto memories = Compound();
    memories->set(u8"minecraft:home", homeTag);

    auto brain = Compound();
    brain->set(u8"memories", memories);

    j[u8"Brain"] = brain;
  }

  static void Pufferfish(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    int state = 0;
    if (HasDefinition(b, u8"+minecraft:half_puff_primary") || HasDefinition(b, u8"+minecraft:half_puff_secondary")) {
      state = 1;
    } else if (HasDefinition(b, u8"+minecraft:full_puff")) {
      state = 2;
    }
    j[u8"PuffState"] = Int(state);
  }

  static void Rabbit(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    CopyIntValues(b, j, {{u8"MoreCarrotTicks"}});
    CopyIntValues(b, j, {{u8"Variant", u8"RabbitType", 0}});
  }

  static void Ravager(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    j[u8"RoarTick"] = Int(0);
    j[u8"StunTick"] = Int(0);
  }

  static void Sheep(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    CopyBoolValues(b, j, {{u8"Sheared"}});
    CopyByteValues(b, j, {{u8"Color"}});
  }

  static void Shulker(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    auto variant = b.int32(u8"Variant", 16);
    j[u8"Color"] = Byte(variant);
  }

  static void SkeletonHorse(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    // summon minecraft:skeleton_horse ~ ~ ~ minecraft:set_trap
    j[u8"SkeletonTrap"] = Bool(HasDefinition(b, u8"+minecraft:skeleton_trap"));
    j[u8"SkeletonTrapTime"] = Int(0);
  }

  static void SnowGolem(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    bool pumpkin = !HasDefinition(b, u8"+minecraft:snowman_sheared");
    j[u8"Pumpkin"] = Bool(pumpkin);
  }

  static void TntMinecart(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    auto fuse = b.byte(u8"Fuse", -1);
    j[u8"TNTFuse"] = Int(fuse);
  }

  static void TropicalFish(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    auto tf = TropicalFish::FromBedrockBucketTag(b);
    j[u8"Variant"] = Int(tf.toJavaVariant());
  }

  static void Turtle(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    auto homePos = props::GetPos3f(b, u8"HomePos");
    if (homePos) {
      j[u8"HomePosX"] = Int(roundf(homePos->fX));
      j[u8"HomePosY"] = Int(roundf(homePos->fY));
      j[u8"HomePosZ"] = Int(roundf(homePos->fZ));
    }

    j[u8"HasEgg"] = Bool(HasDefinition(b, u8"-minecraft:wants_to_lay_egg") || HasDefinition(b, u8"+minecraft:wants_to_lay_egg"));
  }

  static void Villager(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    CopyIntValues(b, j, {{u8"TradeExperience", u8"Xp", 0}});

    j.erase(u8"InLove");

    auto dataJ = Compound();

    VillagerProfession profession(static_cast<VillagerProfession::Variant>(0));
    auto variantB = b.int32(u8"Variant", 0);
    if (variantB == 0) {
      // "Variant" of zombie villager is always 0
      auto definitions = b.listTag(u8"definitions");
      if (definitions) {
        for (auto const &it : *definitions) {
          auto str = it->asString();
          if (!str) {
            continue;
          }
          std::u8string def = str->fValue;
          if (!def.starts_with(u8"+")) {
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
    dataJ->set(u8"profession", u8"minecraft:" + profession.string());

    VillagerType::Variant variant = VillagerType::Plains;
    auto markVariantB = b.int32(u8"MarkVariant");
    if (markVariantB) {
      variant = static_cast<VillagerType::Variant>(*markVariantB);
    } else {
      if (HasDefinition(b, u8"+desert_villager")) {
        variant = VillagerType::Desert;
      } else if (HasDefinition(b, u8"+jungle_villager")) {
        variant = VillagerType::Jungle;
      } else if (HasDefinition(b, u8"+savanna_villager")) {
        variant = VillagerType::Savanna;
      } else if (HasDefinition(b, u8"+snow_villager")) {
        variant = VillagerType::Snow;
      } else if (HasDefinition(b, u8"+swamp_villager")) {
        variant = VillagerType::Swamp;
      } else if (HasDefinition(b, u8"+taiga_villager")) {
        variant = VillagerType::Taiga;
      }
    }
    VillagerType type(variant);
    dataJ->set(u8"type", u8"minecraft:" + type.string());

    auto tradeTier = b.int32(u8"TradeTier", 0);
    dataJ->set(u8"level", Int(tradeTier + 1));

    j[u8"VillagerData"] = dataJ;
  }

  static void WanderingTrader(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    /*
      // spawned with spawn egg
      "entries": [
        {
          "SpawnTimer": -1, // int
          "StopSpawning": 1 // byte
        }
      ],
  */
    int despawnDelay = 0;
    if (auto timestamp = b.int64(u8"TimeStamp"); timestamp) {
      despawnDelay = std::max<i64>(0, *timestamp - ctx.fGameTick);
    }
    j[u8"DespawnDelay"] = Int(despawnDelay);
  }

  static void Wither(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    j[u8"Invul"] = Int(0);
    if (auto healthB = FindAttribute(b, u8"minecraft:health"); healthB) {
      auto max = healthB->float32(u8"Max");
      auto current = healthB->float32(u8"Current");
      if (max && current && *max > 0) {
        auto ratio = std::min(std::max(*current / *max, 0.0f), 1.0f);
        float healthJ = 300.0f * ratio;
        j[u8"Health"] = Float(healthJ);
      }
    }
  }

  static void Zombie(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    j[u8"DrownedConversionTime"] = Int(-1);
    j[u8"CanBreakDoors"] = Bool(HasDefinition(b, u8"+minecraft:can_break_doors"));
    j[u8"InWaterTime"] = Int(-1);
  }

  static void ZombifiedPiglin(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    auto targetId = b.int64(u8"TargetID", -1);
    if (targetId != -1) {
      auto angryAt = Uuid::GenWithI64Seed(targetId);
      j[u8"AngryAt"] = angryAt.toIntArrayTag();
    }
  }
#pragma endregion

#pragma region Behaviors
  static void AbsorptionAmount(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    j[u8"AbsorptionAmount"] = Float(0);
  }

  static void Age(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    CopyIntValues(b, j, {{u8"Age", u8"Age", 0}});
  }

  static void Air(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    CopyShortValues(b, j, {{u8"Air", u8"Air", 300}});
  }

  static void AngerTime(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    j[u8"AngerTime"] = Int(0);
  }

  static void ArmorItems(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    auto armorsB = b.listTag(u8"Armor");
    auto armorsJ = List<Tag::Type::Compound>();
    auto chances = List<Tag::Type::Float>();
    if (armorsB) {
      std::vector<CompoundTagPtr> armors;
      for (auto const &it : *armorsB) {
        auto armorB = it->asCompound();
        CompoundTagPtr armorJ;
        if (armorB) {
          armorJ = Item::From(*armorB, ctx, dataVersion, {});
        }
        if (!armorJ) {
          armorJ = Compound();
        }
        armors.push_back(armorJ);
        chances->push_back(Float(0.085));
      }
      if (armors.size() == 4) {
        armorsJ->push_back(armors[3]);
        armorsJ->push_back(armors[2]);
        armorsJ->push_back(armors[1]);
        armorsJ->push_back(armors[0]);
      }
    }
    j[u8"ArmorItems"] = armorsJ;
    j[u8"ArmorDropChances"] = chances;
  }

  static void AttackTick(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    auto attackTime = b.int16(u8"AttackTime");
    if (attackTime) {
      j[u8"AttackTick"] = Int(*attackTime);
    }
  }

  static void Brain(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    auto memories = Compound();
    auto brain = Compound();
    brain->set(u8"memories", memories);
    j[u8"Brain"] = brain;
  }

  static void Bred(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    j[u8"Bred"] = Bool(false);
  }

  static void CanJoinRaid(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    j[u8"CanJoinRaid"] = Bool(HasDefinition(b, u8"+minecraft:raid_configuration"));
  }

  static void CanPickUpLoot(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    CopyBoolValues(b, j, {{u8"canPickupItems", u8"CanPickUpLoot"}});
  }

  static void ChestedHorse(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    CopyBoolValues(b, j, {{u8"Chested", u8"ChestedHorse", false}});
  }

  static void CollarColor(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    auto owner = b.int64(u8"OwnerNew", -1);
    if (owner == -1) {
      j[u8"CollarColor"] = Byte(14);
    } else {
      CopyByteValues(b, j, {{u8"Color", u8"CollarColor"}});
    }
  }

  static void ConversionTime(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    j[u8"ConversionTime"] = Int(-1);
  }

  static void CopyVariant(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    CopyIntValues(b, j, {{u8"Variant"}});
  }

  static void CustomName(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    auto name = b.string(u8"CustomName");
    if (name) {
      props::Json json;
      props::SetJsonString(json, u8"text", *name);
      j[u8"CustomName"] = String(props::StringFromJson(json));
    }
  }

  static void DeathTime(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    CopyShortValues(b, j, {{u8"DeathTime"}});
  }

  static void Debug(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
  }

  static void EatingHaystack(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    j[u8"EatingHaystack"] = Bool(false);
  }

  static void FallDistance(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    CopyFloatValues(b, j, {{u8"FallDistance"}});
  }

  static void FallFlying(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    j[u8"FallFlying"] = Bool(false);
  }

  static void Fire(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    j[u8"Fire"] = Short(-1);
  }

  static void FoodLevel(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    j[u8"FoodLevel"] = Byte(0);
  }

  static void FromBucket(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    auto naturalSpawn = b.boolean(u8"NaturalSpawn", true);
    j[u8"FromBucket"] = Bool(!naturalSpawn);
  }

  static void HandItems(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    auto itemsJ = List<Tag::Type::Compound>();
    auto chances = List<Tag::Type::Float>();
    auto identifier = b.string(u8"identifier", u8"");
    for (std::u8string key : {u8"Mainhand", u8"Offhand"}) {
      auto listB = b.listTag(key);
      CompoundTagPtr itemJ;
      if (listB && !listB->empty()) {
        auto c = listB->at(0)->asCompound();
        if (c) {
          itemJ = Item::From(*c, ctx, dataVersion, {});
        }
      }
      if (!itemJ) {
        itemJ = Compound();
      }
      itemsJ->push_back(itemJ);
      chances->push_back(Float(HandDropChance(*itemJ, identifier)));
    }
    j[u8"HandItems"] = itemsJ;
    j[u8"HandDropChances"] = chances;
  }

  static void Health(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    auto healthB = FindAttribute(b, u8"minecraft:health");
    if (!healthB) {
      return;
    }
    auto current = healthB->float32(u8"Current");
    if (current) {
      j[u8"Health"] = Float(*current);
    }
  }

  static void HealthWithCustomizedMax(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    auto healthB = FindAttribute(b, u8"minecraft:health");
    if (!healthB) {
      return;
    }
    auto current = healthB->float32(u8"Current");
    auto max = healthB->float32(u8"Max");
    if (current && max) {
      j[u8"Health"] = Float(*current);
      auto attr = Compound();
      attr->set(u8"Name", u8"minecraft:generic.max_health");
      attr->set(u8"Base", Double(*max));
      AddAttribute(attr, j);
    }
  }

  static void HopperMinecart(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    auto enabled = HasDefinition(b, u8"+minecraft:hopper_active");
    j[u8"Enabled"] = Bool(enabled);

    // j[u8"TransferCooldown"] = Int(0); // Removed in 1.19.4
  }

  static void HurtByTimestamp(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    j[u8"HurtByTimestamp"] = Int(0);
  }

  static void HurtTime(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    CopyShortValues(b, j, {{u8"HurtTime"}});
  }

  static void InLove(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    CopyIntValues(b, j, {{u8"InLove", u8"InLove", 0}});
  }

  static void Inventory(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    auto itemsB = b.listTag(u8"ChestItems");
    if (itemsB) {
      auto itemsJ = List<Tag::Type::Compound>();
      for (auto const &it : *itemsB) {
        auto itemB = it->asCompound();
        if (!itemB) {
          continue;
        }
        auto nameB = itemB->string(u8"Name");
        if (!nameB) {
          continue;
        }
        auto itemJ = Item::From(*itemB, ctx, dataVersion, {});
        if (!itemJ) {
          continue;
        }
        itemJ->erase(u8"Slot");
        itemsJ->push_back(itemJ);
      }
      j[u8"Inventory"] = itemsJ;
    }
  }

  static void Invulnerable(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    CopyBoolValues(b, j, {{u8"Invulnerable"}});
  }

  static void IsBaby(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    CopyBoolValues(b, j, {{u8"IsBaby"}});
  }

  static void ItemsFromChestItems(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    CopyChestItems(b, u8"ChestItems", j, u8"Items", ctx, false, dataVersion);
  }

  static void ItemsWithDecorItem(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    if (auto chested = b.boolean(u8"Chested", false); chested) {
      Items(u8"DecorItem", b, j, ctx, dataVersion);
    }

    auto armorsJ = j.listTag(u8"ArmorItems");
    if (!armorsJ) {
      return;
    }
    if (armorsJ->size() < 3) {
      return;
    }
    armorsJ->fValue[2] = Compound();
  }

  static void ItemsWithSaddleItem(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    if (auto chested = b.boolean(u8"Chested", false); chested) {
      Items(u8"SaddleItem", b, j, ctx, dataVersion);
    }
  }

  static void JumpStrength(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    std::u8string nameB = u8"minecraft:horse.jump_strength";
    std::u8string nameJ = nameB;
    auto jumpStrengthAttribute = FindAttribute(b, nameB);
    if (!jumpStrengthAttribute) {
      return;
    }
    auto jumpStrength = jumpStrengthAttribute->float32(u8"Current");
    if (!jumpStrength) {
      return;
    }
    auto attr = Compound();
    std::u8string name = nameJ;
    (*attr)[u8"Base"] = Double(*jumpStrength);
    (*attr)[u8"Name"] = String(name);
    AddAttribute(attr, j);
  }

  static void Minecart(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    auto posB = props::GetPos3f(b, u8"Pos");
    auto onGround = b.boolean(u8"OnGround");
    if (posB && onGround) {
      Pos3d posJ = posB->toD();
      if (*onGround) {
        posJ.fY = posB->fY - 0.35 + 0.0001;
      } else {
        posJ.fY = posB->fY - 0.4375 + 0.0001;
      }
      j[u8"Pos"] = posJ.toListTag();
    }
  }

  static void MovementSpeed(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    auto movementB = FindAttribute(b, u8"minecraft:movement");
    if (!movementB) {
      return;
    }
    auto current = movementB->float32(u8"Current");
    if (current) {
      auto attr = Compound();
      attr->set(u8"Name", u8"minecraft:generic.movement_speed");
      attr->set(u8"Base", Double(*current));
      AddAttribute(attr, j);
    }
  }

  static void NoGravity(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    j[u8"NoGravity"] = Bool(true);
  }

  static void Offers(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    CompoundTagPtr offersB = b.compoundTag(u8"Offers");
    if (!offersB) {
      offersB = b.compoundTag(u8"persistingOffers");
    }
    if (!offersB) {
      return;
    }
    auto recipesB = offersB->listTag(u8"Recipes");
    if (!recipesB) {
      return;
    }

    auto recipesJ = List<Tag::Type::Compound>();
    for (auto const &it : *recipesB) {
      auto recipeB = it->asCompound();
      if (!recipeB) {
        continue;
      }
      auto recipeJ = Recipe(*recipeB, ctx, dataVersion);
      if (!recipeJ) {
        continue;
      }
      recipesJ->push_back(recipeJ);
    }

    auto offersJ = Compound();
    offersJ->set(u8"Recipes", recipesJ);
    j[u8"Offers"] = offersJ;
  }

  static void Size(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    auto sizeB = b.byte(u8"Size", 1);
    j[u8"Size"] = Int(sizeB - 1);
  }

  static void StrayConversionTime(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    j[u8"StrayConversionTime"] = Int(-1);
  }

  static void Strength(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    CopyIntValues(b, j, {{u8"Strength"}});
  }

  static void Tame(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    CopyBoolValues(b, j, {{u8"IsTamed", u8"Tame", false}});
  }

  static void Temper(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    CopyIntValues(b, j, {{u8"Temper", u8"Temper", 0}});
  }

  static void LeftHanded(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    j[u8"LeftHanded"] = Bool(false);
  }

  static void OnGround(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    CopyBoolValues(b, j, {{u8"OnGround"}});
  }

  static void Owner(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    auto ownerNew = b.int64(u8"OwnerNew");
    if (!ownerNew) {
      return;
    }
    if (ownerNew == -1) {
      return;
    }
    Uuid uuid;
    if (auto mapped = ctx.mapLocalPlayerId(*ownerNew); mapped) {
      uuid = *mapped;
    } else {
      uuid = Uuid::GenWithI64Seed(*ownerNew);
    }
    j[u8"Owner"] = uuid.toIntArrayTag();
  }

  static void Painting(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    auto directionB = b.byte(u8"Direction", 0);
    Facing4 direction = Facing4FromBedrockDirection(directionB);
    auto motiveB = b.string(u8"Motive", u8"Aztec");
    Painting::Motive motive = Painting::MotiveFromBedrock(motiveB);
    auto motiveJ = Painting::JavaFromMotive(motive);
    auto posB = b.listTag(u8"Pos");
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

    j[u8"variant"] = String(motiveJ);
    j[u8"facing"] = Byte(directionB);
    j[u8"TileX"] = Int(std::round(tile->fX));
    j[u8"TileY"] = Int(std::round(tile->fY));
    j[u8"TileZ"] = Int(std::round(tile->fZ));
  }

  static void PatrolLeader(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    CopyBoolValues(b, j, {{u8"IsIllagerCaptain", u8"PatrolLeader"}});
  }

  static void Patrolling(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    j[u8"Patrolling"] = Bool(false);
  }

  static void PersistenceRequiredDefault(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    CopyBoolValues(b, j, {{u8"Persistent", u8"PersistenceRequired", false}});
  }

  static void PersistenceRequiredAnimal(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    j[u8"PersistenceRequired"] = Bool(false);
  }

  static void PortalCooldown(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    CopyIntValues(b, j, {{u8"PortalCooldown"}});
  }

  static void Pos(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    auto posB = b.listTag(u8"Pos");
    if (!posB) {
      return;
    }
    if (posB->size() != 3) {
      return;
    }
    if (posB->fType != Tag::Type::Float) {
      return;
    }
    double x = posB->at(0)->asFloat()->fValue + 0.0001;
    double y = posB->at(1)->asFloat()->fValue + 0.0001;
    double z = posB->at(2)->asFloat()->fValue + 0.0001;
    Pos3d posJ(x, y, z);
    j[u8"Pos"] = posJ.toListTag();
  }

  static void Rotation(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    auto rotB = b.listTag(u8"Rotation");
    if (!rotB) {
      return;
    }
    if (rotB->size() != 2) {
      return;
    }
    if (rotB->fType != Tag::Type::Float) {
      return;
    }
    j[u8"Rotation"] = rotB->clone();
  }

  static void Saddle(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    j[u8"Saddle"] = Bool(HasDefinitionWithPrefixAndSuffix(b, u8"+minecraft:", u8"_saddled"));
  }

  static void SaddleItemFromChestItems(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    auto chestItems = b.listTag(u8"ChestItems");
    if (!chestItems) {
      return;
    }
    CompoundTag const *slot0Item = nullptr;
    for (auto const &it : *chestItems) {
      CompoundTag const *c = it->asCompound();
      if (!c) {
        continue;
      }
      auto slot = c->byte(u8"Slot");
      if (slot != 0) {
        continue;
      }
      auto count = c->byte(u8"Count");
      if (count < 1) {
        return;
      }
      slot0Item = c;
      break;
    }
    if (!slot0Item) {
      return;
    }
    auto saddleItem = Item::From(*slot0Item, ctx, dataVersion, {});
    if (!saddleItem) {
      return;
    }
    saddleItem->erase(u8"Slot");
    j[u8"SaddleItem"] = saddleItem;
  }

  static void ShowBottom(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    CopyBoolValues(b, j, {{u8"ShowBottom"}});
  }

  static void Sitting(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    CopyBoolValues(b, j, {{u8"Sitting", u8"Sitting", false}});
  }

  static void Wave(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    /*
      uuid := b.int64(u8"DwellingUniqueID")
      c := db.Get(u8"VILLAGE_(uuid)_RAID")
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
    j[u8"Wave"] = Int(0);
  }
#pragma endregion

#pragma region Behavior Generators
  static Behavior AgeableE(i32 maxBabyAgeJava) {
    return [=](CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
      auto age = b.int32(u8"Age");
      if (!age) {
        return;
      }
      j[u8"Age"] = Int(*age + maxBabyAgeJava);
    };
  }
#pragma endregion

#pragma region Converters
  static CompoundTagPtr Animal(std::u8string const &id, CompoundTag const &b, Context &ctx, int dataVersion) {
    auto ret = LivingEntity(id, b, ctx, dataVersion);
    CompoundTag &j = *ret;
    Age(b, j, ctx, dataVersion);
    InLove(b, j, ctx, dataVersion);
    Owner(b, j, ctx, dataVersion);
    PersistenceRequiredAnimal(b, j, ctx, dataVersion);
    return ret;
  }

  static CompoundTagPtr Base(std::u8string const &id, CompoundTag const &b, Context &ctx, int dataVersion) {
    auto ret = Compound();
    CompoundTag &j = *ret;
    Air(b, j, ctx, dataVersion);
    OnGround(b, j, ctx, dataVersion);
    Pos(b, j, ctx, dataVersion);
    Rotation(b, j, ctx, dataVersion);
    PortalCooldown(b, j, ctx, dataVersion);
    FallDistance(b, j, ctx, dataVersion);
    Fire(b, j, ctx, dataVersion);
    Invulnerable(b, j, ctx, dataVersion);
    return ret;
  }

  static CompoundTagPtr LivingEntity(std::u8string const &id, CompoundTag const &b, Context &ctx, int dataVersion) {
    auto ret = Base(id, b, ctx, dataVersion);
    CompoundTag &j = *ret;
    AbsorptionAmount(b, j, ctx, dataVersion);
    ArmorItems(b, j, ctx, dataVersion);
    Brain(b, j, ctx, dataVersion);
    CanPickUpLoot(b, j, ctx, dataVersion);
    CustomName(b, j, ctx, dataVersion);
    DeathTime(b, j, ctx, dataVersion);
    FallFlying(b, j, ctx, dataVersion);
    HandItems(b, j, ctx, dataVersion);
    Health(b, j, ctx, dataVersion);
    HurtByTimestamp(b, j, ctx, dataVersion);
    HurtTime(b, j, ctx, dataVersion);
    LeftHanded(b, j, ctx, dataVersion);
    PersistenceRequiredDefault(b, j, ctx, dataVersion);
    return ret;
  }
#pragma endregion

#pragma region Utilities
  static CompoundTagPtr FindAttribute(CompoundTag const &entityB, std::u8string const &name) {
    auto attributes = entityB.listTag(u8"Attributes");
    if (!attributes) {
      return nullptr;
    }
    for (auto &attribute : *attributes) {
      auto c = attribute->asCompound();
      if (!c) {
        continue;
      }
      if (c->string(u8"Name") == name) {
        return std::dynamic_pointer_cast<CompoundTag>(attribute);
      }
    }
    return nullptr;
  }

  static void AddAttribute(CompoundTagPtr const &attribute, CompoundTag &entityJ) {
    assert(attribute);
    auto name = attribute->string(u8"Name");
    assert(name);
    if (!name) [[unlikely]] {
      return;
    }
    ListTagPtr attributes = entityJ.listTag(u8"Attributes");
    ListTagPtr replace = List<Tag::Type::Compound>();
    if (!attributes) {
      replace->push_back(attribute);
      entityJ[u8"Attributes"] = replace;
      return;
    }
    bool added = false;
    for (auto const &it : *attributes) {
      auto attr = it->asCompound();
      if (!attr) {
        continue;
      }
      auto n = attr->string(u8"Name");
      if (!n) {
        continue;
      }
      if (*n == *name) {
        replace->push_back(attribute);
        added = true;
      } else {
        replace->push_back(it);
      }
    }
    if (!added) {
      replace->push_back(attribute);
    }
    entityJ[u8"Attributes"] = replace;
  }

  static bool HasDefinition(CompoundTag const &entityB, std::u8string const &definitionToSearch) {
    auto definitions = entityB.listTag(u8"definitions");
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

  static bool HasDefinitionWithPrefixAndSuffix(CompoundTag const &entityB, std::u8string const &prefix, std::u8string const &suffix) {
    auto definitions = entityB.listTag(u8"definitions");
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

  static void Items(std::u8string subItemKey, CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    auto chestItems = b.listTag(u8"ChestItems");
    auto items = List<Tag::Type::Compound>();
    if (chestItems) {
      CompoundTagPtr subItem;
      for (auto const &it : *chestItems) {
        auto itemB = it->asCompound();
        if (!itemB) {
          continue;
        }
        auto itemJ = Item::From(*itemB, ctx, dataVersion, {});
        if (!itemJ) {
          continue;
        }
        auto slot = itemJ->byte(u8"Slot");
        if (!slot) {
          continue;
        }
        if (*slot == 0) {
          itemJ->erase(u8"Slot");
          subItem = itemJ;
        } else {
          itemJ->set(u8"Slot", Byte(*slot + 1));
          items->push_back(itemJ);
        }
      }

      if (subItem) {
        j[subItemKey] = subItem;
      }
    }
    j[u8"Items"] = items;
  }

  static void CopyChestItems(CompoundTag const &b, std::u8string const &keyB, CompoundTag &j, std::u8string const &keyJ, Context &ctx, bool skipEmpty, int dataVersion) {
    auto itemsB = b.listTag(keyB);
    if (itemsB) {
      auto itemsJ = List<Tag::Type::Compound>();
      for (auto const &it : *itemsB) {
        auto itemB = it->asCompound();
        if (!itemB) {
          continue;
        }
        auto nameB = itemB->string(u8"Name");
        if (!nameB) {
          continue;
        }
        auto itemJ = Item::From(*itemB, ctx, dataVersion, {});
        if (!itemJ) {
          continue;
        }
        if (itemJ->empty() && skipEmpty) {
          continue;
        }
        itemsJ->push_back(itemJ);
      }
      j[keyJ] = itemsJ;
    }
  }

  enum class PassengerStatus {
    Normal,
    ContainsLocalPlayer,
  };
  [[nodiscard]] static PassengerStatus Passengers(Uuid const &uid, CompoundTag const &b, Context &ctx, std::map<size_t, Uuid> &passengers) {
    auto links = b.listTag(u8"LinksTag");
    PassengerStatus st = PassengerStatus::Normal;
    if (!links) {
      return st;
    }
    for (size_t index = 0; index < links->size(); index++) {
      auto link = links->at(index)->asCompound();
      if (!link) {
        continue;
      }
      auto id = link->int64(u8"entityID");
      if (!id) {
        continue;
      }
      Uuid passengerUid;
      if (auto localPlayer = ctx.mapLocalPlayerId(*id); localPlayer) {
        passengerUid = *localPlayer;
        st = PassengerStatus::ContainsLocalPlayer;
      } else {
        passengerUid = Uuid::GenWithI64Seed(*id);
      }
      passengers[index] = passengerUid;
    }
    return st;
  }

  static CompoundTagPtr BuyItem(CompoundTag const &recipeB, std::u8string const &suffix, Context &ctx, int dataVersion) {
    using namespace std;
    auto buy = recipeB.compoundTag(u8"buy" + suffix);
    if (!buy) {
      return nullptr;
    }
    auto item = Item::From(*buy, ctx, dataVersion, {.fOfferItem = true});
    if (!item) {
      return nullptr;
    }
    auto count = recipeB.int32(u8"buyCount" + suffix);
    if (!count) {
      return nullptr;
    }
    item->set(u8"Count", Byte(*count));
    return item;
  }

  static CompoundTagPtr Recipe(CompoundTag const &recipeB, Context &ctx, int dataVersion) {
    auto sellB = recipeB.compoundTag(u8"sell");
    if (!sellB) {
      return nullptr;
    }
    auto sellJ = Item::From(*sellB, ctx, dataVersion, {.fOfferItem = true});
    if (!sellJ) {
      return nullptr;
    }

    auto buyA = BuyItem(recipeB, u8"A", ctx, dataVersion);
    auto buyB = BuyItem(recipeB, u8"B", ctx, dataVersion);
    if (!buyA) {
      return nullptr;
    }

    auto ret = Compound();

    ret->set(u8"sell", sellJ);

    ret->set(u8"buy", buyA);
    if (buyB) {
      ret->set(u8"buyB", buyB);
    } else {
      auto air = Compound();
      air->set(u8"id", u8"minecraft:air");
      air->set(u8"Count", Byte(0));
      ret->set(u8"buyB", air);
    }

    ret->set(u8"specialPrice", Int(0));

    CopyIntValues(recipeB, *ret, {{u8"demand"}, {u8"maxUses"}, {u8"uses"}, {u8"traderExp", u8"xp"}});
    CopyByteValues(recipeB, *ret, {{u8"rewardExp"}});
    CopyFloatValues(recipeB, *ret, {{u8"priceMultiplierA", u8"priceMultiplier"}, {u8"priceMultiplierB"}});

    return ret;
  }

  static float HandDropChance(CompoundTag const &itemJ, std::u8string const &entityId) {
    auto itemId = itemJ.string(u8"id");
    if (!itemId) {
      return 0.085;
    }
    if (*itemId == u8"minecraft:nautilus_shell" && entityId == u8"minecraft:drowned") {
      return 2;
    }
    if (*itemId == u8"minecraft:bone" && entityId == u8"minecraft:zombie") {
      return 2;
    }
    return 0.085;
  }

  static void InjectArmorAndOffhand(CompoundTag const &b, CompoundTag &j, Context &ctx, int dataVersion) {
    auto inventoryJ = j.listTag(u8"Inventory");
    if (!inventoryJ) {
      return;
    }
    if (auto armorB = b.listTag(u8"Armor"); armorB && armorB->size() == 4) {
      if (auto bootsB = armorB->at(3)->asCompound(); bootsB) {
        if (auto bootsJ = Item::From(*bootsB, ctx, dataVersion, {}); bootsJ && bootsJ->byte(u8"Count", 0) > 0) {
          bootsJ->set(u8"Slot", Byte(100));
          inventoryJ->push_back(bootsJ);
        }
      }
      if (auto leggingsB = armorB->at(2)->asCompound(); leggingsB) {
        if (auto leggingsJ = Item::From(*leggingsB, ctx, dataVersion, {}); leggingsJ && leggingsJ->byte(u8"Count", 0) > 0) {
          leggingsJ->set(u8"Slot", Byte(101));
          inventoryJ->push_back(leggingsJ);
        }
      }
      if (auto chestplateB = armorB->at(1)->asCompound(); chestplateB) {
        if (auto chestplateJ = Item::From(*chestplateB, ctx, dataVersion, {}); chestplateJ && chestplateJ->byte(u8"Count", 0) > 0) {
          chestplateJ->set(u8"Slot", Byte(102));
          inventoryJ->push_back(chestplateJ);
        }
      }
      if (auto helmetB = armorB->at(0)->asCompound(); helmetB) {
        if (auto helmetJ = Item::From(*helmetB, ctx, dataVersion, {}); helmetJ && helmetJ->byte(u8"Count", 0) > 0) {
          helmetJ->set(u8"Slot", Byte(103));
          inventoryJ->push_back(helmetJ);
        }
      }
    }
    if (auto offhandB = b.listTag(u8"Offhand"); offhandB && offhandB->size() > 0) {
      if (auto offhandItemB = offhandB->at(0)->asCompound(); offhandItemB) {
        if (auto offhandJ = Item::From(*offhandItemB, ctx, dataVersion, {}); offhandJ && offhandJ->byte(u8"Count", 0) > 0) {
          offhandJ->set(u8"Slot", Byte(-106));
          inventoryJ->push_back(offhandJ);
        }
      }
    }
  }
#pragma endregion

  static std::optional<LocalPlayerData> LocalPlayer(CompoundTag const &b, Context &ctx, Uuid const *uuid, int dataVersion) {
    LocalPlayerData data;

    data.fEntity = Base(u8"", b, ctx, dataVersion);
    CompoundTag &j = *data.fEntity;

    AbsorptionAmount(b, j, ctx, dataVersion);
    Brain(b, j, ctx, dataVersion);
    DeathTime(b, j, ctx, dataVersion);
    FallFlying(b, j, ctx, dataVersion);
    Health(b, j, ctx, dataVersion);
    HurtByTimestamp(b, j, ctx, dataVersion);
    HurtTime(b, j, ctx, dataVersion);

    CopyChestItems(b, u8"EnderChestInventory", j, u8"EnderItems", ctx, true, dataVersion);
    CopyChestItems(b, u8"Inventory", j, u8"Inventory", ctx, true, dataVersion);
    InjectArmorAndOffhand(b, j, ctx, dataVersion);

    CopyIntValues(b, j, {{u8"SelectedInventorySlot", u8"SelectedItemSlot"}, {u8"PlayerLevel", u8"XpLevel"}, {u8"EnchantmentSeed", u8"XpSeed"}, {u8"SpawnX"}, {u8"SpawnY"}, {u8"SpawnZ"}});
    CopyShortValues(b, j, {{u8"SleepTimer"}});
    CopyFloatValues(b, j, {{u8"PlayerLevelProgress", u8"XpP"}});
    CopyBoolValues(b, j, {{u8"HasSeenCredits", u8"seenCredits"}});

    GameMode playerGameMode = ctx.fGameMode;
    if (auto playerGameModeB = b.int32(u8"PlayerGameMode"); playerGameModeB) {
      if (auto mode = GameModeFromBedrock(*playerGameModeB); mode) {
        playerGameMode = *mode;
      }
    }
    j[u8"playerGameType"] = Int(JavaFromGameMode(playerGameMode));
    if (playerGameMode != ctx.fGameMode) {
      j[u8"previousPlayerGameType"] = Int(JavaFromGameMode(ctx.fGameMode));
    }

    auto dimensionB = b.int32(u8"DimensionId", 0);
    if (auto dimension = DimensionFromBedrockDimension(dimensionB); dimension) {
      j[u8"Dimension"] = String(JavaStringFromDimension(*dimension));
    }

    auto spawnDimensionB = b.int32(u8"SpawnDimension");
    if (spawnDimensionB) {
      if (auto dimension = DimensionFromBedrockDimension(*spawnDimensionB); dimension) {
        j[u8"SpawnDimension"] = String(JavaStringFromDimension(*dimension));
      }
    }

    j[u8"DataVersion"] = Int(bedrock::kDataVersion);

    if (auto posB = props::GetPos3f(b, u8"Pos"); posB) {
      Pos3d posJ = posB->toD();
      posJ.fY -= 1.62;
      j[u8"Pos"] = posJ.toListTag();
    }

    auto uidB = b.int64(u8"UniqueID");
    if (!uidB) {
      return std::nullopt;
    }
    Uuid uidJ;
    if (uuid) {
      uidJ = *uuid;
    } else {
      i64 v = *uidB;
      uidJ = Uuid::GenWithU64Seed(*(u64 *)&v);
    }
    j[u8"UUID"] = uidJ.toIntArrayTag();

    data.fEntityIdBedrock = *uidB;
    data.fEntityIdJava = uidJ;

    if (auto attributes = b.listTag(u8"Attributes"); attributes) {
      for (auto const &it : *attributes) {
        auto attribute = it->asCompound();
        if (!attribute) {
          continue;
        }
        auto name = attribute->string(u8"Name");
        if (!name) {
          continue;
        }
        if (name == u8"minecraft:player.level") {
          if (auto base = attribute->float32(u8"Base"); base) {
            j[u8"XpTotal"] = Int((int)roundf(*base));
          }
        } else if (name == u8"minecraft:player.exhaustion") {
          if (auto current = attribute->float32(u8"Current"); current) {
            j[u8"foodExhaustionLevel"] = Float(*current);
          }
        } else if (name == u8"minecraft:player.hunger") {
          if (auto current = attribute->float32(u8"Current"); current) {
            j[u8"foodLevel"] = Int((int)roundf(*current));
          }
        } else if (name == u8"minecraft:player.saturation") {
          if (auto current = attribute->float32(u8"Current"); current) {
            j[u8"foodSaturationLevel"] = Float(*current);
          }
        }
      }
    }

    if (auto abilitiesB = b.compoundTag(u8"abilities"); abilitiesB) {
      auto abilitiesJ = Compound();
      CopyBoolValues(*abilitiesB, *abilitiesJ, {{u8"flying"}, {u8"instabuild"}, {u8"invulnerable"}, {u8"build", u8"mayBuild"}, {u8"mayfly"}});
      CopyFloatValues(*abilitiesB, *abilitiesJ, {{u8"flySpeed"}, {u8"walkSpeed"}});
      j[u8"abilities"] = abilitiesJ;
    }

    auto wardenSpawnTracker = Compound();
    CopyIntValues(b, *wardenSpawnTracker, {{u8"WardenThreatLevelIncreaseCooldown", u8"cooldown_ticks"}, {u8"WardenThreatDecreaseTimer", u8"ticks_since_last_warning"}, {u8"WardenThreatLevel", u8"warning_level"}});
    if (!wardenSpawnTracker->empty()) {
      j[u8"warden_spawn_tracker"] = wardenSpawnTracker;
    }

    auto deathDimension = b.int32(u8"DeathDimension");
    auto deathPositionX = b.int32(u8"DeathPositionX");
    auto deathPositionY = b.int32(u8"DeathPositionY");
    auto deathPositionZ = b.int32(u8"DeathPositionZ");
    if (deathDimension && deathPositionX && deathPositionY && deathPositionZ) {
      if (auto dimension = DimensionFromBedrockDimension(*deathDimension); dimension) {
        auto lastDeathLocation = Compound();
        lastDeathLocation->set(u8"dimension", JavaStringFromDimension(*dimension));
        std::vector<i32> value(3);
        value[0] = *deathPositionX;
        value[1] = *deathPositionY;
        value[2] = *deathPositionZ;
        auto posTag = std::make_shared<IntArrayTag>(value);
        lastDeathLocation->set(u8"pos", posTag);

        data.fEntity->set(u8"LastDeathLocation", lastDeathLocation);
      }
    }

    data.fEntity->erase(u8"id");

    data.fShoulderEntityLeft = b.int64(u8"LeftShoulderRiderID");
    data.fShoulderEntityRight = b.int64(u8"RightShoulderPassengerID");

    return data;
  }

  static std::unordered_map<std::u8string_view, Converter> const *GetTable() {
    static std::unique_ptr<std::unordered_map<std::u8string_view, Converter> const> const sTable(CreateTable());
    return sTable.get();
  }

  static std::unordered_map<std::u8string_view, Converter> *CreateTable() {
    auto ret = new std::unordered_map<std::u8string_view, Converter>();

#define E(__name, __conv)                        \
  assert(ret->find(u8"" #__name) == ret->end()); \
  ret->try_emplace(u8"" #__name, __conv);

    E(skeleton, C(Same, LivingEntity, StrayConversionTime));
    E(stray, C(Same, LivingEntity));
    E(creeper, C(Same, LivingEntity, Creeper));
    E(spider, C(Same, LivingEntity));
    E(bat, C(Same, LivingEntity, Bat));
    E(painting, C(Same, Base, Painting));
    E(zombie, C(Same, LivingEntity, IsBaby, Zombie));
    E(chicken, C(Same, Animal, Chicken));
    E(item, C(Same, Base, BedrockEntityBeforeComponentsIntroduced::Item));
    E(armor_stand, C(Same, Base, AbsorptionAmount, ArmorItems, Brain, DeathTime, FallFlying, HandItems, Health, HurtByTimestamp, HurtTime, ArmorStand));
    E(ender_crystal, C(Rename(u8"end_crystal"), Base, ShowBottom, EnderCrystal));
    E(chest_minecart, C(Same, Base, Minecart, ItemsFromChestItems, ChestMinecart));
    E(hopper_minecart, C(Same, Base, Minecart, ItemsFromChestItems, HopperMinecart));
    E(boat, C(Same, Base, Boat));
    E(chest_boat, C(Same, Base, ItemsFromChestItems, Boat));
    E(slime, C(Same, LivingEntity, Size));
    E(salmon, C(Same, LivingEntity, FromBucket));
    E(parrot, C(Same, Animal, Sitting, CopyVariant));
    E(enderman, C(Same, LivingEntity, AngerTime, Enderman));
    E(zombie_pigman, C(Rename(u8"zombified_piglin"), LivingEntity, AngerTime, IsBaby, Zombie, ZombifiedPiglin));
    E(bee, C(Same, Animal, AngerTime, NoGravity, Bee));
    E(blaze, C(Same, LivingEntity));
    E(cow, C(Same, Animal));
    E(elder_guardian, C(Same, LivingEntity));
    E(cod, C(Same, LivingEntity, FromBucket));
    E(fox, C(Same, Animal, Sitting, Fox));
    E(pig, C(Same, Animal, Saddle));
    E(zoglin, C(Same, LivingEntity));
    E(horse, C(Same, Animal, Bred, EatingHaystack, Tame, Temper, HealthWithCustomizedMax, JumpStrength, MovementSpeed, Horse));
    E(husk, C(Same, LivingEntity, IsBaby, Zombie));
    E(sheep, C(Same, Animal, Sheep));
    E(cave_spider, C(Same, LivingEntity));
    E(donkey, C(Same, Animal, Bred, ChestedHorse, EatingHaystack, ItemsWithSaddleItem, Tame, Temper, HealthWithCustomizedMax, MovementSpeed));
    E(drowned, C(Same, LivingEntity, IsBaby, Zombie));
    E(endermite, C(Same, LivingEntity, Endermite));
    E(evocation_illager, C(Rename(u8"evoker"), LivingEntity, CanJoinRaid, PatrolLeader, Patrolling, Wave, Evoker));
    E(cat, C(Same, Animal, CollarColor, Sitting, Cat));
    E(guardian, C(Same, LivingEntity));
    E(llama, C(LlamaName, Animal, Bred, ChestedHorse, EatingHaystack, ItemsWithDecorItem, Tame, Temper, CopyVariant, Strength, Llama));
    E(trader_llama, C(Same, Animal, Bred, ChestedHorse, EatingHaystack, ItemsWithDecorItem, Tame, Temper, CopyVariant, Strength, Llama));
    E(magma_cube, C(Same, LivingEntity, Size));
    E(mooshroom, C(Same, Animal, Mooshroom));
    E(mule, C(Same, Animal, Bred, ChestedHorse, EatingHaystack, ItemsWithSaddleItem, Tame, Temper, HealthWithCustomizedMax, MovementSpeed));
    E(panda, C(Same, Animal, Panda));
    E(phantom, C(Same, LivingEntity, Phantom));
    E(ghast, C(Same, LivingEntity, Ghast));
    E(pillager, C(Same, LivingEntity, CanJoinRaid, Inventory, PatrolLeader, Patrolling, Wave));
    E(pufferfish, C(Same, LivingEntity, FromBucket, Pufferfish));
    E(rabbit, C(Same, Animal, Rabbit));
    E(ravager, C(Same, LivingEntity, AttackTick, CanJoinRaid, PatrolLeader, Patrolling, Wave, Ravager));
    E(silverfish, C(Same, LivingEntity));
    E(skeleton_horse, C(Same, Animal, Bred, EatingHaystack, Tame, Temper, JumpStrength, MovementSpeed, SkeletonHorse));
    E(polar_bear, C(Same, Animal, AngerTime));
    E(glow_squid, C(Same, LivingEntity, GlowSquid));
    E(squid, C(Same, LivingEntity));
    E(strider, C(Same, Animal, Saddle));
    E(turtle, C(Same, Animal, Turtle));
    E(tropicalfish, C(Rename(u8"tropical_fish"), LivingEntity, FromBucket, TropicalFish));
    E(minecart, C(Same, Base, Minecart));
    E(vex, C(Same, LivingEntity, NoGravity));
    E(villager_v2, C(Rename(u8"villager"), Animal, FoodLevel, Inventory, Offers, Villager));
    E(wandering_trader, C(Same, LivingEntity, Age, Inventory, Offers, WanderingTrader));
    E(wolf, C(Same, Animal, AngerTime, CollarColor, Sitting));
    E(zombie_horse, C(Same, Animal, Bred, EatingHaystack, Tame, Temper, JumpStrength, MovementSpeed));
    E(zombie_villager_v2, C(Rename(u8"zombie_villager"), LivingEntity, IsBaby, ConversionTime, Offers, Zombie, Villager));
    E(snow_golem, C(Same, LivingEntity, SnowGolem));
    E(shulker, C(Same, LivingEntity, Shulker));
    E(wither_skeleton, C(Same, LivingEntity));
    E(witch, C(Same, LivingEntity, CanJoinRaid, PatrolLeader, Patrolling, Wave));
    E(iron_golem, C(Same, LivingEntity, AngerTime, IronGolem));
    E(tnt_minecart, C(Same, Base, Minecart, TntMinecart));
    E(goat, C(Same, Animal, Goat));
    E(axolotl, C(Same, Animal, FromBucket, Axolotl));
    E(wither, C(Same, LivingEntity, Wither));
    E(piglin, C(Same, LivingEntity, Inventory, IsBaby, Piglin));
    E(piglin_brute, C(Same, LivingEntity, PiglinBrute));
    E(hoglin, C(Same, Animal, Hoglin));
    E(arrow, C(Same, Base, Owner));
    E(ender_dragon, C(Same, LivingEntity, EnderDragon));
    E(falling_block, C(Same, Base, FallingBlock));

    E(frog, C(Same, Animal, PersistenceRequiredDefault, BedrockEntityBeforeComponentsIntroduced::Frog));
    E(warden, C(Same, LivingEntity));
    E(allay, C(Same, LivingEntity, NoGravity, Inventory, Allay));
    E(tadpole, C(Same, LivingEntity, AgeableE(24000), FromBucket));

    E(camel, C(Same, Animal, Bred, SaddleItemFromChestItems, Tame, Camel));
    E(sniffer, C(Same, Animal));
    E(ocelot, C(Same, Animal, Age, Ocelot));
    E(vindicator, C(Same, LivingEntity));
    E(xp_orb, C(Rename(u8"experience_orb"), Base, ExperienceOrb));
#undef E
    return ret;
  }
};

} // namespace je2be::bedrock
