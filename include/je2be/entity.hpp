#pragma once

namespace j2b {

class Entity {
private:
  using CompoundTag = mcfile::nbt::CompoundTag;
  using EntityData = std::shared_ptr<mcfile::nbt::CompoundTag>;

  struct Context {
    Context(JavaEditionMap const &mapInfo, DimensionDataFragment &ddf) : fMapInfo(mapInfo), fDimensionData(ddf) {}

    std::vector<EntityData> fPassengers;
    JavaEditionMap const &fMapInfo;
    DimensionDataFragment &fDimensionData;
  };

  using Converter = std::function<EntityData(CompoundTag const &, Context &)>;

  using Behavior = std::function<EntityData(EntityData const &, CompoundTag const &, Context &)>;

  struct Convert {
    template <class... Arg>
    Convert(Converter base, Arg... args) : fBase(base), fBehaviors(std::initializer_list<Behavior>{args...}) {}

    EntityData operator()(CompoundTag const &input, Context &ctx) const {
      auto c = fBase(input, ctx);
      if (!c)
        return nullptr;
      for (auto const &b : fBehaviors) {
        auto next = b(c, input, ctx);
        if (!next)
          continue;
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

  static std::vector<EntityData> From(mcfile::nbt::CompoundTag const &tag, JavaEditionMap const &mapInfo, DimensionDataFragment &ddf) {
    using namespace props;
    auto id = tag.string("id");
    if (!id)
      return std::vector<EntityData>();
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
    Context ctx(mapInfo, ddf);
    auto converted = found->second(tag, ctx);
    if (!ctx.fPassengers.empty()) {
      std::copy(ctx.fPassengers.begin(), ctx.fPassengers.end(), std::back_inserter(ret));
    }
    if (converted) {
      ret.push_back(converted);
    }
    return ret;
  }

  static bool DegAlmostEquals(float a, float b) {
    a = fmod(fmod(a, 360) + 360, 360);
    assert(0 <= a && a < 360);
    b = fmod(fmod(b, 360) + 360, 360);
    assert(0 <= b && b < 360);
    float diff = fmod(fmod(a - b, 360) + 360, 360);
    assert(0 <= diff && diff < 360);
    float const tolerance = 1;
    if (0 <= diff && diff < tolerance) {
      return true;
    } else if (360 - tolerance < diff) {
      return true;
    } else {
      return false;
    }
  }

  static bool RotAlmostEquals(Rotation const &rot, float yaw, float pitch) { return DegAlmostEquals(rot.fYaw, yaw) && DegAlmostEquals(rot.fPitch, pitch); }

  static std::optional<std::tuple<Pos, std::shared_ptr<CompoundTag>, std::string>> ToTileEntityBlock(CompoundTag const &c) {
    using namespace std;
    using namespace props;
    auto id = c.string("id");
    assert(id);
    if (*id == "minecraft:item_frame") {
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
      string key = "minecraft:frame[facing_direction=" + to_string(facing) + ",item_frame_map_bit=" + (map ? "true" : "false") + "]";
      b->insert({
          {"name", String("minecraft:frame")},
          {"version", Int(BlockData::kBlockDataVersion)},
          {"states", states},
      });
      Pos pos(*tileX, *tileY, *tileZ);
      return make_tuple(pos, b, key);
    }

    return nullopt;
  }

  static std::shared_ptr<mcfile::nbt::CompoundTag> ToTileEntityData(CompoundTag const &c, JavaEditionMap const &mapInfo, DimensionDataFragment &ddf) {
    using namespace props;
    auto id = c.string("id");
    assert(id);
    if (*id == "minecraft:item_frame") {
      auto tag = std::make_shared<CompoundTag>();
      auto tileX = c.int32("TileX");
      auto tileY = c.int32("TileY");
      auto tileZ = c.int32("TileZ");
      if (!tileX || !tileY || !tileZ)
        return nullptr;
      tag->insert({
          {"id", String("ItemFrame")},
          {"isMovable", Bool(true)},
          {"x", Int(*tileX)},
          {"y", Int(*tileY)},
          {"z", Int(*tileZ)},
      });
      auto itemRotation = c.byte("ItemRotation", 0);
      auto itemDropChange = c.float32("ItemDropChange", 1);
      auto found = c.find("Item");
      if (found != c.end() && found->second->id() == mcfile::nbt::Tag::TAG_Compound) {
        auto item = std::dynamic_pointer_cast<CompoundTag>(found->second);
        auto m = Item::From(item, mapInfo, ddf);
        if (m) {
          tag->insert(make_pair("Item", m));
          tag->insert(make_pair("ItemRotation", Float(itemRotation * 45)));
          tag->insert(make_pair("ItemDropChange", Float(itemDropChange)));
        }
      }
      return tag;
    }

    return nullptr;
  }

  static bool IsTileEntity(CompoundTag const &tag) {
    auto id = tag.string("id");
    if (!id)
      return false;
    return *id == "minecraft:item_frame";
  }

  std::shared_ptr<mcfile::nbt::CompoundTag> toCompoundTag() const {
    using namespace std;
    using namespace props;
    using namespace mcfile::nbt;
    auto tag = make_shared<CompoundTag>();
    auto tags = make_shared<ListTag>();
    tags->fType = Tag::TAG_Compound;
    auto definitions = make_shared<ListTag>();
    definitions->fType = Tag::TAG_String;
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

  static EntityData LocalPlayer(CompoundTag const &tag, JavaEditionMap const &mapInfo, DimensionDataFragment &ddf) {
    using namespace mcfile::nbt;
    using namespace props;

    Context ctx(mapInfo, ddf);
    auto entity = LivingEntity(tag, ctx);
    if (!entity) {
      return nullptr;
    }

    entity->set("format_version", String("1.12.0"));
    entity->set("identifier", String("minecraft:player"));

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
      auto outInventory = ConvertAnyItemList(inventory, 36, mapInfo, ddf);
      entity->set("Inventory", outInventory);

      // Armor
      auto armor = InitItemList(4);
      auto boots = ItemAtSlot(*inventory, 100);
      if (boots) {
        auto outBoots = Item::From(boots, mapInfo, ddf);
        if (outBoots) {
          armor->at(3) = outBoots;
        }
      }
      auto leggings = ItemAtSlot(*inventory, 101);
      if (leggings) {
        auto outLeggings = Item::From(leggings, mapInfo, ddf);
        if (outLeggings) {
          armor->at(2) = outLeggings;
        }
      }
      auto chestplate = ItemAtSlot(*inventory, 102);
      if (chestplate) {
        auto outChestplate = Item::From(chestplate, mapInfo, ddf);
        if (outChestplate) {
          armor->at(1) = outChestplate;
        }
      }
      auto helmet = ItemAtSlot(*inventory, 103);
      if (helmet) {
        auto outHelmet = Item::From(helmet, mapInfo, ddf);
        if (outHelmet) {
          armor->at(0) = outHelmet;
        }
      }
      entity->set("Armor", armor);

      // Offhand
      auto offhand = ItemAtSlot(*inventory, -106);
      if (offhand) {
        auto offhandItem = Item::From(offhand, mapInfo, ddf);
        if (offhandItem) {
          auto outOffhand = InitItemList(1);
          outOffhand->at(0) = offhandItem;
          entity->set("Offhand", outOffhand);
        }
      }
    }

    auto enderItems = tag.listTag("EnderItems");
    if (enderItems) {
      auto enderChestInventory = ConvertAnyItemList(enderItems, 27, mapInfo, ddf);
      entity->set("EnderChestInventory", enderChestInventory);
    }

    return entity;
  }

private:
  static std::shared_ptr<CompoundTag> ItemAtSlot(mcfile::nbt::ListTag const &items, uint32_t slot) {
    for (auto const &it : items) {
      auto item = std::dynamic_pointer_cast<CompoundTag>(it);
      if (!item)
        continue;
      auto s = item->byte("Slot");
      if (!s)
        continue;
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
    E("chicken", Convert(Animal, AgeableA("chicken"), Vehicle()));
    A("cod");

    E("cow", Convert(Animal, AgeableA("cow")));
    E("creeper", Convert(Monster, Creeper));
    A("dolphin");
    E("donkey", Convert(Animal, TameableB("donkey"), ChestedHorse("donkey"), Steerable("donkey")));
    M("drowned");
    M("elder_guardian");
    M("enderman");
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
    A("salmon");
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
    E("hopper_minecart", Convert(StorageMinecart, Minecart, Definitions("+minecraft:hopper_minecart")));
    E("chest_minecart", Convert(StorageMinecart, Minecart, Definitions("+minecraft:chest_minecart")));
    E("tnt_minecart", Convert(EntityBase, Vehicle(), Minecart, Definitions("+minecraft:tnt_minecart", "+minecraft:inactive")));
    E("snow_golem", Convert(Mob, SnowGolem));
    E("iron_golem", Convert(Mob, Definitions("+minecraft:iron_golem"), IronGolem));

    E("item", Item);
    E("ender_dragon", EnderDragon);
    E("experience_orb", Convert(LivingEntity, Rename("xp_orb"), ExperienceOrb));
    E("item_frame", Null); // item_frame is tile entity in BE.
#undef A
#undef M
#undef E
    return table;
  }

  static EntityData Null(CompoundTag const &tag, Context &ctx) { return nullptr; }

  static EntityData IronGolem(EntityData const &c, CompoundTag const &tag, Context &) {
    using namespace props;

    auto playerCreated = tag.boolean("PlayerCreated", false);
    auto angryAt = GetUUID(tag, {.fIntArray = "AngryAt"});
    auto angerTime = tag.int32("AngerTime", 0);

    if (playerCreated) {
      AddDefinition(c, "+minecraft:player_created");
    } else {
      AddDefinition(c, "+minecraft:village_created");
    }
    c->set("IsAngry", Bool(angerTime > 0));
    if (angryAt) {
      c->set("TargetID", Long(*angryAt));
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
    auto pos = props::GetVec(tag, "Pos");
    if (!pos) {
      return c;
    }
    pos->fY = std::round(pos->fY * 2) * 0.5;
    c->set("Pos", pos->toListTag());
    return c;
  }

  static EntityData Bee(EntityData const &c, CompoundTag const &tag, Context &) {
    auto hivePos = props::GetPos(tag, "HivePos");
    if (hivePos) {
      Vec homePos(hivePos->fX, hivePos->fY, hivePos->fZ);
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
    ctx.fDimensionData.addAutonomousEntity(c);
    return c;
  }

  static EntityData Villager(EntityData const &c, CompoundTag const &tag, Context &) {
    using namespace std;
    using namespace props;
    auto data = tag.compoundTag("VillagerData");
    if (data) {
      auto inProfession = data->string("profession");
      auto inType = data->string("type");
      auto level = data->int32("level", 1);
      if (inProfession && inType) {
        auto profession = strings::LTrim(*inProfession, "minecraft:");
        int32_t variant = 0;
        if (profession == "shepherd") {
          variant = 3;
        } else if (profession == "farmer") {
          variant = 1;
        } else if (profession == "fisherman") {
          variant = 2;
        } else if (profession == "butcher") {
          variant = 11;
        } else if (profession == "armorer") {
          variant = 8;
        } else if (profession == "cartographer") {
          variant = 6;
        } else if (profession == "fletcher") {
          variant = 4;
        } else if (profession == "weaponsmith") {
          variant = 9;
        } else if (profession == "toolsmith") {
          variant = 10;
        } else if (profession == "mason") {
          variant = 13;
        } else if (profession == "leatherworker") {
          variant = 12;
        } else if (profession == "cleric") {
          variant = 7;
        } else if (profession == "librarian") {
          variant = 5;
        }
        c->set("PreferredProfession", String(profession));
        AddDefinition(c, "+" + profession);

        auto type = strings::LTrim(*inType, "minecraft:");
        int32_t mark = 0;
        if (type == "savanna") {
          mark = 3;
        } else if (type == "plains") {
          mark = 0;
        } else if (type == "desert") {
          mark = 1;
        } else if (type == "jungle") {
          mark = 2;
        } else if (type == "snow") {
          mark = 4;
        } else if (type == "swamp") {
          mark = 5;
        } else if (type == "taiga") {
          mark = 6;
        }
        AddDefinition(c, "+" + type + "_villager");

        c->set("Variant", Int(variant));
        c->set("MarkVariant", Int(mark));
        c->set("TradeTier", Int(level - 1));
      }
    }
    auto age = tag.int32("Age", 0);
    if (age < 0) {
      c->set("Age", Int(age));
      AddDefinition(c, "+baby");
    } else {
      AddDefinition(c, "+adult");
    }
    AddDefinition(c, "+minecraft:villager_v2");
    return c;
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
    if (!e)
      return nullptr;

    auto item = tag.compoundTag("Item");
    if (!item)
      return nullptr;
    auto beItem = Item::From(item, ctx.fMapInfo, ctx.fDimensionData);
    if (!beItem)
      return nullptr;

    auto ret = e->toCompoundTag();
    ret->set("Item", beItem);

    auto thrower = GetUUID(tag, {.fIntArray = "Thrower"});
    int64_t owner = -1;
    if (thrower) {
      owner = *thrower;
    }
    ret->set("OwnerID", Long(owner));
    ret->set("OwnerNew", Long(owner));

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
    using namespace mcfile::nbt;

    auto e = BaseProperties(tag);
    if (!e)
      return nullptr;
    auto c = e->toCompoundTag();

    auto chestItems = std::make_shared<ListTag>();
    chestItems->fType = Tag::TAG_Compound;

    auto items = tag.listTag("Items");
    if (items) {
      for (auto const &it : *items) {
        if (it->id() != Tag::TAG_Compound)
          continue;
        auto item = std::dynamic_pointer_cast<CompoundTag>(it);
        if (!item)
          continue;
        auto converted = Item::From(item, ctx.fMapInfo, ctx.fDimensionData);
        if (!converted)
          continue;
        auto slot = item->byte("Slot");
        if (!slot)
          continue;
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
    auto pos = props::GetVec(tag, "Pos");
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
      c->set("Pos", pos->toListTag());
    }
    return c;
  }

  static EntityData TropicalFish(EntityData const &c, CompoundTag const &tag, Context &) {
    using namespace props;
    auto variant = tag.int32("Variant", 0);
    auto tf = j2b::TropicalFish::FromVariant(variant);
    c->set("Variant", Int(tf.fSmall ? 0 : 1));
    c->set("MarkVariant", Int(tf.fPattern));
    c->set("Color", Byte(tf.fBodyColor));
    c->set("Color2", Byte(tf.fPatternColor));
    return c;
  }

  static EntityData TraderLlama(EntityData const &c, CompoundTag const &tag, Context &) {
    using namespace mcfile::nbt;
    AddDefinition(c, "+minecraft:llama_wandering_trader");
    AddDefinition(c, "-minecraft:llama_wild");
    AddDefinition(c, "+minecraft:llama_tamed");
    AddDefinition(c, "+minecraft:strength_3");
    c->set("InventoryVersion", props::String("1.16.40"));
    c->set("MarkVariant", props::Int(1));
    c->set("IsTamed", props::Bool(true));
    return c;
  }

  static std::shared_ptr<mcfile::nbt::ListTag> InitItemList(uint32_t capacity) {
    using namespace mcfile::nbt;
    auto items = std::make_shared<ListTag>();
    items->fType = Tag::TAG_Compound;
    for (int i = 0; i < capacity; i++) {
      auto empty = Item::Empty();
      empty->set("Slot", props::Byte(i));
      empty->set("Count", props::Byte(0));
      items->push_back(empty);
    }
    return items;
  }

  static std::shared_ptr<mcfile::nbt::ListTag> ConvertAnyItemList(std::shared_ptr<mcfile::nbt::ListTag> const &input, uint32_t capacity, JavaEditionMap const &mapInfo, DimensionDataFragment &ddf) {
    auto ret = InitItemList(capacity);
    for (auto const &it : *input) {
      auto item = std::dynamic_pointer_cast<CompoundTag>(it);
      if (!item)
        continue;
      auto converted = Item::From(item, mapInfo, ddf);
      if (!converted)
        continue;
      auto slot = item->byte("Slot");
      if (!slot)
        continue;
      if (*slot < 0 || capacity <= *slot)
        continue;
      auto count = item->byte("Count");
      if (!count)
        continue;
      converted->set("Slot", props::Byte(*slot));
      converted->set("Count", props::Byte(*count));
      ret->at(*slot) = converted;
    }
    return ret;
  }

  static void AddChestItem(EntityData const &c, std::shared_ptr<CompoundTag> const &item, int8_t slot, int8_t count) {
    using namespace props;
    using namespace mcfile::nbt;
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
    using namespace mcfile::nbt;

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
      auto colorCode = ColorCodeJavaFromName(carpetColor);
      auto beCarpetColor = BedrockNameFromColorCodeJava(colorCode);
      auto armor = std::make_shared<CompoundTag>();
      armor->insert({{"Count", Byte(1)}, {"Damage", Short(0)}, {"Name", String("minecraft:carpet")}, {"WasPickedUp", Bool(false)}});
      auto block = std::make_shared<CompoundTag>();
      block->insert({{"name", String("minecraft:carpet")}, {"version", Int(BlockData::kBlockDataVersion)}});
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
      using namespace mcfile::nbt;
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
        if (!item)
          continue;
        auto slot = item->byte("Slot");
        if (!slot)
          continue;
        int8_t idx = *slot - 1;
        if (idx < 0 || 16 <= idx)
          continue;
        auto count = item->byte("Count");
        if (!count)
          continue;
        auto outItem = Item::From(item, ctx.fMapInfo, ctx.fDimensionData);
        if (!outItem)
          continue;
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
    if (collarColor && GetOwnerUUID(tag)) {
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
    auto size = tag.int32("Size");
    int8_t variant = 0;
    if (size) {
      variant = int8_t(*size) + 1;
    }
    c->set("Variant", props::Int(variant));
    c->set("Size", props::Byte(variant));
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
      using namespace mcfile::nbt;

      auto links = std::make_shared<ListTag>();
      links->fType = Tag::TAG_Compound;

      auto passengers = tag.query("Passengers")->asList();
      if (passengers) {
        for (int i = 0; i < passengers->size(); i++) {
          auto const &p = passengers->at(i);
          auto comp = p->asCompound();
          if (!comp)
            continue;

          auto entities = From(*comp, ctx.fMapInfo, ctx.fDimensionData);
          if (entities.empty())
            continue;

          auto const &passenger = entities[0];
          auto uid = passenger->int64("UniqueID");
          if (!uid)
            continue;
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
    using namespace mcfile::nbt;

    AddDefinition(c, "+minecraft:boat");

    auto type = tag.string("Type", "oak");
    int32_t variant = 0;
    if (type == "oak") {
      variant = 0;
    } else if (type == "spruce") {
      variant = 1;
    } else if (type == "birch") {
      variant = 2;
    } else if (type == "jungle") {
      variant = 3;
    } else if (type == "acacia") {
      variant = 4;
    } else if (type == "dark_oak") {
      variant = 5;
    }
    c->set("Variant", props::Int(variant));

    auto rotation = props::GetRotation(*c, "Rotation");
    if (rotation) {
      Rotation rot(rotation->fYaw + 90, rotation->fPitch);
      c->set("Rotation", rot.toListTag());
    }

    auto pos = props::GetVec(*c, "Pos");
    auto onGround = c->boolean("OnGround", false);
    if (pos && onGround) {
      int iy = (int)floor(pos->fY);
      pos->fY = iy + 0.35f;
      c->set("Pos", pos->toListTag());
    }

    return c;
  }

  static Behavior Rename(std::string const &name) {
    return [=](EntityData const &c, CompoundTag const &tag, Context &) {
      auto id = tag.string("id");
      if (!id)
        return c;
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
      auto uuid = GetEntityUUID(tag);
      if (x && y && z && uuid) {
        int64_t targetUUID = *uuid;
        int64_t leasherId = XXHash::Digest(&targetUUID, sizeof(targetUUID));

        Entity e(leasherId);
        e.fPos = Vec(*x + 0.5f, *y + 0.25f, *z + 0.5f);
        e.fIdentifier = "minecraft:leash_knot";
        auto leashEntityData = e.toCompoundTag();
        ctx.fPassengers.push_back(leashEntityData);

        c->set("LeasherID", props::Long(leasherId));
      }
    }

    return c;
  }

  static EntityData Bat(EntityData const &c, CompoundTag const &tag, Context &) {
    using namespace mcfile::nbt;
    auto batFlags = tag.boolean("BatFlags", false);
    c->set("BatFlags", props::Bool(batFlags));
    AddDefinition(c, "+minecraft:bat");
    return c;
  }

  static std::optional<int64_t> GetOwnerUUID(CompoundTag const &tag) { return props::GetUUID(tag, {.fIntArray = "Owner", .fHexString = "OwnerUUID"}); }

  static EntityData Cat(EntityData const &c, CompoundTag const &tag, Context &) {
    using namespace mcfile::nbt;
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
      auto owner = GetOwnerUUID(tag);
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
      auto owner = GetOwnerUUID(tag);
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

  static void AddDefinition(EntityData const &c, std::string const &definition) {
    using namespace mcfile::nbt;

    auto found = c->find("definitions");
    auto d = std::make_shared<ListTag>();
    d->fType = Tag::TAG_String;
    if (found != c->end()) {
      auto current = found->second->asList();
      if (current && current->fType == Tag::TAG_String) {
        for (auto c : *current) {
          if (c->asString()) {
            d->push_back(props::String(c->asString()->fValue));
          }
        }
      }
    }
    d->push_back(props::String(definition));
    c->set("definitions", d);
  }

  static void RemoveDefinition(EntityData const &c, std::string const &definition) {
    using namespace mcfile::nbt;

    auto found = c->find("definitions");
    auto d = std::make_shared<ListTag>();
    d->fType = Tag::TAG_String;
    if (found != c->end()) {
      auto current = found->second->asList();
      if (current && current->fType == Tag::TAG_String) {
        for (auto c : *current) {
          if (c->asString() && c->asString()->fValue != definition) {
            d->push_back(props::String(c->asString()->fValue));
          }
        }
      }
    }
    c->set("definitions", d);
  }

  static EntityData Sittable(EntityData const &c, CompoundTag const &tag, Context &) {
    auto sitting = tag.boolean("Sitting", false);
    c->set("Sitting", props::Bool(sitting));
    return c;
  }

  static EntityData LivingEntity(CompoundTag const &tag, Context &ctx) {
    using namespace props;
    auto e = BaseProperties(tag);
    if (!e)
      return nullptr;
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
    AddDefinition(ret, "+" + e->fIdentifier);
    ret->erase("Motion");
    ret->erase("Dir");
    return ret;
  }

  static EntityData Mob(CompoundTag const &tag, Context &ctx) {
    using namespace props;
    auto ret = LivingEntity(tag, ctx);
    if (!ret)
      return ret;

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
          ret->set("Attributes", attributes);
        }
      }
    }

    return ret;
  }

  static std::shared_ptr<mcfile::nbt::ListTag> GetArmor(CompoundTag const &tag, Context &ctx) {
    using namespace mcfile::nbt;
    auto armors = std::make_shared<ListTag>();
    armors->fType = Tag::TAG_Compound;

    auto found = tag.find("ArmorItems");
    if (found != tag.end()) {
      auto list = found->second->asList();
      if (list && list->fType == Tag::TAG_Compound) {
        for (int i = 0; i < 4 && i < list->size(); i++) {
          auto item = std::dynamic_pointer_cast<CompoundTag>(list->at(i));
          if (item) {
            auto converted = Item::From(item, ctx.fMapInfo, ctx.fDimensionData);
            armors->push_back(converted);
          } else {
            armors->push_back(Item::Empty());
          }
        }
      }
    }
    for (int i = 0; i < 4; i++) {
      if (armors->size() < i + 1) {
        armors->push_back(Item::Empty());
      } else {
        if (!armors->at(i)) {
          armors->at(i) = Item::Empty();
        }
      }
    }

    auto ret = std::make_shared<ListTag>();
    ret->fType = Tag::TAG_Compound;
    ret->push_back(armors->at(3));
    ret->push_back(armors->at(2));
    ret->push_back(armors->at(1));
    ret->push_back(armors->at(0));

    return ret;
  }

  static std::shared_ptr<mcfile::nbt::ListTag> GetMainhand(CompoundTag const &input, Context &ctx) { return HandItem<0>(input, ctx); }

  static std::shared_ptr<mcfile::nbt::ListTag> GetOffhand(CompoundTag const &input, Context &ctx) { return HandItem<1>(input, ctx); }

  template <size_t index>
  static std::shared_ptr<mcfile::nbt::ListTag> HandItem(CompoundTag const &input, Context &ctx) {
    using namespace mcfile::nbt;
    auto ret = std::make_shared<ListTag>();
    ret->fType = Tag::TAG_Compound;

    auto mainHand = input.listTag("HandItems");
    std::shared_ptr<CompoundTag> item;

    if (mainHand && mainHand->fType == Tag::TAG_Compound && index < mainHand->fValue.size()) {
      auto inItem = std::dynamic_pointer_cast<CompoundTag>(mainHand->fValue[index]);
      if (inItem) {
        item = Item::From(inItem, ctx.fMapInfo, ctx.fDimensionData);
      }
    }
    if (!item) {
      item = Item::Empty();
    }
    ret->fValue.push_back(item);

    return ret;
  }

  static std::optional<int64_t> GetEntityUUID(CompoundTag const &tag) { return props::GetUUID(tag, {.fLeastAndMostPrefix = "UUID", .fIntArray = "UUID"}); }

  static std::optional<Entity> BaseProperties(CompoundTag const &tag) {
    using namespace props;
    using namespace mcfile::nbt;
    using namespace std;

    auto fallDistance = tag.float32("FallDistance");
    auto fire = tag.int16("Fire");
    auto invulnerable = tag.boolean("Invulnerable");
    auto onGround = tag.boolean("OnGround");
    auto portalCooldown = tag.int32("PortalCooldown");
    auto motion = GetVec(tag, "Motion");
    auto pos = GetVec(tag, "Pos");
    auto rotation = GetRotation(tag, "Rotation");
    auto uuid = GetEntityUUID(tag);
    auto id = tag.string("id");
    auto customName = GetJson(tag, "CustomName");

    if (!uuid)
      return nullopt;

    Entity e(*uuid);
    if (motion)
      e.fMotion = *motion;
    if (pos)
      e.fPos = *pos;
    if (rotation)
      e.fRotation = *rotation;
    if (fallDistance)
      e.fFallDistance = *fallDistance;
    if (fire)
      e.fFire = *fire;
    if (invulnerable)
      e.fInvulnerable = *invulnerable;
    if (onGround)
      e.fOnGround = *onGround;
    if (portalCooldown)
      e.fPortalCooldown = *portalCooldown;
    if (id)
      e.fIdentifier = *id;
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
    if (!e)
      return nullptr;
    e->fIdentifier = "minecraft:ender_crystal";
    auto c = e->toCompoundTag();
    return c;
  }

  static EntityData Painting(CompoundTag const &tag, Context &ctx) {
    using namespace props;
    using namespace mcfile::nbt;
    using namespace std;

    auto facing = tag.byte("Facing");
    auto motive = tag.string("Motive");
    auto beMotive = PaintingMotive(*motive);
    auto size = PaintingSize(*motive);
    if (!beMotive || !size)
      return nullptr;

    auto tileX = tag.int32("TileX");
    auto tileY = tag.int32("TileY");
    auto tileZ = tag.int32("TileZ");
    if (!tileX || !tileY || !tileZ)
      return nullptr;

    Vec normals[4] = {Vec(0, 0, 1), Vec(-1, 0, 0), Vec(0, -1, 0), Vec(1, 0, 0)};
    Vec normal = normals[*facing];

    float const thickness = 1.0f / 32.0f;

    int dh = 0;
    int dv = 0;
    if (size->fWidth >= 4) {
      dh = 1;
    }
    if (size->fHeight >= 3) {
      dv = 1;
    }

    float x, z;
    float y = *tileY - dv + size->fHeight * 0.5f;

    // facing
    // 0: south
    // 1: west
    // 2: north
    // 3: east
    if (*facing == 0) {
      x = *tileX - dh + size->fWidth * 0.5f;
      ;
      z = *tileZ + thickness;
    } else if (*facing == 1) {
      x = *tileX + 1 - thickness;
      z = *tileZ - dh + size->fWidth * 0.5f;
    } else if (*facing == 2) {
      x = *tileX + 1 + dh - size->fWidth * 0.5f;
      z = *tileZ + 1 - thickness;
    } else {
      x = *tileX + thickness;
      z = *tileZ + 1 + dh - size->fWidth * 0.5f;
    }

    auto e = BaseProperties(tag);
    if (!e)
      return nullptr;
    e->fIdentifier = "minecraft:painting";
    e->fPos = Vec(x, y, z);
    auto c = e->toCompoundTag();
    c->set("Motive", String(*beMotive));
    c->set("Direction", Byte(*facing));
    return c;
  }

  static std::optional<Size> PaintingSize(std::string const &motive) {
    using namespace std;
    static unordered_map<string, Size> const mapping = {
        {"minecraft:pigscene", Size(4, 4)},
        {"minecraft:burning_skull", Size(4, 4)},
        {"minecraft:pointer", Size(4, 4)},
        {"minecraft:skeleton", Size(4, 3)},
        {"minecraft:donkey_kong", Size(4, 3)},
        {"minecraft:fighters", Size(4, 2)},
        {"minecraft:skull_and_roses", Size(2, 2)},
        {"minecraft:match", Size(2, 2)},
        {"minecraft:bust", Size(2, 2)},
        {"minecraft:stage", Size(2, 2)},
        {"minecraft:void", Size(2, 2)},
        {"minecraft:wither", Size(2, 2)},
        {"minecraft:sunset", Size(2, 1)},
        {"minecraft:courbet", Size(2, 1)},
        {"minecraft:creebet", Size(2, 1)},
        {"minecraft:sea", Size(2, 1)},
        {"minecraft:wanderer", Size(1, 2)},
        {"minecraft:graham", Size(1, 2)},
        {"minecraft:aztec2", Size(1, 1)},
        {"minecraft:alban", Size(1, 1)},
        {"minecraft:bomb", Size(1, 1)},
        {"minecraft:kebab", Size(1, 1)},
        {"minecraft:wasteland", Size(1, 1)},
        {"minecraft:aztec", Size(1, 1)},
        {"minecraft:plant", Size(1, 1)},
        {"minecraft:pool", Size(2, 1)},
    };
    auto found = mapping.find(motive);
    if (found != mapping.end()) {
      return found->second;
    }
    return nullopt;
  }

  static std::optional<std::string> PaintingMotive(std::string const &je) {
    using namespace std;
    static unordered_map<string, string> const mapping = {
        {"minecraft:bust", "Bust"},
        {"minecraft:pigscene", "Pigscene"},
        {"minecraft:burning_skull", "BurningSkull"},
        {"minecraft:pointer", "Pointer"},
        {"minecraft:skeleton", "Skeleton"},
        {"minecraft:donkey_kong", "DonkeyKong"},
        {"minecraft:fighters", "Fighters"},
        {"minecraft:skull_and_roses", "SkullAndRoses"},
        {"minecraft:match", "Match"},
        {"minecraft:bust", "Bust"},
        {"minecraft:stage", "Stage"},
        {"minecraft:void", "Void"},
        {"minecraft:wither", "Wither"},
        {"minecraft:sunset", "Sunset"},
        {"minecraft:courbet", "Courbet"},
        {"minecraft:creebet", "Creebet"},
        {"minecraft:sea", "Sea"},
        {"minecraft:wanderer", "Wanderer"},
        {"minecraft:graham", "Graham"},
        {"minecraft:aztec2", "Aztec2"},
        {"minecraft:alban", "Alban"},
        {"minecraft:bomb", "Bomb"},
        {"minecraft:kebab", "Kebab"},
        {"minecraft:wasteland", "Wasteland"},
        {"minecraft:aztec", "Aztec"},
        {"minecraft:plant", "Plant"},
        {"minecraft:pool", "Pool"},
    };
    auto found = mapping.find(je);
    if (found != mapping.end()) {
      return found->second;
    }
    return nullopt;
  }

public:
  std::vector<std::string> fDefinitions;
  Vec fMotion;
  Vec fPos;
  Rotation fRotation;
  std::vector<std::shared_ptr<mcfile::nbt::CompoundTag>> fTags;
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

} // namespace j2b
