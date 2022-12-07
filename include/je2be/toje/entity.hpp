#pragma once

namespace je2be::toje {

class Entity {
public:
  static CompoundTagPtr ItemFrameFromBedrock(mcfile::Dimension d, Pos3i pos, mcfile::be::Block const &blockJ, CompoundTag const &blockEntityB, Context &ctx) {
    auto ret = Compound();
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
    CompoundTagPtr fEntity;

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
    e->set("UUID", uuid.toIntArrayTag());

    if (ctx.setShoulderEntityIfItIs(*uid, e)) {
      return std::nullopt;
    }

    Result r;

    auto st = Passengers(uuid, entityB, ctx, r.fPassengers);
    if (st == PassengerStatus::ContainsLocalPlayer) {
      ctx.setRootVehicle(uuid);
    }

    auto leasherId = entityB.int64("LeasherID", -1);
    if (leasherId != -1) {
      r.fLeasherId = leasherId;

      // NOTE: This "UUID" property will be replaced to "X", "Y", and "Z" when the leasher is a leash_knot.
      auto leasherIdJ = Uuid::GenWithI64Seed(leasherId);
      auto leash = Compound();
      leash->set("UUID", leasherIdJ.toIntArrayTag());
      e->set("Leash", leash);
    }

    r.fUuid = uuid;
    r.fEntity = e;
    return r;
  }

  using Converter = std::function<CompoundTagPtr(std::string const &id, CompoundTag const &eneityB, Context &ctx)>;
  using Namer = std::function<std::string(std::string const &nameB, CompoundTag const &entityB)>;
  using Behavior = std::function<void(CompoundTag const &entityB, CompoundTag &entityJ, Context &ctx)>;

  struct C {
    template <class... Arg>
    C(Namer namer, Converter base, Arg... behaviors) : fNamer(namer), fBase(base), fBehaviors(std::initializer_list<Behavior>{behaviors...}) {}

    CompoundTagPtr operator()(std::string const &id, CompoundTag const &entityB, Context &ctx) const {
      auto name = fNamer(id, entityB);
      auto t = fBase(id, entityB, ctx);
      t->set("id", String(name));
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
  static std::string LlamaName(std::string const &nameB, CompoundTag const &entityB) {
    if (HasDefinition(entityB, "+minecraft:llama_wandering_trader")) {
      // legacy
      return "minecraft:trader_llama";
    } else {
      return "minecraft:llama";
    }
  }

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
  static void Allay(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    if (auto ownerNew = b.int64("OwnerNew"); ownerNew) {
      Uuid uuid;
      if (auto mapped = ctx.mapLocalPlayerId(*ownerNew); mapped) {
        uuid = *mapped;
      } else {
        uuid = Uuid::GenWithI64Seed(*ownerNew);
      }
      auto brain = Compound();
      auto memories = Compound();
      auto likedPlayer = Compound();
      likedPlayer->set("value", uuid.toIntArrayTag());
      memories->set("minecraft:liked_player", likedPlayer);
      brain->set("memories", memories);
      j["Brain"] = brain;
    }
    j["PersistenceRequired"] = Bool(false);

    j["CanDuplicate"] = Bool(true);
    CopyLongValues(b, j, {{"AllayDuplicationCooldown", "DuplicationCooldown"}});
  }

  static void ArmorStand(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    j.erase("ArmorDropChances");
    j.erase("HandDropChances");
    j["DisabledSlots"] = Int(false);
    j["Invisible"] = Bool(false);
    j["NoBasePlate"] = Bool(false);
    j["Small"] = Bool(false);

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
    j["ShowArms"] = Bool(showArms);
  }

  static void Axolotl(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    auto variantB = b.int32("Variant", 0);
    j["Variant"] = Int(je2be::Axolotl::JavaVariantFromBedrockVariant(variantB));
  }

  static void Bat(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    CopyBoolValues(b, j, {{"BatFlags"}});
  }

  static void Bee(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    j["CannotEnterHiveTicks"] = Int(0);
    j["CropsGrownSincePollination"] = Int(0);
    auto hasNectar = HasDefinition(b, "+has_nectar");
    j["HasNectar"] = Bool(hasNectar);
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
    j["HasStung"] = Bool(false);
  }

  static void Boat(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    auto variant = b.int32("Variant", 0);
    auto type = Boat::JavaTypeFromBedrockVariant(variant);
    j["Type"] = String(type);

    if (!ctx.fDataPack1_20Update && type == "bamboo") {
      ctx.fDataPack1_20Update = true;
    }

    if (auto rotB = props::GetRotation(b, "Rotation"); rotB) {
      je2be::Rotation rotJ(Rotation::ClampDegreesBetweenMinus180And180(rotB->fYaw - 90), rotB->fPitch);
      j["Rotation"] = rotJ.toListTag();
    }

    if (auto posB = props::GetPos3f(b, "Pos"); posB) {
      auto posJ = posB->toD();
      if (b.boolean("OnGround", false)) {
        posJ.fY = round(posB->fY - 0.375);
      }
      j["Pos"] = posJ.toListTag();
    }
  }

  static void Camel(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    ctx.fDataPack1_20Update = true;
    CopyBoolValues(b, j, {{"Sitting", "IsSitting"}});
    j["Temper"] = Int(0);
  }

  static void Cat(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    using Cat = je2be::Cat;

    auto variantB = b.int32("Variant", 8);
    Cat::Type catType = Cat::CatTypeFromBedrockVariant(variantB);
    j["variant"] = String("minecraft:" + Cat::JavaVariantFromCatType(catType));
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
        j["EggLayTime"] = Int(*spawnTimer);
        break;
      }
    }
    j["IsChickenJockey"] = Bool(false);
  }

  static void Creeper(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    j["ExplosionRadius"] = Byte(3);
    j["Fuse"] = Short(30);
    j["ignited"] = Bool(false);
    if (HasDefinition(b, "+minecraft:charged_creeper")) {
      j["powered"] = Bool(true);
    }
  }

  static void EnderCrystal(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    auto x = b.int32("BlockTargetX");
    auto y = b.int32("BlockTargetY");
    auto z = b.int32("BlockTargetZ");
    if (x && y && z) {
      auto beamTarget = Compound();
      beamTarget->set("X", Int(*x));
      beamTarget->set("Y", Int(*y));
      beamTarget->set("Z", Int(*z));
      j["BeamTarget"] = beamTarget;
    }
  }

  static void EnderDragon(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    auto deathTime = b.int32("DeathTime", 0);
    j["DragonDeathTime"] = Int(deathTime);
    if (deathTime > 0) {
      j["DragonPhase"] = Int(9);
    } else {
      j["DragonPhase"] = Int(0);
    }
    j["PersistenceRequired"] = Bool(false);
  }

  static void Enderman(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    using namespace std;
    auto carriedBlockTagB = b.compoundTag("carriedBlock");
    if (carriedBlockTagB) {
      auto carriedBlockB = mcfile::be::Block::FromCompound(*carriedBlockTagB);
      if (carriedBlockB) {
        auto carriedBlockJ = BlockData::From(*carriedBlockB);
        if (carriedBlockJ) {
          if (carriedBlockJ->fId == mcfile::blocks::minecraft::grass_block) {
            map<string, string> props(carriedBlockJ->fProperties);
            props["snowy"] = "false";
            carriedBlockJ = make_shared<mcfile::je::Block const>(carriedBlockJ->fName, props);
          }
          j["carriedBlockState"] = carriedBlockJ->toCompoundTag();
        }
      }
    }
  }

  static void Endermite(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    CopyIntValues(b, j, {{"Lifetime"}});
  }

  static void Evoker(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    j["SpellTicks"] = Int(0);
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

  static void FallingBlock(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    if (auto fallingBlockB = b.compoundTag("FallingBlock"); fallingBlockB) {
      if (auto blockB = mcfile::be::Block::FromCompound(*fallingBlockB); blockB) {
        if (auto blockJ = BlockData::From(*blockB); blockJ) {
          j["BlockState"] = blockJ->toCompoundTag();
        }
      }
    }

    if (auto time = b.byte("Time"); time) {
      int8_t i8 = *time;
      uint8_t u8 = *(uint8_t *)&i8;
      j["Time"] = Int(u8);
    }
  }

  static void Fox(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    auto variant = b.int32("Variant", 0);
    std::string type;
    if (variant == 1) {
      type = "snow";
    } else {
      type = "red";
    }
    j["Type"] = String(type);

    j["Sleeping"] = Bool(HasDefinition(b, "+minecraft:fox_ambient_sleep"));
    j["Crouching"] = Bool(false);

    auto trusted = List<Tag::Type::IntArray>();
    auto trustedPlayers = b.int32("TrustedPlayersAmount", 0);
    if (trustedPlayers > 0) {
      for (int i = 0; i < trustedPlayers; i++) {
        auto uuidB = b.int64("TrustedPlayer" + std::to_string(i));
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
    j["Trusted"] = trusted;
  }

  static void Frog(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    auto variantB = b.int32("Variant", 0);
    auto variantJ = Frog::JavaVariantFromBedrockVariant(variantB);
    j["variant"] = String(variantJ);
  }

  static void Ghast(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    j["ExplosionPower"] = Byte(1);
  }

  static void GlowSquid(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    j["DarkTicksRemaining"] = Int(0);
  }

  static void Goat(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    bool screamer = HasDefinition(b, "+goat_screamer") || HasDefinition(b, "+ram_screamer") || HasDefinition(b, "+interact_screamer");
    j["IsScreamingGoat"] = Bool(screamer);

    auto hornCount = b.int32("GoatHornCount", 0);
    j["HasRightHorn"] = Bool(hornCount == 2); // Goat loses right horn first in BE.
    j["HasLeftHorn"] = Bool(hornCount > 0);
  }

  static void Hoglin(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    if (HasDefinition(b, "-angry_hoglin")) {
      j["CannotBeHunted"] = Bool(true);
    }
  }

  static void Horse(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    auto armorDropChances = List<Tag::Type::Float>();
    armorDropChances->push_back(Float(0.085));
    armorDropChances->push_back(Float(0.085));
    armorDropChances->push_back(Float(0));
    armorDropChances->push_back(Float(0.085));
    j["ArmorDropChances"] = armorDropChances;

    int32_t variantB = b.int32("Variant", 0);
    int32_t markVariantB = b.int32("MarkVariant", 0);
    uint32_t uVariantB = *(uint32_t *)&variantB;
    uint32_t uMarkVariantB = *(uint32_t *)&markVariantB;
    uint32_t uVariantJ = (0xf & uVariantB) | ((0xf & uMarkVariantB) << 8);
    j["Variant"] = Int(*(int32_t *)&uVariantJ);

    if (auto chestItems = b.listTag("ChestItems"); chestItems) {
      // NOTE: horse's b["Chested"] is always false, so we cannot use ItemsWithSaddleItem function for horse.
      for (auto const &it : *chestItems) {
        auto itemB = it->asCompound();
        if (!itemB) {
          continue;
        }
        auto itemJ = Item::From(*itemB, ctx);
        if (!itemJ) {
          continue;
        }
        auto slotJ = itemJ->byte("Slot");
        itemJ->erase("Slot");
        if (slotJ == 0) {
          j["SaddleItem"] = itemJ;
        } else if (slotJ == 1) {
          j["ArmorItem"] = itemJ;
        }
      }
    }
  }

  static void IronGolem(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    auto target = b.int64("TargetID", -1);
    if (target != -1) {
      auto angryAt = Uuid::GenWithI64Seed(target);
      j["AngryAt"] = angryAt.toIntArrayTag();
    }

    j["PlayerCreated"] = Bool(HasDefinition(b, "+minecraft:player_created"));
  }

  static void Item(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    j["PickupDelay"] = Short(0);
    auto itemB = b.compoundTag("Item");
    if (itemB) {
      auto itemJ = toje::Item::From(*itemB, ctx);
      if (itemJ) {
        j["Item"] = itemJ;
      }
    }
    CopyShortValues(b, j, {{"Age"}, {"Health"}});
  }

  static void Llama(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    if (HasDefinition(b, "+minecraft:llama_wandering_trader")) {
      // Ignoring "IsTamed" property here, because it is always true for natural spawned wandering trader llama.
      auto tamed = b.int64("OwnerNew", -1) != -1;
      j["Tame"] = Bool(tamed);
    }
  }

  static void Mooshroom(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    auto variant = b.int32("Variant", 0);
    std::string type = "red";
    if (variant == 1) {
      type = "brown";
    }
    j["Type"] = String(type);
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
        j["MainGene"] = String(Panda::JavaGeneNameFromGene(Panda::GeneFromBedrockAllele(*main)));
        j["HiddenGene"] = String(Panda::JavaGeneNameFromGene(Panda::GeneFromBedrockAllele(*hidden)));
      }
    }
  }

  static void Phantom(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    j["Size"] = Int(0);
  }

  static void Piglin(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    if (HasDefinition(b, "+not_hunter")) {
      j["CannotHunt"] = Bool(true);
    }
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

    auto value = Compound();
    value->set("dimension", String(JavaStringFromDimension(*dim)));
    std::vector<int32_t> pos;
    pos.push_back((int)round(homePos->fX));
    pos.push_back((int)round(homePos->fY));
    pos.push_back((int)round(homePos->fZ));
    auto posTag = std::make_shared<IntArrayTag>(pos);
    value->set("pos", posTag);

    auto homeTag = Compound();
    homeTag->set("value", value);

    auto memories = Compound();
    memories->set("minecraft:home", homeTag);

    auto brain = Compound();
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
    j["PuffState"] = Int(state);
  }

  static void Rabbit(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    CopyIntValues(b, j, {{"MoreCarrotTicks"}});
    CopyIntValues(b, j, {{"Variant", "RabbitType", 0}});
  }

  static void Ravager(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    j["RoarTick"] = Int(0);
    j["StunTick"] = Int(0);
  }

  static void Sheep(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    CopyBoolValues(b, j, {{"Sheared"}});
    CopyByteValues(b, j, {{"Color"}});
  }

  static void Shulker(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    auto variant = b.int32("Variant", 16);
    j["Color"] = Byte(variant);
  }

  static void SkeletonHorse(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    // summon minecraft:skeleton_horse ~ ~ ~ minecraft:set_trap
    j["SkeletonTrap"] = Bool(HasDefinition(b, "+minecraft:skeleton_trap"));
    j["SkeletonTrapTime"] = Int(0);
  }

  static void SnowGolem(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    bool pumpkin = !HasDefinition(b, "+minecraft:snowman_sheared");
    j["Pumpkin"] = Bool(pumpkin);
  }

  static void TntMinecart(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    auto fuse = b.byte("Fuse", -1);
    j["TNTFuse"] = Int(fuse);
  }

  static void TropicalFish(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    auto tf = TropicalFish::FromBedrockBucketTag(b);
    j["Variant"] = Int(tf.toJavaVariant());
  }

  static void Turtle(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    auto homePos = props::GetPos3f(b, "HomePos");
    if (homePos) {
      j["HomePosX"] = Int(roundf(homePos->fX));
      j["HomePosY"] = Int(roundf(homePos->fY));
      j["HomePosZ"] = Int(roundf(homePos->fZ));
    }

    j["HasEgg"] = Bool(HasDefinition(b, "-minecraft:wants_to_lay_egg") || HasDefinition(b, "+minecraft:wants_to_lay_egg"));
  }

  static void Villager(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    CopyIntValues(b, j, {{"TradeExperience", "Xp", 0}});

    j.erase("InLove");

    auto dataJ = Compound();

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
    dataJ->set("profession", String("minecraft:" + profession.string()));

    VillagerType::Variant variant = VillagerType::Plains;
    auto markVariantB = b.int32("MarkVariant");
    if (markVariantB) {
      variant = static_cast<VillagerType::Variant>(*markVariantB);
    } else {
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
      } else if (HasDefinition(b, "+taiga_villager")) {
        variant = VillagerType::Taiga;
      }
    }
    VillagerType type(variant);
    dataJ->set("type", String("minecraft:" + type.string()));

    auto tradeTier = b.int32("TradeTier", 0);
    dataJ->set("level", Int(tradeTier + 1));

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
    int despawnDelay = 0;
    if (auto timestamp = b.int64("TimeStamp"); timestamp) {
      despawnDelay = std::max<int64_t>(0, *timestamp - ctx.fGameTick);
    }
    j["DespawnDelay"] = Int(despawnDelay);
  }

  static void Wither(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    j["Invul"] = Int(0);
    if (auto healthB = FindAttribute(b, "minecraft:health"); healthB) {
      auto max = healthB->float32("Max");
      auto current = healthB->float32("Current");
      if (max && current && *max > 0) {
        auto ratio = std::min(std::max(*current / *max, 0.0f), 1.0f);
        float healthJ = 300.0f * ratio;
        j["Health"] = Float(healthJ);
      }
    }
  }

  static void Zombie(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    j["DrownedConversionTime"] = Int(-1);
    j["CanBreakDoors"] = Bool(false);
    j["InWaterTime"] = Int(-1);
  }

  static void ZombifiedPiglin(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    auto targetId = b.int64("TargetID", -1);
    if (targetId != -1) {
      auto angryAt = Uuid::GenWithI64Seed(targetId);
      j["AngryAt"] = angryAt.toIntArrayTag();
    }
  }
#pragma endregion

#pragma region Behaviors
  static void AbsorptionAmount(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    j["AbsorptionAmount"] = Float(0);
  }

  static void Age(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    CopyIntValues(b, j, {{"Age", "Age", 0}});
  }

  static void Air(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    CopyShortValues(b, j, {{"Air", "Air", 300}});
  }

  static void AngerTime(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    j["AngerTime"] = Int(0);
  }

  static void ArmorItems(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    auto armorsB = b.listTag("Armor");
    auto armorsJ = List<Tag::Type::Compound>();
    auto chances = List<Tag::Type::Float>();
    if (armorsB) {
      std::vector<CompoundTagPtr> armors;
      for (auto const &it : *armorsB) {
        auto armorB = it->asCompound();
        CompoundTagPtr armorJ;
        if (armorB) {
          armorJ = Item::From(*armorB, ctx);
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
    j["ArmorItems"] = armorsJ;
    j["ArmorDropChances"] = chances;
  }

  static void AttackTick(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    auto attackTime = b.int16("AttackTime");
    if (attackTime) {
      j["AttackTick"] = Int(*attackTime);
    }
  }

  static void Brain(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    auto memories = Compound();
    auto brain = Compound();
    brain->set("memories", memories);
    j["Brain"] = brain;
  }

  static void Bred(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    j["Bred"] = Bool(false);
  }

  static void CanJoinRaid(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    j["CanJoinRaid"] = Bool(HasDefinition(b, "+minecraft:raid_configuration"));
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
      j["CollarColor"] = Byte(14);
    } else {
      CopyByteValues(b, j, {{"Color", "CollarColor"}});
    }
  }

  static void ConversionTime(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    j["ConversionTime"] = Int(-1);
  }

  static void CopyVariant(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    CopyIntValues(b, j, {{"Variant"}});
  }

  static void CustomName(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    auto name = b.string("CustomName");
    if (name) {
      nlohmann::json json;
      json["text"] = *name;
      j["CustomName"] = String(nlohmann::to_string(json));
    }
  }

  static void DeathTime(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    CopyShortValues(b, j, {{"DeathTime"}});
  }

  static void Debug(CompoundTag const &b, CompoundTag &j, Context &ctx) {
  }

  static void EatingHaystack(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    j["EatingHaystack"] = Bool(false);
  }

  static void FallDistance(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    CopyFloatValues(b, j, {{"FallDistance"}});
  }

  static void FallFlying(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    j["FallFlying"] = Bool(false);
  }

  static void Fire(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    j["Fire"] = Short(-1);
  }

  static void FoodLevel(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    j["FoodLevel"] = Byte(0);
  }

  static void FromBucket(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    auto naturalSpawn = b.boolean("NaturalSpawn", true);
    j["FromBucket"] = Bool(!naturalSpawn);
  }

  static void HandItems(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    auto itemsJ = List<Tag::Type::Compound>();
    auto chances = List<Tag::Type::Float>();
    auto identifier = b.string("identifier", "");
    for (std::string key : {"Mainhand", "Offhand"}) {
      auto listB = b.listTag(key);
      CompoundTagPtr itemJ;
      if (listB && !listB->empty()) {
        auto c = listB->at(0)->asCompound();
        if (c) {
          itemJ = Item::From(*c, ctx);
        }
      }
      if (!itemJ) {
        itemJ = Compound();
      }
      itemsJ->push_back(itemJ);
      chances->push_back(Float(HandDropChance(*itemJ, identifier)));
    }
    j["HandItems"] = itemsJ;
    j["HandDropChances"] = chances;
  }

  static void Health(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    auto healthB = FindAttribute(b, "minecraft:health");
    if (!healthB) {
      return;
    }
    auto current = healthB->float32("Current");
    if (current) {
      j["Health"] = Float(*current);
    }
  }

  static void HealthWithCustomizedMax(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    auto healthB = FindAttribute(b, "minecraft:health");
    if (!healthB) {
      return;
    }
    auto current = healthB->float32("Current");
    auto max = healthB->float32("Max");
    if (current && max) {
      j["Health"] = Float(*current);
      auto attr = Compound();
      attr->set("Name", String("minecraft:generic.max_health"));
      attr->set("Base", Double(*max));
      AddAttribute(attr, j);
    }
  }

  static void HopperMinecart(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    auto enabled = HasDefinition(b, "+minecraft:hopper_active");
    j["Enabled"] = Bool(enabled);

    j["TransferCooldown"] = Int(0);
  }

  static void HurtByTimestamp(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    j["HurtByTimestamp"] = Int(0);
  }

  static void HurtTime(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    CopyShortValues(b, j, {{"HurtTime"}});
  }

  static void InLove(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    CopyIntValues(b, j, {{"InLove", "InLove", 0}});
  }

  static void Inventory(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    auto itemsB = b.listTag("ChestItems");
    if (itemsB) {
      auto itemsJ = List<Tag::Type::Compound>();
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
        itemJ->erase("Slot");
        itemsJ->push_back(itemJ);
      }
      j["Inventory"] = itemsJ;
    }
  }

  static void Invulnerable(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    CopyBoolValues(b, j, {{"Invulnerable"}});
  }

  static void IsBaby(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    CopyBoolValues(b, j, {{"IsBaby"}});
  }

  static void ItemsFromChestItems(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    CopyChestItems(b, "ChestItems", j, "Items", ctx, false);
  }

  static void ItemsWithDecorItem(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    if (auto chested = b.boolean("Chested", false); chested) {
      Items("DecorItem", b, j, ctx);
    }

    auto armorsJ = j.listTag("ArmorItems");
    if (!armorsJ) {
      return;
    }
    if (armorsJ->size() < 3) {
      return;
    }
    armorsJ->fValue[2] = Compound();
  }

  static void ItemsWithSaddleItem(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    if (auto chested = b.boolean("Chested", false); chested) {
      Items("SaddleItem", b, j, ctx);
    }
  }

  static void JumpStrength(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    std::string nameB = "minecraft:horse.jump_strength";
    std::string nameJ = nameB;
    auto jumpStrengthAttribute = FindAttribute(b, nameB);
    if (!jumpStrengthAttribute) {
      return;
    }
    auto jumpStrength = jumpStrengthAttribute->float32("Current");
    if (!jumpStrength) {
      return;
    }
    auto attr = Compound();
    std::string name = nameJ;
    (*attr)["Base"] = Double(*jumpStrength);
    (*attr)["Name"] = String(name);
    AddAttribute(attr, j);
  }

  static void Minecart(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    auto posB = props::GetPos3f(b, "Pos");
    auto onGround = b.boolean("OnGround");
    if (posB && onGround) {
      Pos3d posJ = posB->toD();
      int iy = (int)floor(posJ.fY);
      double dy = posJ.fY - iy;
      // Java
      //   on ground: ground level +0
      //   on rail: ground level +0.0625
      // Bedrock
      //   on ground: graound level +0.35
      //   on rail: ground level +0.5
      if (*onGround) {
        posJ.fY = posB->fY - 0.35;
      } else if (dy == 0.5) {
        posJ.fY = posB->fY - 0.5 + 0.0625;
      }
      j["Pos"] = posJ.toListTag();
    }
  }

  static void MovementSpeed(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    auto movementB = FindAttribute(b, "minecraft:movement");
    if (!movementB) {
      return;
    }
    auto current = movementB->float32("Current");
    if (current) {
      auto attr = Compound();
      attr->set("Name", String("minecraft:generic.movement_speed"));
      attr->set("Base", Double(*current));
      AddAttribute(attr, j);
    }
  }

  static void NoGravity(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    j["NoGravity"] = Bool(true);
  }

  static void Offers(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    CompoundTagPtr offersB = b.compoundTag("Offers");
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

    auto recipesJ = List<Tag::Type::Compound>();
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

    auto offersJ = Compound();
    offersJ->set("Recipes", recipesJ);
    j["Offers"] = offersJ;
  }

  static void Size(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    auto sizeB = b.byte("Size", 1);
    j["Size"] = Int(sizeB - 1);
  }

  static void StrayConversionTime(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    j["StrayConversionTime"] = Int(-1);
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
    j["LeftHanded"] = Bool(false);
  }

  static void OnGround(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    CopyBoolValues(b, j, {{"OnGround"}});
  }

  static void Owner(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    auto ownerNew = b.int64("OwnerNew");
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

    j["variant"] = String(motiveJ);
    j["facing"] = Byte(directionB);
    j["TileX"] = Int(std::round(tile->fX));
    j["TileY"] = Int(std::round(tile->fY));
    j["TileZ"] = Int(std::round(tile->fZ));
  }

  static void PatrolLeader(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    CopyBoolValues(b, j, {{"IsIllagerCaptain", "PatrolLeader"}});
  }

  static void Patrolling(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    j["Patrolling"] = Bool(false);
  }

  static void PersistenceRequiredDefault(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    CopyBoolValues(b, j, {{"Persistent", "PersistenceRequired", false}});
  }

  static void PersistenceRequiredAnimal(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    j["PersistenceRequired"] = Bool(false);
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
    j["Saddle"] = Bool(HasDefinitionWithPrefixAndSuffix(b, "+minecraft:", "_saddled"));
  }

  static void SaddleItemFromChestItems(CompoundTag const &b, CompoundTag &j, Context &ctx) {
    auto chestItems = b.listTag("ChestItems");
    if (!chestItems) {
      return;
    }
    CompoundTag const *slot0Item = nullptr;
    for (auto const &it : *chestItems) {
      CompoundTag const *c = it->asCompound();
      if (!c) {
        continue;
      }
      auto slot = c->byte("Slot");
      if (slot != 0) {
        continue;
      }
      auto count = c->byte("Count");
      if (count < 1) {
        return;
      }
      slot0Item = c;
      break;
    }
    if (!slot0Item) {
      return;
    }
    auto saddleItem = Item::From(*slot0Item, ctx);
    if (!saddleItem) {
      return;
    }
    saddleItem->erase("Slot");
    j["SaddleItem"] = saddleItem;
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
    j["Wave"] = Int(0);
  }
#pragma endregion

#pragma region Behavior Generators
  static Behavior AgeableE(int32_t maxBabyAgeJava) {
    return [=](CompoundTag const &b, CompoundTag &j, Context &ctx) {
      auto age = b.int32("Age");
      if (!age) {
        return;
      }
      j["Age"] = Int(*age + maxBabyAgeJava);
    };
  }
#pragma endregion

#pragma region Converters
  static CompoundTagPtr Animal(std::string const &id, CompoundTag const &b, Context &ctx) {
    auto ret = LivingEntity(id, b, ctx);
    CompoundTag &j = *ret;
    Age(b, j, ctx);
    InLove(b, j, ctx);
    Owner(b, j, ctx);
    PersistenceRequiredAnimal(b, j, ctx);
    return ret;
  }

  static CompoundTagPtr Base(std::string const &id, CompoundTag const &b, Context &ctx) {
    auto ret = Compound();
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

  static CompoundTagPtr LivingEntity(std::string const &id, CompoundTag const &b, Context &ctx) {
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
    PersistenceRequiredDefault(b, j, ctx);
    return ret;
  }
#pragma endregion

#pragma region Utilities
  static CompoundTagPtr FindAttribute(CompoundTag const &entityB, std::string const &name) {
    auto attributes = entityB.listTag("Attributes");
    if (!attributes) {
      return nullptr;
    }
    for (auto &attribute : *attributes) {
      auto c = attribute->asCompound();
      if (!c) {
        continue;
      }
      if (c->string("Name") == name) {
        return std::dynamic_pointer_cast<CompoundTag>(attribute);
      }
    }
    return nullptr;
  }

  static void AddAttribute(CompoundTagPtr const &attribute, CompoundTag &entityJ) {
    assert(attribute);
    auto name = attribute->string("Name");
    assert(name);
    ListTagPtr attributes = entityJ.listTag("Attributes");
    ListTagPtr replace = List<Tag::Type::Compound>();
    if (!attributes) {
      replace->push_back(attribute);
      entityJ["Attributes"] = replace;
      return;
    }
    bool added = false;
    for (auto const &it : *attributes) {
      auto attr = it->asCompound();
      if (!attr) {
        continue;
      }
      auto n = attr->string("Name");
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
    entityJ["Attributes"] = replace;
  }

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
    auto items = List<Tag::Type::Compound>();
    if (chestItems) {
      CompoundTagPtr subItem;
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
          itemJ->set("Slot", Byte(*slot + 1));
          items->push_back(itemJ);
        }
      }

      if (subItem) {
        j[subItemKey] = subItem;
      }
    }
    j["Items"] = items;
  }

  static void CopyChestItems(CompoundTag const &b, std::string const &keyB, CompoundTag &j, std::string const &keyJ, Context &ctx, bool skipEmpty) {
    auto itemsB = b.listTag(keyB);
    if (itemsB) {
      auto itemsJ = List<Tag::Type::Compound>();
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
    auto links = b.listTag("LinksTag");
    PassengerStatus st = PassengerStatus::Normal;
    if (!links) {
      return st;
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

  static CompoundTagPtr BuyItem(CompoundTag const &recipeB, std::string const &suffix, Context &ctx) {
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
    item->set("Count", Byte(*count));
    return item;
  }

  static CompoundTagPtr Recipe(CompoundTag const &recipeB, Context &ctx) {
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

    auto ret = Compound();

    ret->set("sell", sellJ);

    ret->set("buy", buyA);
    if (buyB) {
      ret->set("buyB", buyB);
    } else {
      auto air = Compound();
      air->set("id", String("minecraft:air"));
      air->set("Count", Byte(1));
      ret->set("buyB", air);
    }

    ret->set("specialPrice", Int(0));

    CopyIntValues(recipeB, *ret, {{"demand"}, {"maxUses"}, {"uses"}, {"traderExp", "xp"}});
    CopyByteValues(recipeB, *ret, {{"rewardExp"}});
    CopyFloatValues(recipeB, *ret, {{"priceMultiplierA", "priceMultiplier"}, {"priceMultiplierB"}});

    return ret;
  }

  static float HandDropChance(CompoundTag const &itemJ, std::string const &entityId) {
    auto itemId = itemJ.string("id");
    if (!itemId) {
      return 0.085;
    }
    if (*itemId == "minecraft:nautilus_shell" && entityId == "minecraft:drowned") {
      return 2;
    }
    if (*itemId == "minecraft:bone" && entityId == "minecraft:zombie") {
      return 2;
    }
    return 0.085;
  }
#pragma endregion

  struct LocalPlayerData {
    CompoundTagPtr fEntity;
    int64_t fEntityIdBedrock;
    Uuid fEntityIdJava;
    std::optional<int64_t> fShoulderEntityLeft;
    std::optional<int64_t> fShoulderEntityRight;
  };

  static std::optional<LocalPlayerData> LocalPlayer(CompoundTag const &b, Context &ctx, std::optional<Uuid> uuid) {
    LocalPlayerData data;

    data.fEntity = Base("", b, ctx);
    CompoundTag &j = *data.fEntity;

    AbsorptionAmount(b, j, ctx);
    Brain(b, j, ctx);
    DeathTime(b, j, ctx);
    FallFlying(b, j, ctx);
    Health(b, j, ctx);
    HurtByTimestamp(b, j, ctx);
    HurtTime(b, j, ctx);

    CopyChestItems(b, "EnderChestInventory", j, "EnderItems", ctx, true);
    CopyChestItems(b, "Inventory", j, "Inventory", ctx, true);

    CopyIntValues(b, j, {{"SelectedInventorySlot", "SelectedItemSlot"}, {"PlayerLevel", "XpLevel"}, {"EnchantmentSeed", "XpSeed"}, {"SpawnX"}, {"SpawnY"}, {"SpawnZ"}});
    CopyShortValues(b, j, {{"SleepTimer"}});
    CopyFloatValues(b, j, {{"PlayerLevelProgress", "XpP"}});
    CopyBoolValues(b, j, {{"HasSeenCredits", "seenCredits"}});

    GameMode playerGameMode = ctx.fGameMode;
    if (auto playerGameModeB = b.int32("PlayerGameMode"); playerGameModeB) {
      if (auto mode = GameModeFromBedrock(*playerGameModeB); mode) {
        playerGameMode = *mode;
      }
    }
    j["playerGameType"] = Int(JavaFromGameMode(playerGameMode));
    if (playerGameMode != ctx.fGameMode) {
      j["previousPlayerGameType"] = Int(JavaFromGameMode(ctx.fGameMode));
    }

    auto dimensionB = b.int32("DimensionId", 0);
    if (auto dimension = DimensionFromBedrockDimension(dimensionB); dimension) {
      j["Dimension"] = String(JavaStringFromDimension(*dimension));
    }

    auto spawnDimensionB = b.int32("SpawnDimension");
    if (spawnDimensionB) {
      if (auto dimension = DimensionFromBedrockDimension(*spawnDimensionB); dimension) {
        j["SpawnDimension"] = String(JavaStringFromDimension(*dimension));
      }
    }

    j["DataVersion"] = Int(toje::kDataVersion);

    if (auto posB = props::GetPos3f(b, "Pos"); posB) {
      Pos3d posJ = posB->toD();
      posJ.fY -= 1.62;
      j["Pos"] = posJ.toListTag();
    }

    auto uidB = b.int64("UniqueID");
    if (!uidB) {
      return std::nullopt;
    }
    Uuid uidJ;
    if (uuid) {
      uidJ = *uuid;
    } else {
      int64_t v = *uidB;
      uidJ = Uuid::GenWithU64Seed(*(uint64_t *)&v);
    }
    j["UUID"] = uidJ.toIntArrayTag();

    data.fEntityIdBedrock = *uidB;
    data.fEntityIdJava = uidJ;

    if (auto attributes = b.listTag("Attributes"); attributes) {
      for (auto const &it : *attributes) {
        auto attribute = it->asCompound();
        if (!attribute) {
          continue;
        }
        auto name = attribute->string("Name");
        if (!name) {
          continue;
        }
        if (name == "minecraft:player.level") {
          if (auto base = attribute->float32("Base"); base) {
            j["XpTotal"] = Int((int)roundf(*base));
          }
        } else if (name == "minecraft:player.exhaustion") {
          if (auto current = attribute->float32("Current"); current) {
            j["foodExhaustionLevel"] = Float(*current);
          }
        } else if (name == "minecraft:player.hunger") {
          if (auto current = attribute->float32("Current"); current) {
            j["foodLevel"] = Int((int)roundf(*current));
          }
        } else if (name == "minecraft:player.saturation") {
          if (auto current = attribute->float32("Current"); current) {
            j["foodSaturationLevel"] = Float(*current);
          }
        }
      }
    }

    if (auto abilitiesB = b.compoundTag("abilities"); abilitiesB) {
      auto abilitiesJ = Compound();
      CopyBoolValues(*abilitiesB, *abilitiesJ, {{"flying"}, {"instabuild"}, {"invulnerable"}, {"build", "mayBuild"}, {"mayfly"}});
      CopyFloatValues(*abilitiesB, *abilitiesJ, {{"flySpeed"}, {"walkSpeed"}});
      j["abilities"] = abilitiesJ;
    }

    auto wardenSpawnTracker = Compound();
    CopyIntValues(b, *wardenSpawnTracker, {{"WardenThreatLevelIncreaseCooldown", "cooldown_ticks"}, {"WardenThreatDecreaseTimer", "ticks_since_last_warning"}, {"WardenThreatLevel", "warning_level"}});
    if (!wardenSpawnTracker->empty()) {
      j["warden_spawn_tracker"] = wardenSpawnTracker;
    }

    auto deathDimension = b.int32("DeathDimension");
    auto deathPositionX = b.int32("DeathPositionX");
    auto deathPositionY = b.int32("DeathPositionY");
    auto deathPositionZ = b.int32("DeathPositionZ");
    if (deathDimension && deathPositionX && deathPositionY && deathPositionZ) {
      if (auto dimension = DimensionFromBedrockDimension(*deathDimension); dimension) {
        auto lastDeathLocation = Compound();
        lastDeathLocation->set("dimension", String(JavaStringFromDimension(*dimension)));
        std::vector<int32_t> value(3);
        value[0] = *deathPositionX;
        value[1] = *deathPositionY;
        value[2] = *deathPositionZ;
        auto posTag = std::make_shared<IntArrayTag>(value);
        lastDeathLocation->set("pos", posTag);

        data.fEntity->set("LastDeathLocation", lastDeathLocation);
      }
    }

    data.fEntity->erase("id");

    data.fShoulderEntityLeft = b.int64("LeftShoulderRiderID");
    data.fShoulderEntityRight = b.int64("RightShoulderPassengerID");

    return data;
  }

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
    E(ender_crystal, C(Rename("end_crystal"), Base, ShowBottom, EnderCrystal));
    E(chest_minecart, C(Same, Base, Minecart, ItemsFromChestItems, ChestMinecart));
    E(hopper_minecart, C(Same, Base, Minecart, ItemsFromChestItems, HopperMinecart));
    E(boat, C(Same, Base, Boat));
    E(chest_boat, C(Same, Base, ItemsFromChestItems, Boat));
    E(slime, C(Same, LivingEntity, Size));
    E(salmon, C(Same, LivingEntity, FromBucket));
    E(parrot, C(Same, Animal, Sitting, CopyVariant));
    E(enderman, C(Same, LivingEntity, AngerTime, Enderman));
    E(zombie_pigman, C(Rename("zombified_piglin"), LivingEntity, AngerTime, IsBaby, Zombie, ZombifiedPiglin));
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
    E(evocation_illager, C(Rename("evoker"), LivingEntity, CanJoinRaid, PatrolLeader, Patrolling, Wave, Evoker));
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
    E(tropicalfish, C(Rename("tropical_fish"), LivingEntity, FromBucket, TropicalFish));
    E(minecart, C(Same, Base, Minecart));
    E(vex, C(Same, LivingEntity, NoGravity));
    E(villager_v2, C(Rename("villager"), Animal, FoodLevel, Inventory, Offers, Villager));
    E(wandering_trader, C(Same, LivingEntity, Age, Inventory, Offers, WanderingTrader));
    E(wolf, C(Same, Animal, AngerTime, CollarColor, Sitting));
    E(zombie_horse, C(Same, Animal, Bred, EatingHaystack, Tame, Temper, JumpStrength, MovementSpeed));
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
    E(piglin, C(Same, LivingEntity, Inventory, IsBaby, Piglin));
    E(piglin_brute, C(Same, LivingEntity, PiglinBrute));
    E(hoglin, C(Same, Animal, IsBaby, Hoglin));
    E(arrow, C(Same, Base, Owner));
    E(ender_dragon, C(Same, LivingEntity, EnderDragon));
    E(falling_block, C(Same, Base, FallingBlock));

    E(frog, C(Same, Animal, PersistenceRequiredDefault, Entity::Frog));
    E(warden, C(Same, LivingEntity));
    E(allay, C(Same, LivingEntity, NoGravity, Inventory, Allay));
    E(tadpole, C(Same, LivingEntity, AgeableE(24000), FromBucket));

    E(camel, C(Same, Animal, Bred, SaddleItemFromChestItems, Tame, Camel));
#undef E
    return ret;
  }
};

} // namespace je2be::toje
