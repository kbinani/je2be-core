#pragma once

namespace je2be::tobe {

class Entity {
private:
  using EntityData = std::shared_ptr<CompoundTag>;

  struct Context {
    Context(JavaEditionMap const &mapInfo, WorldData &wd) : fMapInfo(mapInfo), fWorldData(wd) {}

    std::vector<EntityData> fPassengers;
    JavaEditionMap const &fMapInfo;
    WorldData &fWorldData;
  };

  using Converter = std::function<EntityData(CompoundTag const &, Context &)>;

  using Behavior = std::function<EntityData(EntityData const &, CompoundTag const &, Context &)>;

  struct Convert {
    template <class... Arg>
    Convert(Converter base, Arg... args) : fBase(base), fBehaviors(std::initializer_list<Behavior>{args...}) {}

    EntityData operator()(CompoundTag const &input, Context &ctx) const {
      auto c = fBase(input, ctx);
      if (!c) {
        return nullptr;
      }
      for (auto const &b : fBehaviors) {
        auto next = b(c, input, ctx);
        if (!next) {
          continue;
        }
        c = next;
      }
      return c;
    }

  private:
    Converter fBase;
    std::vector<Behavior> fBehaviors;
  };

public:
  explicit Entity(int64_t uid) : fMotion(0, 0, 0), fPos(0, 0, 0), fRotation(0, 0), fUniqueId(uid) {}

  static std::vector<EntityData> From(CompoundTag const &tag, JavaEditionMap const &mapInfo, WorldData &wd) {
    using namespace props;
    auto id = tag.string("id");
    if (!id) {
      return std::vector<EntityData>();
    }
    static std::unique_ptr<std::unordered_map<std::string, Converter> const> const table(CreateEntityTable());
    auto found = table->find(*id);
    std::vector<EntityData> ret;
    if (found == table->end()) {
      auto converted = Default(tag);
      if (converted) {
        ret.push_back(converted);
      }
      return ret;
    }
    Context ctx(mapInfo, wd);
    auto converted = found->second(tag, ctx);
    if (!ctx.fPassengers.empty()) {
      std::copy(ctx.fPassengers.begin(), ctx.fPassengers.end(), std::back_inserter(ret));
    }
    if (converted) {
      ret.push_back(converted);
    }
    return ret;
  }

  static bool RotAlmostEquals(Rotation const &rot, float yaw, float pitch) { return Rotation::DegAlmostEquals(rot.fYaw, yaw) && Rotation::DegAlmostEquals(rot.fPitch, pitch); }

  static std::optional<std::tuple<Pos3i, std::shared_ptr<CompoundTag>, std::string>> ToTileEntityBlock(CompoundTag const &c) {
    using namespace std;
    using namespace props;
    auto id = c.string("id");
    assert(id);
    if (*id == "minecraft:item_frame") {
      return ToItemFrameTileEntityBlock(c, "minecraft:frame");
    } else if (*id == "minecraft:glow_item_frame") {
      return ToItemFrameTileEntityBlock(c, "minecraft:glow_frame");
    }

    return nullopt;
  }

  static std::tuple<Pos3i, std::shared_ptr<CompoundTag>, std::string> ToItemFrameTileEntityBlock(CompoundTag const &c, std::string const &name) {
    using namespace std;
    using namespace props;
    auto tileX = c.int32("TileX");
    auto tileY = c.int32("TileY");
    auto tileZ = c.int32("TileZ");

    auto rot = GetRotation(c, "Rotation");
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

    auto b = make_shared<CompoundTag>();
    auto states = make_shared<CompoundTag>();
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

  static std::shared_ptr<CompoundTag> ToTileEntityData(CompoundTag const &c, JavaEditionMap const &mapInfo, WorldData &wd) {
    using namespace props;
    auto id = c.string("id");
    assert(id);
    if (*id == "minecraft:item_frame") {
      return ToItemFrameTileEntityData(c, mapInfo, wd, "ItemFrame");
    } else if (*id == "minecraft:glow_item_frame") {
      return ToItemFrameTileEntityData(c, mapInfo, wd, "GlowItemFrame");
    }
    return nullptr;
  }

  static std::shared_ptr<CompoundTag> ToItemFrameTileEntityData(CompoundTag const &c, JavaEditionMap const &mapInfo, WorldData &wd, std::string const &name) {
    using namespace props;
    auto tag = std::make_shared<CompoundTag>();
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
      auto m = Item::From(item, mapInfo, wd);
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

  std::shared_ptr<CompoundTag> toCompoundTag() const {
    using namespace std;
    using namespace props;
    auto tag = make_shared<CompoundTag>();
    auto tags = make_shared<ListTag>(Tag::Type::Compound);
    auto definitions = make_shared<ListTag>(Tag::Type::String);
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
        {"OwnerNew", Int(fOwnerNew)},
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

  static EntityData LocalPlayer(CompoundTag const &tag, JavaEditionMap const &mapInfo, WorldData &wd) {
    using namespace std;
    using namespace mcfile;
    using namespace props;

    Context ctx(mapInfo, wd);
    auto entity = LivingEntity(tag, ctx);
    if (!entity) {
      return nullptr;
    }

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

    auto xpLevel = tag.int32("XpLevel");
    auto xpProgress = tag.float32("XpP");
    if (xpLevel && xpProgress) {
      entity->set("PlayerLevel", Int(*xpLevel));
      entity->set("PlayerLevelProgress", Float(*xpProgress));
    }

    auto xpSeed = tag.int32("XpSeed");
    if (xpSeed) {
      entity->set("EnchantmentSeed", Int(*xpSeed));
    }

    auto selectedItemSlot = tag.int32("SelectedItemSlot");
    if (selectedItemSlot) {
      entity->set("SelectedInventorySlot", Int(*selectedItemSlot));
    }
    entity->set("SelectedContainerId", Int(0));

    auto playerGameType = tag.int32("playerGameType");
    if (playerGameType) {
      entity->set("PlayerGameMode", Int(*playerGameType));
    }

    auto portalCooldown = tag.int32("PortalCooldown");
    if (portalCooldown) {
      entity->set("PortalCooldown", Int(*portalCooldown));
    }

    auto seenCredits = tag.boolean("seenCredits");
    if (seenCredits) {
      entity->set("HasSeenCredits", Bool(*seenCredits));
    }

    auto inventory = tag.listTag("Inventory");
    if (inventory) {
      auto outInventory = ConvertAnyItemList(inventory, 36, mapInfo, wd);
      entity->set("Inventory", outInventory);

      // Armor
      auto armor = InitItemList(4);
      auto boots = ItemAtSlot(*inventory, 100);
      if (boots) {
        auto outBoots = Item::From(boots, mapInfo, wd);
        if (outBoots) {
          armor->at(3) = outBoots;
        }
      }
      auto leggings = ItemAtSlot(*inventory, 101);
      if (leggings) {
        auto outLeggings = Item::From(leggings, mapInfo, wd);
        if (outLeggings) {
          armor->at(2) = outLeggings;
        }
      }
      auto chestplate = ItemAtSlot(*inventory, 102);
      if (chestplate) {
        auto outChestplate = Item::From(chestplate, mapInfo, wd);
        if (outChestplate) {
          armor->at(1) = outChestplate;
        }
      }
      auto helmet = ItemAtSlot(*inventory, 103);
      if (helmet) {
        auto outHelmet = Item::From(helmet, mapInfo, wd);
        if (outHelmet) {
          armor->at(0) = outHelmet;
        }
      }
      entity->set("Armor", armor);

      // Offhand
      auto offhand = ItemAtSlot(*inventory, -106);
      if (offhand) {
        auto offhandItem = Item::From(offhand, mapInfo, wd);
        if (offhandItem) {
          auto outOffhand = InitItemList(1);
          outOffhand->at(0) = offhandItem;
          entity->set("Offhand", outOffhand);
        }
      }
    }

    auto enderItems = tag.listTag("EnderItems");
    if (enderItems) {
      auto enderChestInventory = ConvertAnyItemList(enderItems, 27, mapInfo, wd);
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
      spawnDimension = DimensionFromString(*spawnDimensionString);
    }
    if (spawnX && spawnY && spawnZ && spawnDimension) {
      entity->set("SpawnX", Int(*spawnX));
      entity->set("SpawnY", Int(*spawnY));
      entity->set("SpawnZ", Int(*spawnZ));
      int dimension = IntFromDimension(*spawnDimension);
      entity->set("SpawnDimension", Int(dimension));
      entity->set("SpawnBlockPositionX", Int(*spawnX));
      entity->set("SpawnBlockPositionY", Int(*spawnY));
      entity->set("SpawnBlockPositionZ", Int(*spawnZ));
    }

    auto dimensionString = tag.string("Dimension");
    if (dimensionString) {
      auto dimension = DimensionFromString(*dimensionString);
      if (dimension) {
        entity->set("DimensionId", Int(IntFromDimension(*dimension)));
      }
    }

    auto pos = GetPos3d(tag, "Pos");
    if (pos) {
      //ng 1.620001f
      //ok 1.62001f
      double offset = 1.62001;
      pos->fY += offset;
      entity->set("Pos", pos->toF().toListTag());
    }

    auto definitions = make_shared<ListTag>(Tag::Type::String);
    definitions->push_back(String("+minecraft:player"));
    definitions->push_back(String("+"));
    entity->set("definitions", definitions);

    return entity;
  }

private:
  static std::shared_ptr<CompoundTag> ItemAtSlot(ListTag const &items, uint32_t slot) {
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
#define E(__name, __func) table->insert(std::make_pair("minecraft:" __name, __func))
#define A(__name) table->insert(std::make_pair("minecraft:" __name, Animal))
#define M(__name) table->insert(std::make_pair("minecraft:" __name, Monster))

    E("painting", Painting);
    E("end_crystal", EndCrystal);

    E("bat", Convert(Mob, Bat));
    E("bee", Convert(Animal, AgeableA("bee"), Bee));
    M("blaze");
    E("cat", Convert(Animal, AgeableA("cat"), TameableA("cat"), Sittable, CollarColorable, Cat));
    M("cave_spider");
    E("chicken", Convert(Animal, AgeableA("chicken"), Vehicle(), Chicken));
    A("cod");

    E("cow", Convert(Animal, AgeableA("cow")));
    E("creeper", Convert(Monster, Creeper));
    A("dolphin");
    E("donkey", Convert(Animal, TameableB("donkey"), ChestedHorse("donkey"), Steerable("donkey")));
    M("drowned");
    M("elder_guardian");
    E("enderman", Convert(Monster, Enderman));
    M("endermite");
    E("evoker", Convert(Monster, Rename("evocation_illager")));

    E("fox", Convert(Animal, Fox));
    M("ghast");
    M("guardian");
    M("hoglin");
    E("horse", Convert(Animal, TameableB("horse"), AgeableA("horse"), Steerable("horse"), Horse));
    E("husk", Convert(Monster, AgeableA("husk")));
    E("llama", Convert(Animal, AgeableA("llama"), TameableB("llama"), ChestedHorse("llama"), Llama));
    E("magma_cube", Convert(Monster, Slime));
    E("mooshroom", Convert(Animal, AgeableA("mooshroom"), Mooshroom));

    E("mule", Convert(Animal, TameableB("mule"), ChestedHorse("mule"), Steerable("mule")));
    A("ocelot");
    E("panda", Convert(Animal, AgeableA("panda")));
    E("parrot", Convert(Animal, TameableA("parrot"), Sittable, Parrot));
    M("phantom");
    E("pig", Convert(Animal, AgeableA("pig"), Steerable("pig")));
    M("piglin");
    M("piglin_brute");
    M("pillager");

    A("polar_bear");
    A("pufferfish");
    E("rabbit", Convert(Animal, AgeableC, Rabbit));
    M("ravager");
    E("salmon", Convert(Mob, Salmon));
    E("sheep", Convert(Animal, AgeableA("sheep"), Colorable("sheep"), Definitions("+minecraft:sheep_dyeable", "+minecraft:rideable_wooly", "+minecraft:loot_wooly")));
    E("shulker", Convert(Monster, Shulker));
    M("silverfish");
    M("skeleton"); // lefty skeleton does not exist in Bedrock?

    A("skeleton_horse");
    E("slime", Convert(Monster, Slime));
    E("spider", Convert(Monster, Vehicle("spider")));
    A("squid");
    M("stray");
    E("strider", Convert(Animal, Steerable("strider"), AgeableA("strider"), DetectSuffocation, Vehicle("strider")));
    E("trader_llama", Convert(Animal, Rename("llama"), AgeableA("llama"), Llama, TraderLlama));
    E("tropical_fish", Convert(Animal, Rename("tropicalfish"), TropicalFish));
    A("turtle");

    M("vex");
    E("villager", Convert(Animal, Rename("villager_v2"), Villager));
    M("vindicator");
    A("wandering_trader");
    M("witch");
    M("wither_skeleton");
    E("wolf", Convert(Animal, TameableA("wolf"), Sittable, CollarColorable));
    M("zoglin");
    E("zombie", Convert(Monster, AgeableB("zombie")));

    M("zombie_horse");
    E("zombie_villager", Convert(Animal, Rename("zombie_villager_v2")));
    E("zombified_piglin", Convert(Monster, Rename("zombie_pigman"), AgeableB("pig_zombie")));

    E("boat", Convert(EntityBase, Vehicle(), Boat));
    E("minecart", Convert(EntityBase, Vehicle(), Minecart, Definitions("+minecraft:minecart")));
    E("armor_stand", Convert(LivingEntity, ArmorStand));
    E("hopper_minecart", Convert(StorageMinecart, Minecart, Definitions("+minecraft:hopper_minecart"), HopperMinecart));
    E("chest_minecart", Convert(StorageMinecart, Minecart, Definitions("+minecraft:chest_minecart")));
    E("tnt_minecart", Convert(EntityBase, Vehicle(), Minecart, Definitions("+minecraft:tnt_minecart", "+minecraft:inactive")));
    E("snow_golem", Convert(Mob, SnowGolem));
    E("iron_golem", Convert(Mob, Definitions("+minecraft:iron_golem"), IronGolem));

    E("item", Item);
    E("ender_dragon", EnderDragon);
    E("experience_orb", Convert(LivingEntity, Rename("xp_orb"), ExperienceOrb));
    E("item_frame", Null);      // item_frame is tile entity in BE.
    E("glow_item_frame", Null); // glow_item_frame is tile entity in BE.

    E("glow_squid", Convert(Animal, Definitions("+minecraft:glow_squid")));
    E("axolotl", Convert(Animal, AgeableA("axolotl"), Axolotl));
    E("goat", Convert(Animal, AgeableA("goat"), Goat));
    E("falling_block", Convert(EntityBase, FallingBlock));
#undef A
#undef M
#undef E
    return table;
  }

  static EntityData Enderman(EntityData const &c, CompoundTag const &tag, Context &) {
    auto carriedBlockTagJ = tag.compoundTag("carriedBlockState");
    if (carriedBlockTagJ) {
      auto name = carriedBlockTagJ->string("Name");
      if (name) {
        auto carriedBlockJ = std::make_shared<mcfile::je::Block const>(*name);
        auto carriedBlockB = BlockData::From(carriedBlockJ);
        if (carriedBlockB) {
          c->set("carriedBlock", carriedBlockB);
        }
      }
    }
    return c;
  }

  static EntityData Salmon(EntityData const &c, CompoundTag const &tag, Context &) {
    auto fromBucket = tag.boolean("FromBucket", false);
    if (fromBucket) {
      c->set("Persistent", props::Bool(true));
    } else {
      c->set("NaturalSpawn", props::Bool(true));
    }
    return c;
  }

  static EntityData FallingBlock(EntityData const &c, CompoundTag const &tag, Context &) {
    auto blockState = tag.compoundTag("BlockState");
    if (!blockState) {
      return nullptr;
    }
    auto name = blockState->string("Name");
    if (!name) {
      return nullptr;
    }
    auto block = BlockData::From(std::make_shared<mcfile::je::Block>(*name));
    if (!block) {
      return nullptr;
    }
    c->set("FallingBlock", block);

    auto fallDistance = tag.float32("FallDistance", 0);
    c->set("FallDistance", props::Float(fallDistance));

    auto time = std::clamp(tag.int32("Time", 0), 0, 255);
    c->set("Time", props::Byte(time));

    return c;
  }

  static EntityData Null(CompoundTag const &tag, Context &ctx) { return nullptr; }

  static EntityData Goat(EntityData const &c, CompoundTag const &tag, Context &) {
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
    return c;
  }

  static EntityData Axolotl(EntityData const &c, CompoundTag const &tag, Context &) {
    using namespace props;
    auto originalVariant = std::clamp(tag.int32("Variant", 0), 0, 4);
    static const int variantMapping[5] = {0, 3, 2, 1, 4};
    static const std::string definitionMapping[5] = {
        "+axolotl_lucy",
        "+axolotl_wild",
        "+axolotl_gold",
        "+axolotl_cyan",
        "+axolotl_blue"};
    auto variant = variantMapping[originalVariant];
    auto definition = definitionMapping[originalVariant];
    c->set("Variant", Int(variant));
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

    return c;
  }

  static EntityData IronGolem(EntityData const &c, CompoundTag const &tag, Context &) {
    using namespace props;

    auto playerCreated = tag.boolean("PlayerCreated", false);
    auto angryAt = GetUuid(tag, {.fIntArray = "AngryAt"});
    auto angerTime = tag.int32("AngerTime", 0);

    if (playerCreated) {
      AddDefinition(c, "+minecraft:player_created");
    } else {
      AddDefinition(c, "+minecraft:village_created");
    }
    c->set("IsAngry", Bool(angerTime > 0));
    if (angryAt) {
      int64_t targetId = UuidRegistrar::ToId(*angryAt);
      c->set("TargetID", Long(targetId));
    }
    return c;
  }

  static EntityData ExperienceOrb(EntityData const &c, CompoundTag const &tag, Context &) {
    auto value = tag.int16("Value");
    if (value) {
      c->set("experience value", props::Int(*value));
    }
    AddDefinition(c, "+minecraft:xp_orb");
    return c;
  }

  static EntityData DetectSuffocation(EntityData const &c, CompoundTag const &tag, Context &) {
    AddDefinition(c, "-minecraft:start_suffocating");
    AddDefinition(c, "+minecraft:detect_suffocating");
    return c;
  }

  static EntityData ArmorStand(EntityData const &c, CompoundTag const &tag, Context &) {
    auto pos = props::GetPos3d(tag, "Pos");
    if (!pos) {
      return c;
    }
    pos->fY = std::round(pos->fY * 2) * 0.5;
    c->set("Pos", pos->toF().toListTag());
    auto attributes = EntityAttributes::Mob("minecraft:armor_stand");
    if (attributes) {
      c->set("Attributes", attributes->toListTag());
    }

    auto poseJ = tag.compoundTag("Pose");
    if (poseJ) {
      auto indexB = ArmorStand::BedrockMostSimilarPoseIndexFromJava(*poseJ);
      auto showArms = tag.boolean("ShowArms", false);
      if (indexB) {
        if (indexB != 1 || showArms) {
          auto poseB = std::make_shared<CompoundTag>();
          poseB->set("PoseIndex", props::Int(*indexB));
          poseB->set("LastSignal", props::Int(0));
          c->set("Pose", poseB);
        }
      }
    }

    return c;
  }

  static EntityData Bee(EntityData const &c, CompoundTag const &tag, Context &) {
    auto hivePos = props::GetPos3d(tag, "HivePos");
    if (hivePos) {
      Pos3f homePos = hivePos->toF();
      c->set("HomePos", homePos.toListTag());
    }
    AddDefinition(c, "+track_attacker");
    AddDefinition(c, "+shelter_detection");
    auto hasNectar = tag.boolean("HasNectar", false);
    if (hasNectar) {
      AddDefinition(c, "+has_nectar");
    }
    AddDefinition(c, "+default_sound");
    AddDefinition(c, "+find_hive");
    return c;
  }

  static EntityData SnowGolem(EntityData const &c, CompoundTag const &tag, Context &) {
    auto pumpkin = tag.boolean("Pumpkin", true);
    if (!pumpkin) {
      AddDefinition(c, "+minecraft:snowman_sheared");
    }
    return c;
  }

  static EntityData EnderDragon(CompoundTag const &tag, Context &ctx) {
    auto c = Monster(tag, ctx);
    AddDefinition(c, "-dragon_sitting");
    AddDefinition(c, "+dragon_flying");
    c->set("Persistent", props::Bool(true));
    c->set("IsAutonomous", props::Bool(true));
    c->set("LastDimensionId", props::Int(2));
    ctx.fWorldData.addAutonomousEntity(c);
    return c;
  }

  static EntityData Villager(EntityData const &c, CompoundTag const &tag, Context &ctx) {
    using namespace std;
    using namespace props;

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
      c->set("PreferredProfession", String(profession->string()));
      AddDefinition(c, "+" + profession->string());
      c->set("Variant", Int(profession->variant()));
      c->set("TradeTablePath", String(profession->tradeTablePath()));
    }
    if (type) {
      AddDefinition(c, "+" + type->string() + "_villager");
      c->set("MarkVariant", Int(type->variant()));
    }

    auto age = tag.int32("Age", 0);
    if (age < 0) {
      c->set("Age", Int(age));
      AddDefinition(c, "+baby");
    } else {
      AddDefinition(c, "+adult");
    }
    AddDefinition(c, "+minecraft:villager_v2");

    auto offers = tag.compoundTag("Offers");
    if (offers) {
      auto recipes = offers->listTag("Recipes");
      if (recipes) {
        auto rs = make_shared<ListTag>(Tag::Type::Compound);
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
          int tier = i / 2;
          converted->set("tier", Int(tier));
          rs->push_back(converted);
        }
        if (!rs->empty()) {
          auto of = make_shared<CompoundTag>();
          of->set("Recipes", rs);
          auto expRequirements = make_shared<ListTag>(Tag::Type::Compound);
          auto tier0 = make_shared<CompoundTag>();
          tier0->set("0", Int(0));
          auto tier1 = make_shared<CompoundTag>();
          tier1->set("1", Int(10));
          auto tier2 = make_shared<CompoundTag>();
          tier2->set("2", Int(70));
          auto tier3 = make_shared<CompoundTag>();
          tier3->set("3", Int(150));
          auto tier4 = make_shared<CompoundTag>();
          tier4->set("4", Int(250));
          expRequirements->push_back(tier0);
          expRequirements->push_back(tier1);
          expRequirements->push_back(tier2);
          expRequirements->push_back(tier3);
          expRequirements->push_back(tier4);
          of->set("TierExpRequirements", expRequirements);
          c->set("Offers", of);
        }
      }
    }

    auto tradeExp = tag.int32("Xp", 0);
    c->set("TradeExperience", Int(tradeExp));

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
    c->set("TradeTier", Int(tradeTier));

    return c;
  }

  static std::shared_ptr<CompoundTag> BedrockRecipieFromJava(CompoundTag const &java, Context &ctx) {
    using namespace std;
    using namespace props;

    auto buyA = java.compoundTag("buy");
    auto buyB = java.compoundTag("buyB");
    auto sell = java.compoundTag("sell");

    if (!buyA || !sell) {
      return nullptr;
    }
    auto bedrock = make_shared<CompoundTag>();

    {
      auto count = buyA->byte("Count", 0);
      auto item = Item::From(buyA, ctx.fMapInfo, ctx.fWorldData);
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
      auto item = Item::From(buyB, ctx.fMapInfo, ctx.fWorldData);
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
      auto item = Item::From(sell, ctx.fMapInfo, ctx.fWorldData);
      if (!item) {
        return nullptr;
      }
      bedrock->set("sell", item);
    }

    auto demand = java.int32("demand", 0);
    bedrock->set("demand", Int(demand));

    auto maxUses = java.int32("maxUses", 16);
    bedrock->set("maxUses", Int(maxUses));

    auto priceMultiplierA = java.float32("priceMultiplier", 0.05f);
    bedrock->set("priceMultiplierA", Float(priceMultiplierA));

    //BE: priceMultiplierB
    bedrock->set("priceMultiplierB", Float(0));

    auto rewardExp = java.boolean("rewardExp", true);
    bedrock->set("rewardExp", Bool(rewardExp));

    //JE: specialPrice(int)

    auto xp = java.int32("xp", 1);
    bedrock->set("traderExp", Int(xp));

    auto uses = java.int32("uses", 0);
    bedrock->set("uses", Int(uses));

    return bedrock;
  }

  static EntityData Fox(EntityData const &c, CompoundTag const &tag, Context &) {
    auto type = tag.string("Type", "red");
    int32_t variant = 0;
    if (type == "red") {
      variant = 0;
    } else if (type == "snow") {
      variant = 1;
    }
    c->set("Variant", props::Int(variant));
    return c;
  }

  static EntityData Item(CompoundTag const &tag, Context &ctx) {
    using namespace props;
    auto e = BaseProperties(tag);
    if (!e) {
      return nullptr;
    }

    auto item = tag.compoundTag("Item");
    if (!item) {
      return nullptr;
    }
    auto beItem = Item::From(item, ctx.fMapInfo, ctx.fWorldData);
    if (!beItem) {
      return nullptr;
    }

    auto ret = e->toCompoundTag();
    ret->set("Item", beItem);

    auto thrower = GetUuid(tag, {.fIntArray = "Thrower"});
    int64_t owner = -1;
    if (thrower) {
      owner = UuidRegistrar::ToId(*thrower);
    }
    ret->set("OwnerID", Long(owner));
    ret->set("OwnerNew", Long(owner));

    CopyShortValues(tag, *ret, {{"Health"}, {"Age"}});

    return ret;
  }

  static EntityData Horse(EntityData const &c, CompoundTag const &tag, Context &) {
    auto variant = tag.int32("Variant", 0);
    auto baseColor = 0xf & variant;
    auto markings = 0xf & (variant >> 8);
    c->set("Variant", props::Int(baseColor));
    c->set("MarkVariant", props::Int(markings));
    return c;
  }

  static EntityData StorageMinecart(CompoundTag const &tag, Context &ctx) {
    auto e = BaseProperties(tag);
    if (!e) {
      return nullptr;
    }
    auto c = e->toCompoundTag();

    auto chestItems = std::make_shared<ListTag>(Tag::Type::Compound);

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
        auto converted = Item::From(item, ctx.fMapInfo, ctx.fWorldData);
        if (!converted) {
          continue;
        }
        auto slot = item->byte("Slot");
        if (!slot) {
          continue;
        }
        converted->set("Slot", props::Byte(*slot));
        chestItems->push_back(converted);
      }
    }

    c->set("ChestItems", chestItems);

    return c;
  }

  static EntityData Parrot(EntityData const &c, CompoundTag const &tag, Context &) {
    auto variant = tag.int32("Variant", 0);
    c->set("Variant", props::Int(variant));
    return c;
  }

  static EntityData Minecart(EntityData const &c, CompoundTag const &tag, Context &) {
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
      c->set("Pos", pos->toF().toListTag());
    }
    return c;
  }

  static EntityData HopperMinecart(EntityData const &c, CompoundTag const &tag, Context &) {
    auto enabled = tag.boolean("Enabled", true);
    if (enabled) {
      AddDefinition(c, "+minecraft:hopper_active");
    }
    return c;
  }

  static EntityData TropicalFish(EntityData const &c, CompoundTag const &tag, Context &) {
    using namespace props;
    auto variant = tag.int32("Variant", 0);
    auto tf = TropicalFish::FromJavaVariant(variant);
    c->set("Variant", Int(tf.fSmall ? 0 : 1));
    c->set("MarkVariant", Int(tf.fPattern));
    c->set("Color", Byte(tf.fBodyColor));
    c->set("Color2", Byte(tf.fPatternColor));
    return c;
  }

  static EntityData TraderLlama(EntityData const &c, CompoundTag const &tag, Context &) {
    AddDefinition(c, "+minecraft:llama_wandering_trader");
    AddDefinition(c, "-minecraft:llama_wild");
    AddDefinition(c, "+minecraft:llama_tamed");
    AddDefinition(c, "+minecraft:strength_3");
    c->set("InventoryVersion", props::String("1.16.40"));
    c->set("MarkVariant", props::Int(1));
    c->set("IsTamed", props::Bool(true));
    return c;
  }

  static std::shared_ptr<ListTag> InitItemList(uint32_t capacity) {
    auto items = std::make_shared<ListTag>(Tag::Type::Compound);
    for (int i = 0; i < capacity; i++) {
      auto empty = Item::Empty();
      empty->set("Slot", props::Byte(i));
      empty->set("Count", props::Byte(0));
      items->push_back(empty);
    }
    return items;
  }

  static std::shared_ptr<ListTag> ConvertAnyItemList(std::shared_ptr<ListTag> const &input, uint32_t capacity, JavaEditionMap const &mapInfo, WorldData &wd) {
    auto ret = InitItemList(capacity);
    for (auto const &it : *input) {
      auto item = std::dynamic_pointer_cast<CompoundTag>(it);
      if (!item) {
        continue;
      }
      auto converted = Item::From(item, mapInfo, wd);
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
      converted->set("Slot", props::Byte(*slot));
      converted->set("Count", props::Byte(*count));
      ret->at(*slot) = converted;
    }
    return ret;
  }

  static void AddChestItem(EntityData const &c, std::shared_ptr<CompoundTag> const &item, int8_t slot, int8_t count) {
    using namespace props;
    item->set("Slot", Byte(slot));
    item->set("Count", Byte(count));
    auto chestItems = c->listTag("ChestItems");
    if (!chestItems) {
      chestItems = InitItemList(16);
    }
    chestItems->at(slot) = item;
    c->set("ChestItems", chestItems);
  }

  static EntityData Llama(EntityData const &c, CompoundTag const &tag, Context &) {
    using namespace props;

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
    c->set("Variant", Int(variant));
    AddDefinition(c, "+minecraft:llama_" + color);

    auto armors = c->listTag("Armor");

    auto decorItemId = tag.query("DecorItem/id")->asString();
    if (decorItemId && strings::StartsWith(decorItemId->fValue, "minecraft:") && strings::EndsWith(decorItemId->fValue, "_carpet")) {
      auto carpetColor = strings::Trim("minecraft:", decorItemId->fValue, "_carpet");
      auto colorCode = ColorCodeJavaFromJavaName(carpetColor);
      auto beCarpetColor = BedrockNameFromColorCodeJava(colorCode);
      auto armor = std::make_shared<CompoundTag>();
      armor->insert({{"Count", Byte(1)}, {"Damage", Short(0)}, {"Name", String("minecraft:carpet")}, {"WasPickedUp", Bool(false)}});
      auto block = std::make_shared<CompoundTag>();
      block->insert({{"name", String("minecraft:carpet")}, {"version", Int(kBlockDataVersion)}});
      auto states = std::make_shared<CompoundTag>();
      states->set("color", String(beCarpetColor));
      block->set("states", states);
      armor->set("Block", block);

      if (armors && armors->size() > 1) {
        armors->at(1) = armor;
      }

      AddChestItem(c, armor, 0, 1);
    }

    return c;
  }

  static Behavior ChestedHorse(std::string const &definitionKey) {
    return [=](EntityData const &c, CompoundTag const &tag, Context &ctx) {
      using namespace props;
      auto chested = tag.boolean("ChestedHorse", false);
      c->set("Chested", Bool(chested));
      if (!chested) {
        AddDefinition(c, "+minecraft:" + definitionKey + "_unchested");
        return c;
      }
      auto chestItems = tag.listTag("Items");
      if (!chestItems) {
        AddDefinition(c, "+minecraft:" + definitionKey + "_unchested");
        return c;
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
        auto outItem = Item::From(item, ctx.fMapInfo, ctx.fWorldData);
        if (!outItem) {
          continue;
        }
        AddChestItem(c, outItem, idx, *count);
      }
      AddDefinition(c, "-minecraft:" + definitionKey + "_unchested");
      AddDefinition(c, "+minecraft:" + definitionKey + "_chested");
      return c;
    };
  }

  static EntityData Shulker(EntityData const &c, CompoundTag const &tag, Context &) {
    auto color = tag.byte("Color", 16);
    if (0 <= color && color <= 15) {
      c->set("Color", props::Byte(color));
      AddDefinition(c, "+minecraft:shulker_" + BedrockNameFromColorCodeJava((ColorCodeJava)color));
    } else {
      AddDefinition(c, "+minecraft:shulker_undyed");
    }
    c->set("Variant", props::Int(color));
    return c;
  }

  template <class... Arg>
  static Behavior Definitions(Arg... defs) {
    return [=](EntityData const &c, CompoundTag const &tag, Context &) {
      for (std::string const &def : std::initializer_list<std::string>{defs...}) {
        AddDefinition(c, def);
      }
      return c;
    };
  }

  static Behavior Colorable(std::string const &definitionKey) {
    return [=](EntityData const &c, CompoundTag const &tag, Context &) {
      auto color = tag.byte("Color", 0);
      c->set("Color", props::Byte(color));
      AddDefinition(c, "+minecraft:" + definitionKey + "_" + BedrockNameFromColorCodeJava((ColorCodeJava)color));
      return c;
    };
  }

  static EntityData Rabbit(EntityData const &c, CompoundTag const &tag, Context &) {
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
      c->set("CustomName", props::String("The Killer Bunny"));
    }
    c->set("Variant", props::Int(variant));
    AddDefinition(c, "+coat_" + coat);
    return c;
  }

  static EntityData Debug(EntityData const &c, CompoundTag const &tag, Context &) { return c; }

  static EntityData CollarColorable(EntityData const &c, CompoundTag const &tag, Context &) {
    auto collarColor = tag.byte("CollarColor");
    if (collarColor && GetOwnerUuid(tag)) {
      c->set("Color", props::Byte(*collarColor));
    }
    return c;
  }

  static Behavior Steerable(std::string const &definitionKey) {
    return [=](EntityData const &c, CompoundTag const &tag, Context &) {
      using namespace props;
      auto saddle = tag.boolean("Saddle", false);
      auto saddleItem = tag.compoundTag("SaddleItem");
      if (saddle || saddleItem) {
        AddDefinition(c, "-minecraft:" + definitionKey + "_unsaddled");
        AddDefinition(c, "+minecraft:" + definitionKey + "_saddled");
        if (saddleItem) {
          auto item = std::make_shared<CompoundTag>();
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
      c->set("Saddled", props::Bool(saddle));
      return c;
    };
  }

  static EntityData Mooshroom(EntityData const &c, CompoundTag const &tag, Context &) {
    auto type = tag.string("Type", "red");
    int32_t variant = 0;
    if (type == "brown") {
      variant = 1;
      AddDefinition(c, "+minecraft:mooshroom_brown");
    } else {
      AddDefinition(c, "+minecraft:mooshroom_red");
    }
    c->set("Variant", props::Int(variant));
    return c;
  }

  static EntityData Slime(EntityData const &c, CompoundTag const &tag, Context &) {
    auto sizeJ = tag.int32("Size", 0);
    int sizeB = sizeJ + 1;
    c->set("Variant", props::Int(sizeB));
    c->set("Size", props::Byte(sizeB));

    auto attributes = EntityAttributes::Slime(sizeB);
    c->set("Attributes", attributes.toListTag());

    return c;
  }

  static EntityData EntityBase(CompoundTag const &tag, Context &ctx) {
    auto e = BaseProperties(tag);
    if (!e) {
      return nullptr;
    }
    return e->toCompoundTag();
  }

  static Behavior Vehicle(std::optional<std::string> jockeyDefinitionKey = std::nullopt) {
    return [=](EntityData const &c, CompoundTag const &tag, Context &ctx) {
      auto links = std::make_shared<ListTag>(Tag::Type::Compound);

      auto passengers = tag.query("Passengers")->asList();
      if (passengers) {
        for (int i = 0; i < passengers->size(); i++) {
          auto const &p = passengers->at(i);
          auto comp = p->asCompound();
          if (!comp) {
            continue;
          }

          auto entities = From(*comp, ctx.fMapInfo, ctx.fWorldData);
          if (entities.empty()) {
            continue;
          }

          auto const &passenger = entities[0];
          auto uid = passenger->int64("UniqueID");
          if (!uid) {
            continue;
          }
          auto link = std::make_shared<CompoundTag>();
          link->set("entityID", props::Long(*uid));
          link->set("linkID", props::Int(i));
          links->push_back(link);

          auto passengerId = passenger->string("identifier");
          if (passengerId && *passengerId == "minecraft:zombie") {
            AddDefinition(passenger, "+minecraft:zombie_jockey");
          }

          ctx.fPassengers.push_back(passenger);
        }

        if (jockeyDefinitionKey) {
          AddDefinition(c, "+minecraft:" + *jockeyDefinitionKey + "_parent_jockey");
        }
      }

      c->set("LinksTag", links);
      return c;
    };
  }

  static EntityData Boat(EntityData const &c, CompoundTag const &tag, Context &) {
    AddDefinition(c, "+minecraft:boat");

    auto type = tag.string("Type", "oak");
    int32_t variant = Boat::BedrockVariantFromJavaType(type);
    c->set("Variant", props::Int(variant));

    auto rotation = props::GetRotation(*c, "Rotation");
    if (rotation) {
      Rotation rot(Rotation::ClampDegreesBetweenMinus180And180(rotation->fYaw + 90), rotation->fPitch);
      c->set("Rotation", rot.toListTag());
    }

    auto pos = props::GetPos3d(*c, "Pos");
    auto onGround = c->boolean("OnGround", false);
    if (pos && onGround) {
      int iy = (int)floor(pos->fY);
      pos->fY = iy + 0.35;
      c->set("Pos", pos->toF().toListTag());
    }

    return c;
  }

  static Behavior Rename(std::string const &name) {
    return [=](EntityData const &c, CompoundTag const &tag, Context &) {
      auto id = tag.string("id");
      if (!id) {
        return c;
      }
      RemoveDefinition(c, "+" + *id);
      AddDefinition(c, "+minecraft:" + name);
      c->set("identifier", props::String("minecraft:" + name));
      return c;
    };
  }

  static EntityData Creeper(EntityData const &c, CompoundTag const &tag, Context &) {
    using namespace props;
    auto powered = tag.boolean("powered", false);
    if (powered) {
      AddDefinition(c, "+minecraft:charged_creeper");
      AddDefinition(c, "+minecraft:exploding");
    }
    return c;
  }

  static EntityData Monster(CompoundTag const &tag, Context &ctx) {
    auto c = Mob(tag, ctx);
    c->set("SpawnedByNight", props::Bool(false));
    auto persistenceRequired = tag.boolean("PersistenceRequired", true);
    bool persistent = false;
    if (persistenceRequired && tag.string("CustomName")) {
      persistent = true;
    }
    c->set("Persistent", props::Bool(persistent));
    return c;
  }

  static EntityData Animal(CompoundTag const &tag, Context &ctx) {
    auto c = Mob(tag, ctx);
    if (!c) {
      return nullptr;
    }
    c->set("Persistent", props::Bool(true));

    auto leash = tag.compoundTag("Leash");
    if (leash) {
      auto x = leash->int32("X");
      auto y = leash->int32("Y");
      auto z = leash->int32("Z");
      auto uuid = GetEntityUuid(tag);
      if (x && y && z && uuid) {
        int64_t leasherId = UuidRegistrar::LeasherIdFor(*uuid);

        Entity e(leasherId);
        e.fPos = Pos3f(*x + 0.5f, *y + 0.25f, *z + 0.5f);
        e.fIdentifier = "minecraft:leash_knot";
        auto leashEntityData = e.toCompoundTag();
        ctx.fPassengers.push_back(leashEntityData);

        c->set("LeasherID", props::Long(leasherId));
      }
    }

    return c;
  }

  static EntityData Bat(EntityData const &c, CompoundTag const &tag, Context &) {
    auto batFlags = tag.boolean("BatFlags", false);
    c->set("BatFlags", props::Bool(batFlags));
    AddDefinition(c, "+minecraft:bat");
    return c;
  }

  static std::optional<int64_t> GetOwnerUuid(CompoundTag const &tag) {
    auto uuid = props::GetUuid(tag, {.fIntArray = "Owner", .fHexString = "OwnerUUID"});
    if (!uuid) {
      return std::nullopt;
    }
    return UuidRegistrar::ToId(*uuid);
  }

  static EntityData Cat(EntityData const &c, CompoundTag const &tag, Context &) {
    using namespace std;
    auto catType = tag.int32("CatType");
    if (catType) {
      int32_t variant = 0;
      std::string type;
      switch (*catType) {
      case 0:
        type = "tabby";
        variant = 8;
        break;
      case 1:
        type = "tuxedo";
        variant = 1;
        break;
      case 2:
        type = "red";
        variant = 2;
        break;
      case 3:
        type = "siamese";
        variant = 3;
        break;
      case 4:
        type = "british";
        variant = 4;
        break;
      case 5:
        type = "calico";
        variant = 5;
        break;
      case 6:
        type = "persian";
        variant = 6;
        break;
      case 7:
        type = "ragdoll";
        variant = 7;
        break;
      case 8:
        type = "white";
        variant = 0;
        break;
      case 9:
        type = "jellie";
        variant = 10;
        break;
      case 10:
        type = "black";
        variant = 9;
        break;
      }
      if (!type.empty()) {
        AddDefinition(c, "+minecraft:cat_" + type);
      }
      c->set("Variant", props::Int(variant));
    }
    c->set("DwellingUniqueID", props::String("00000000-0000-0000-0000-000000000000"));
    c->set("RewardPlayersOnFirstFounding", props::Bool(true));
    return c;
  }

  static EntityData Chicken(EntityData const &c, CompoundTag const &tag, Context &) {
    using namespace std;
    auto eggLayTime = tag.int32("EggLayTime");
    if (eggLayTime) {
      auto entries = make_shared<ListTag>(Tag::Type::Compound);
      auto timer = make_shared<CompoundTag>();
      timer->set("SpawnTimer", props::Int(*eggLayTime));
      timer->set("StopSpawning", props::Bool(false));
      entries->push_back(timer);
      c->set("entries", entries);
    }
    return c;
  }

  static Behavior AgeableA(std::string const &definitionKey) {
    return [=](EntityData const &c, CompoundTag const &tag, Context &) {
      auto age = tag.int32("Age", 0);
      if (age < 0) {
        AddDefinition(c, "+minecraft:" + definitionKey + "_baby");
        c->set("Age", props::Int(age));
      } else {
        AddDefinition(c, "+minecraft:" + definitionKey + "_adult");
        c->erase("Age");
      }
      c->set("IsBaby", props::Bool(age < 0));
      return c;
    };
  }

  static Behavior AgeableB(std::string const &definitionKey) {
    return [=](EntityData const &c, CompoundTag const &tag, Context &) {
      auto baby = tag.boolean("IsBaby", false);
      if (baby) {
        AddDefinition(c, "+minecraft:" + definitionKey + "_baby");
      } else {
        AddDefinition(c, "+minecraft:" + definitionKey + "_adult");
      }
      c->set("IsBaby", props::Bool(baby));
      return c;
    };
  }

  static EntityData AgeableC(EntityData const &c, CompoundTag const &tag, Context &) {
    auto age = tag.int32("Age", 0);
    if (age < 0) {
      AddDefinition(c, "+baby");
      c->set("Age", props::Int(age));
    } else {
      AddDefinition(c, "+adult");
      c->erase("Age");
    }
    c->set("IsBaby", props::Bool(age < 0));
    return c;
  }

  static Behavior TameableA(std::string const &definitionKey) {
    return [=](EntityData const &c, CompoundTag const &tag, Context &) {
      auto owner = GetOwnerUuid(tag);
      if (owner) {
        c->set("OwnerNew", props::Long(*owner));
        AddDefinition(c, "-minecraft:" + definitionKey + "_wild");
        AddDefinition(c, "+minecraft:" + definitionKey + "_tame");
        c->set("IsTamed", props::Bool(true));
      } else {
        AddDefinition(c, "+minecraft:" + definitionKey + "_wild");
      }
      return c;
    };
  }

  static Behavior TameableB(std::string const &definitionKey) {
    return [=](EntityData const &c, CompoundTag const &tag, Context &) {
      auto owner = GetOwnerUuid(tag);
      if (owner) {
        c->set("OwnerNew", props::Long(*owner));
        AddDefinition(c, "-minecraft:" + definitionKey + "_wild");
        AddDefinition(c, "+minecraft:" + definitionKey + "_tamed");
        c->set("IsTamed", props::Bool(true));
      } else {
        AddDefinition(c, "+minecraft:" + definitionKey + "_wild");
      }
      return c;
    };
  }

  static void AddDefinition(EntityData const &tag, std::string const &definition) {
    auto found = tag->find("definitions");
    auto d = std::make_shared<ListTag>(Tag::Type::String);
    if (found != tag->end()) {
      auto current = found->second->asList();
      if (current && current->fType == Tag::Type::String) {
        for (auto c : *current) {
          if (c->asString()) {
            d->push_back(props::String(c->asString()->fValue));
          }
        }
      }
    }
    d->push_back(props::String(definition));
    tag->set("definitions", d);
  }

  static void RemoveDefinition(EntityData const &tag, std::string const &definition) {
    auto found = tag->find("definitions");
    auto d = std::make_shared<ListTag>(Tag::Type::String);
    if (found != tag->end()) {
      auto current = found->second->asList();
      if (current && current->fType == Tag::Type::String) {
        for (auto c : *current) {
          if (c->asString() && c->asString()->fValue != definition) {
            d->push_back(props::String(c->asString()->fValue));
          }
        }
      }
    }
    tag->set("definitions", d);
  }

  static EntityData Sittable(EntityData const &c, CompoundTag const &tag, Context &) {
    auto sitting = tag.boolean("Sitting", false);
    c->set("Sitting", props::Bool(sitting));
    return c;
  }

  static EntityData LivingEntity(CompoundTag const &tag, Context &ctx) {
    using namespace props;
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
    c["HurtTime"] = Short(0);
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
      AddDefinition(ret, "+" + e->fIdentifier);
    }
    ret->erase("Motion");
    ret->erase("Dir");
    return ret;
  }

  static EntityData Mob(CompoundTag const &tag, Context &ctx) {
    using namespace props;
    auto ret = LivingEntity(tag, ctx);
    if (!ret) {
      return ret;
    }

    auto id = tag.string("id");
    if (id) {
      if (*id == "minecraft:horse" || *id == "minecraft:donkey" || *id == "minecraft:mule" || *id == "minecraft:skeleton_horse" || *id == "minecraft:zombie_horse") {
        auto attributes = EntityAttributes::AnyHorse(tag);
        if (attributes) {
          ret->set("Attributes", attributes);
        }
      } else {
        auto attributes = EntityAttributes::Mob(*id);
        if (attributes) {
          ret->set("Attributes", attributes->toListTag());
        }
      }
    }

    return ret;
  }

  static std::shared_ptr<ListTag> GetArmor(CompoundTag const &tag, Context &ctx) {
    auto armors = std::make_shared<ListTag>(Tag::Type::Compound);
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
          auto converted = Item::From(item, ctx.fMapInfo, ctx.fWorldData);
          if (converted) {
            armors->fValue[i] = converted;
          }
        }
      }
    }

    auto ret = std::make_shared<ListTag>(Tag::Type::Compound);
    ret->push_back(armors->at(3));
    ret->push_back(armors->at(2));
    ret->push_back(armors->at(1));
    ret->push_back(armors->at(0));

    return ret;
  }

  static std::shared_ptr<ListTag> GetMainhand(CompoundTag const &input, Context &ctx) { return HandItem<0>(input, ctx); }

  static std::shared_ptr<ListTag> GetOffhand(CompoundTag const &input, Context &ctx) { return HandItem<1>(input, ctx); }

  template <size_t index>
  static std::shared_ptr<ListTag> HandItem(CompoundTag const &input, Context &ctx) {
    auto ret = std::make_shared<ListTag>(Tag::Type::Compound);

    auto mainHand = input.listTag("HandItems");
    std::shared_ptr<CompoundTag> item;

    if (mainHand && mainHand->fType == Tag::Type::Compound && index < mainHand->fValue.size()) {
      auto inItem = std::dynamic_pointer_cast<CompoundTag>(mainHand->fValue[index]);
      if (inItem) {
        item = Item::From(inItem, ctx.fMapInfo, ctx.fWorldData);
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
    using namespace props;
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

  static EntityData Default(CompoundTag const &tag) {
    auto e = BaseProperties(tag);
    if (!e) {
      return nullptr;
    }
    return e->toCompoundTag();
  }

  static EntityData EndCrystal(CompoundTag const &tag, Context &ctx) {
    auto e = BaseProperties(tag);
    if (!e) {
      return nullptr;
    }
    e->fIdentifier = "minecraft:ender_crystal";
    auto c = e->toCompoundTag();
    CopyBoolValues(tag, *c, {{"ShowBottom"}});
    return c;
  }

  static EntityData Painting(CompoundTag const &tag, Context &ctx) {
    using namespace props;
    using namespace std;

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

public:
  std::vector<std::string> fDefinitions;
  Pos3f fMotion;
  Pos3f fPos;
  Rotation fRotation;
  std::vector<std::shared_ptr<CompoundTag>> fTags;
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
