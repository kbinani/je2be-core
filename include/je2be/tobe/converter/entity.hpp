#pragma once

namespace je2be::tobe {

class Entity {
private:
  struct ConverterContext {
    explicit ConverterContext(Context const &ctx) : fCtx(ctx) {}

    Context const &fCtx;
    std::vector<CompoundTagPtr> fPassengers;
    std::unordered_map<Pos2i, std::vector<CompoundTagPtr>, Pos2iHasher> fLeashKnots;
  };
  using Converter = std::function<CompoundTagPtr(CompoundTag const &, ConverterContext &)>;

  using Behavior = std::function<void(CompoundTag &, CompoundTag const &, ConverterContext &)>;

  struct C {
    template <class... Arg>
    C(Converter base, Arg... args) : fBase(base), fBehaviors(std::initializer_list<Behavior>{args...}) {}

    CompoundTagPtr operator()(CompoundTag const &input, ConverterContext &ctx) const {
      auto c = fBase(input, ctx);
      if (!c) {
        return nullptr;
      }
      for (auto const &b : fBehaviors) {
        b(*c, input, ctx);
      }
      return c;
    }

  private:
    Converter fBase;
    std::vector<Behavior> fBehaviors;
  };

public:
  explicit Entity(int64_t uid) : fMotion(0, 0, 0), fPos(0, 0, 0), fRotation(0, 0), fUniqueId(uid) {}

  struct Result {
    CompoundTagPtr fEntity;
    std::vector<CompoundTagPtr> fPassengers;
    std::unordered_map<Pos2i, std::vector<CompoundTagPtr>, Pos2iHasher> fLeashKnots;
  };

  static Result From(CompoundTag const &tag, Context const &ctx) {
    using namespace std;
    auto id = tag.string("id");
    Result result;
    if (!id) {
      return result;
    }
    static unique_ptr<unordered_map<string, Converter> const> const table(CreateEntityTable());
    auto found = table->find(*id);
    if (found == table->end()) {
      auto converted = Default(tag);
      if (converted) {
        result.fEntity = converted;
      }
      return result;
    }
    ConverterContext cctx(ctx);
    auto converted = found->second(tag, cctx);
    if (converted) {
      result.fEntity = converted;
    }
    result.fPassengers.swap(cctx.fPassengers);
    for (auto const &it : cctx.fLeashKnots) {
      auto const &leashKnots = it.second;
      copy(leashKnots.begin(), leashKnots.end(), back_inserter(result.fLeashKnots[it.first]));
    }
    return result;
  }

  static bool RotAlmostEquals(Rotation const &rot, float yaw, float pitch) { return Rotation::DegAlmostEquals(rot.fYaw, yaw) && Rotation::DegAlmostEquals(rot.fPitch, pitch); }

  static std::optional<std::tuple<Pos3i, CompoundTagPtr, std::string>> ToTileEntityBlock(CompoundTag const &c) {
    using namespace std;
    auto id = c.string("id");
    assert(id);
    if (*id == "minecraft:item_frame") {
      return ToItemFrameTileEntityBlock(c, "minecraft:frame");
    } else if (*id == "minecraft:glow_item_frame") {
      return ToItemFrameTileEntityBlock(c, "minecraft:glow_frame");
    }

    return nullopt;
  }

  static std::tuple<Pos3i, CompoundTagPtr, std::string> ToItemFrameTileEntityBlock(CompoundTag const &c, std::string const &name) {
    using namespace std;
    auto tileX = c.int32("TileX");
    auto tileY = c.int32("TileY");
    auto tileZ = c.int32("TileZ");

    auto rot = props::GetRotation(c, "Rotation");
    int32_t facing = 0;
    if (RotAlmostEquals(*rot, 0, -90)) {
      // up
      facing = 1;
    } else if (RotAlmostEquals(*rot, 180, 0)) {
      // north
      facing = 2;
    } else if (RotAlmostEquals(*rot, 270, 0)) {
      // east
      facing = 5;
    } else if (RotAlmostEquals(*rot, 0, 0)) {
      // south
      facing = 3;
    } else if (RotAlmostEquals(*rot, 90, 0)) {
      // west
      facing = 4;
    } else if (RotAlmostEquals(*rot, 0, 90)) {
      // down
      facing = 0;
    }

    bool map = false;
    auto itemId = c.query("Item/id");
    if (itemId) {
      auto itemIdString = itemId->asString();
      if (itemIdString) {
        string itemName = itemIdString->fValue;
        if (itemName == "minecraft:filled_map") {
          map = true;
        }
      }
    }

    auto b = Compound();
    auto states = Compound();
    states->insert({
        {"facing_direction", Int(facing)},
        {"item_frame_map_bit", Bool(map)},
    });
    string key = name + "[facing_direction=" + to_string(facing) + ",item_frame_map_bit=" + (map ? "true" : "false") + "]";
    b->insert({
        {"name", String(name)},
        {"version", Int(kBlockDataVersion)},
        {"states", states},
    });
    Pos3i pos(*tileX, *tileY, *tileZ);
    return make_tuple(pos, b, key);
  }

  static CompoundTagPtr ToTileEntityData(CompoundTag const &c, Context const &ctx) {
    auto id = c.string("id");
    assert(id);
    if (*id == "minecraft:item_frame") {
      return ToItemFrameTileEntityData(c, ctx, "ItemFrame");
    } else if (*id == "minecraft:glow_item_frame") {
      return ToItemFrameTileEntityData(c, ctx, "GlowItemFrame");
    }
    return nullptr;
  }

  static CompoundTagPtr ToItemFrameTileEntityData(CompoundTag const &c, Context const &ctx, std::string const &name) {
    auto tag = Compound();
    auto tileX = c.int32("TileX");
    auto tileY = c.int32("TileY");
    auto tileZ = c.int32("TileZ");
    if (!tileX || !tileY || !tileZ) {
      return nullptr;
    }
    tag->insert({
        {"id", String(name)},
        {"isMovable", Bool(true)},
        {"x", Int(*tileX)},
        {"y", Int(*tileY)},
        {"z", Int(*tileZ)},
    });
    auto itemRotation = c.byte("ItemRotation", 0);
    auto itemDropChance = c.float32("ItemDropChance", 1);
    auto found = c.find("Item");
    if (found != c.end() && found->second->type() == Tag::Type::Compound) {
      auto item = std::dynamic_pointer_cast<CompoundTag>(found->second);
      auto m = Item::From(item, ctx);
      if (m) {
        tag->insert(make_pair("Item", m));
        tag->insert(make_pair("ItemRotation", Float(itemRotation * 45)));
        tag->insert(make_pair("ItemDropChance", Float(itemDropChance)));
      }
    }
    return tag;
  }

  static bool IsTileEntity(CompoundTag const &tag) {
    auto id = tag.string("id");
    if (!id) {
      return false;
    }
    return *id == "minecraft:item_frame" || *id == "minecraft:glow_item_frame";
  }

  CompoundTagPtr toCompoundTag() const {
    using namespace std;
    auto tag = Compound();
    auto tags = List<Tag::Type::Compound>();
    auto definitions = List<Tag::Type::String>();
    for (auto const &d : fDefinitions) {
      definitions->push_back(String(d));
    }
    tag->insert({
        {"definitions", definitions},
        {"Motion", fMotion.toListTag()},
        {"Pos", fPos.toListTag()},
        {"Rotation", fRotation.toListTag()},
        {"Tags", tags},
        {"Chested", Bool(fChested)},
        {"Color2", Byte(fColor2)},
        {"Color", Byte(fColor)},
        {"Dir", Byte(fDir)},
        {"FallDistance", Float(fFallDistance)},
        {"Fire", Short(std::max((int16_t)0, fFire))},
        {"identifier", String(fIdentifier)},
        {"Invulnerable", Bool(fInvulnerable)},
        {"IsAngry", Bool(fIsAngry)},
        {"IsAutonomous", Bool(fIsAutonomous)},
        {"IsBaby", Bool(fIsBaby)},
        {"IsEating", Bool(fIsEating)},
        {"IsGliding", Bool(fIsGliding)},
        {"IsGlobal", Bool(fIsGlobal)},
        {"IsIllagerCaptain", Bool(fIsIllagerCaptain)},
        {"IsOrphaned", Bool(fIsOrphaned)},
        {"IsRoaring", Bool(fIsRoaring)},
        {"IsScared", Bool(fIsScared)},
        {"IsStunned", Bool(fIsStunned)},
        {"IsSwimming", Bool(fIsSwimming)},
        {"IsTamed", Bool(fIsTamed)},
        {"IsTrusting", Bool(fIsTrusting)},
        {"LastDimensionId", Int(fLastDimensionId)},
        {"LootDropped", Bool(fLootDropped)},
        {"MarkVariant", Int(fMarkVariant)},
        {"OnGround", Bool(fOnGround)},
        {"OwnerNew", Long(fOwnerNew)},
        {"PortalCooldown", Int(fPortalCooldown)},
        {"Saddled", Bool(fSaddled)},
        {"Sheared", Bool(fSheared)},
        {"ShowBottom", Bool(fShowBottom)},
        {"Sitting", Bool(fSitting)},
        {"SkinID", Int(fSkinId)},
        {"Strength", Int(fStrength)},
        {"StrengthMax", Int(fStrengthMax)},
        {"UniqueID", Long(fUniqueId)},
        {"Variant", Int(fVariant)},
    });
    if (fCustomName) {
      tag->set("CustomName", String(*fCustomName));
      tag->set("CustomNameVisible", Bool(fCustomNameVisible));
    }
    return tag;
  }

  struct LocalPlayerResult {
    CompoundTagPtr fEntity;
    int64_t fUid;
    mcfile::Dimension fDimension;
    Pos2i fChunk;
  };
  static std::optional<LocalPlayerResult> LocalPlayer(CompoundTag const &tag, Context const &ctx) {
    using namespace std;
    using namespace mcfile;

    ConverterContext cctx(ctx);
    auto entity = LivingEntity(tag, cctx);
    if (!entity) {
      return nullopt;
    }
    auto uuid = GetEntityUuid(tag);
    if (!uuid) {
      return nullopt;
    }

    auto pos = props::GetPos3d(tag, "Pos");
    if (!pos) {
      return nullopt;
    }

    // ng 1.620001f
    // ok 1.62001f
    double y = pos->fY + 1.62001;
    if (auto rootVehicle = tag.compoundTag("RootVehicle"); rootVehicle) {
      if (auto vehicleEntity = rootVehicle->compoundTag("Entity"); vehicleEntity) {
        auto vehicleId = vehicleEntity->string("id");
        if (vehicleId == "minecraft:boat" || vehicleId == "minecraft:chest_boat") {
          auto boatPos = GetBoatPos(*vehicleEntity);
          if (boatPos) {
            y = boatPos->fY + 1.24501;
          }
        }
      }
    }
    pos->fY = y;
    entity->set("Pos", pos->toF().toListTag());

    entity->set("format_version", String("1.12.0"));
    entity->set("identifier", String("minecraft:player"));
    entity->set("IsOutOfControl", Bool(false));
    entity->set("OwnerNew", Long(-1));
    entity->set("SleepTimer", Short(0));
    entity->set("Sleeping", Bool(false));
    entity->set("Sneaking", Bool(false));
    entity->set("TicksSinceLastWarning", Int(0));
    entity->set("WarningCooldownTicks", Int(0));
    entity->set("WarningLevel", Int(0));

    entity->erase("LastDimensionId");
    entity->erase("BodyRot");
    entity->erase("BreedCooldown");
    entity->erase("Fire");
    entity->erase("InLove");
    entity->erase("LoveCause");
    entity->erase("limitedLife");

    CopyIntValues(tag, *entity, {{"SelectedItemSlot", "SelectedInventorySlot"}, {"XpSeed", "EnchantmentSeed"}, {"PortalCooldown"}});
    CopyFloatValues(tag, *entity, {{"XpP", "PlayerLevelProgress"}});
    CopyBoolValues(tag, *entity, {{"seenCredits", "HasSeenCredits"}});

    entity->set("PlayerGameMode", Int(std::clamp(tag.int32("playerGameType", 0), 0, 2)));
    entity->set("SelectedContainerId", Int(0));

    auto inventory = tag.listTag("Inventory");
    if (inventory) {
      auto outInventory = ConvertAnyItemList(inventory, 36, ctx);
      entity->set("Inventory", outInventory);

      // Armor
      auto armor = InitItemList(4);
      auto boots = ItemAtSlot(*inventory, 100);
      if (boots) {
        auto outBoots = Item::From(boots, ctx);
        if (outBoots) {
          armor->at(3) = outBoots;
        }
      }
      auto leggings = ItemAtSlot(*inventory, 101);
      if (leggings) {
        auto outLeggings = Item::From(leggings, ctx);
        if (outLeggings) {
          armor->at(2) = outLeggings;
        }
      }
      auto chestplate = ItemAtSlot(*inventory, 102);
      if (chestplate) {
        auto outChestplate = Item::From(chestplate, ctx);
        if (outChestplate) {
          armor->at(1) = outChestplate;
        }
      }
      auto helmet = ItemAtSlot(*inventory, 103);
      if (helmet) {
        auto outHelmet = Item::From(helmet, ctx);
        if (outHelmet) {
          armor->at(0) = outHelmet;
        }
      }
      entity->set("Armor", armor);

      // Offhand
      auto offhand = ItemAtSlot(*inventory, -106);
      if (offhand) {
        auto offhandItem = Item::From(offhand, ctx);
        if (offhandItem) {
          auto outOffhand = InitItemList(1);
          outOffhand->at(0) = offhandItem;
          entity->set("Offhand", outOffhand);
        }
      }
    }

    auto enderItems = tag.listTag("EnderItems");
    if (enderItems) {
      auto enderChestInventory = ConvertAnyItemList(enderItems, 27, ctx);
      entity->set("EnderChestInventory", enderChestInventory);
    }

    auto abilities = tag.compoundTag("abilities");
    if (abilities) {
      auto converted = PlayerAbilities::Import(*abilities);
      entity->set("abilities", converted.toCompoundTag());
    }

    auto spawnX = tag.int32("SpawnX");
    auto spawnY = tag.int32("SpawnY");
    auto spawnZ = tag.int32("SpawnZ");
    auto spawnDimensionString = tag.string("SpawnDimension");
    std::optional<mcfile::Dimension> spawnDimension;
    if (spawnDimensionString) {
      spawnDimension = DimensionFromJavaString(*spawnDimensionString);
    }
    if (spawnX && spawnY && spawnZ && spawnDimension) {
      entity->set("SpawnX", Int(*spawnX));
      entity->set("SpawnY", Int(*spawnY));
      entity->set("SpawnZ", Int(*spawnZ));
      int dimension = BedrockDimensionFromDimension(*spawnDimension);
      entity->set("SpawnDimension", Int(dimension));
      entity->set("SpawnBlockPositionX", Int(*spawnX));
      entity->set("SpawnBlockPositionY", Int(*spawnY));
      entity->set("SpawnBlockPositionZ", Int(*spawnZ));
    }

    auto dimensionString = tag.string("Dimension");
    mcfile::Dimension dim = mcfile::Dimension::Overworld;
    if (dimensionString) {
      auto dimension = DimensionFromJavaString(*dimensionString);
      if (dimension) {
        dim = *dimension;
        entity->set("DimensionId", Int(BedrockDimensionFromDimension(*dimension)));
      }
    }

    auto definitions = List<Tag::Type::String>();
    definitions->push_back(String("+minecraft:player"));
    definitions->push_back(String("+"));
    entity->set("definitions", definitions);

    auto xpLevel = tag.int32("XpLevel", 0);
    entity->set("PlayerLevel", Int(xpLevel));

    auto attrs = EntityAttributes::Player(tag.float32("Health"));
    auto attrsTag = attrs.toListTag();
    if (auto xpTotal = tag.int32("XpTotal"); xpTotal) {
      EntityAttributes::Attribute level(*xpTotal, xpLevel, 24791);
      attrsTag->push_back(level.toCompoundTag("player.level"));
    }
    if (auto foodExhaustionLevel = tag.float32("foodExhaustionLevel"); foodExhaustionLevel) {
      EntityAttributes::Attribute exhaustion(0, *foodExhaustionLevel, 4);
      attrsTag->push_back(exhaustion.toCompoundTag("player.exhaustion"));
    }
    if (auto foodLevel = tag.int32("foodLevel"); foodLevel) {
      EntityAttributes::Attribute hunger(20, *foodLevel, 20);
      attrsTag->push_back(hunger.toCompoundTag("player.hunger"));
    }
    if (auto foodSaturatonLevel = tag.float32("foodSaturationLevel"); foodSaturatonLevel) {
      EntityAttributes::Attribute saturation(20, *foodSaturatonLevel, 20);
      attrsTag->push_back(saturation.toCompoundTag("player.saturation"));
    }
    entity->set("Attributes", attrsTag);

    CopyShortValues(tag, *entity, {{"SleepTimer"}});

    LocalPlayerResult result;
    result.fEntity = entity;
    result.fDimension = dim;
    result.fUid = *uuid;
    int cx = mcfile::Coordinate::ChunkFromBlock((int)floorf(pos->fX));
    int cz = mcfile::Coordinate::ChunkFromBlock((int)floorf(pos->fZ));
    result.fChunk = Pos2i(cx, cz);

    return result;
  }

private:
  static CompoundTagPtr ItemAtSlot(ListTag const &items, uint32_t slot) {
    for (auto const &it : items) {
      auto item = std::dynamic_pointer_cast<CompoundTag>(it);
      if (!item) {
        continue;
      }
      auto s = item->byte("Slot");
      if (!s) {
        continue;
      }
      if (*s == slot) {
        return item;
      }
    }
    return nullptr;
  }

  static std::unordered_map<std::string, Converter> *CreateEntityTable() {
    auto table = new std::unordered_map<std::string, Converter>();
#define E(__name, __func) table->insert(std::make_pair("minecraft:" #__name, __func))
#define A(__name) table->insert(std::make_pair("minecraft:" #__name, Animal))
#define M(__name) table->insert(std::make_pair("minecraft:" #__name, Monster))

    E(painting, Painting);
    E(end_crystal, EndCrystal);

    E(bat, C(Mob, Bat));
    E(bee, C(Animal, AgeableA("bee"), Bee));
    M(blaze);
    E(cat, C(Animal, AgeableA("cat"), TameableA("cat"), Sittable, CollarColorable, Cat));
    M(cave_spider);
    E(chicken, C(Animal, AgeableA("chicken"), Vehicle(), Chicken));
    E(cod, C(Mob, PersistentFromFromBucket));

    E(cow, C(Animal, AgeableA("cow")));
    E(creeper, C(Monster, Creeper));
    A(dolphin);
    E(donkey, C(Animal, TameableB("donkey"), ChestedHorse("donkey"), Steerable("donkey"), Temper));
    E(drowned, C(Monster, AgeableD("drowned")));
    M(elder_guardian);
    E(enderman, C(Monster, Enderman));
    E(endermite, C(Monster, Endermite));
    E(evoker, C(Monster, Rename("evocation_illager"), CanJoinRaid));

    E(fox, C(Animal, Fox));
    M(ghast);
    M(guardian);
    E(hoglin, C(Animal, AgeableA("hoglin"), Hoglin));
    E(horse, C(Animal, TameableB("horse"), AgeableA("horse"), Steerable("horse"), Temper, Horse));
    E(husk, C(Monster, AgeableA("husk")));
    E(llama, C(Animal, AgeableA("llama"), TameableB("llama"), ChestedHorse("llama"), Llama));
    E(magma_cube, C(Monster, Slime));
    E(mooshroom, C(Animal, AgeableA("mooshroom"), Mooshroom));

    E(mule, C(Animal, TameableB("mule"), ChestedHorse("mule"), Steerable("mule"), Temper));
    A(ocelot);
    E(panda, C(Animal, AgeableA("panda"), Panda));
    E(parrot, C(Animal, TameableA("parrot"), Sittable, Parrot));
    M(phantom);
    E(pig, C(Animal, AgeableA("pig"), Steerable("pig")));
    E(piglin, C(Monster, ChestItemsFromInventory, AgeableB("piglin"), Piglin));
    E(piglin_brute, C(Monster, PiglinBrute));
    E(pillager, C(Monster, CanJoinRaid, ChestItemsFromInventory));

    A(polar_bear);
    E(pufferfish, C(Mob, PersistentFromFromBucket, Pufferfish));
    E(rabbit, C(Animal, AgeableC, Rabbit));
    E(ravager, C(Monster, AttackTime, CanJoinRaid));
    E(salmon, C(Mob, PersistentFromFromBucket));
    E(sheep, C(Animal, AgeableA("sheep"), Colorable("sheep"), Definitions("+minecraft:sheep_dyeable", "+minecraft:rideable_wooly", "+minecraft:loot_wooly"), Sheep));
    E(shulker, C(Monster, Shulker));
    M(silverfish);
    M(skeleton); // lefty skeleton does not exist in Bedrock?

    E(skeleton_horse, C(Animal, SkeletonHorse));
    E(slime, C(Monster, Slime));
    E(spider, C(Monster, Vehicle("spider")));
    A(squid);
    M(stray);
    E(strider, C(Animal, Steerable("strider"), AgeableA("strider"), DetectSuffocation, Vehicle("strider")));
    E(trader_llama, C(Animal, Rename("llama"), AgeableA("llama"), Llama, TraderLlama));
    E(tropical_fish, C(Mob, Rename("tropicalfish"), PersistentFromFromBucket, TropicalFish));
    E(turtle, C(Animal, Turtle));

    M(vex);
    E(villager, C(Animal, Rename("villager_v2"), Offers(4, "Offers"), ChestItemsFromInventory, Villager));
    M(vindicator);
    E(wandering_trader, C(Animal, Offers(0, "Offers"), ChestItemsFromInventory, WanderingTrader));
    E(witch, C(Monster, CanJoinRaid));
    M(wither_skeleton);
    E(wolf, C(Animal, TameableA("wolf"), Sittable, CollarColorable, Wolf));
    M(zoglin);
    E(zombie, C(Monster, AgeableB("zombie")));

    M(zombie_horse);
    E(zombie_villager, C(Animal, Rename("zombie_villager_v2"), Offers(4, "persistingOffers"), ZombieVillager));
    E(zombified_piglin, C(Monster, Rename("zombie_pigman"), AgeableB("pig_zombie")));

    E(boat, C(EntityBase, Vehicle(), Entity::Boat("boat")));
    E(chest_boat, C(EntityBase, Vehicle(), ChestItemsFromItems, Entity::Boat("chest_boat")));
    E(minecart, C(EntityBase, Vehicle(), Minecart, Definitions("+minecraft:minecart")));
    E(armor_stand, C(LivingEntity, ArmorStand));
    E(hopper_minecart, C(EntityBase, ChestItemsFromItems, Minecart, Definitions("+minecraft:hopper_minecart"), HopperMinecart));
    E(chest_minecart, C(EntityBase, ChestItemsFromItems, Minecart, Definitions("+minecraft:chest_minecart"), ChestMinecart));
    E(tnt_minecart, C(EntityBase, Vehicle(), Minecart, TntMinecart));
    E(snow_golem, C(Mob, SnowGolem));
    E(iron_golem, C(Mob, Definitions("+minecraft:iron_golem"), IronGolem));

    E(item, Item);
    E(ender_dragon, EnderDragon);
    E(experience_orb, C(LivingEntity, Rename("xp_orb"), ExperienceOrb));
    E(item_frame, Null);      // item_frame is tile entity in BE.
    E(glow_item_frame, Null); // glow_item_frame is tile entity in BE.

    E(glow_squid, C(Animal, Definitions("+minecraft:glow_squid")));
    E(axolotl, C(Animal, AgeableA("axolotl"), Axolotl));
    E(goat, C(Animal, AgeableA("goat"), Goat));
    E(falling_block, C(EntityBase, FallingBlock));
    E(wither, C(Mob, Definitions("+minecraft:wither")));
    E(arrow, C(EntityBase, Arrow));

    E(frog, C(Animal, Definitions("+minecraft:frog"), Frog));
    E(warden, C(Monster, Definitions("+minecraft:warden")));
#undef A
#undef M
#undef E
    return table;
  }

#pragma region DedicatedBehaviors
  static void ArmorStand(CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
    auto pos = props::GetPos3d(tag, "Pos");
    if (!pos) {
      return;
    }
    pos->fY = std::round(pos->fY * 2) * 0.5;
    c["Pos"] = pos->toF().toListTag();

    auto health = tag.float32("Health");
    auto attributes = EntityAttributes::Mob("minecraft:armor_stand", health);
    if (attributes) {
      c["Attributes"] = attributes->toListTag();
    }

    auto poseJ = tag.compoundTag("Pose");
    if (poseJ) {
      auto indexB = ArmorStand::BedrockMostSimilarPoseIndexFromJava(*poseJ);
      auto showArms = tag.boolean("ShowArms", false);
      if (indexB) {
        if (indexB != 1 || showArms) {
          auto poseB = Compound();
          poseB->set("PoseIndex", Int(*indexB));
          poseB->set("LastSignal", Int(0));
          c["Pose"] = poseB;
        }
      }
    }
  }

  static void Arrow(CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
    auto owner = GetOwnerUuid(tag);
    if (owner) {
      c["OwnerID"] = Long(*owner);
      c["OwnerNew"] = Long(*owner);
    }
  }

  static void Axolotl(CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
    auto originalVariant = std::clamp(tag.int32("Variant", 0), 0, 4);
    static const std::string definitionMapping[5] = {
        "+axolotl_lucy",
        "+axolotl_wild",
        "+axolotl_gold",
        "+axolotl_cyan",
        "+axolotl_blue"};
    auto variant = je2be::Axolotl::BedrockVariantFromJavaVariant(originalVariant);
    auto definition = definitionMapping[originalVariant];
    c["Variant"] = Int(variant);
    AddDefinition(c, definition);

    auto onGround = tag.boolean("OnGround", false);
    if (onGround) {
      AddDefinition(c, "+axolotl_on_land");
      AddDefinition(c, "-axolotl_in_water");
    } else {
      AddDefinition(c, "-axolotl_on_land");
      AddDefinition(c, "+axolotl_in_water");
    }

    auto air = tag.int16("Air", 6000);
    if (air < 0) {
      AddDefinition(c, "+axolotl_dried");
    } else {
      AddDefinition(c, "-axolotl_dried");
    }
  }

  static void Bat(CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
    CopyBoolValues(tag, c, {{"BatFlags"}});
    AddDefinition(c, "+minecraft:bat");
  }

  static void Bee(CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
    auto hivePos = props::GetPos3d(tag, "HivePos");
    if (hivePos) {
      Pos3f homePos = hivePos->toF();
      c["HomePos"] = homePos.toListTag();
    }
    AddDefinition(c, "+track_attacker");
    AddDefinition(c, "+shelter_detection");
    auto hasNectar = tag.boolean("HasNectar", false);
    if (hasNectar) {
      AddDefinition(c, "+has_nectar");
    }
    AddDefinition(c, "+default_sound");
    AddDefinition(c, "+find_hive");
  }

  static std::optional<Pos3d> GetBoatPos(CompoundTag const &j) {
    auto pos = props::GetPos3d(j, "Pos");
    if (!pos) {
      return std::nullopt;
    }
    auto onGround = j.boolean("OnGround", false);
    if (onGround) {
      int iy = (int)floor(pos->fY);
      pos->fY = iy + 0.375;
      return *pos;
    } else {
      return pos;
    }
  }

  static void Cat(CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
    auto catType = tag.int32("CatType");
    if (catType) {
      Cat::Type ct = Cat::CatTypeFromJavaCatType(*catType);
      int32_t variant = Cat::BedrockVariantFromJavaCatType(ct);
      std::string type = Cat::BedrockNameFromCatType(ct);
      if (!type.empty()) {
        AddDefinition(c, "+minecraft:cat_" + type);
      }
      c["Variant"] = Int(variant);
    }
    c["DwellingUniqueID"] = String("00000000-0000-0000-0000-000000000000");
    c["RewardPlayersOnFirstFounding"] = Bool(true);
  }

  static void ChestMinecart(CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
    LootTable::JavaToBedrock(tag, c);
  }

  static void Chicken(CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
    using namespace std;
    auto eggLayTime = tag.int32("EggLayTime");
    if (eggLayTime) {
      auto entries = List<Tag::Type::Compound>();
      auto timer = Compound();
      timer->set("SpawnTimer", Int(*eggLayTime));
      timer->set("StopSpawning", Bool(false));
      entries->push_back(timer);
      c["entries"] = entries;
    }
  }

  static void Creeper(CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
    auto powered = tag.boolean("powered", false);
    if (powered) {
      AddDefinition(c, "+minecraft:charged_creeper");
      AddDefinition(c, "+minecraft:exploding");
    }
  }

  static void Enderman(CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
    auto carriedBlockTagJ = tag.compoundTag("carriedBlockState");
    if (carriedBlockTagJ) {
      auto name = carriedBlockTagJ->string("Name");
      if (name) {
        auto carriedBlockJ = std::make_shared<mcfile::je::Block const>(*name);
        auto carriedBlockB = BlockData::From(carriedBlockJ);
        if (carriedBlockB) {
          c["carriedBlock"] = carriedBlockB;
        }
      }
    }
  }

  static void Endermite(CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
    CopyIntValues(tag, c, {{"Lifetime"}});
  }

  static void ExperienceOrb(CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
    auto value = tag.int16("Value");
    if (value) {
      c["experience value"] = Int(*value);
    }
    AddDefinition(c, "+minecraft:xp_orb");
  }

  static void FallingBlock(CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
    auto blockState = tag.compoundTag("BlockState");
    if (!blockState) {
      return;
    }
    auto name = blockState->string("Name");
    if (!name) {
      return;
    }
    auto block = BlockData::From(std::make_shared<mcfile::je::Block>(*name));
    if (!block) {
      return;
    }
    c["FallingBlock"] = block;

    CopyFloatValues(tag, c, {{"FallDistance"}});

    uint8_t time = mcfile::Clamp<uint8_t>(tag.int32("Time", 0));
    c["Time"] = Byte(*(int8_t *)&time);

    /*
    Variant:
    152 anvil
    6707 sand
    6708 red_sand
    3660 concretepowder
    3661 orange concretepowder
    */
  }

  static void Fox(CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
    auto type = tag.string("Type", "red");
    int32_t variant = 0;
    if (type == "red") {
      variant = 0;
    } else if (type == "snow") {
      variant = 1;
    }
    c["Variant"] = Int(variant);

    if (tag.boolean("Sleeping", false)) {
      AddDefinition(c, "+minecraft:fox_ambient_sleep");
    }

    auto trustedJ = tag.listTag("Trusted");
    if (trustedJ) {
      int index = 0;
      for (auto const &it : *trustedJ) {
        auto uuidTag = it->asIntArray();
        if (!uuidTag) {
          continue;
        }
        auto uuidJ = Uuid::FromIntArrayTag(*uuidTag);
        if (!uuidJ) {
          continue;
        }
        int64_t uuidB = UuidRegistrar::ToId(*uuidJ);
        std::string key = "TrustedPlayer" + std::to_string(index);
        c[key] = Long(uuidB);
        index++;
      }
      if (index > 0) {
        c["TrustedPlayersAmount"] = Int(index);
      }
    }
  }

  static void Frog(CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
    auto variantJ = tag.string("variant", "");
    auto variantB = Frog::BedrockVariantFromJavaVariant(variantJ);
    auto definition = Frog::BedrockDefinitionFromJavaVariant(variantJ);
    AddDefinition(c, definition);
    c["Variant"] = Int(variantB);
  }

  static void Goat(CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
    auto screaming = tag.boolean("IsScreamingGoat", false);
    if (screaming) {
      AddDefinition(c, "+goat_screamer");
      AddDefinition(c, "+ram_screamer");
      AddDefinition(c, "+interact_screamer");
    } else {
      AddDefinition(c, "+goat_default");
      AddDefinition(c, "+ram_default");
      AddDefinition(c, "+interact_default");
    }
  }

  static void Hoglin(CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
    AddDefinition(c, "+huntable_adult");
    AddDefinition(c, "+zombification_sensor");
    if (tag.boolean("CannotBeHunted", false)) {
      AddDefinition(c, "-angry_hoglin");
    }
  }

  static void Horse(CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
    auto variant = tag.int32("Variant", 0);
    auto baseColor = 0xf & variant;
    auto markings = 0xf & (variant >> 8);
    c["Variant"] = Int(baseColor);
    c["MarkVariant"] = Int(markings);
  }

  static void HopperMinecart(CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
    auto enabled = tag.boolean("Enabled", true);
    if (enabled) {
      AddDefinition(c, "+minecraft:hopper_active");
    }
  }

  static void IronGolem(CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
    auto playerCreated = tag.boolean("PlayerCreated", false);
    auto angryAt = props::GetUuid(tag, {.fIntArray = "AngryAt"});
    auto angerTime = tag.int32("AngerTime", 0);

    if (playerCreated) {
      AddDefinition(c, "+minecraft:player_created");
    } else {
      AddDefinition(c, "+minecraft:village_created");
    }
    c["IsAngry"] = Bool(angerTime > 0);
    if (angryAt) {
      int64_t targetId = UuidRegistrar::ToId(*angryAt);
      c["TargetID"] = Long(targetId);
    }
  }

  static void Llama(CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
    auto variant = tag.int32("Variant", 0);
    std::string color = "creamy";
    switch (variant) {
    case 3:
      color = "gray";
      break;
    case 1:
      color = "white";
      break;
    case 2:
      color = "brown";
      break;
    case 0:
      color = "creamy";
      break;
    }
    c["Variant"] = Int(variant);
    AddDefinition(c, "+minecraft:llama_" + color);

    auto armors = c.listTag("Armor");

    auto decorItemId = tag.query("DecorItem/id")->asString();
    if (decorItemId && decorItemId->fValue.starts_with("minecraft:") && decorItemId->fValue.ends_with("_carpet")) {
      auto carpetColor = strings::Trim("minecraft:", decorItemId->fValue, "_carpet");
      auto colorCode = ColorCodeJavaFromJavaName(carpetColor);
      auto beCarpetColor = BedrockNameFromColorCodeJava(colorCode);
      auto armor = Compound();
      armor->insert({{"Count", Byte(1)}, {"Damage", Short(0)}, {"Name", String("minecraft:carpet")}, {"WasPickedUp", Bool(false)}});
      auto block = Compound();
      block->insert({{"name", String("minecraft:carpet")}, {"version", Int(kBlockDataVersion)}});
      auto states = Compound();
      states->set("color", String(beCarpetColor));
      block->set("states", states);
      armor->set("Block", block);

      if (armors && armors->size() > 1) {
        armors->at(1) = armor;
      }

      AddChestItem(c, armor, 0, 1);
    }

    CopyIntValues(tag, c, {{"Strength"}});
  }

  static void Minecart(CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
    auto pos = props::GetPos3d(tag, "Pos");
    auto onGround = tag.boolean("OnGround");
    if (pos && onGround) {
      // Java
      //   on ground: ground level +0
      //   on rail: ground level +0.0625
      // Bedrock
      //   on ground: graound level +0.35
      //   on rail: ground level +0.5
      int32_t iy = (int32_t)floor(pos->fY);
      double dy = pos->fY - iy;
      if (*onGround) {
        // on ground
        pos->fY = iy + 0.35;
      } else if (1.0 / 16.0 <= dy && dy < 2.0 / 16.0) {
        // on rail
        pos->fY = iy + 0.5;
      }
      c["Pos"] = pos->toF().toListTag();
    }
  }

  static void Mooshroom(CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
    auto type = tag.string("Type", "red");
    int32_t variant = 0;
    if (type == "brown") {
      variant = 1;
      AddDefinition(c, "+minecraft:mooshroom_brown");
    } else {
      AddDefinition(c, "+minecraft:mooshroom_red");
    }
    c["Variant"] = Int(variant);
  }

  static void Panda(CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
    auto mainGene = tag.string("MainGene", "normal");
    auto hiddenGene = tag.string("HiddenGene", "normal");

    Panda::Gene main = Panda::GeneFromJavaName(mainGene);
    Panda::Gene hidden = Panda::GeneFromJavaName(hiddenGene);

    auto genes = List<Tag::Type::Compound>();
    auto gene = Compound();
    gene->set("MainAllele", Int(Panda::BedrockAlleleFromGene(main)));
    gene->set("HiddenAllele", Int(Panda::BedrockAlleleFromGene(hidden)));
    genes->push_back(gene);

    c["GeneArray"] = genes;
  }

  static void Parrot(CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
    CopyIntValues(tag, c, {{"Variant"}});
  }

  static void Piglin(CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
    if (auto cannotHant = tag.boolean("CannotHunt", false); cannotHant) {
      AddDefinition(c, "+not_hunter");
    }
  }

  static void PiglinBrute(CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
    if (auto homeTag = tag.query("Brain/memories/minecraft:home/value"); homeTag) {
      if (auto home = homeTag->asCompound(); home) {
        auto dimension = home->string("dimension");
        auto pos = home->intArrayTag("pos");
        if (dimension && pos && pos->value().size() == 3) {
          if (auto d = DimensionFromJavaString(*dimension); d) {
            auto const &posv = pos->value();
            Pos3f posB(posv[0], posv[1], posv[2]);
            c["HomeDimensionId"] = Int(BedrockDimensionFromDimension(*d));
            c["HomePos"] = posB.toListTag();
          }
        }
      }
    }
  }

  static void Pufferfish(CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
    auto puffState = tag.int32("PuffState", 0);

    /*
    "+minecraft:normal_puff"

    "-minecraft:normal_puff",
    "+minecraft:half_puff_primary"

    "-minecraft:full_puff",
    "-minecraft:normal_puff",
    "+minecraft:half_puff_primary"

    "-minecraft:full_puff",
    "+minecraft:half_puff_secondary"

    "+minecraft:full_puff",
    "-minecraft:deflate_sensor",
    "+minecraft:start_deflate"
    */

    std::string def;
    switch (puffState) {
    case 1:
      def = "+minecraft:half_puff_primary";
      break;
    case 2:
      def = "+minecraft:full_puff";
      break;
    case 0:
    default:
      def = "+minecraft:normal_puff";
      break;
    }
    AddDefinition(c, def);
  }

  static void Rabbit(CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
    auto type = tag.int32("RabbitType", 0);
    std::string coat = "brown";
    int32_t variant = 0;
    if (type == 0) {
      coat = "brown";
      variant = 0;
    } else if (type == 2) {
      coat = "black";
      variant = 2;
    } else if (type == 5) {
      coat = "salt";
      variant = 5;
    } else if (type == 1) {
      coat = "white";
      variant = 1;
    } else if (type == 3) {
      coat = "splotched";
      variant = 3;
    } else if (type == 4) {
      coat = "desert";
      variant = 4;
    } else if (type == 99) {
      coat = "white";
      variant = 1;
      c["CustomName"] = String("The Killer Bunny");
    }
    c["Variant"] = Int(variant);
    AddDefinition(c, "+coat_" + coat);

    CopyIntValues(tag, c, {{"MoreCarrotTicks"}});
  }

  static void Sheep(CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
    CopyBoolValues(tag, c, {{"Sheared", "Sheared", false}});
  }

  static void Shulker(CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
    auto color = tag.byte("Color", 16);
    if (0 <= color && color <= 15) {
      c["Color"] = Byte(color);
      AddDefinition(c, "+minecraft:shulker_" + BedrockNameFromColorCodeJava((ColorCodeJava)color));
    } else {
      AddDefinition(c, "+minecraft:shulker_undyed");
    }
    c["Variant"] = Int(color);
  }

  static void SkeletonHorse(CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
    auto trap = tag.boolean("SkeletonTrap", false);
    if (trap) {
      AddDefinition(c, "+minecraft:skeleton_trap");
      AddDefinition(c, "+minecraft:lightning_immune");
    }
  }

  static void Slime(CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
    auto sizeJ = tag.int32("Size", 0);
    int sizeB = sizeJ + 1;
    c["Variant"] = Int(sizeB);
    c["Size"] = Byte(sizeB);

    auto health = tag.float32("Health");
    auto attributes = EntityAttributes::Slime(sizeB, health);
    c["Attributes"] = attributes.toListTag();
  }

  static void SnowGolem(CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
    auto pumpkin = tag.boolean("Pumpkin", true);
    if (!pumpkin) {
      AddDefinition(c, "+minecraft:snowman_sheared");
    }
  }

  static void TntMinecart(CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
    AddDefinition(c, "+minecraft:tnt_minecart");
    auto fuse = tag.int32("TNTFuse", -1);
    if (fuse < 0) {
      AddDefinition(c, "+minecraft:inactive");
    } else {
      c["Fuse"] = Byte(fuse);
      AddDefinition(c, "-minecraft:inactive");
      AddDefinition(c, "+minecraft:primed_tnt");
    }
  }

  static void TraderLlama(CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
    AddDefinition(c, "+minecraft:llama_wandering_trader");
    AddDefinition(c, "-minecraft:llama_wild");
    AddDefinition(c, "+minecraft:llama_tamed");
    AddDefinition(c, "+minecraft:strength_3");
    c["InventoryVersion"] = String("1.16.40");
    c["MarkVariant"] = Int(1);
    c["IsTamed"] = Bool(true);
  }

  static void TropicalFish(CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
    auto variant = tag.int32("Variant", 0);
    auto tf = TropicalFish::FromJavaVariant(variant);
    c["Variant"] = Int(tf.fSmall ? 0 : 1);
    c["MarkVariant"] = Int(tf.fPattern);
    c["Color"] = Byte(tf.fBodyColor);
    c["Color2"] = Byte(tf.fPatternColor);
  }

  static void Turtle(CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
    auto x = tag.int32("HomePosX");
    auto y = tag.int32("HomePosY");
    auto z = tag.int32("HomePosZ");
    if (x && y && z) {
      auto homePos = List<Tag::Type::Float>();
      homePos->push_back(Float(*x));
      homePos->push_back(Float(*y));
      homePos->push_back(Float(*z));
      c["HomePos"] = homePos;
    }

    if (tag.boolean("HasEgg", false)) {
      AddDefinition(c, "-minecraft:pregnant");
      AddDefinition(c, "-minecraft:wants_to_lay_egg");
    }
  }

  static void Villager(CompoundTag &c, CompoundTag const &tag, ConverterContext &ctx) {
    using namespace std;

    auto data = tag.compoundTag("VillagerData");
    optional<VillagerProfession> profession;
    optional<VillagerType> type;
    if (data) {
      auto inProfession = data->string("profession");
      if (inProfession) {
        profession = VillagerProfession::FromJavaProfession(*inProfession);
      }

      auto inType = data->string("type");
      if (inType) {
        type = VillagerType::FromJavaType(*inType);
      }
    }

    if (profession) {
      c["PreferredProfession"] = String(profession->string());
      AddDefinition(c, "+" + profession->string());
      c["Variant"] = Int(profession->variant());
      if (auto tradeTablePath = profession->tradeTablePath(); tradeTablePath) {
        c["TradeTablePath"] = String(*tradeTablePath);
      }
    }
    if (type) {
      AddDefinition(c, "+" + type->string() + "_villager");
      c["MarkVariant"] = Int(type->variant());
    }

    auto age = tag.int32("Age", 0);
    if (age < 0) {
      c["Age"] = Int(age);
      AddDefinition(c, "+baby");
    } else {
      AddDefinition(c, "+adult");
    }
    AddDefinition(c, "+minecraft:villager_v2");

    auto tradeExp = tag.int32("Xp", 0);
    c["TradeExperience"] = Int(tradeExp);

    int tradeTier = 0;
    if (tradeExp >= 250) {
      tradeTier = 4;
    } else if (tradeExp >= 150) {
      tradeTier = 3;
    } else if (tradeExp >= 70) {
      tradeTier = 2;
    } else if (tradeExp >= 10) {
      tradeTier = 1;
    }
    c["TradeTier"] = Int(tradeTier);
  }

  static void WanderingTrader(CompoundTag &c, CompoundTag const &tag, ConverterContext &ctx) {
    if (auto despawnDelay = tag.int32("DespawnDelay"); despawnDelay) {
      int64_t timestamp = ctx.fCtx.fGameTick + *despawnDelay;
      c["TimeStamp"] = Long(timestamp);
    }
  }

  static void Wolf(CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
    auto health = tag.float32("Health");
    auto owner = GetOwnerUuid(tag);

    auto attributes = EntityAttributes::Wolf(!!owner, health);
    c["Attributes"] = attributes.toListTag();
  }

  static void ZombieVillager(CompoundTag &c, CompoundTag const &tag, ConverterContext &ctx) {
    using namespace std;
    auto data = tag.compoundTag("VillagerData");
    optional<VillagerProfession> profession;
    if (data) {
      auto inProfession = data->string("profession");
      if (inProfession) {
        profession = VillagerProfession::FromJavaProfession(*inProfession);
        AddDefinition(c, "+" + profession->string());
      }
      auto inType = data->string("type");
      if (inType) {
        auto type = VillagerType::FromJavaType(*inType);
        if (type) {
          switch (type->variant()) {
          case VillagerType::Plains:
            AddDefinition(c, "+villager_skin_1");
            break;
          case VillagerType::Desert:
            AddDefinition(c, "+villager_skin_4");
            AddDefinition(c, "+desert_villager");
            break;
          case VillagerType::Jungle:
            AddDefinition(c, "+villager_skin_4");
            AddDefinition(c, "+jungle_villager");
            break;
          case VillagerType::Savanna:
            AddDefinition(c, "+villager_skin_5");
            AddDefinition(c, "+savanna_villager");
            break;
          case VillagerType::Snow:
            AddDefinition(c, "+villager_skin_3");
            AddDefinition(c, "+snow_villager");
            break;
          case VillagerType::Swamp:
            AddDefinition(c, "+villager_skin_1");
            AddDefinition(c, "+swamp_villager");
            break;
          case VillagerType::Taiga:
            AddDefinition(c, "+villager_skin_2");
            break;
          }
        }
      }
    }

    auto offers = tag.compoundTag("Offers");
    if (offers) {
      c["Persistent"] = Bool(true);
    }

    CopyIntValues(tag, c, {{"Xp", "TradeExperience"}});
  }
#pragma endregion

#pragma region Behaviors
  static void AgeableC(CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
    auto age = tag.int32("Age", 0);
    if (age < 0) {
      AddDefinition(c, "+baby");
      c["Age"] = Int(age);
    } else {
      AddDefinition(c, "+adult");
      c.erase("Age");
    }
    c["IsBaby"] = Bool(age < 0);
  }

  static void AttackTime(CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
    auto attackTick = tag.int32("AttackTick", 0);
    c["AttackTime"] = Short(attackTick);
  }

  static void CanJoinRaid(CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
    auto canJoin = tag.boolean("CanJoinRaid");
    if (canJoin == true) {
      AddDefinition(c, "+minecraft:raid_configuration");
    }
  }

  static void CollarColorable(CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
    auto collarColor = tag.byte("CollarColor");
    if (collarColor && GetOwnerUuid(tag)) {
      c["Color"] = Byte(*collarColor);
    }
  }

  static void Debug(CompoundTag &c, CompoundTag const &tag, ConverterContext &) {}

  static void DetectSuffocation(CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
    AddDefinition(c, "-minecraft:start_suffocating");
    AddDefinition(c, "+minecraft:detect_suffocating");
  }

  static void ChestItemsFromInventory(CompoundTag &c, CompoundTag const &tag, ConverterContext &ctx) {
    auto inventory = tag.listTag("Inventory");
    auto chestItems = List<Tag::Type::Compound>();
    if (inventory) {
      // items in "Inventory" does not have "Slot" property. So we can't use ConvertAnyItemList here.
      int8_t slot = 0;
      for (auto const &it : *inventory) {
        auto itemJ = std::dynamic_pointer_cast<CompoundTag>(it);
        if (!itemJ) {
          continue;
        }
        CompoundTagPtr itemB = Item::From(itemJ, ctx.fCtx);
        if (!itemB) {
          itemB = Compound();
          itemB->set("Count", Byte(0));
          itemB->set("Damage", Short(0));
          itemB->set("Name", String(""));
          itemB->set("WasPickedUp", Bool(false));
        }
        itemB->set("Slot", Byte(slot));
        chestItems->push_back(itemB);
        slot++;
      }
    }
    c["ChestItems"] = chestItems;
  }

  static void ChestItemsFromItems(CompoundTag &c, CompoundTag const &tag, ConverterContext &ctx) {
    auto chestItems = List<Tag::Type::Compound>();

    auto items = tag.listTag("Items");
    if (items) {
      for (auto const &it : *items) {
        if (it->type() != Tag::Type::Compound) {
          continue;
        }
        auto item = std::dynamic_pointer_cast<CompoundTag>(it);
        if (!item) {
          continue;
        }
        auto converted = Item::From(item, ctx.fCtx);
        if (!converted) {
          continue;
        }
        auto slot = item->byte("Slot");
        if (!slot) {
          continue;
        }
        converted->set("Slot", Byte(*slot));
        chestItems->push_back(converted);
      }
    }

    c["ChestItems"] = chestItems;
  }

  static void PersistentFromFromBucket(CompoundTag &c, CompoundTag const &tag, ConverterContext &ctx) {
    auto fromBucket = tag.boolean("FromBucket", false);
    if (fromBucket) {
      c["Persistent"] = Bool(true);
    } else {
      c["NaturalSpawn"] = Bool(true);
    }
  }

  static void Sittable(CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
    CopyBoolValues(tag, c, {{"Sitting"}});
  }

  static void Temper(CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
    CopyIntValues(tag, c, {{"Temper"}});
  }
#pragma endregion

#pragma region Converters
  static CompoundTagPtr Animal(CompoundTag const &tag, ConverterContext &ctx) {
    auto c = Mob(tag, ctx);
    if (!c) {
      return nullptr;
    }

    c->set("Persistent", Bool(true));

    auto leash = tag.compoundTag("Leash");
    if (leash) {
      auto x = leash->int32("X");
      auto y = leash->int32("Y");
      auto z = leash->int32("Z");
      auto leashedUuid = GetEntityUuid(tag);
      auto leasherUuid = props::GetUuid(*leash, {.fIntArray = "UUID"});
      if (x && y && z && leashedUuid) {
        int64_t leasherId = UuidRegistrar::LeasherIdFor(*leashedUuid);

        Entity e(leasherId);
        e.fPos = Pos3f(*x + 0.5f, *y + 0.25f, *z + 0.5f);
        e.fIdentifier = "minecraft:leash_knot";
        auto leashEntityData = e.toCompoundTag();
        int cx = mcfile::Coordinate::ChunkFromBlock(*x);
        int cz = mcfile::Coordinate::ChunkFromBlock(*z);
        ctx.fLeashKnots[{cx, cz}].push_back(leashEntityData);

        c->set("LeasherID", Long(leasherId));
      } else if (leasherUuid) {
        auto leasherUuidB = UuidRegistrar::ToId(*leasherUuid);
        c->set("LeasherID", Long(leasherUuidB));
      }
    }

    return c;
  }

  static CompoundTagPtr Default(CompoundTag const &tag) {
    auto e = BaseProperties(tag);
    if (!e) {
      return nullptr;
    }
    return e->toCompoundTag();
  }

  static CompoundTagPtr EndCrystal(CompoundTag const &tag, ConverterContext &ctx) {
    auto e = BaseProperties(tag);
    if (!e) {
      return nullptr;
    }
    e->fIdentifier = "minecraft:ender_crystal";
    auto c = e->toCompoundTag();
    CopyBoolValues(tag, *c, {{"ShowBottom"}});

    if (auto beamTarget = tag.compoundTag("BeamTarget"); beamTarget) {
      auto x = beamTarget->int32("X");
      auto y = beamTarget->int32("Y");
      auto z = beamTarget->int32("Z");
      if (x && y && z) {
        c->set("BlockTargetX", Int(*x));
        c->set("BlockTargetY", Int(*y));
        c->set("BlockTargetZ", Int(*z));
      }
    }

    return c;
  }

  static CompoundTagPtr EnderDragon(CompoundTag const &tag, ConverterContext &ctx) {
    auto c = Monster(tag, ctx);
    AddDefinition(*c, "-dragon_sitting");
    AddDefinition(*c, "+dragon_flying");
    c->set("Persistent", Bool(true));
    c->set("IsAutonomous", Bool(true));
    c->set("LastDimensionId", Int(2));

    auto dragonDeathTime = tag.int32("DragonDeathTime", 0);
    if (dragonDeathTime > 0) {
      c->set("DeathTime", Int(dragonDeathTime));
      RemoveDefinition(*c, "+dragon_flying");
      AddDefinition(*c, "-dragon_flying");
      AddDefinition(*c, "+dragon_death");
    }

    ctx.fCtx.fWorldData.addAutonomousEntity(c);
    return c;
  }

  static CompoundTagPtr EntityBase(CompoundTag const &tag, ConverterContext &ctx) {
    auto e = BaseProperties(tag);
    if (!e) {
      return nullptr;
    }
    return e->toCompoundTag();
  }

  static CompoundTagPtr Item(CompoundTag const &tag, ConverterContext &ctx) {
    auto e = BaseProperties(tag);
    if (!e) {
      return nullptr;
    }

    auto item = tag.compoundTag("Item");
    if (!item) {
      return nullptr;
    }
    auto beItem = Item::From(item, ctx.fCtx);
    if (!beItem) {
      return nullptr;
    }

    auto ret = e->toCompoundTag();
    ret->set("Item", beItem);

    auto thrower = props::GetUuid(tag, {.fIntArray = "Thrower"});
    int64_t owner = -1;
    if (thrower) {
      owner = UuidRegistrar::ToId(*thrower);
    }
    ret->set("OwnerID", Long(owner));
    ret->set("OwnerNew", Long(owner));

    CopyShortValues(tag, *ret, {{"Health"}, {"Age"}});

    return ret;
  }

  static CompoundTagPtr LivingEntity(CompoundTag const &tag, ConverterContext &ctx) {
    auto e = BaseProperties(tag);
    if (!e) {
      return nullptr;
    }
    auto ret = e->toCompoundTag();
    auto &c = *ret;
    auto air = tag.int16("Air", 300);
    auto armor = GetArmor(tag, ctx);
    auto mainhand = GetMainhand(tag, ctx);
    auto offhand = GetOffhand(tag, ctx);
    auto canPickupLoot = tag.boolean("CanPickUpLoot", false);
    auto deathTime = tag.int16("DeathTime", 0);
    auto hurtTime = tag.int16("HurtTime", 0);
    auto inLove = tag.boolean("InLove", false);
    c["Armor"] = armor;
    c["Mainhand"] = mainhand;
    c["Offhand"] = offhand;
    c["Air"] = Short(air);
    c["AttackTime"] = Short(0);
    c["BodyRot"] = Float(0);
    c["boundX"] = Int(0);
    c["boundY"] = Int(0);
    c["boundZ"] = Int(0);
    c["BreedCooldown"] = Int(0);
    c["canPickupItems"] = Bool(canPickupLoot);
    c["Dead"] = Bool(false);
    c["DeathTime"] = Short(0);
    c["hasBoundOrigin"] = Bool(false);
    c["hasSetCanPickupItems"] = Bool(true);
    c["HurtTime"] = Short(tag.int16("HurtTime", 0));
    c["InLove"] = Bool(false);
    c["IsPregnant"] = Bool(false);
    c["LeasherID"] = Long(-1);
    c["limitedLife"] = Int(0);
    c["LoveCause"] = Long(0);
    c["NaturalSpawn"] = Bool(false);
    c["Surface"] = Bool(false);
    c["TargetID"] = Long(-1);
    c["TradeExperience"] = Int(0);
    c["TradeTier"] = Int(0);
    if (!e->fIdentifier.empty()) {
      AddDefinition(*ret, "+" + e->fIdentifier);
    }
    ret->erase("Motion");
    ret->erase("Dir");
    return ret;
  }

  static CompoundTagPtr Mob(CompoundTag const &tag, ConverterContext &ctx) {
    auto ret = LivingEntity(tag, ctx);
    if (!ret) {
      return ret;
    }

    auto fromBucket = tag.boolean("FromBucket");
    if (fromBucket) {
      ret->set("Persistent", Bool(*fromBucket));
      ret->set("PersistenceRequired", Bool(false));
    } else {
      CopyBoolValues(tag, *ret, {{"PersistenceRequired", "Persistent", false}});
    }

    auto id = tag.string("id");
    if (id) {
      auto health = tag.float32("Health");
      if (*id == "minecraft:horse" || *id == "minecraft:donkey" || *id == "minecraft:mule" || *id == "minecraft:skeleton_horse" || *id == "minecraft:zombie_horse") {
        auto attributes = EntityAttributes::AnyHorse(tag, health);
        if (attributes) {
          ret->set("Attributes", attributes);
        }
      } else {
        auto attributes = EntityAttributes::Mob(*id, health);
        if (attributes) {
          ret->set("Attributes", attributes->toListTag());
        }
      }
    }

    return ret;
  }

  static CompoundTagPtr Monster(CompoundTag const &tag, ConverterContext &ctx) {
    auto c = Mob(tag, ctx);
    c->set("SpawnedByNight", Bool(false));
    return c;
  }

  static CompoundTagPtr Null(CompoundTag const &tag, ConverterContext &ctx) { return nullptr; }

  static CompoundTagPtr Painting(CompoundTag const &tag, ConverterContext &ctx) {
    auto facing = tag.byte("Facing", 0);
    Facing4 f4 = Facing4FromBedrockDirection(facing);
    auto motiveJ = tag.string("Motive", "minecraft:aztec");
    Painting::Motive motive = Painting::MotiveFromJava(motiveJ);
    auto motiveB = Painting::BedrockFromMotive(motive);

    auto tileX = tag.int32("TileX");
    auto tileY = tag.int32("TileY");
    auto tileZ = tag.int32("TileZ");
    if (!tileX || !tileY || !tileZ) {
      return nullptr;
    }
    auto pos = Painting::BedrockPosFromJavaTilePos(Pos3i(*tileX, *tileY, *tileZ), f4, motive);
    if (!pos) {
      return nullptr;
    }

    auto e = BaseProperties(tag);
    if (!e) {
      return nullptr;
    }
    e->fIdentifier = "minecraft:painting";
    e->fPos = *pos;
    auto c = e->toCompoundTag();
    c->set("Motive", String(motiveB));
    c->set("Direction", Byte(facing));
    return c;
  }
#pragma endregion

#pragma region BehaviorGenerators
  static Behavior AgeableA(std::string const &definitionKey) {
    return [=](CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
      auto age = tag.int32("Age", 0);
      if (age < 0) {
        AddDefinition(c, "+minecraft:" + definitionKey + "_baby");
        c["Age"] = Int(age);
      } else {
        AddDefinition(c, "+minecraft:" + definitionKey + "_adult");
        c.erase("Age");
      }
      c["IsBaby"] = Bool(age < 0);
    };
  }

  static Behavior AgeableB(std::string const &definitionKey) {
    return [=](CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
      auto baby = tag.boolean("IsBaby", false);
      if (baby) {
        AddDefinition(c, "+minecraft:" + definitionKey + "_baby");
      } else {
        AddDefinition(c, "+minecraft:" + definitionKey + "_adult");
      }
      c["IsBaby"] = Bool(baby);
    };
  }

  static Behavior AgeableD(std::string const &definitionKey) {
    return [=](CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
      auto baby = tag.boolean("IsBaby", false);
      if (baby) {
        AddDefinition(c, "+minecraft:baby_" + definitionKey);
      } else {
        AddDefinition(c, "+minecraft:adult_" + definitionKey);
      }
      c["IsBaby"] = Bool(baby);
    };
  }

  static Behavior Boat(std::string const &definitionKey) {
    return [=](CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
      AddDefinition(c, "+minecraft:" + definitionKey);

      auto type = tag.string("Type", "oak");
      int32_t variant = Boat::BedrockVariantFromJavaType(type);
      c["Variant"] = Int(variant);

      auto rotation = props::GetRotation(c, "Rotation");
      if (rotation) {
        Rotation rot(Rotation::ClampDegreesBetweenMinus180And180(rotation->fYaw + 90), rotation->fPitch);
        c["Rotation"] = rot.toListTag();
      }

      auto pos = GetBoatPos(tag);
      c["Pos"] = pos->toF().toListTag();
    };
  }

  static Behavior ChestedHorse(std::string const &definitionKey) {
    return [=](CompoundTag &c, CompoundTag const &tag, ConverterContext &ctx) {
      auto chested = tag.boolean("ChestedHorse", false);
      c["Chested"] = Bool(chested);
      if (!chested) {
        AddDefinition(c, "+minecraft:" + definitionKey + "_unchested");
        return;
      }
      auto chestItems = tag.listTag("Items");
      if (!chestItems) {
        AddDefinition(c, "+minecraft:" + definitionKey + "_unchested");
        return;
      }

      for (auto const &it : *chestItems) {
        auto item = std::dynamic_pointer_cast<CompoundTag>(it);
        if (!item) {
          continue;
        }
        auto slot = item->byte("Slot");
        if (!slot) {
          continue;
        }
        int8_t idx = *slot - 1;
        if (idx < 0 || 16 <= idx) {
          continue;
        }
        auto count = item->byte("Count");
        if (!count) {
          continue;
        }
        auto outItem = Item::From(item, ctx.fCtx);
        if (!outItem) {
          continue;
        }
        AddChestItem(c, outItem, idx, *count);
      }
      AddDefinition(c, "-minecraft:" + definitionKey + "_unchested");
      AddDefinition(c, "+minecraft:" + definitionKey + "_chested");
    };
  }

  static Behavior Colorable(std::string const &definitionKey) {
    return [=](CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
      auto color = tag.byte("Color", 0);
      c["Color"] = Byte(color);
      CopyByteValues(tag, c, {{"Color"}});
      AddDefinition(c, "+minecraft:" + definitionKey + "_" + BedrockNameFromColorCodeJava((ColorCodeJava)color));
    };
  }

  template <class... Arg>
  static Behavior Definitions(Arg... defs) {
    return [=](CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
      for (std::string const &def : std::initializer_list<std::string>{defs...}) {
        AddDefinition(c, def);
      }
    };
  }

  static Behavior Offers(int maxTradeTier, std::string offersKey) {
    return [maxTradeTier, offersKey](CompoundTag &c, CompoundTag const &tag, ConverterContext &ctx) {
      using namespace std;

      auto offers = tag.compoundTag("Offers");
      if (offers) {
        auto recipes = offers->listTag("Recipes");
        if (recipes) {
          auto rs = List<Tag::Type::Compound>();
          for (int i = 0; i < recipes->size(); i++) {
            auto recipe = recipes->at(i);
            auto r = recipe->asCompound();
            if (!r) {
              continue;
            }
            auto converted = BedrockRecipieFromJava(*r, ctx);
            if (!converted) {
              continue;
            }
            int tier = min(i / 2, maxTradeTier);
            converted->set("tier", Int(tier));
            rs->push_back(converted);
          }
          if (!rs->empty()) {
            auto of = Compound();
            of->set("Recipes", rs);
            auto expRequirements = List<Tag::Type::Compound>();
            if (maxTradeTier >= 0) {
              auto tier0 = Compound();
              tier0->set("0", Int(0));
              expRequirements->push_back(tier0);
            }
            if (maxTradeTier >= 1) {
              auto tier1 = Compound();
              tier1->set("1", Int(10));
              expRequirements->push_back(tier1);
            }
            if (maxTradeTier >= 2) {
              auto tier2 = Compound();
              tier2->set("2", Int(70));
              expRequirements->push_back(tier2);
            }
            if (maxTradeTier >= 3) {
              auto tier3 = Compound();
              tier3->set("3", Int(150));
              expRequirements->push_back(tier3);
            }
            if (maxTradeTier >= 4) {
              auto tier4 = Compound();
              tier4->set("4", Int(250));
              expRequirements->push_back(tier4);
            }
            of->set("TierExpRequirements", expRequirements);
            c[offersKey] = of;
          }
        }
      }
    };
  }

  static Behavior Rename(std::string const &name) {
    return [=](CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
      auto id = tag.string("id");
      if (!id) {
        return;
      }
      RemoveDefinition(c, "+" + *id);
      AddDefinition(c, "+minecraft:" + name);
      c["identifier"] = String("minecraft:" + name);
    };
  }

  static Behavior Steerable(std::string const &definitionKey) {
    return [=](CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
      auto saddle = tag.boolean("Saddle", false);
      auto saddleItem = tag.compoundTag("SaddleItem");
      if (saddle || saddleItem) {
        AddDefinition(c, "-minecraft:" + definitionKey + "_unsaddled");
        AddDefinition(c, "+minecraft:" + definitionKey + "_saddled");
        if (saddleItem) {
          auto item = Compound();
          item->insert({
              {"Damage", Byte(0)},
              {"Name", String("minecraft:saddle")},
              {"WasPickedUp", Bool(false)},
          });
          AddChestItem(c, item, 0, 1);
        }
      } else {
        AddDefinition(c, "+minecraft:" + definitionKey + "_unsaddled");
      }
      c["Saddled"] = Bool(saddle);
    };
  }

  static Behavior TameableA(std::string const &definitionKey) {
    return [=](CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
      auto owner = GetOwnerUuid(tag);
      if (owner) {
        c["OwnerNew"] = Long(*owner);
        AddDefinition(c, "-minecraft:" + definitionKey + "_wild");
        AddDefinition(c, "+minecraft:" + definitionKey + "_tame");
        c["IsTamed"] = Bool(true);
      } else {
        AddDefinition(c, "+minecraft:" + definitionKey + "_wild");
      }
    };
  }

  static Behavior TameableB(std::string const &definitionKey) {
    return [=](CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
      auto owner = GetOwnerUuid(tag);
      if (owner) {
        c["OwnerNew"] = Long(*owner);
        AddDefinition(c, "-minecraft:" + definitionKey + "_wild");
        AddDefinition(c, "+minecraft:" + definitionKey + "_tamed");
        c["IsTamed"] = Bool(true);
      } else {
        AddDefinition(c, "+minecraft:" + definitionKey + "_wild");
      }
    };
  }

  static Behavior Vehicle(std::optional<std::string> jockeyDefinitionKey = std::nullopt) {
    return [=](CompoundTag &c, CompoundTag const &tag, ConverterContext &ctx) {
      auto links = List<Tag::Type::Compound>();

      auto passengers = tag.query("Passengers")->asList();
      if (passengers) {
        for (int i = 0; i < passengers->size(); i++) {
          auto const &p = passengers->at(i);
          auto comp = p->asCompound();
          if (!comp) {
            continue;
          }

          auto ret = From(*comp, ctx.fCtx);
          if (!ret.fEntity) {
            continue;
          }

          auto const &passenger = ret.fEntity;
          auto uid = passenger->int64("UniqueID");
          if (!uid) {
            continue;
          }
          auto link = Compound();
          link->set("entityID", Long(*uid));
          link->set("linkID", Int(i));
          links->push_back(link);

          auto passengerId = passenger->string("identifier");
          if (passengerId && *passengerId == "minecraft:zombie") {
            AddDefinition(*passenger, "+minecraft:zombie_jockey");
          }

          ctx.fPassengers.push_back(passenger);
        }

        if (jockeyDefinitionKey) {
          AddDefinition(c, "+minecraft:" + *jockeyDefinitionKey + "_parent_jockey");
        }
      }

      if (!links->empty()) {
        c["LinksTag"] = links;
      }
    };
  }
#pragma endregion

#pragma region Utilities
  static CompoundTagPtr BedrockRecipieFromJava(CompoundTag const &java, ConverterContext &ctx) {
    using namespace std;

    auto buyA = java.compoundTag("buy");
    auto buyB = java.compoundTag("buyB");
    auto sell = java.compoundTag("sell");

    if (!buyA || !sell) {
      return nullptr;
    }
    auto bedrock = Compound();

    {
      auto count = buyA->byte("Count", 0);
      auto item = Item::From(buyA, ctx.fCtx);
      if (item && count > 0) {
        bedrock->set("buyA", item);
        bedrock->set("buyCountA", Int(count));
      } else {
        return nullptr;
      }
    }

    if (buyB) {
      auto count = buyB->byte("Count", 0);
      auto id = buyB->string("id", "minecraft:air");
      auto item = Item::From(buyB, ctx.fCtx);
      if (id != "minecraft:air" && item && count > 0) {
        bedrock->set("buyB", item);
        bedrock->set("buyCountB", Int(count));
      } else {
        bedrock->set("buyCountB", Int(0));
      }
    } else {
      bedrock->set("buyCountB", Int(0));
    }

    {
      auto count = sell->byte("Count", 0);
      if (count <= 0) {
        return nullptr;
      }
      auto item = Item::From(sell, ctx.fCtx);
      if (!item) {
        return nullptr;
      }
      bedrock->set("sell", item);
    }

    CopyIntValues(java, *bedrock, {{"demand"}, {"maxUses"}, {"uses"}, {"xp", "traderExp"}});
    CopyByteValues(java, *bedrock, {{"rewardExp"}});
    CopyFloatValues(java, *bedrock, {{"priceMultiplier", "priceMultiplierA"}, {"priceMultiplierB"}});

    return bedrock;
  }

  static ListTagPtr InitItemList(uint32_t capacity) {
    auto items = List<Tag::Type::Compound>();
    for (int i = 0; i < capacity; i++) {
      auto empty = Item::Empty();
      empty->set("Slot", Byte(i));
      empty->set("Count", Byte(0));
      items->push_back(empty);
    }
    return items;
  }

  static ListTagPtr ConvertAnyItemList(ListTagPtr const &input, uint32_t capacity, Context const &ctx) {
    auto ret = InitItemList(capacity);
    for (auto const &it : *input) {
      auto item = std::dynamic_pointer_cast<CompoundTag>(it);
      if (!item) {
        continue;
      }
      auto converted = Item::From(item, ctx);
      if (!converted) {
        continue;
      }
      auto slot = item->byte("Slot");
      if (!slot) {
        continue;
      }
      if (*slot < 0 || capacity <= *slot) {
        continue;
      }
      auto count = item->byte("Count");
      if (!count) {
        continue;
      }
      converted->set("Slot", Byte(*slot));
      converted->set("Count", Byte(*count));
      ret->at(*slot) = converted;
    }
    return ret;
  }

  static void AddChestItem(CompoundTag &c, CompoundTagPtr const &item, int8_t slot, int8_t count) {
    item->set("Slot", Byte(slot));
    item->set("Count", Byte(count));
    auto chestItems = c.listTag("ChestItems");
    if (!chestItems) {
      chestItems = InitItemList(16);
    }
    chestItems->at(slot) = item;
    c["ChestItems"] = chestItems;
  }

  static std::optional<int64_t> GetOwnerUuid(CompoundTag const &tag) {
    auto uuid = props::GetUuid(tag, {.fIntArray = "Owner", .fHexString = "OwnerUUID"});
    if (!uuid) {
      return std::nullopt;
    }
    return UuidRegistrar::ToId(*uuid);
  }

  static void AddDefinition(CompoundTag &tag, std::string const &definition) {
    auto found = tag.find("definitions");
    auto d = List<Tag::Type::String>();
    if (found != tag.end()) {
      auto current = found->second->asList();
      if (current && current->fType == Tag::Type::String) {
        for (auto c : *current) {
          if (c->asString()) {
            d->push_back(String(c->asString()->fValue));
          }
        }
      }
    }
    d->push_back(String(definition));
    tag["definitions"] = d;
  }

  static void RemoveDefinition(CompoundTag &tag, std::string const &definition) {
    auto found = tag.find("definitions");
    auto d = List<Tag::Type::String>();
    if (found != tag.end()) {
      auto current = found->second->asList();
      if (current && current->fType == Tag::Type::String) {
        for (auto c : *current) {
          if (c->asString() && c->asString()->fValue != definition) {
            d->push_back(String(c->asString()->fValue));
          }
        }
      }
    }
    tag["definitions"] = d;
  }

  static ListTagPtr GetArmor(CompoundTag const &tag, ConverterContext &ctx) {
    auto armors = List<Tag::Type::Compound>();
    for (int i = 0; i < 4; i++) {
      armors->push_back(Item::Empty());
    }

    auto found = tag.find("ArmorItems");
    if (found != tag.end()) {
      auto list = found->second->asList();
      if (list && list->fType == Tag::Type::Compound) {
        for (int i = 0; i < 4 && i < list->size(); i++) {
          auto item = std::dynamic_pointer_cast<CompoundTag>(list->at(i));
          if (!item) {
            continue;
          }
          auto converted = Item::From(item, ctx.fCtx);
          if (converted) {
            armors->fValue[i] = converted;
          }
        }
      }
    }

    auto ret = List<Tag::Type::Compound>();
    ret->push_back(armors->at(3));
    ret->push_back(armors->at(2));
    ret->push_back(armors->at(1));
    ret->push_back(armors->at(0));

    return ret;
  }

  static ListTagPtr GetMainhand(CompoundTag const &input, ConverterContext &ctx) { return HandItem<0>(input, ctx); }

  static ListTagPtr GetOffhand(CompoundTag const &input, ConverterContext &ctx) { return HandItem<1>(input, ctx); }

  template <size_t index>
  static ListTagPtr HandItem(CompoundTag const &input, ConverterContext &ctx) {
    auto ret = List<Tag::Type::Compound>();

    auto mainHand = input.listTag("HandItems");
    CompoundTagPtr item;

    if (mainHand && mainHand->fType == Tag::Type::Compound && index < mainHand->fValue.size()) {
      auto inItem = std::dynamic_pointer_cast<CompoundTag>(mainHand->fValue[index]);
      if (inItem) {
        item = Item::From(inItem, ctx.fCtx);
      }
    }
    if (!item) {
      item = Item::Empty();
    }
    ret->fValue.push_back(item);

    return ret;
  }

  static std::optional<int64_t> GetEntityUuid(CompoundTag const &tag) {
    auto uuid = props::GetUuid(tag, {.fLeastAndMostPrefix = "UUID", .fIntArray = "UUID"});
    if (!uuid) {
      return std::nullopt;
    }
    return UuidRegistrar::ToId(*uuid);
  }

  static std::optional<Entity> BaseProperties(CompoundTag const &tag) {
    using namespace je2be::props;
    using namespace std;

    auto fallDistance = tag.float32("FallDistance");
    auto fire = tag.int16("Fire");
    auto invulnerable = tag.boolean("Invulnerable");
    auto onGround = tag.boolean("OnGround");
    auto portalCooldown = tag.int32("PortalCooldown");
    auto motion = GetPos3d(tag, "Motion");
    auto pos = GetPos3d(tag, "Pos");
    auto rotation = GetRotation(tag, "Rotation");
    auto uuid = GetEntityUuid(tag);
    auto id = tag.string("id");
    auto customName = GetJson(tag, "CustomName");

    if (!uuid) {
      return nullopt;
    }

    Entity e(*uuid);
    if (motion) {
      e.fMotion = motion->toF();
    }
    if (pos) {
      e.fPos = pos->toF();
    }
    if (rotation) {
      e.fRotation = *rotation;
    }
    if (fallDistance) {
      e.fFallDistance = *fallDistance;
    }
    if (fire) {
      e.fFire = *fire;
    }
    if (invulnerable) {
      e.fInvulnerable = *invulnerable;
    }
    if (onGround) {
      e.fOnGround = *onGround;
    }
    if (portalCooldown) {
      e.fPortalCooldown = *portalCooldown;
    }
    if (id) {
      e.fIdentifier = *id;
    }
    if (customName) {
      auto text = customName->find("text");
      if (text != customName->end() && text->is_string()) {
        auto t = text->get<std::string>();
        e.fCustomName = t;
        e.fCustomNameVisible = true;
      }
    }

    return e;
  }
#pragma endregion

public:
  std::vector<std::string> fDefinitions;
  Pos3f fMotion;
  Pos3f fPos;
  Rotation fRotation;
  std::vector<CompoundTagPtr> fTags;
  bool fChested = false;
  int8_t fColor2 = 0;
  int8_t fColor = 0;
  int8_t fDir = 0;
  float fFallDistance = 0;
  int16_t fFire = 0;
  std::string fIdentifier;
  bool fInvulnerable = false;
  bool fIsAngry = false;
  bool fIsAutonomous = false;
  bool fIsBaby = false;
  bool fIsEating = false;
  bool fIsGliding = false;
  bool fIsGlobal = false;
  bool fIsIllagerCaptain = false;
  bool fIsOrphaned = false;
  bool fIsRoaring = false;
  bool fIsScared = false;
  bool fIsStunned = false;
  bool fIsSwimming = false;
  bool fIsTamed = false;
  bool fIsTrusting = false;
  int32_t fLastDimensionId = 0;
  bool fLootDropped = false;
  int32_t fMarkVariant = 0;
  bool fOnGround = true;
  int64_t fOwnerNew = -1;
  int32_t fPortalCooldown = 0;
  bool fSaddled = false;
  bool fSheared = false;
  bool fShowBottom = false;
  bool fSitting = false;
  int32_t fSkinId = 0;
  int32_t fStrength = 0;
  int32_t fStrengthMax = 0;
  int64_t const fUniqueId;
  int32_t fVariant = 0;
  std::optional<std::string> fCustomName;
  bool fCustomNameVisible = false;
};

} // namespace je2be::tobe
