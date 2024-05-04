#include "java/_entity.hpp"

#include "_default-map.hpp"
#include "_dimension-ext.hpp"
#include "_namespace.hpp"
#include "_nbt-ext.hpp"
#include "_props.hpp"
#include "entity/_armor-stand.hpp"
#include "entity/_axolotl.hpp"
#include "entity/_boat.hpp"
#include "entity/_cat.hpp"
#include "entity/_entity-attributes.hpp"
#include "entity/_frog.hpp"
#include "entity/_painting.hpp"
#include "entity/_panda.hpp"
#include "entity/_tropical-fish.hpp"
#include "entity/_wolf.hpp"
#include "enums/_color-code-java.hpp"
#include "enums/_facing4.hpp"
#include "enums/_facing6.hpp"
#include "enums/_game-mode.hpp"
#include "enums/_villager-profession.hpp"
#include "enums/_villager-type.hpp"
#include "java/_block-data.hpp"
#include "java/_context.hpp"
#include "java/_item.hpp"
#include "java/_player-abilities.hpp"
#include "java/_uuid-registrar.hpp"
#include "java/_versions.hpp"
#include "java/_world-data.hpp"
#include "tile-entity/_loot-table.hpp"

namespace je2be::java {

class Entity::Impl {
  struct ConverterContext {
    ConverterContext(Context &ctx, int dataVersion, std::set<Flag> const &flags) : fCtx(ctx), fDataVersion(dataVersion), fFlags(flags) {}

    Context &fCtx;
    std::vector<CompoundTagPtr> fPassengers;
    std::unordered_map<Pos2i, std::vector<CompoundTagPtr>, Pos2iHasher> fLeashKnots;
    int const fDataVersion;
    std::set<Flag> const &fFlags;
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

  struct Rep {
    explicit Rep(i64 uid) : fMotion(0, 0, 0), fPos(0, 0, 0), fRotation(0, 0), fUniqueId(uid) {}

    CompoundTagPtr toCompoundTag() const {
      using namespace std;
      auto tag = Compound();
      auto tags = List<Tag::Type::Compound>();
      auto definitions = List<Tag::Type::String>();
      for (auto const &d : fDefinitions) {
        definitions->push_back(String(d));
      }
      tag->insert({
          {u8"definitions", definitions},
          {u8"Motion", fMotion.toListTag()},
          {u8"Pos", fPos.toListTag()},
          {u8"Rotation", fRotation.toListTag()},
          {u8"Tags", tags},
          {u8"Chested", Bool(fChested)},
          {u8"Color2", Byte(fColor2)},
          {u8"Color", Byte(fColor)},
          {u8"Dir", Byte(fDir)},
          {u8"FallDistance", Float(fFallDistance)},
          {u8"Fire", Short(std::max((i16)0, fFire))},
          {u8"identifier", String(fIdentifier)},
          {u8"Invulnerable", Bool(fInvulnerable)},
          {u8"IsAngry", Bool(fIsAngry)},
          {u8"IsAutonomous", Bool(fIsAutonomous)},
          {u8"IsBaby", Bool(fIsBaby)},
          {u8"IsEating", Bool(fIsEating)},
          {u8"IsGliding", Bool(fIsGliding)},
          {u8"IsGlobal", Bool(fIsGlobal)},
          {u8"IsIllagerCaptain", Bool(fIsIllagerCaptain)},
          {u8"IsOrphaned", Bool(fIsOrphaned)},
          {u8"IsRoaring", Bool(fIsRoaring)},
          {u8"IsScared", Bool(fIsScared)},
          {u8"IsStunned", Bool(fIsStunned)},
          {u8"IsSwimming", Bool(fIsSwimming)},
          {u8"IsTamed", Bool(fIsTamed)},
          {u8"IsTrusting", Bool(fIsTrusting)},
          {u8"LastDimensionId", Int(fLastDimensionId)},
          {u8"LootDropped", Bool(fLootDropped)},
          {u8"MarkVariant", Int(fMarkVariant)},
          {u8"OnGround", Bool(fOnGround)},
          {u8"OwnerNew", Long(fOwnerNew)},
          {u8"PortalCooldown", Int(fPortalCooldown)},
          {u8"Saddled", Bool(fSaddled)},
          {u8"Sheared", Bool(fSheared)},
          {u8"ShowBottom", Bool(fShowBottom)},
          {u8"Sitting", Bool(fSitting)},
          {u8"SkinID", Int(fSkinId)},
          {u8"Strength", Int(fStrength)},
          {u8"StrengthMax", Int(fStrengthMax)},
          {u8"UniqueID", Long(fUniqueId)},
          {u8"Variant", Int(fVariant)},
      });
      if (fCustomName) {
        tag->set(u8"CustomName", *fCustomName);
        tag->set(u8"CustomNameVisible", Bool(fCustomNameVisible));
      }
      return tag;
    }

    std::vector<std::u8string> fDefinitions;
    Pos3f fMotion;
    Pos3f fPos;
    Rotation fRotation;
    std::vector<CompoundTagPtr> fTags;
    bool fChested = false;
    i8 fColor2 = 0;
    i8 fColor = 0;
    i8 fDir = 0;
    float fFallDistance = 0;
    i16 fFire = 0;
    std::u8string fIdentifier;
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
    i32 fLastDimensionId = 0;
    bool fLootDropped = false;
    i32 fMarkVariant = 0;
    bool fOnGround = true;
    i64 fOwnerNew = -1;
    i32 fPortalCooldown = 0;
    bool fSaddled = false;
    bool fSheared = false;
    bool fShowBottom = false;
    bool fSitting = false;
    i32 fSkinId = 0;
    i32 fStrength = 0;
    i32 fStrengthMax = 0;
    i64 const fUniqueId;
    i32 fVariant = 0;
    std::optional<std::u8string> fCustomName;
    bool fCustomNameVisible = false;
  };

public:
  static Result From(CompoundTag const &tag, Context &ctx, int dataVersion, std::set<Flag> flags) {
    using namespace std;
    auto rawId = tag.string(u8"id");
    Result result;
    if (!rawId) {
      return result;
    }
    auto id = MigrateName(*rawId);
    if (!id.starts_with(u8"minecraft:")) {
      return result;
    }
    static unique_ptr<unordered_map<u8string, Converter> const> const table(CreateEntityTable());
    auto found = table->find(Namespace::Remove(id));
    if (found == table->end()) {
      auto converted = Default(tag);
      if (converted) {
        result.fEntity = converted;
      }
      return result;
    }
    ConverterContext cctx(ctx, dataVersion, flags);
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

  static std::optional<std::pair<Pos3i, CompoundTagPtr>> ToTileEntityBlock(CompoundTag const &c) {
    using namespace std;
    auto rawId = c.string(u8"id");
    assert(rawId);
    if (!rawId) [[unlikely]] {
      return nullopt;
    }
    auto id = MigrateName(*rawId);
    if (id == u8"minecraft:item_frame") {
      return ToItemFrameTileEntityBlock(c, u8"minecraft:frame");
    } else if (id == u8"minecraft:glow_item_frame") {
      return ToItemFrameTileEntityBlock(c, u8"minecraft:glow_frame");
    }

    return nullopt;
  }

  static std::pair<Pos3i, CompoundTagPtr> ToItemFrameTileEntityBlock(CompoundTag const &c, std::u8string const &name) {
    using namespace std;

    PositionAndFacing paf = GetItemFrameTilePositionAndFacing(c);

    bool map = false;
    auto itemId = c.query(u8"Item/id");
    if (itemId) {
      auto itemIdString = itemId->asString();
      if (itemIdString) {
        u8string itemName = itemIdString->fValue;
        if (itemName == u8"minecraft:filled_map") {
          map = true;
        }
      }
    }

    auto b = Compound();
    auto states = Compound();
    states->insert({
        {u8"facing_direction", Int(paf.fFacing)},
        {u8"item_frame_map_bit", Bool(map)},
        {u8"item_frame_photo_bit", Bool(false)},
    });
    b->insert({
        {u8"name", String(name)},
        {u8"version", Int(kBlockDataVersion)},
        {u8"states", states},
    });
    return make_pair(paf.fPosition, b);
  }

  static CompoundTagPtr ToTileEntityData(CompoundTag const &c, Context &ctx, int dataVersion) {
    auto rawId = c.string(u8"id");
    assert(rawId);
    if (!rawId) {
      return nullptr;
    }
    auto id = MigrateName(*rawId);
    if (id == u8"minecraft:item_frame") {
      return ToItemFrameTileEntityData(c, ctx, u8"ItemFrame", dataVersion);
    } else if (id == u8"minecraft:glow_item_frame") {
      return ToItemFrameTileEntityData(c, ctx, u8"GlowItemFrame", dataVersion);
    }
    return nullptr;
  }

  static CompoundTagPtr ToItemFrameTileEntityData(CompoundTag const &c, Context &ctx, std::u8string const &name, int dataVersion) {
    auto tag = Compound();

    PositionAndFacing paf = GetItemFrameTilePositionAndFacing(c);
    tag->insert({
        {u8"id", String(name)},
        {u8"isMovable", Bool(true)},
        {u8"x", Int(paf.fPosition.fX)},
        {u8"y", Int(paf.fPosition.fY)},
        {u8"z", Int(paf.fPosition.fZ)},
    });
    auto itemRotation = c.byte(u8"ItemRotation", 0);
    auto itemDropChance = c.float32(u8"ItemDropChance", 1);
    auto found = c.find(u8"Item");
    if (found != c.end() && found->second->type() == Tag::Type::Compound) {
      auto item = std::dynamic_pointer_cast<CompoundTag>(found->second);
      auto m = Item::From(item, ctx, dataVersion);
      if (m) {
        tag->insert(make_pair(u8"Item", m));
        tag->insert(make_pair(u8"ItemRotation", Float(itemRotation * 45)));
        tag->insert(make_pair(u8"ItemDropChance", Float(itemDropChance)));
      }
    }
    return tag;
  }

  static bool IsTileEntity(CompoundTag const &tag) {
    auto rawId = tag.string(u8"id");
    if (!rawId) {
      return false;
    }
    auto id = MigrateName(*rawId);
    return id == u8"minecraft:item_frame" || id == u8"minecraft:glow_item_frame";
  }

  static std::optional<Entity::LocalPlayerResult> LocalPlayer(CompoundTag const &tag, Context &ctx, int dataVersion, std::set<Flag> flags) {
    using namespace std;
    using namespace mcfile;

    ConverterContext cctx(ctx, dataVersion, flags);
    auto entity = LivingEntity(tag, cctx);
    if (!entity) {
      return nullopt;
    }
    auto uuid = GetEntityUuid(tag);
    if (!uuid) {
      return nullopt;
    }

    auto pos = props::GetPos3d(tag, u8"Pos");
    if (!pos) {
      return nullopt;
    }

    // ng 1.620001f
    // ok 1.62001f
    double y = pos->fY + 1.62001;
    if (auto rootVehicle = tag.compoundTag(u8"RootVehicle"); rootVehicle) {
      if (auto vehicleEntity = rootVehicle->compoundTag(u8"Entity"); vehicleEntity) {
        if (auto vehicleRawId = vehicleEntity->string(u8"id"); vehicleRawId) {
          auto vehicleId = MigrateName(*vehicleRawId);
          if (vehicleId == u8"minecraft:boat" || vehicleId == u8"minecraft:chest_boat") {
            auto boatPos = GetBoatPos(*vehicleEntity);
            if (boatPos) {
              y = boatPos->fY + 1.24501;
            }
          }
        }
      }
    }
    pos->fY = y;
    entity->set(u8"Pos", pos->toF().toListTag());

    entity->set(u8"format_version", u8"1.12.0");
    entity->set(u8"identifier", u8"minecraft:player");
    entity->set(u8"IsOutOfControl", Bool(false));
    entity->set(u8"OwnerNew", Long(-1));
    entity->set(u8"SleepTimer", Short(0));
    entity->set(u8"Sleeping", Bool(false));
    entity->set(u8"Sneaking", Bool(false));
    entity->set(u8"TicksSinceLastWarning", Int(0));
    entity->set(u8"WarningCooldownTicks", Int(0));
    entity->set(u8"WarningLevel", Int(0));

    entity->erase(u8"LastDimensionId");
    entity->erase(u8"BodyRot");
    entity->erase(u8"BreedCooldown");
    entity->erase(u8"Fire");
    entity->erase(u8"InLove");
    entity->erase(u8"LoveCause");
    entity->erase(u8"limitedLife");

    CopyIntValues(tag, *entity, {{u8"SelectedItemSlot", u8"SelectedInventorySlot"}, {u8"XpSeed", u8"EnchantmentSeed"}, {u8"PortalCooldown"}});
    CopyFloatValues(tag, *entity, {{u8"XpP", u8"PlayerLevelProgress"}});
    CopyBoolValues(tag, *entity, {{u8"seenCredits", u8"HasSeenCredits"}});

    GameMode gameType = ctx.fGameType;
    if (auto playerGameTypeJ = tag.int32(u8"playerGameType"); playerGameTypeJ) {
      if (auto type = GameModeFromJava(*playerGameTypeJ); type) {
        gameType = *type;
      }
    }
    entity->set(u8"PlayerGameMode", Int(BedrockFromGameMode(gameType)));
    entity->set(u8"SelectedContainerId", Int(0));

    auto inventory = tag.listTag(u8"Inventory");
    if (inventory) {
      auto outInventory = ConvertAnyItemList(inventory, 36, ctx, dataVersion);
      entity->set(u8"Inventory", outInventory);

      // Armor
      auto armor = InitItemList(4);
      auto boots = ItemAtSlot(*inventory, 100);
      if (boots) {
        auto outBoots = Item::From(boots, ctx, dataVersion);
        if (outBoots) {
          armor->at(3) = outBoots;
        }
      }
      auto leggings = ItemAtSlot(*inventory, 101);
      if (leggings) {
        auto outLeggings = Item::From(leggings, ctx, dataVersion);
        if (outLeggings) {
          armor->at(2) = outLeggings;
        }
      }
      auto chestplate = ItemAtSlot(*inventory, 102);
      if (chestplate) {
        auto outChestplate = Item::From(chestplate, ctx, dataVersion);
        if (outChestplate) {
          armor->at(1) = outChestplate;
        }
      }
      auto helmet = ItemAtSlot(*inventory, 103);
      if (helmet) {
        auto outHelmet = Item::From(helmet, ctx, dataVersion);
        if (outHelmet) {
          armor->at(0) = outHelmet;
        }
      }
      entity->set(u8"Armor", armor);

      // Offhand
      auto offhand = ItemAtSlot(*inventory, -106);
      if (offhand) {
        auto offhandItem = Item::From(offhand, ctx, dataVersion);
        if (offhandItem) {
          auto outOffhand = InitItemList(1);
          outOffhand->at(0) = offhandItem;
          entity->set(u8"Offhand", outOffhand);
        }
      }
    }

    auto enderItems = tag.listTag(u8"EnderItems");
    if (enderItems) {
      auto enderChestInventory = ConvertAnyItemList(enderItems, 27, ctx, dataVersion);
      entity->set(u8"EnderChestInventory", enderChestInventory);
    }

    auto abilities = tag.compoundTag(u8"abilities");
    if (abilities) {
      auto converted = PlayerAbilities::Import(*abilities);
      if (ctx.fAllowCommand) {
        converted.fOp = true;
        converted.fTeleport = true;
      }
      entity->set(u8"abilities", converted.toCompoundTag());
    }

    auto spawnX = tag.int32(u8"SpawnX");
    auto spawnY = tag.int32(u8"SpawnY");
    auto spawnZ = tag.int32(u8"SpawnZ");
    auto spawnDimensionString = tag.string(u8"SpawnDimension");
    std::optional<mcfile::Dimension> spawnDimension;
    if (spawnDimensionString) {
      spawnDimension = DimensionFromJavaString(*spawnDimensionString);
    }
    if (spawnX && spawnY && spawnZ && spawnDimension) {
      entity->set(u8"SpawnX", Int(*spawnX));
      entity->set(u8"SpawnY", Int(*spawnY));
      entity->set(u8"SpawnZ", Int(*spawnZ));
      int dimension = BedrockDimensionFromDimension(*spawnDimension);
      entity->set(u8"SpawnDimension", Int(dimension));
      entity->set(u8"SpawnBlockPositionX", Int(*spawnX));
      entity->set(u8"SpawnBlockPositionY", Int(*spawnY));
      entity->set(u8"SpawnBlockPositionZ", Int(*spawnZ));
    }

    auto dimensionString = tag.string(u8"Dimension");
    mcfile::Dimension dim = mcfile::Dimension::Overworld;
    if (dimensionString) {
      auto dimension = DimensionFromJavaString(*dimensionString);
      if (dimension) {
        dim = *dimension;
        entity->set(u8"DimensionId", Int(BedrockDimensionFromDimension(*dimension)));
      }
    }

    auto definitions = List<Tag::Type::String>();
    definitions->push_back(String(u8"+minecraft:player"));
    definitions->push_back(String(u8"+"));
    entity->set(u8"definitions", definitions);

    auto xpLevel = tag.int32(u8"XpLevel", 0);
    entity->set(u8"PlayerLevel", Int(xpLevel));

    auto attrs = EntityAttributes::Player(tag.float32(u8"Health"));
    auto attrsTag = attrs.toBedrockListTag();
    if (auto xpTotal = tag.int32(u8"XpTotal"); xpTotal) {
      EntityAttributes::Attribute level(*xpTotal, xpLevel, 24791);
      attrsTag->push_back(level.toCompoundTag(u8"player.level"));
    }
    if (auto foodExhaustionLevel = tag.float32(u8"foodExhaustionLevel"); foodExhaustionLevel) {
      EntityAttributes::Attribute exhaustion(0, *foodExhaustionLevel, 4);
      attrsTag->push_back(exhaustion.toCompoundTag(u8"player.exhaustion"));
    }
    if (auto foodLevel = tag.int32(u8"foodLevel"); foodLevel) {
      EntityAttributes::Attribute hunger(20, *foodLevel, 20);
      attrsTag->push_back(hunger.toCompoundTag(u8"player.hunger"));
    }
    if (auto foodSaturatonLevel = tag.float32(u8"foodSaturationLevel"); foodSaturatonLevel) {
      EntityAttributes::Attribute saturation(20, *foodSaturatonLevel, 20);
      attrsTag->push_back(saturation.toCompoundTag(u8"player.saturation"));
    }
    entity->set(u8"Attributes", attrsTag);

    CopyShortValues(tag, *entity, {{u8"SleepTimer"}});

    if (auto wardenSpawnTracker = tag.compoundTag(u8"warden_spawn_tracker"); wardenSpawnTracker) {
      CopyIntValues(*wardenSpawnTracker, *entity, {{u8"cooldown_ticks", u8"WardenThreatLevelIncreaseCooldown"}, {u8"ticks_since_last_warning", u8"WardenThreatDecreaseTimer"}, {u8"warning_level", u8"WardenThreatLevel"}});
    }

    bool hasDiedBefore = false;
    if (auto lastDeathLocation = tag.compoundTag(u8"LastDeathLocation"); lastDeathLocation) {
      if (auto lastDeathDimensionString = lastDeathLocation->string(u8"dimension"); lastDeathDimensionString) {
        auto lastDeathDimension = DimensionFromJavaString(*lastDeathDimensionString);
        auto p = props::GetPos3iFromIntArrayTag(*lastDeathLocation, u8"pos");
        if (lastDeathDimension && p) {
          entity->set(u8"DeathDimension", Int(BedrockDimensionFromDimension(*lastDeathDimension)));
          entity->set(u8"DeathPositionX", Int(p->fX));
          entity->set(u8"DeathPositionY", Int(p->fY));
          entity->set(u8"DeathPositionZ", Int(p->fZ));
          hasDiedBefore = true;
        }
      }
    }
    entity->set(u8"HasDiedBefore", Bool(hasDiedBefore));

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
  static std::u8string MigrateName(std::u8string const &rawName) {
    std::u8string name = Namespace::Remove(strings::SnakeFromUpperCamel(rawName));
    if (name == u8"entity_horse") {
      name = u8"horse";
    } else if (name == u8"pig_zombie") {
      name = u8"zombified_piglin";
    } else if (name == u8"lava_slime") {
      name = u8"magma_cube";
    } else if (name == u8"mushroom_cow") {
      name = u8"mooshroom";
    } else if (name == u8"ozelot") {
      name = u8"ocelot";
    } else if (name == u8"villager_golem") {
      name = u8"iron_golem";
    } else if (name == u8"snow_man") {
      name = u8"snow_golem";
    }
    return u8"minecraft:" + name;
  }

  static CompoundTagPtr ItemAtSlot(ListTag const &items, u32 slot) {
    for (auto const &it : items) {
      auto item = std::dynamic_pointer_cast<CompoundTag>(it);
      if (!item) {
        continue;
      }
      auto s = item->byte(u8"Slot");
      if (!s) {
        continue;
      }
      if (*s == slot) {
        return item;
      }
    }
    return nullptr;
  }

  static std::unordered_map<std::u8string, Converter> *CreateEntityTable() {
    auto table = new std::unordered_map<std::u8string, Converter>();
#define E(__name, __func) table->try_emplace(u8"" #__name, __func)
#define A(__name) table->try_emplace(u8"" #__name, Animal)
#define M(__name) table->try_emplace(u8"" #__name, Monster)

    E(painting, Painting);
    E(end_crystal, EndCrystal);

    E(bat, C(Mob, Bat));
    E(bee, C(Animal, AgeableA(u8"bee"), Bee));
    M(blaze);
    E(cat, C(Animal, AgeableA(u8"minecraft:cat"), TameableA(u8"cat"), Sittable, CollarColorable, Cat));
    M(cave_spider);
    E(chicken, C(Animal, AgeableA(u8"minecraft:chicken"), Vehicle(), Chicken));
    E(cod, C(Mob, PersistentFromFromBucket));

    E(cow, C(Animal, AgeableA(u8"minecraft:cow")));
    E(creeper, C(Monster, Creeper));
    A(dolphin);
    E(donkey, C(Animal, TameableB(u8"donkey"), AgeableA(u8"minecraft:donkey"), ChestedHorse(u8"donkey"), Steerable(u8"donkey", {.fAddAlwaysUnsaddledDefinition = false}), Temper));
    E(drowned, C(Monster, AgeableD(u8"drowned"), Zombie, Drowned));
    M(elder_guardian);
    E(enderman, C(Monster, Enderman));
    E(endermite, C(Monster, Endermite));
    E(evoker, C(Monster, Rename(u8"evocation_illager"), CanJoinRaid));

    E(fox, C(Animal, AgeableA(u8"minecraft:fox"), Fox));
    M(ghast);
    M(guardian);
    E(hoglin, C(Animal, AgeableA(u8"minecraft:hoglin"), Hoglin));
    E(horse, C(Animal, TameableB(u8"horse"), AgeableA(u8"minecraft:horse"), Temper, Horse));
    E(husk, C(Monster, AgeableB(u8"zombie_husk")));
    E(llama, C(Animal, AgeableA(u8"minecraft:llama"), TameableB(u8"llama"), ChestedHorse(u8"llama"), Definitions(u8"+minecraft:strength_3"), Llama));
    E(magma_cube, C(Monster, Slime));
    E(mooshroom, C(Animal, AgeableA(u8"minecraft:cow"), Mooshroom));

    E(mule, C(Animal, TameableB(u8"mule"), AgeableA(u8"minecraft:mule"), ChestedHorse(u8"mule"), Steerable(u8"mule", {.fAddAlwaysUnsaddledDefinition = false}), Temper));
    E(ocelot, C(Animal, TameableA(u8"ocelot"), AgeableA(u8"minecraft:ocelot"), Trustable(u8"minecraft:ocelot")));
    E(panda, C(Animal, AgeableA(u8"minecraft:panda"), Panda));
    E(parrot, C(Animal, TameableA(u8"parrot"), Sittable, Parrot));
    M(phantom);
    E(pig, C(Animal, AgeableA(u8"minecraft:pig"), Steerable(u8"pig", {.fAddAlwaysUnsaddledDefinition = true})));
    E(piglin, C(Monster, ChestItemsFromInventory, Piglin));
    E(piglin_brute, C(Monster, PiglinBrute));
    E(pillager, C(Monster, CanJoinRaid, ChestItemsFromInventory));

    E(polar_bear, C(Animal, PolarBear));
    E(pufferfish, C(Mob, PersistentFromFromBucket, Pufferfish));
    E(rabbit, C(Animal, AgeableC, Rabbit));
    E(ravager, C(Monster, AttackTime, CanJoinRaid));
    E(salmon, C(Mob, PersistentFromFromBucket));
    E(sheep, C(Animal, AgeableA(u8"minecraft:sheep"), Colorable(u8"sheep"), Definitions(u8"+minecraft:sheep_dyeable", u8"+minecraft:rideable_wooly", u8"+minecraft:loot_wooly"), Sheep));
    E(shulker, C(Monster, Shulker));
    M(silverfish);
    M(skeleton); // lefty skeleton does not exist in Bedrock?

    E(skeleton_horse, C(Animal, Definitions(u8"+minecraft:skeleton_horse_adult"), SkeletonHorse));
    E(slime, C(Monster, Slime));
    E(spider, C(Monster, Vehicle(u8"spider")));
    A(squid);
    M(stray);
    E(strider, C(Animal, AgeableA(u8"minecraft:strider"), DetectSuffocation, Vehicle(u8"strider"), Definitions(u8"+minecraft:strider_pathing_behaviors"), Strider));
    E(trader_llama, C(Animal, AgeableA(u8"minecraft:llama"), Llama, Definitions(u8"+minecraft:llama_unchested", u8"-minecraft:llama_defend_trader", u8"-minecraft:llama_persistence"), TraderLlama));
    E(tropical_fish, C(Mob, Rename(u8"tropicalfish"), PersistentFromFromBucket, TropicalFish));
    E(turtle, C(Animal, AgeableF, Turtle));

    M(vex);
    E(villager, C(Animal, Rename(u8"villager_v2"), Offers(4, u8"Offers"), ChestItemsFromInventory, Villager));
    E(vindicator, C(Monster, Definitions(u8"+minecraft:default_targeting")));
    E(wandering_trader, C(Animal, Offers(0, u8"Offers"), ChestItemsFromInventory, WanderingTrader));
    E(witch, C(Monster, CanJoinRaid));
    M(wither_skeleton);
    E(wolf, C(Animal, TameableA(u8"wolf"), AgeableA(u8"minecraft:wolf"), Sittable, CollarColorable, Wolf));
    M(zoglin);
    E(zombie, C(Monster, AgeableB(u8"zombie"), Definitions(u8"+minecraft:can_have_equipment"), Zombie));

    E(zombie_horse, C(Monster, AgeableA(u8"minecraft:horse")));
    E(zombie_villager, C(Animal, Rename(u8"zombie_villager_v2"), Offers(4, u8"persistingOffers"), Villager, ZombieVillager));
    E(zombified_piglin, C(Monster, Rename(u8"zombie_pigman"), AgeableB(u8"pig_zombie"), ZombiePigman));

    E(boat, C(EntityBase, Vehicle(), Impl::Boat(u8"boat")));
    E(chest_boat, C(EntityBase, Vehicle(), ChestItemsFromItems, Impl::Boat(u8"chest_boat")));
    E(minecart, C(EntityBase, Vehicle(), Minecart, Definitions(u8"+minecraft:minecart")));
    E(armor_stand, C(LivingEntity, ArmorStand));
    E(hopper_minecart, C(EntityBase, ChestItemsFromItems, Minecart, Definitions(u8"+minecraft:hopper_minecart"), HopperMinecart));
    E(chest_minecart, C(EntityBase, ChestItemsFromItems, Minecart, Definitions(u8"+minecraft:chest_minecart"), ChestMinecart));
    E(tnt_minecart, C(EntityBase, Vehicle(), Minecart, TntMinecart));
    E(snow_golem, C(Mob, SnowGolem));
    E(iron_golem, C(Mob, Definitions(u8"+minecraft:iron_golem"), IronGolem));

    E(item, Item);
    E(ender_dragon, EnderDragon);
    E(experience_orb, C(LivingEntity, Rename(u8"xp_orb"), ExperienceOrb));
    E(item_frame, Null);      // item_frame is tile entity in BE.
    E(glow_item_frame, Null); // glow_item_frame is tile entity in BE.

    E(glow_squid, C(Animal, Definitions(u8"+minecraft:glow_squid")));
    E(axolotl, C(Animal, AgeableA(u8"axolotl"), PersistentFromFromBucket, Axolotl));
    E(goat, C(Animal, AgeableA(u8"goat"), Goat));
    E(falling_block, C(EntityBase, FallingBlock));
    E(wither, C(Mob, Definitions(u8"+minecraft:wither"), Wither));
    E(arrow, C(EntityBase, Arrow));

    E(frog, C(Animal, Definitions(u8"+minecraft:frog"), Frog));
    E(warden, C(Monster, Definitions(u8"+minecraft:warden")));
    E(allay, C(Animal, Definitions(u8"+minecraft:allay", u8"+pickup_item"), ChestItemsFromInventory, Allay));
    E(tadpole, C(Animal, AgeableE(24000), Definitions(u8"+minecraft:tadpole"), PersistentFromFromBucket));

    E(camel, C(Animal, Definitions(u8"+minecraft:camel"), AgeableA(u8"minecraft:camel"), Steerable(u8"camel", {.fAddAlwaysUnsaddledDefinition = false}), Camel));
    E(sniffer, C(Animal, Definitions(u8"+minecraft:sniffer", u8"+pushable"), AgeableA(u8"sniffer")));
    E(text_display, Null);
    E(block_display, Null);
    E(item_display, Null);
    E(interaction, Null);

    E(armadillo, C(Animal, Definitions(u8"+minecraft:armadillo"), AgeableF, Armadillo));
    E(bogged, C(Monster, Definitions(u8"+minecraft:bogged", u8"+minecraft:ranged_attack"), Bogged));
    E(breeze, C(Monster, Definitions(u8"+minecraft:breeze")));
#undef A
#undef M
#undef E
    return table;
  }

#pragma region DedicatedBehaviors
  static void Allay(CompoundTag &b, CompoundTag const &j, ConverterContext &ctx) {
    if (auto brain = j.compoundTag(u8"Brain"); brain) {
      if (auto memories = brain->compoundTag(u8"memories"); memories) {
        if (auto likedPlayer = memories->compoundTag(u8"minecraft:liked_player"); likedPlayer) {
          if (auto likedPlayerUid = props::GetUuidWithFormatIntArray(*likedPlayer, u8"value"); likedPlayerUid) {
            auto uidB = UuidRegistrar::ToId(*likedPlayerUid);
            b[u8"OwnerNew"] = Long(uidB);
          }
        }
      }
    }

    CopyLongValues(j, b, {{u8"DuplicationCooldown", u8"AllayDuplicationCooldown"}});
  }

  static void Armadillo(CompoundTag &c, CompoundTag const &tag, ConverterContext &ctx) {
    auto state = tag.string(u8"state", u8"idle");
    auto age = tag.int32(u8"Age", 0);
    auto properties = Compound();
    if (state == u8"idle") {
      if (age >= 0) {
        AddDefinition(c, u8"+minecraft:adult_unrolled");
      } else {
        AddDefinition(c, u8"+minecraft:baby_unrolled");
      }
      AddDefinition(c, u8"+minecraft:unrolled");
      properties->set(u8"minecraft:armadillo_state", u8"unrolled");
    } else if (state == u8"scared") {
      AddDefinition(c, u8"-minecraft:unrolled");
      AddDefinition(c, u8"-minecraft:adult_unrolled");
      AddDefinition(c, u8"-minecraft:baby_unrolled");
      AddDefinition(c, u8"+minecraft:rolled_up");
      AddDefinition(c, u8"+minecraft:rolled_up_without_threats");
      // AddDefinition(c, u8"+minecraft:rolled_up_with_threats");

      properties->set(u8"minecraft:armadillo_state", u8"rolled_up_relaxing");
      // properties->set(u8"minecraft:armadillo_state", u8"rolled_up");
    }
    if (auto scuteTime = tag.int32(u8"scute_time"); scuteTime) {
      auto entries = List<Tag::Type::Compound>();
      auto entry = Compound();
      entry->set(u8"SpawnTimer", Int(*scuteTime));
      entry->set(u8"StopSpawning", Bool(false));
      entries->push_back(entry);
      c[u8"entries"] = entries;
    }
  }

  static void ArmorStand(CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
    auto pos = props::GetPos3d(tag, u8"Pos");
    if (!pos) {
      return;
    }
    pos->fY = std::round(pos->fY * 2) * 0.5;
    c[u8"Pos"] = pos->toF().toListTag();

    auto health = tag.float32(u8"Health");
    auto attributes = EntityAttributes::Mob(u8"minecraft:armor_stand", health);
    if (attributes) {
      c[u8"Attributes"] = attributes->toBedrockListTag();
    }

    auto poseJ = tag.compoundTag(u8"Pose");
    if (poseJ) {
      auto indexB = ArmorStand::BedrockMostSimilarPoseIndexFromJava(*poseJ);
      auto showArms = tag.boolean(u8"ShowArms", false);
      if (indexB) {
        if (indexB != 1 || showArms) {
          auto poseB = Compound();
          poseB->set(u8"PoseIndex", Int(*indexB));
          poseB->set(u8"LastSignal", Int(0));
          c[u8"Pose"] = poseB;
        }
      }
    }
  }

  static void Arrow(CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
    auto owner = GetOwnerUuid(tag);
    if (owner) {
      c[u8"OwnerID"] = Long(*owner);
      c[u8"OwnerNew"] = Long(*owner);
    }
  }

  static void Axolotl(CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
    auto originalVariant = std::clamp(tag.int32(u8"Variant", 0), 0, 4);
    static const std::u8string definitionMapping[5] = {
        u8"+axolotl_lucy",
        u8"+axolotl_wild",
        u8"+axolotl_gold",
        u8"+axolotl_cyan",
        u8"+axolotl_blue"};
    auto variant = je2be::Axolotl::BedrockVariantFromJavaVariant(originalVariant);
    auto definition = definitionMapping[originalVariant];
    c[u8"Variant"] = Int(variant);
    AddDefinition(c, definition);

    auto onGround = tag.boolean(u8"OnGround", false);
    if (onGround) {
      AddDefinition(c, u8"+axolotl_on_land");
      AddDefinition(c, u8"-axolotl_in_water");
    } else {
      AddDefinition(c, u8"-axolotl_on_land");
      AddDefinition(c, u8"+axolotl_in_water");
    }

    auto air = tag.int16(u8"Air", 6000);
    if (air < 0) {
      AddDefinition(c, u8"+axolotl_dried");
    } else {
      AddDefinition(c, u8"-axolotl_dried");
    }
  }

  static void Bat(CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
    CopyBoolValues(tag, c, {{u8"BatFlags"}});
    AddDefinition(c, u8"+minecraft:bat");
  }

  static void Bee(CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
    auto hivePos = props::GetPos3d(tag, u8"hive_pos");
    if (!hivePos) {
      hivePos = props::GetPos3d(tag, u8"HivePos");
    }
    if (hivePos) {
      Pos3f homePos = hivePos->toF();
      c[u8"HomePos"] = homePos.toListTag();
    }
    AddDefinition(c, u8"+track_attacker");
    AddDefinition(c, u8"+shelter_detection");
    auto hasNectar = tag.boolean(u8"HasNectar", false);
    if (hasNectar) {
      AddDefinition(c, u8"+has_nectar");
    }
  }

  static void Bogged(CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
    auto sheared = tag.boolean(u8"sheared", false);
    if (sheared) {
      AddDefinition(c, u8"+minecraft:bogged_sheared");
    }
    c[u8"Sheared"] = Bool(sheared);
  }

  static std::optional<Pos3d> GetBoatPos(CompoundTag const &j) {
    auto pos = props::GetPos3d(j, u8"Pos");
    if (!pos) {
      return std::nullopt;
    }
    auto onGround = j.boolean(u8"OnGround", false);
    if (onGround) {
      int iy = (int)floor(pos->fY);
      pos->fY = iy + 0.375;
      return *pos;
    } else {
      return pos;
    }
  }

  static void Camel(CompoundTag &c, CompoundTag const &tag, ConverterContext &ctx) {
    auto isSitting = tag.boolean(u8"IsSitting"); // Used only during 1.20 experimental. LastPoseTick is used in 1.9.4pre2
    auto lastPoseTick = tag.int64(u8"LastPoseTick");

    bool sitting = false;
    if ((lastPoseTick && *lastPoseTick < 0) || isSitting == true) {
      sitting = true;
    }

    c[u8"Sitting"] = Bool(sitting);
    if (sitting) {
      AddDefinition(c, u8"+minecraft:camel_sitting");
    } else {
      AddDefinition(c, u8"-minecraft:camel_sitting");
      AddDefinition(c, u8"+minecraft:camel_standing");
    }
    CopyBoolValues(tag, c, {{u8"Tame", u8"IsTamed"}});
  }

  static void Cat(CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
    using Cat = je2be::Cat;

    Cat::Type ct = Cat::Type::Tabby;
    if (auto variantJ = tag.string(u8"variant"); variantJ) {
      // 1.19
      std::u8string name = Namespace::Remove(*variantJ);
      ct = Cat::CatTypeFromJavaVariant(name);
    } else if (auto catType = tag.int32(u8"CatType"); catType) {
      // 1.18
      ct = Cat::CatTypeFromJavaLegacyCatType(*catType);
    }
    i32 variantB = Cat::BedrockVariantFromCatType(ct);
    std::u8string type = Cat::BedrockDefinitionKeyFromCatType(ct);
    AddDefinition(c, u8"+minecraft:cat_" + type);
    c[u8"Variant"] = Int(variantB);
    c[u8"DwellingUniqueID"] = String(u8"00000000-0000-0000-0000-000000000000");
    c[u8"RewardPlayersOnFirstFounding"] = Bool(true);
  }

  static void ChestMinecart(CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
    LootTable::JavaToBedrock(tag, c);
  }

  static void Chicken(CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
    using namespace std;
    auto eggLayTime = tag.int32(u8"EggLayTime");
    if (eggLayTime) {
      auto entries = List<Tag::Type::Compound>();
      auto timer = Compound();
      timer->set(u8"SpawnTimer", Int(*eggLayTime));
      timer->set(u8"StopSpawning", Bool(false));
      entries->push_back(timer);
      c[u8"entries"] = entries;
    }
  }

  static void Creeper(CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
    auto powered = tag.boolean(u8"powered", false);
    if (powered) {
      AddDefinition(c, u8"+minecraft:charged_creeper");
      AddDefinition(c, u8"+minecraft:exploding");
    }
  }

  static void Drowned(CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
    bool melee = true;
    if (auto handItems = tag.listTag(u8"HandItems"); handItems && !handItems->fValue.empty()) {
      if (auto item = handItems->fValue[0]->asCompound(); item) {
        if (Item::Count(*item, 0) > 0 && item->string(u8"id") == u8"minecraft:trident") {
          melee = false;
        }
      }
    }
    if (melee) {
      AddDefinition(c, u8"+minecraft:melee_equipment");
      AddDefinition(c, u8"+minecraft:melee_mode");
    } else {
      AddDefinition(c, u8"+minecraft:mode_switcher");
      AddDefinition(c, u8"+minecraft:ranged_equipment");
      AddDefinition(c, u8"+minecraft:ranged_mode");
    }
  }

  static void Enderman(CompoundTag &c, CompoundTag const &tag, ConverterContext &ctx) {
    auto carriedBlockTagJ = tag.compoundTag(u8"carriedBlockState");
    if (carriedBlockTagJ) {
      auto name = carriedBlockTagJ->string(u8"Name");
      if (name) {
        auto carriedBlockJ = mcfile::je::Block::FromName(*name, ctx.fDataVersion);
        auto carriedBlockB = BlockData::From(carriedBlockJ, nullptr, {.fItem = true});
        if (carriedBlockB) {
          c[u8"carriedBlock"] = carriedBlockB;
        }
      }
    }
  }

  static void Endermite(CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
    CopyIntValues(tag, c, {{u8"Lifetime"}});
  }

  static void ExperienceOrb(CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
    auto value = tag.int16(u8"Value");
    auto count = tag.int32(u8"Count", 1);
    if (value) {
      c[u8"experience value"] = Int(*value * count);
    }
    CopyShortValues(tag, c, {{u8"Age"}});
    AddDefinition(c, u8"+minecraft:xp_orb");
  }

  static void FallingBlock(CompoundTag &c, CompoundTag const &tag, ConverterContext &ctx) {
    auto blockState = tag.compoundTag(u8"BlockState");
    if (!blockState) {
      return;
    }
    auto name = blockState->string(u8"Name");
    if (!name) {
      return;
    }
    auto block = BlockData::From(mcfile::je::Block::FromName(*name, ctx.fDataVersion), nullptr, {.fItem = true});
    if (!block) {
      return;
    }
    c[u8"FallingBlock"] = block;

    CopyFloatValues(tag, c, {{u8"FallDistance"}});

    u8 time = mcfile::Clamp<u8>(tag.int32(u8"Time", 0));
    c[u8"Time"] = Byte(*(i8 *)&time);
  }

  static void Fox(CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
    auto type = tag.string(u8"Type", u8"red");
    i32 variant = 0;
    if (type == u8"red") {
      variant = 0;
    } else if (type == u8"snow") {
      variant = 1;
    }
    c[u8"Variant"] = Int(variant);
    AddDefinition(c, u8"+minecraft:fox_" + type);

    if (tag.boolean(u8"Sleeping", false)) {
      AddDefinition(c, u8"+minecraft:fox_ambient_sleep");
    }

    auto trustedJ = tag.listTag(u8"Trusted");
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
        i64 uuidB = UuidRegistrar::ToId(*uuidJ);
        std::u8string key = u8"TrustedPlayer" + mcfile::String::ToString(index);
        c[key] = Long(uuidB);
        index++;
      }
      if (index > 0) {
        c[u8"TrustedPlayersAmount"] = Int(index);
      }
    }

    if (auto handItems = tag.listTag(u8"HandItems"); handItems && !handItems->fValue.empty()) {
      if (auto itemJ = handItems->fValue[0]->asCompound(); itemJ) {
        if (Item::Count(*itemJ, 0) > 0) {
          AddDefinitionFlag(c, u8"minecraft:fox_with_item", true);
        }
      }
    }

    AddDefinition(c, u8"+minecraft:fox_ambient_normal");
  }

  static void Frog(CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
    auto variantJ = tag.string(u8"variant", u8"");
    auto variantB = Frog::BedrockVariantFromJavaVariant(variantJ);
    auto definition = Frog::BedrockDefinitionFromJavaVariant(variantJ);
    AddDefinition(c, definition);
    c[u8"Variant"] = Int(variantB);
  }

  static void Goat(CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
    auto screaming = tag.boolean(u8"IsScreamingGoat", false);
    if (screaming) {
      AddDefinition(c, u8"+goat_screamer");
      AddDefinition(c, u8"+ram_screamer");
      AddDefinition(c, u8"+interact_screamer");
    } else {
      AddDefinition(c, u8"+goat_default");
      AddDefinition(c, u8"+ram_default");
      AddDefinition(c, u8"+interact_default");
    }
    AddDefinition(c, u8"-attack_cooldown");

    i32 hornCount = 0;
    if (tag.boolean(u8"HasLeftHorn", false)) {
      hornCount++;
    }
    if (tag.boolean(u8"HasRightHorn", false)) {
      hornCount++;
    }
    c[u8"GoatHornCount"] = Int(hornCount);
  }

  static void Hoglin(CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
    AddDefinition(c, u8"+huntable_adult");
    AddDefinition(c, u8"+zombification_sensor");
    if (tag.boolean(u8"CannotBeHunted", false)) {
      AddDefinition(c, u8"-angry_hoglin");
    }
  }

  static void Horse(CompoundTag &c, CompoundTag const &tag, ConverterContext &ctx) {
    auto variant = tag.int32(u8"Variant", 0);
    auto baseColor = 0xf & variant;
    auto markings = 0xf & (variant >> 8);
    c[u8"Variant"] = Int(baseColor);
    c[u8"MarkVariant"] = Int(markings);

    if (auto armorItemJ = FallbackPtr<CompoundTag>(tag, {u8"body_armor_item", u8"ArmorItem"}); armorItemJ) {
      if (auto armorItemB = Item::From(armorItemJ, ctx.fCtx, ctx.fDataVersion); armorItemB) {
        AddChestItem(c, armorItemB, 1, 1);
      }
    }

    static DefaultMap<i32, std::u8string> const sVariant(
        {
            {0, u8"base_white"},
            {1, u8"base_creamy"},
            {2, u8"base_chestnut"},
            {3, u8"base_brown"},
            {4, u8"base_black"},
            {5, u8"base_gray"},
            {6, u8"base_darkbrown"},
        },
        u8"base_white");
    AddDefinition(c, u8"+minecraft:" + sVariant.at(baseColor));

    static DefaultMap<i32, std::u8string> const sMarkVariant(
        {
            {0, u8"markings_none"},
            {1, u8"markings_white_details"},
            {2, u8"markings_white_fields"},
            {3, u8"markings_white_dots"},
            {4, u8"markings_black_dots"},
        },
        u8"markings_none");
    AddDefinition(c, u8"+minecraft:" + sMarkVariant.at(markings));

    bool saddle = false;
    if (auto saddleItem = tag.compoundTag(u8"SaddleItem"); saddleItem) {
      if (Item::Count(*saddleItem, 0) > 0) {
        AddDefinition(c, u8"+minecraft:horse_saddled");
        auto item = Compound();
        item->insert({
            {u8"Damage", Byte(0)},
            {u8"Name", String(u8"minecraft:saddle")},
            {u8"WasPickedUp", Bool(false)},
        });
        AddChestItem(c, item, 0, 1);
        saddle = true;
      }
    }
    c[u8"Saddled"] = Bool(saddle);
  }

  static void HopperMinecart(CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
    auto enabled = tag.boolean(u8"Enabled", true);
    if (enabled) {
      AddDefinition(c, u8"+minecraft:hopper_active");
    }
  }

  static void IronGolem(CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
    auto playerCreated = tag.boolean(u8"PlayerCreated", false);
    auto angryAt = props::GetUuid(tag, {.fIntArray = u8"AngryAt"});
    auto angerTime = tag.int32(u8"AngerTime", 0);

    if (playerCreated) {
      AddDefinition(c, u8"+minecraft:player_created");
    } else {
      AddDefinition(c, u8"+minecraft:village_created");
    }
    c[u8"IsAngry"] = Bool(angerTime > 0);
    if (angryAt) {
      i64 targetId = UuidRegistrar::ToId(*angryAt);
      c[u8"TargetID"] = Long(targetId);
    }
  }

  static void Llama(CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
    auto variant = tag.int32(u8"Variant", 0);
    std::u8string color = u8"creamy";
    switch (variant) {
    case 3:
      color = u8"gray";
      break;
    case 1:
      color = u8"white";
      break;
    case 2:
      color = u8"brown";
      break;
    case 0:
      color = u8"creamy";
      break;
    }
    c[u8"Variant"] = Int(variant);
    AddDefinition(c, u8"+minecraft:llama_" + color);

    auto armors = c.listTag(u8"Armor");

    auto decorItemId = tag.query(u8"DecorItem/id")->asString();
    if (decorItemId && decorItemId->fValue.starts_with(u8"minecraft:") && decorItemId->fValue.ends_with(u8"_carpet")) {
      auto carpetColor = strings::RemovePrefixAndSuffix(u8"minecraft:", decorItemId->fValue, u8"_carpet");
      auto colorCode = ColorCodeJavaFromJavaName(carpetColor);
      auto beCarpetColor = BedrockNameFromColorCodeJava(colorCode);
      auto armor = Compound();
      armor->insert({{u8"Count", Byte(1)}, {u8"Damage", Short(0)}, {u8"Name", String(u8"minecraft:carpet")}, {u8"WasPickedUp", Bool(false)}});
      auto block = Compound();
      block->insert({{u8"name", String(u8"minecraft:carpet")}, {u8"version", Int(kBlockDataVersion)}});
      auto states = Compound();
      states->set(u8"color", beCarpetColor);
      block->set(u8"states", states);
      armor->set(u8"Block", block);

      if (armors && armors->size() > 1) {
        armors->at(1) = armor;
      }

      AddChestItem(c, armor, 0, 1);
    }

    CopyIntValues(tag, c, {{u8"Strength"}});
  }

  static void Minecart(CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
    auto pos = props::GetPos3d(tag, u8"Pos");
    auto onGround = tag.boolean(u8"OnGround");
    if (pos && onGround) {
      if (*onGround) {
        // on ground
        pos->fY += 0.35;
      } else {
        // on rail
        pos->fY += 0.4375;
      }
      c[u8"Pos"] = pos->toF().toListTag();
    }
  }

  static void Mooshroom(CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
    auto type = tag.string(u8"Type", u8"red");
    i32 variant = 0;
    if (type == u8"brown") {
      variant = 1;
      AddDefinition(c, u8"+minecraft:mooshroom_brown");
    } else {
      AddDefinition(c, u8"+minecraft:mooshroom_red");
    }
    c[u8"Variant"] = Int(variant);
  }

  static void Panda(CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
    auto mainGene = tag.string(u8"MainGene", u8"normal");
    auto hiddenGene = tag.string(u8"HiddenGene", u8"normal");

    Panda::Gene main = Panda::GeneFromJavaName(mainGene);
    Panda::Gene hidden = Panda::GeneFromJavaName(hiddenGene);

    auto genes = List<Tag::Type::Compound>();
    auto gene = Compound();
    gene->set(u8"MainAllele", Int(Panda::BedrockAlleleFromGene(main)));
    gene->set(u8"HiddenAllele", Int(Panda::BedrockAlleleFromGene(hidden)));
    genes->push_back(gene);

    c[u8"GeneArray"] = genes;
  }

  static void Parrot(CompoundTag &c, CompoundTag const &tag, ConverterContext &ctx) {
    auto variantJ = tag.int32(u8"Variant", 0);
    c[u8"Variant"] = Int(variantJ);

    std::u8string def;
    switch (variantJ) {
    case 0:
      def = u8"+minecraft:parrot_red";
      break;
    case 1:
      def = u8"+minecraft:parrot_blue";
      break;
    case 2:
      def = u8"+minecraft:parrot_green";
      break;
    case 3:
      def = u8"+minecraft:parrot_cyan";
      break;
    case 4:
      def = u8"+minecraft:parrot_silver";
      break;
    }
    if (!def.empty()) {
      AddDefinition(c, def);
    }
    AddDefinition(c, u8"+minecraft:parrot_adult");
    if (ctx.fFlags.count(Flag::ShoulderRider) == 0) {
      AddDefinition(c, u8"+minecraft:parrot_not_riding_player");
    }
  }

  static void Piglin(CompoundTag &c, CompoundTag const &tag, ConverterContext &ctx) {
    if (auto cannotHant = tag.boolean(u8"CannotHunt", false); cannotHant) {
      AddDefinition(c, u8"+not_hunter");
    } else {
      AddDefinition(c, u8"+hunter");
    }

    CompoundTagPtr itemInHand;
    if (auto inHand = tag.listTag(u8"HandItems"); inHand) {
      for (auto const &it : *inHand) {
        if (auto item = dynamic_pointer_cast<CompoundTag>(it); item) {
          itemInHand = Item::From(item, ctx.fCtx, ctx.fDataVersion);
          if (itemInHand) {
            break;
          }
        }
      }
    }
    if (itemInHand) {
      c.set(u8"ItemInHand", itemInHand);
      auto name = itemInHand->string(u8"Name");
      if (name == u8"minecraft:crossbow") {
        AddDefinition(c, u8"+ranged_unit");
      } else if (name == u8"minecraft:golden_sword") {
        AddDefinition(c, u8"+melee_unit");
      }
    }
    AddDefinition(c, u8"+alert_for_attack_targets");
    AddDefinition(c, u8"+zombification_sensor");
    AddDefinition(c, u8"+attack_cooldown");
    AddDefinition(c, u8"+interactable_piglin");

    bool baby = tag.boolean(u8"IsBaby", false);
    c.set(u8"IsBaby", Bool(baby));
    if (baby) {
      AddDefinition(c, u8"+piglin_baby");
    } else {
      AddDefinition(c, u8"+piglin_adult");
    }

    if (!c.listTag(u8"ChestItems")) {
      c.set(u8"ChestItems", InitItemList(8));
    }
  }

  static void PiglinBrute(CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
    if (auto homeTag = tag.query(u8"Brain/memories/minecraft:home/value"); homeTag) {
      if (auto home = homeTag->asCompound(); home) {
        auto dimension = home->string(u8"dimension");
        auto pos = home->intArrayTag(u8"pos");
        if (dimension && pos && pos->value().size() == 3) {
          if (auto d = DimensionFromJavaString(*dimension); d) {
            auto const &posv = pos->value();
            Pos3f posB(posv[0], posv[1], posv[2]);
            c[u8"HomeDimensionId"] = Int(BedrockDimensionFromDimension(*d));
            c[u8"HomePos"] = posB.toListTag();
          }
        }
      }
    }
    AddDefinition(c, u8"+zombification_sensor");
    AddDefinition(c, u8"+alert_for_attack_targets");
    AddDefinition(c, u8"+melee_unit");
    AddDefinition(c, u8"+go_back_to_spawn");
  }

  static void PolarBear(CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
    if (tag.int32(u8"Age", 0) >= 0) {
      AddDefinition(c, u8"+minecraft:adult");
      AddDefinition(c, u8"+minecraft:adult_wild");
    } else {
      AddDefinition(c, u8"+minecraft:baby");
      AddDefinition(c, u8"+minecraft:baby_wild");
    }
  }

  static void Pufferfish(CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
    auto puffState = tag.int32(u8"PuffState", 0);

    /*
    "-minecraft:full_puff",
    "-minecraft:half_puff_secondary",
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

    switch (puffState) {
    case 1:
      AddDefinitionFlag(c, u8"minecraft:normal_puff", false);
      AddDefinitionFlag(c, u8"minecraft:half_puff_primary", true);
      break;
    case 2:
      AddDefinitionFlag(c, u8"minecraft:full_puff", true);
      AddDefinitionFlag(c, u8"minecraft:deflate_sensor", false);
      AddDefinitionFlag(c, u8"minecraft:start_deflate", true);
      break;
    case 0:
    default:
      AddDefinitionFlag(c, u8"minecraft:full_puff", false);
      AddDefinitionFlag(c, u8"minecraft:half_puff_secondary", false);
      AddDefinitionFlag(c, u8"minecraft:normal_puff", true);
      break;
    }
  }

  static void Rabbit(CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
    auto type = tag.int32(u8"RabbitType", 0);
    std::u8string coat = u8"brown";
    i32 variant = 0;
    if (type == 0) {
      coat = u8"brown";
      variant = 0;
    } else if (type == 2) {
      coat = u8"black";
      variant = 2;
    } else if (type == 5) {
      coat = u8"salt";
      variant = 5;
    } else if (type == 1) {
      coat = u8"white";
      variant = 1;
    } else if (type == 3) {
      coat = u8"splotched";
      variant = 3;
    } else if (type == 4) {
      coat = u8"desert";
      variant = 4;
    } else if (type == 99) {
      coat = u8"white";
      variant = 1;
      c[u8"CustomName"] = String(u8"The Killer Bunny");
    }
    c[u8"Variant"] = Int(variant);
    AddDefinition(c, u8"+coat_" + coat);

    CopyIntValues(tag, c, {{u8"MoreCarrotTicks"}});
  }

  static void Sheep(CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
    CopyBoolValues(tag, c, {{u8"Sheared", u8"Sheared", false}});
    AddDefinitionFlag(c, u8"minecraft:sheep_sheared", tag.boolean(u8"Sheared", false));
  }

  static void Shulker(CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
    auto color = tag.byte(u8"Color", 16);
    if (0 <= color && color <= 15) {
      c[u8"Color"] = Byte(color);
      AddDefinition(c, u8"+minecraft:shulker_" + BedrockNameFromColorCodeJava((ColorCodeJava)color));
    } else {
      AddDefinition(c, u8"+minecraft:shulker_undyed");
    }
    c[u8"Variant"] = Int(color);
  }

  static void SkeletonHorse(CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
    auto trap = tag.boolean(u8"SkeletonTrap", false);
    if (trap) {
      AddDefinition(c, u8"+minecraft:skeleton_trap");
      AddDefinition(c, u8"+minecraft:lightning_immune");
    }
  }

  static void Slime(CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
    auto sizeJ = tag.int32(u8"Size", 0);
    int sizeB = sizeJ + 1;
    c[u8"Variant"] = Int(sizeB);
    c[u8"Size"] = Byte(sizeB);

    auto health = tag.float32(u8"Health");
    auto attributes = EntityAttributes::Slime(sizeB, health);
    c[u8"Attributes"] = attributes.toBedrockListTag();

    // J  B  B(definition)
    // 3  4  +minecraft:slime_large
    // 1  2  +minecraft:slime_medium
    // 0  1  +minecraft:slime_small
    switch (sizeJ) {
    case 3:
      AddDefinition(c, u8"+minecraft:slime_large");
      break;
    case 1:
      AddDefinition(c, u8"+minecraft:slime_medium");
      break;
    case 0:
      AddDefinition(c, u8"+minecraft:slime_small");
      break;
    }
    AddDefinition(c, u8"+minecraft:slime_calm");
  }

  static void SnowGolem(CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
    auto pumpkin = tag.boolean(u8"Pumpkin", true);
    if (!pumpkin) {
      AddDefinition(c, u8"+minecraft:snowman_sheared");
    }
  }

  static void Strider(CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
    if (tag.int32(u8"Age", 0) >= 0) {
      if (tag.boolean(u8"Saddle", false)) {
        AddDefinition(c, u8"-minecraft:strider_unsaddled");
        AddDefinition(c, u8"+minecraft:strider_saddled");
      } else {
        AddDefinition(c, u8"+minecraft:strider_unsaddled");
      }
      AddDefinition(c, u8"-minecraft:strider_baby");
    }
  }

  static void TntMinecart(CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
    AddDefinition(c, u8"+minecraft:tnt_minecart");
    auto fuse = tag.int32(u8"TNTFuse", -1);
    if (fuse < 0) {
      AddDefinition(c, u8"+minecraft:inactive");
    } else {
      c[u8"Fuse"] = Byte(fuse);
      AddDefinition(c, u8"-minecraft:inactive");
      AddDefinition(c, u8"+minecraft:primed_tnt");
    }
  }

  static void TraderLlama(CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
    AddDefinition(c, u8"+minecraft:llama_wandering_trader");
    AddDefinition(c, u8"-minecraft:llama_wild");
    AddDefinition(c, u8"+minecraft:llama_tamed");
    AddDefinition(c, u8"+minecraft:strength_3");
    c[u8"InventoryVersion"] = String(u8"1.16.40");
    c[u8"MarkVariant"] = Int(1);
    c[u8"IsTamed"] = Bool(true);
  }

  static void TropicalFish(CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
    auto variantJ = tag.int32(u8"Variant", 0);
    auto tf = TropicalFish::FromJavaVariant(variantJ);

    i32 variant = tf.fSmall ? 0 : 1;
    c[u8"Variant"] = Int(variant);
    static DefaultMap<i32, std::u8string> const sVariant(
        {
            {0, u8"tropicalfish_variant_a"},
            {1, u8"tropicalfish_variant_b"},
        },
        u8"tropicalfish_variant_a");
    AddDefinition(c, u8"+minecraft:" + sVariant.at(variant));

    i32 markVariant = tf.fPattern;
    c[u8"MarkVariant"] = Int(markVariant);
    static DefaultMap<i32, std::u8string> const sMarkVariant(
        {
            {0, u8"tropicalfish_variant_pattern_1"},
            {1, u8"tropicalfish_variant_pattern_2"},
            {2, u8"tropicalfish_variant_pattern_3"},
            {3, u8"tropicalfish_variant_pattern_4"},
            {4, u8"tropicalfish_variant_pattern_5"},
            {5, u8"tropicalfish_variant_pattern_6"},
        },
        u8"tropicalfish_variant_pattern_1");
    AddDefinition(c, u8"+minecraft:" + sMarkVariant.at(markVariant));

    i8 color = tf.fBodyColor;
    c[u8"Color"] = Byte(color);
    static DefaultMap<i8, std::u8string> const sColor(
        {
            {0, u8"tropicalfish_base_white"},
            {1, u8"tropicalfish_base_orange"},
            {2, u8"tropicalfish_base_magenta"},
            {3, u8"tropicalfish_base_lightblue"},
            {4, u8"tropicalfish_base_yellow"},
            {5, u8"tropicalfish_base_lightgreen"},
            {6, u8"tropicalfish_base_pink"},
            {7, u8"tropicalfish_base_gray"},
            {8, u8"tropicalfish_base_silver"},
            {9, u8"tropicalfish_base_cyan"},
            {10, u8"tropicalfish_base_purple"},
            {11, u8"tropicalfish_base_blue"},
            {12, u8"tropicalfish_base_brown"},
            {13, u8"tropicalfish_base_green"},
            {14, u8"tropicalfish_base_red"},
        },
        u8"tropicalfish_base_white");
    AddDefinition(c, u8"+minecraft:" + sColor.at(color));

    i8 color2 = tf.fPatternColor;
    c[u8"Color2"] = Byte(color2);
    static DefaultMap<i8, std::u8string> const sColor2(
        {
            {0, u8"tropicalfish_pattern_white"},
            {1, u8"tropicalfish_pattern_orange"},
            {2, u8"tropicalfish_pattern_magenta"},
            {3, u8"tropicalfish_pattern_lightblue"},
            {4, u8"tropicalfish_pattern_yellow"},
            {5, u8"tropicalfish_pattern_lightgreen"},
            {6, u8"tropicalfish_pattern_pink"},
            {7, u8"tropicalfish_pattern_gray"},
            {8, u8"tropicalfish_pattern_silver"},
            {9, u8"tropicalfish_pattern_cyan"},
            {10, u8"tropicalfish_pattern_purple"},
            {11, u8"tropicalfish_pattern_blue"},
            {12, u8"tropicalfish_pattern_brown"},
            {13, u8"tropicalfish_pattern_green"},
            {14, u8"tropicalfish_pattern_red"},
        },
        u8"tropicalfish_pattern_white");
    AddDefinition(c, u8"+minecraft:" + sColor2.at(color2));
  }

  static void Turtle(CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
    auto x = tag.int32(u8"HomePosX");
    auto y = tag.int32(u8"HomePosY");
    auto z = tag.int32(u8"HomePosZ");
    if (x && y && z) {
      auto homePos = List<Tag::Type::Float>();
      homePos->push_back(Float(*x));
      homePos->push_back(Float(*y));
      homePos->push_back(Float(*z));
      c[u8"HomePos"] = homePos;
    }

    if (tag.boolean(u8"HasEgg", false)) {
      AddDefinition(c, u8"-minecraft:pregnant");
      AddDefinition(c, u8"-minecraft:wants_to_lay_egg");
    }
  }

  static void Villager(CompoundTag &c, CompoundTag const &tag, ConverterContext &ctx) {
    using namespace std;

    auto data = tag.compoundTag(u8"VillagerData");
    optional<VillagerProfession> profession;
    optional<VillagerType> type;
    if (data) {
      auto inProfession = data->string(u8"profession");
      if (inProfession) {
        profession = VillagerProfession::FromJavaProfession(*inProfession);
      }

      auto inType = data->string(u8"type");
      if (inType) {
        type = VillagerType::FromJavaType(*inType);
      }
    }

    if (profession) {
      c[u8"PreferredProfession"] = String(profession->string());
      AddDefinition(c, u8"+" + profession->string());
      c[u8"Variant"] = Int(profession->variant());
      if (auto tradeTablePath = profession->tradeTablePath(); tradeTablePath) {
        c[u8"TradeTablePath"] = String(*tradeTablePath);
      }
    }
    if (type) {
      AddDefinition(c, u8"+" + type->string() + u8"_villager");
      c[u8"MarkVariant"] = Int(type->variant());
    }

    auto age = tag.int32(u8"Age", 0);
    if (age < 0) {
      c[u8"Age"] = Int(age);
      AddDefinition(c, u8"+baby");
    } else {
      AddDefinition(c, u8"+adult");
    }
    AddDefinition(c, u8"+minecraft:villager_v2");

    auto tradeExp = tag.int32(u8"Xp", 0);
    c[u8"TradeExperience"] = Int(tradeExp);

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
    c[u8"TradeTier"] = Int(tradeTier);

    if (auto uuid = c.int64(u8"UniqueID"); uuid) {
      // Set random SkinID based on UniqueID
      i64 id = *uuid;
      u64 seed = *(i64 *)&id;
      std::mt19937_64 mt(seed);
      std::uniform_int_distribution<i32> distribution(0, 5);
      i32 skinId = distribution(mt);
      c[u8"SkinID"] = Int(skinId);
      AddDefinition(c, u8"+villager_skin_" + mcfile::String::ToString(skinId));
    }
  }

  static void WanderingTrader(CompoundTag &c, CompoundTag const &tag, ConverterContext &ctx) {
    if (auto despawnDelay = tag.int32(u8"DespawnDelay"); despawnDelay) {
      i64 timestamp = ctx.fCtx.fGameTick + *despawnDelay;
      c[u8"TimeStamp"] = Long(timestamp);
    }
  }

  static void Wither(CompoundTag &c, CompoundTag const &tag, ConverterContext &ctx) {
    auto health = tag.float32(u8"Health");
    auto attributes = EntityAttributes::Wither(ctx.fCtx.fDifficultyBedrock, health);
    c[u8"Attributes"] = attributes.toBedrockListTag();
  }

  static void Wolf(CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
    auto health = tag.float32(u8"Health");
    auto owner = GetOwnerUuid(tag);
    auto variant = tag.string(u8"variant");

    auto attributes = EntityAttributes::Wolf(!!owner, health);
    c[u8"Attributes"] = attributes.toBedrockListTag();

    auto properties = Compound();
    properties->set(u8"minecraft:has_increased_max_health", Bool(owner != std::nullopt));
    properties->set(u8"minecraft:is_armorable", Bool(owner != std::nullopt));
    c[u8"properties"] = properties;

    if (owner) {
      AddDefinition(c, u8"+minecraft:wolf_increased_max_health");
      AddDefinition(c, u8"+minecraft:wolf_armorable");
    }

    if (variant) {
      auto name = Namespace::Remove(*variant);
      AddDefinition(c, u8"+minecraft:wolf_" + name);

      i32 variantB = Wolf::BedrockVariantFromJavaVariant(*variant);
      c[u8"Variant"] = Int(variantB);
    }
  }

  static void Zombie(CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
    if (tag.boolean(u8"CanBreakDoors", false)) {
      AddDefinition(c, u8"+minecraft:can_break_doors");
    }
  }

  static void ZombiePigman(CompoundTag &tagB, CompoundTag const &tagJ, ConverterContext &ctx) {
    auto angryAt = props::GetUuid(tagJ, {.fIntArray = u8"AngryAt"});
    if (angryAt) {
      AddDefinition(tagB, u8"-minecraft:pig_zombie_calm");
      AddDefinition(tagB, u8"+minecraft:pig_zombie_angry");

      tagB[u8"IsAngry"] = Bool(true);

      auto targetId = UuidRegistrar::ToId(*angryAt);
      tagB[u8"TargetID"] = Long(targetId);
    } else {
      AddDefinition(tagB, u8"+minecraft:pig_zombie_calm");
    }
  }

  static void ZombieVillager(CompoundTag &c, CompoundTag const &tag, ConverterContext &ctx) {
    auto offers = tag.compoundTag(u8"Offers");
    if (offers) {
      c[u8"Persistent"] = Bool(true);
    }
  }
#pragma endregion

#pragma region Behaviors
  static void AgeableC(CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
    auto age = tag.int32(u8"Age", 0);
    if (age < 0) {
      AddDefinition(c, u8"+baby");
      c[u8"Age"] = Int(age);
    } else {
      AddDefinition(c, u8"+adult");
      c.erase(u8"Age");
    }
    c[u8"IsBaby"] = Bool(age < 0);
  }

  static void AgeableF(CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
    auto age = tag.int32(u8"Age", 0);
    if (age < 0) {
      AddDefinition(c, u8"+minecraft:baby");
      c[u8"Age"] = Int(age);
    } else {
      AddDefinition(c, u8"-minecraft:baby");
      AddDefinition(c, u8"+minecraft:adult");
      c.erase(u8"Age");
    }
    c[u8"IsBaby"] = Bool(age < 0);
  }

  static void AttackTime(CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
    auto attackTick = tag.int32(u8"AttackTick", 0);
    c[u8"AttackTime"] = Short(attackTick);
  }

  static void CanJoinRaid(CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
    auto canJoin = tag.boolean(u8"CanJoinRaid");
    if (canJoin == true) {
      AddDefinition(c, u8"+minecraft:raid_configuration");
    }
  }

  static void CollarColorable(CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
    auto collarColor = tag.byte(u8"CollarColor");
    if (collarColor && GetOwnerUuid(tag)) {
      c[u8"Color"] = Byte(*collarColor);
    }
  }

  //static void Debug(CompoundTag &c, CompoundTag const &tag, ConverterContext &) {}

  static void DetectSuffocation(CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
    AddDefinition(c, u8"-minecraft:start_suffocating");
    AddDefinition(c, u8"+minecraft:detect_suffocating");
  }

  static void ChestItemsFromInventory(CompoundTag &c, CompoundTag const &tag, ConverterContext &ctx) {
    auto inventory = tag.listTag(u8"Inventory");
    auto chestItems = List<Tag::Type::Compound>();
    if (inventory) {
      // items in "Inventory" does not have "Slot" property. So we can't use ConvertAnyItemList here.
      i8 slot = 0;
      for (auto const &it : *inventory) {
        auto itemJ = std::dynamic_pointer_cast<CompoundTag>(it);
        if (!itemJ) {
          continue;
        }
        CompoundTagPtr itemB = Item::From(itemJ, ctx.fCtx, ctx.fDataVersion);
        if (!itemB) {
          itemB = Compound();
          itemB->set(u8"Count", Byte(0));
          itemB->set(u8"Damage", Short(0));
          itemB->set(u8"Name", u8"");
          itemB->set(u8"WasPickedUp", Bool(false));
        }
        itemB->set(u8"Slot", Byte(slot));
        chestItems->push_back(itemB);
        slot++;
      }
    }
    c[u8"ChestItems"] = chestItems;
  }

  static void ChestItemsFromItems(CompoundTag &c, CompoundTag const &tag, ConverterContext &ctx) {
    auto chestItems = List<Tag::Type::Compound>();

    auto items = tag.listTag(u8"Items");
    if (items) {
      for (auto const &it : *items) {
        if (it->type() != Tag::Type::Compound) {
          continue;
        }
        auto item = std::dynamic_pointer_cast<CompoundTag>(it);
        if (!item) {
          continue;
        }
        auto converted = Item::From(item, ctx.fCtx, ctx.fDataVersion);
        if (!converted) {
          continue;
        }
        auto slot = item->byte(u8"Slot");
        if (!slot) {
          continue;
        }
        converted->set(u8"Slot", Byte(*slot));
        chestItems->push_back(converted);
      }
    }

    c[u8"ChestItems"] = chestItems;
  }

  static void PersistentFromFromBucket(CompoundTag &c, CompoundTag const &tag, ConverterContext &ctx) {
    auto fromBucket = tag.boolean(u8"FromBucket", false);
    c[u8"NaturalSpawn"] = Bool(!fromBucket);

    if (fromBucket) {
      c[u8"Persistent"] = Bool(true);
    } else {
      auto persistenceRequired = tag.boolean(u8"PersistenceRequired", false);
      c[u8"Persistent"] = Bool(persistenceRequired);
    }
  }

  static void Sittable(CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
    CopyBoolValues(tag, c, {{u8"Sitting"}});
  }

  static void Temper(CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
    CopyIntValues(tag, c, {{u8"Temper"}});
  }
#pragma endregion

#pragma region Converters
  static CompoundTagPtr Animal(CompoundTag const &tag, ConverterContext &ctx) {
    auto c = Mob(tag, ctx);
    if (!c) {
      return nullptr;
    }

    c->set(u8"Persistent", Bool(true));

    if (auto leashIntArray = tag.intArrayTag(u8"leash"); leashIntArray && leashIntArray->fValue.size() == 3) {
      // java 1.20.5 or later
      auto x = leashIntArray->fValue[0];
      auto y = leashIntArray->fValue[1];
      auto z = leashIntArray->fValue[2];
      auto leashedUuid = GetEntityUuid(tag);
      i64 leasherId = UuidRegistrar::LeasherIdFor(*leashedUuid);

      Rep e(leasherId);
      e.fPos = Pos3f(x + 0.5f, y + 0.25f, z + 0.5f);
      e.fIdentifier = u8"minecraft:leash_knot";
      auto leashEntityData = e.toCompoundTag();
      int cx = mcfile::Coordinate::ChunkFromBlock(x);
      int cz = mcfile::Coordinate::ChunkFromBlock(z);
      ctx.fLeashKnots[{cx, cz}].push_back(leashEntityData);

      c->set(u8"LeasherID", Long(leasherId));
    } else if (auto leashCompound = FallbackPtr<CompoundTag>(tag, {u8"leash", u8"Leash"}); leashCompound) {
      if (auto leasherUuid = props::GetUuid(*leashCompound, {.fIntArray = u8"UUID"}); leasherUuid) {
        auto leasherUuidB = UuidRegistrar::ToId(*leasherUuid);
        c->set(u8"LeasherID", Long(leasherUuidB));
      } else {
        auto x = leashCompound->int32(u8"X");
        auto y = leashCompound->int32(u8"Y");
        auto z = leashCompound->int32(u8"Z");
        if (x && y && z) {
          auto leashedUuid = GetEntityUuid(tag);
          i64 leasherId = UuidRegistrar::LeasherIdFor(*leashedUuid);

          Rep e(leasherId);
          e.fPos = Pos3f(*x + 0.5f, *y + 0.25f, *z + 0.5f);
          e.fIdentifier = u8"minecraft:leash_knot";
          auto leashEntityData = e.toCompoundTag();
          int cx = mcfile::Coordinate::ChunkFromBlock(*x);
          int cz = mcfile::Coordinate::ChunkFromBlock(*z);
          ctx.fLeashKnots[{cx, cz}].push_back(leashEntityData);

          c->set(u8"LeasherID", Long(leasherId));
        }
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
    e->fIdentifier = u8"minecraft:ender_crystal";
    auto c = e->toCompoundTag();
    CopyBoolValues(tag, *c, {{u8"ShowBottom"}});

    if (auto beamTarget = tag.compoundTag(u8"BeamTarget"); beamTarget) {
      auto x = beamTarget->int32(u8"X");
      auto y = beamTarget->int32(u8"Y");
      auto z = beamTarget->int32(u8"Z");
      if (x && y && z) {
        c->set(u8"BlockTargetX", Int(*x));
        c->set(u8"BlockTargetY", Int(*y));
        c->set(u8"BlockTargetZ", Int(*z));
      }
    }

    return c;
  }

  static CompoundTagPtr EnderDragon(CompoundTag const &tag, ConverterContext &ctx) {
    auto c = Monster(tag, ctx);
    if (!c) {
      return nullptr;
    }
    AddDefinition(*c, u8"-dragon_sitting");
    AddDefinition(*c, u8"+dragon_flying");
    c->set(u8"Persistent", Bool(true));
    c->set(u8"IsAutonomous", Bool(true));
    c->set(u8"LastDimensionId", Int(2));

    auto dragonDeathTime = tag.int32(u8"DragonDeathTime", 0);
    if (dragonDeathTime > 0) {
      c->set(u8"DeathTime", Int(dragonDeathTime));
      RemoveDefinition(*c, u8"+dragon_flying");
      AddDefinition(*c, u8"-dragon_flying");
      AddDefinition(*c, u8"+dragon_death");
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

    auto item = tag.compoundTag(u8"Item");
    if (!item) {
      return nullptr;
    }
    auto beItem = Item::From(item, ctx.fCtx, ctx.fDataVersion);
    if (!beItem) {
      return nullptr;
    }

    auto ret = e->toCompoundTag();
    ret->set(u8"Item", beItem);

    auto thrower = props::GetUuid(tag, {.fIntArray = u8"Thrower"});
    i64 owner = -1;
    if (thrower) {
      owner = UuidRegistrar::ToId(*thrower);
    }
    ret->set(u8"OwnerID", Long(owner));
    ret->set(u8"OwnerNew", Long(owner));

    CopyShortValues(tag, *ret, {{u8"Health"}, {u8"Age"}});

    return ret;
  }

  static CompoundTagPtr LivingEntity(CompoundTag const &tag, ConverterContext &ctx) {
    auto e = BaseProperties(tag);
    if (!e) {
      return nullptr;
    }
    auto ret = e->toCompoundTag();
    auto &c = *ret;
    auto air = tag.int16(u8"Air", 300);
    auto armor = GetArmor(tag, ctx);
    auto mainhand = GetMainhand(tag, ctx);
    auto offhand = GetOffhand(tag, ctx);
    auto canPickupLoot = tag.boolean(u8"CanPickUpLoot", false);
    auto deathTime = tag.int16(u8"DeathTime", 0);
    auto hurtTime = tag.int16(u8"HurtTime", 0);
    auto inLove = tag.boolean(u8"InLove", false);
    c[u8"Armor"] = armor;
    c[u8"Mainhand"] = mainhand;
    c[u8"Offhand"] = offhand;
    c[u8"Air"] = Short(air);
    c[u8"AttackTime"] = Short(0);
    c[u8"BodyRot"] = Float(0);
    c[u8"boundX"] = Int(0);
    c[u8"boundY"] = Int(0);
    c[u8"boundZ"] = Int(0);
    c[u8"BreedCooldown"] = Int(0);
    c[u8"canPickupItems"] = Bool(canPickupLoot);
    c[u8"Dead"] = Bool(false);
    c[u8"DeathTime"] = Short(deathTime);
    c[u8"hasBoundOrigin"] = Bool(false);
    c[u8"hasSetCanPickupItems"] = Bool(true);
    c[u8"HurtTime"] = Short(hurtTime);
    c[u8"InLove"] = Bool(inLove);
    c[u8"IsPregnant"] = Bool(false);
    c[u8"LeasherID"] = Long(-1);
    c[u8"limitedLife"] = Int(0);
    c[u8"LoveCause"] = Long(0);
    c[u8"NaturalSpawn"] = Bool(false);
    c[u8"Surface"] = Bool(false);
    c[u8"TargetID"] = Long(-1);
    c[u8"TradeExperience"] = Int(0);
    c[u8"TradeTier"] = Int(0);
    if (!e->fIdentifier.empty()) {
      AddDefinition(*ret, u8"+" + e->fIdentifier);
    }
    ret->erase(u8"Motion");
    ret->erase(u8"Dir");
    return ret;
  }

  static CompoundTagPtr Mob(CompoundTag const &tag, ConverterContext &ctx) {
    auto ret = LivingEntity(tag, ctx);
    if (!ret) {
      return ret;
    }

    auto fromBucket = tag.boolean(u8"FromBucket");
    if (fromBucket) {
      ret->set(u8"Persistent", Bool(*fromBucket));
      ret->set(u8"PersistenceRequired", Bool(false));
    } else {
      CopyBoolValues(tag, *ret, {{u8"PersistenceRequired", u8"Persistent", false}});
    }

    if (auto rawId = tag.string(u8"id"); rawId) {
      auto id = MigrateName(*rawId);
      auto health = tag.float32(u8"Health");
      if (id == u8"minecraft:horse" || id == u8"minecraft:donkey" || id == u8"minecraft:mule" || id == u8"minecraft:skeleton_horse" || id == u8"minecraft:zombie_horse") {
        auto attributes = EntityAttributes::AnyHorseFromJava(tag, health);
        if (attributes) {
          ret->set(u8"Attributes", attributes);
        }
      } else {
        auto attributes = EntityAttributes::Mob(id, health);
        if (attributes) {
          ret->set(u8"Attributes", attributes->toBedrockListTag());
        }
      }
    }

    return ret;
  }

  static CompoundTagPtr Monster(CompoundTag const &tag, ConverterContext &ctx) {
    auto c = Mob(tag, ctx);
    if (!c) {
      return nullptr;
    }
    c->set(u8"SpawnedByNight", Bool(false));
    return c;
  }

  static CompoundTagPtr Null(CompoundTag const &tag, ConverterContext &ctx) { return nullptr; }

  static CompoundTagPtr Painting(CompoundTag const &tag, ConverterContext &ctx) {
    auto tileX = tag.int32(u8"TileX");
    auto tileY = tag.int32(u8"TileY");
    auto tileZ = tag.int32(u8"TileZ");
    if (!tileX || !tileY || !tileZ) {
      return nullptr;
    }

    Facing4 f4;
    i8 facing;
    Painting::Motive motive;

    if (auto dir = tag.byte(u8"Dir"); dir) {
      switch (*dir) {
      case 1:
        f4 = Facing4::West;
        break;
      case 2:
        f4 = Facing4::South;
        break;
      case 3:
        f4 = Facing4::East;
        break;
      case 0:
      default:
        f4 = Facing4::North;
        break;
      }
      facing = BedrockDirectionFromFacing4(f4);

      Pos2i vec = Pos2iFromFacing4(f4);
      tileX = *tileX + vec.fX;
      tileZ = *tileZ + vec.fZ;

      motive = Painting::MotiveFromBedrock(tag.string(u8"Motive", u8"Aztec"));
    } else {
      facing = tag.byte(u8"facing", tag.byte(u8"Facing", 0)); // 1.19: "facing", 1.18: "Facing"
      f4 = Facing4FromBedrockDirection(facing);
      auto motiveJ = tag.string(u8"variant", tag.string(u8"Motive", u8"minecraft:aztec")); // 1.19: "variant", 1.18: "Motive"
      motive = Painting::MotiveFromJava(motiveJ);
    }

    auto motiveB = Painting::BedrockFromMotive(motive);

    auto pos = Painting::BedrockPosFromJavaTilePos(Pos3i(*tileX, *tileY, *tileZ), f4, motive);
    if (!pos) {
      return nullptr;
    }

    auto yaw = Rotation::ClampDegreesBetween0And360(YawFromFacing4(f4));
    auto rotation = List<Tag::Type::Float>();
    rotation->push_back(Float(yaw));
    rotation->push_back(Float(0));

    auto e = BaseProperties(tag);
    if (!e) {
      return nullptr;
    }
    e->fIdentifier = u8"minecraft:painting";
    e->fPos = *pos;
    auto c = e->toCompoundTag();
    c->set(u8"Motive", motiveB);
    c->set(u8"Direction", Byte(facing));
    c->set(u8"Rotation", rotation);
    return c;
  }
#pragma endregion

#pragma region BehaviorGenerators
  static Behavior AgeableA(std::u8string const &definitionKey) {
    return [=](CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
      auto age = tag.int32(u8"Age", 0);
      if (age < 0) {
        AddDefinition(c, u8"+" + definitionKey + u8"_baby");
        c[u8"Age"] = Int(age);
      } else {
        AddDefinition(c, u8"+" + definitionKey + u8"_adult");
        c.erase(u8"Age");
      }
      c[u8"IsBaby"] = Bool(age < 0);
    };
  }

  static Behavior AgeableB(std::u8string const &definitionKey) {
    return [=](CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
      auto baby = tag.boolean(u8"IsBaby", false);
      if (baby) {
        AddDefinition(c, u8"+minecraft:" + definitionKey + u8"_baby");
      } else {
        AddDefinition(c, u8"+minecraft:" + definitionKey + u8"_adult");
      }
      c[u8"IsBaby"] = Bool(baby);
    };
  }

  static Behavior AgeableD(std::u8string const &definitionKey) {
    return [=](CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
      auto baby = tag.boolean(u8"IsBaby", false);
      if (baby) {
        AddDefinition(c, u8"+minecraft:baby_" + definitionKey);
      } else {
        AddDefinition(c, u8"+minecraft:adult_" + definitionKey);
      }
      c[u8"IsBaby"] = Bool(baby);
    };
  }

  static Behavior AgeableE(i32 maxBabyAgeJava) {
    return [=](CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
      auto age = tag.int32(u8"Age");
      if (!age) {
        return;
      }
      c[u8"Age"] = Int(*age - maxBabyAgeJava);
    };
  }

  static Behavior Boat(std::u8string const &definitionKey) {
    return [=](CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
      auto type = tag.string(u8"Type", u8"oak");
      i32 variant = Boat::BedrockVariantFromJavaType(type);
      c[u8"Variant"] = Int(variant);

      AddDefinition(c, u8"+minecraft:" + definitionKey);
      AddDefinition(c, u8"+");
      if (type == u8"bamboo") {
        AddDefinition(c, u8"+minecraft:can_ride_bamboo");
      } else {
        AddDefinition(c, u8"+minecraft:can_ride_default");
      }

      auto rotation = props::GetRotation(c, u8"Rotation");
      if (rotation) {
        Rotation rot(Rotation::ClampDegreesBetweenMinus180And180(rotation->fYaw + 90), rotation->fPitch);
        c[u8"Rotation"] = rot.toListTag();
      }

      auto pos = GetBoatPos(tag);
      c[u8"Pos"] = pos->toF().toListTag();
    };
  }

  static Behavior ChestedHorse(std::u8string const &definitionKey) {
    return [=](CompoundTag &c, CompoundTag const &tag, ConverterContext &ctx) {
      auto chested = tag.boolean(u8"ChestedHorse", false);
      c[u8"Chested"] = Bool(chested);
      if (!chested) {
        return;
      }
      auto chestItems = tag.listTag(u8"Items");
      if (!chestItems) {
        return;
      }

      for (auto const &it : *chestItems) {
        auto item = std::dynamic_pointer_cast<CompoundTag>(it);
        if (!item) {
          continue;
        }
        auto slot = item->byte(u8"Slot");
        if (!slot) {
          continue;
        }
        i8 idx;
        if (ctx.fDataVersion >= 3809) {
          // 24w09a
          // 24w06a
          // 24w05a
          idx = *slot + 1;
        } else {
          // 24w04a
          idx = *slot - 1;
        }
        if (idx < 0 || 16 <= idx) {
          continue;
        }
        auto count = Item::Count(*item);
        if (!count) {
          continue;
        }
        auto outItem = Item::From(item, ctx.fCtx, ctx.fDataVersion);
        if (!outItem) {
          continue;
        }
        AddChestItem(c, outItem, idx, *count);
      }
      if (auto armor = tag.compoundTag(u8"body_armor_item"); armor) {
        if (auto converted = Item::From(armor, ctx.fCtx, ctx.fDataVersion); converted) {
          AddChestItem(c, converted, 0, 1);
        }
      }
      AddDefinition(c, u8"-minecraft:" + definitionKey + u8"_unchested");
      AddDefinition(c, u8"+minecraft:" + definitionKey + u8"_chested");
    };
  }

  static Behavior Colorable(std::u8string const &definitionKey) {
    return [=](CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
      auto color = tag.byte(u8"Color", 0);
      c[u8"Color"] = Byte(color);
      CopyByteValues(tag, c, {{u8"Color"}});
      AddDefinition(c, u8"+minecraft:" + definitionKey + u8"_" + BedrockNameFromColorCodeJava((ColorCodeJava)color));
    };
  }

  template <class... Arg>
  static Behavior Definitions(Arg... defs) {
    return [=](CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
      for (std::u8string const &def : std::initializer_list<std::u8string>{defs...}) {
        AddDefinition(c, def);
      }
    };
  }

  static Behavior Offers(int maxTradeTier, std::u8string offersKey) {
    return [maxTradeTier, offersKey](CompoundTag &c, CompoundTag const &tag, ConverterContext &ctx) {
      using namespace std;

      auto offers = tag.compoundTag(u8"Offers");
      if (offers) {
        auto recipes = offers->listTag(u8"Recipes");
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
            converted->set(u8"tier", Int(tier));
            rs->push_back(converted);
          }
          if (!rs->empty()) {
            auto of = Compound();
            of->set(u8"Recipes", rs);
            auto expRequirements = List<Tag::Type::Compound>();
            if (maxTradeTier >= 0) {
              auto tier0 = Compound();
              tier0->set(u8"0", Int(0));
              expRequirements->push_back(tier0);
            }
            if (maxTradeTier >= 1) {
              auto tier1 = Compound();
              tier1->set(u8"1", Int(10));
              expRequirements->push_back(tier1);
            }
            if (maxTradeTier >= 2) {
              auto tier2 = Compound();
              tier2->set(u8"2", Int(70));
              expRequirements->push_back(tier2);
            }
            if (maxTradeTier >= 3) {
              auto tier3 = Compound();
              tier3->set(u8"3", Int(150));
              expRequirements->push_back(tier3);
            }
            if (maxTradeTier >= 4) {
              auto tier4 = Compound();
              tier4->set(u8"4", Int(250));
              expRequirements->push_back(tier4);
            }
            of->set(u8"TierExpRequirements", expRequirements);
            c[offersKey] = of;
          }
        }
      }
    };
  }

  static Behavior Rename(std::u8string const &name) {
    return [=](CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
      auto rawId = tag.string(u8"id");
      if (!rawId) {
        return;
      }
      auto id = MigrateName(*rawId);
      RemoveDefinition(c, u8"+" + id);
      AddDefinition(c, u8"+minecraft:" + name);
      c[u8"identifier"] = String(u8"minecraft:" + name);
    };
  }

  struct SteerableOptions {
    bool fAddAlwaysUnsaddledDefinition = true;
  };
  static Behavior Steerable(std::u8string const &definitionKey, SteerableOptions opt) {
    return [=](CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
      auto saddle = tag.boolean(u8"Saddle", false);
      auto saddleItem = tag.compoundTag(u8"SaddleItem");
      if (saddle || saddleItem) {
        AddDefinition(c, u8"-minecraft:" + definitionKey + u8"_unsaddled");
        AddDefinition(c, u8"+minecraft:" + definitionKey + u8"_saddled");
        if (saddleItem) {
          auto item = Compound();
          item->insert({
              {u8"Damage", Byte(0)},
              {u8"Name", String(u8"minecraft:saddle")},
              {u8"WasPickedUp", Bool(false)},
          });
          AddChestItem(c, item, 0, 1);
        }
      } else if (opt.fAddAlwaysUnsaddledDefinition) {
        AddDefinition(c, u8"+minecraft:" + definitionKey + u8"_unsaddled");
      }
      c[u8"Saddled"] = Bool(saddle);
    };
  }

  static Behavior TameableA(std::u8string const &definitionKey) {
    return [=](CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
      auto owner = GetOwnerUuid(tag);
      if (owner) {
        c[u8"OwnerNew"] = Long(*owner);
        AddDefinition(c, u8"-minecraft:" + definitionKey + u8"_wild");
        AddDefinition(c, u8"+minecraft:" + definitionKey + u8"_tame");
        c[u8"IsTamed"] = Bool(true);
      } else {
        AddDefinition(c, u8"+minecraft:" + definitionKey + u8"_wild");
      }
    };
  }

  static Behavior TameableB(std::u8string const &definitionKey) {
    return [=](CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
      auto owner = GetOwnerUuid(tag);
      if (owner) {
        c[u8"OwnerNew"] = Long(*owner);
        AddDefinition(c, u8"-minecraft:" + definitionKey + u8"_wild");
        AddDefinition(c, u8"+minecraft:" + definitionKey + u8"_tamed");
        c[u8"IsTamed"] = Bool(true);
      } else {
        AddDefinition(c, u8"+minecraft:" + definitionKey + u8"_wild");
      }
    };
  }

  static Behavior Trustable(std::u8string const &definitionKey) {
    return [=](CompoundTag &c, CompoundTag const &tag, ConverterContext &) {
      CopyBoolValues(tag, c, {{u8"Trusting", u8"IsTrusting"}});
      if (tag.boolean(u8"Trusting", false)) {
        AddDefinition(c, u8"+" + definitionKey + u8"_trusting");
      }
    };
  }

  static Behavior Vehicle(std::optional<std::u8string> jockeyDefinitionKey = std::nullopt) {
    return [=](CompoundTag &c, CompoundTag const &tag, ConverterContext &ctx) {
      auto links = List<Tag::Type::Compound>();

      auto passengers = tag.query(u8"Passengers")->asList();
      if (passengers) {
        for (int i = 0; i < passengers->size(); i++) {
          auto const &p = passengers->at(i);
          auto comp = p->asCompound();
          if (!comp) {
            continue;
          }

          auto ret = From(*comp, ctx.fCtx, ctx.fDataVersion, {});
          if (!ret.fEntity) {
            continue;
          }

          auto const &passenger = ret.fEntity;
          auto uid = passenger->int64(u8"UniqueID");
          if (!uid) {
            continue;
          }
          auto link = Compound();
          link->set(u8"entityID", Long(*uid));
          link->set(u8"linkID", Int(i));
          links->push_back(link);

          auto passengerId = passenger->string(u8"identifier");
          if (passengerId && *passengerId == u8"minecraft:zombie") {
            AddDefinition(*passenger, u8"+minecraft:zombie_jockey");
          }

          ctx.fPassengers.push_back(passenger);
        }

        if (jockeyDefinitionKey) {
          AddDefinition(c, u8"+minecraft:" + *jockeyDefinitionKey + u8"_parent_jockey");
        }
      }

      if (!links->empty()) {
        c[u8"LinksTag"] = links;
      }
    };
  }
#pragma endregion

#pragma region Utilities
  static CompoundTagPtr BedrockRecipieFromJava(CompoundTag const &java, ConverterContext &ctx) {
    using namespace std;

    auto buyA = java.compoundTag(u8"buy");
    auto buyB = java.compoundTag(u8"buyB");
    auto sell = java.compoundTag(u8"sell");

    if (!buyA || !sell) {
      return nullptr;
    }
    auto bedrock = Compound();

    {
      auto count = Item::Count(*buyA, 0);
      auto item = Item::From(buyA, ctx.fCtx, ctx.fDataVersion);
      if (item && count > 0) {
        bedrock->set(u8"buyA", item);
        bedrock->set(u8"buyCountA", Int(count));
      } else {
        return nullptr;
      }
    }

    if (buyB) {
      auto count = Item::Count(*buyB, 0);
      auto id = buyB->string(u8"id", u8"minecraft:air");
      auto item = Item::From(buyB, ctx.fCtx, ctx.fDataVersion);
      if (id != u8"minecraft:air" && item && count > 0) {
        bedrock->set(u8"buyB", item);
        bedrock->set(u8"buyCountB", Int(count));
      } else {
        bedrock->set(u8"buyCountB", Int(0));
      }
    } else {
      bedrock->set(u8"buyCountB", Int(0));
    }

    {
      auto count = Item::Count(*sell, 0);
      if (count <= 0) {
        return nullptr;
      }
      auto item = Item::From(sell, ctx.fCtx, ctx.fDataVersion);
      if (!item) {
        return nullptr;
      }
      bedrock->set(u8"sell", item);
    }

    CopyIntValues(java, *bedrock, {{u8"demand"}, {u8"maxUses"}, {u8"uses"}, {u8"xp", u8"traderExp"}});
    CopyByteValues(java, *bedrock, {{u8"rewardExp"}});
    CopyFloatValues(java, *bedrock, {{u8"priceMultiplier", u8"priceMultiplierA"}, {u8"priceMultiplierB"}});

    return bedrock;
  }

  static ListTagPtr InitItemList(u32 capacity) {
    auto items = List<Tag::Type::Compound>();
    for (int i = 0; i < capacity; i++) {
      auto empty = Item::Empty();
      empty->set(u8"Slot", Byte(i));
      empty->set(u8"Count", Byte(0));
      items->push_back(empty);
    }
    return items;
  }

  static ListTagPtr ConvertAnyItemList(ListTagPtr const &input, u32 capacity, Context &ctx, int dataVersion) {
    auto ret = InitItemList(capacity);
    for (auto const &it : *input) {
      auto item = std::dynamic_pointer_cast<CompoundTag>(it);
      if (!item) {
        continue;
      }
      auto converted = Item::From(item, ctx, dataVersion);
      if (!converted) {
        continue;
      }
      auto slot = item->byte(u8"Slot");
      if (!slot) {
        continue;
      }
      if (*slot < 0 || capacity <= *slot) {
        continue;
      }
      auto count = Item::Count(*item);
      if (!count) {
        continue;
      }
      converted->set(u8"Slot", Byte(*slot));
      converted->set(u8"Count", Byte(*count));
      ret->at(*slot) = converted;
    }
    return ret;
  }

  static void AddChestItem(CompoundTag &c, CompoundTagPtr const &item, i8 slot, i8 count) {
    item->set(u8"Slot", Byte(slot));
    item->set(u8"Count", Byte(count));
    auto chestItems = c.listTag(u8"ChestItems");
    if (!chestItems) {
      chestItems = InitItemList(16);
    }
    chestItems->at(slot) = item;
    c[u8"ChestItems"] = chestItems;
  }

  static std::optional<i64> GetOwnerUuid(CompoundTag const &tag) {
    auto uuid = props::GetUuid(tag, {.fIntArray = u8"Owner", .fHexString = u8"OwnerUUID"});
    if (!uuid) {
      return std::nullopt;
    }
    return UuidRegistrar::ToId(*uuid);
  }

  static void AddDefinition(CompoundTag &tag, std::u8string const &definition) {
    auto found = tag.find(u8"definitions");
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
    tag[u8"definitions"] = d;
  }

  static void AddDefinitionFlag(CompoundTag &tag, std::u8string const &body, bool flag) {
    std::u8string definition = (flag ? u8"+" : u8"-") + body;
    AddDefinition(tag, definition);
  }

  static void RemoveDefinition(CompoundTag &tag, std::u8string const &definition) {
    auto found = tag.find(u8"definitions");
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
    tag[u8"definitions"] = d;
  }

  static ListTagPtr GetArmor(CompoundTag const &tag, ConverterContext &ctx) {
    auto armors = List<Tag::Type::Compound>();
    for (int i = 0; i < 4; i++) {
      armors->push_back(Item::Empty());
    }

    if (auto found = tag.find(u8"ArmorItems"); found != tag.end()) {
      auto list = found->second->asList();
      if (list && list->fType == Tag::Type::Compound) {
        for (int i = 0; i < 4 && i < list->size(); i++) {
          auto item = std::dynamic_pointer_cast<CompoundTag>(list->at(i));
          if (!item) {
            continue;
          }
          auto converted = Item::From(item, ctx.fCtx, ctx.fDataVersion);
          if (converted) {
            armors->fValue[i] = converted;
          }
        }
      }
    } else if (auto equipment = tag.listTag(u8"Equipment"); equipment && equipment->fType == Tag::Type::Compound) {
      // legacy
      if (equipment->fValue.size() >= 3) {
        if (auto leggings = std::dynamic_pointer_cast<CompoundTag>(equipment->fValue[2]); leggings) {
          if (auto converted = Item::From(leggings, ctx.fCtx, ctx.fDataVersion); converted) {
            armors->fValue[0] = converted;
          }
        }
      }
      if (equipment->fValue.size() >= 4) {
        if (auto chestplate = std::dynamic_pointer_cast<CompoundTag>(equipment->fValue[3]); chestplate) {
          if (auto converted = Item::From(chestplate, ctx.fCtx, ctx.fDataVersion); converted) {
            armors->fValue[1] = converted;
          }
        }
      }
      if (equipment->fValue.size() >= 5) {
        if (auto helmet = std::dynamic_pointer_cast<CompoundTag>(equipment->fValue[4]); helmet) {
          if (auto converted = Item::From(helmet, ctx.fCtx, ctx.fDataVersion); converted) {
            armors->fValue[2] = converted;
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

    auto mainHand = input.listTag(u8"HandItems");
    CompoundTagPtr item;

    if (mainHand && mainHand->fType == Tag::Type::Compound && index < mainHand->fValue.size()) {
      auto inItem = std::dynamic_pointer_cast<CompoundTag>(mainHand->fValue[index]);
      if (inItem) {
        item = Item::From(inItem, ctx.fCtx, ctx.fDataVersion);
      }
    } else if (auto equipment = input.listTag(u8"Equipment"); equipment && equipment->fType == Tag::Type::Compound && index < equipment->fValue.size()) {
      // legacy
      auto inItem = std::dynamic_pointer_cast<CompoundTag>(equipment->fValue[index]);
      if (inItem) {
        item = Item::From(inItem, ctx.fCtx, ctx.fDataVersion);
      }
    }
    if (!item) {
      item = Item::Empty();
    }
    ret->fValue.push_back(item);

    return ret;
  }

  static std::optional<i64> GetEntityUuid(CompoundTag const &tag) {
    auto uuid = props::GetUuid(tag, {.fLeastAndMostPrefix = u8"UUID", .fIntArray = u8"UUID"});
    if (!uuid) {
      return std::nullopt;
    }
    return UuidRegistrar::ToId(*uuid);
  }

  struct PositionAndFacing {
    Pos3i fPosition;
    i32 fFacing;
  };
  static PositionAndFacing GetItemFrameTilePositionAndFacing(CompoundTag const &c) {
    using namespace std;
    i32 tileX = c.int32(u8"TileX", 0);
    i32 tileY = c.int32(u8"TileY", 0);
    i32 tileZ = c.int32(u8"TileZ", 0);

    i32 facing = 0;
    Pos3i pos(tileX, tileY, tileZ);

    auto dir = c.byte(u8"Dir");
    auto direction = c.byte(u8"Direction");
    if (dir && direction) {
      // 1.7.10: Tile{X,Y,Z} points to the base block of ItemFrame
      Pos3i normal;
      switch (*direction) {
      case 3: // dir = 3
        normal = Pos3iFromFacing6(Facing6::East);
        facing = 5;
        break;
      case 2: // dir = 0
        normal = Pos3iFromFacing6(Facing6::North);
        facing = 2;
        break;
      case 1: // dir = 1
        normal = Pos3iFromFacing6(Facing6::West);
        facing = 4;
        break;
      case 0: // dir = 2
      default:
        normal = Pos3iFromFacing6(Facing6::South);
        facing = 3;
        break;
      }
      pos = pos + normal;
    } else if (auto rot = props::GetRotation(c, u8"Rotation"); rot) {
      // 1.8 or later
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
    }
    PositionAndFacing ret;
    ret.fPosition = pos;
    ret.fFacing = facing;
    return ret;
  }

  static std::optional<Rep> BaseProperties(CompoundTag const &tag) {
    using namespace je2be::props;
    using namespace std;

    auto fallDistance = tag.float32(u8"FallDistance");
    auto fire = tag.int16(u8"Fire");
    auto invulnerable = tag.boolean(u8"Invulnerable");
    auto onGround = tag.boolean(u8"OnGround");
    auto portalCooldown = tag.int32(u8"PortalCooldown");
    auto motion = GetPos3d(tag, u8"Motion");
    auto pos = GetPos3d(tag, u8"Pos");
    auto rotation = GetRotation(tag, u8"Rotation");
    auto uuid = GetEntityUuid(tag);
    auto id = tag.string(u8"id");
    auto customNameJ = tag.string(u8"CustomName");

    if (!uuid) {
      return nullopt;
    }

    Rep e(*uuid);
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
      e.fIdentifier = MigrateName(*id);
    }
    if (customNameJ) {
      auto text = props::GetTextComponent(*customNameJ);
      if (!text.empty()) {
        e.fCustomName = text;
        e.fCustomNameVisible = true;
      }
    }

    return e;
  }
#pragma endregion
};

Entity::Result Entity::From(CompoundTag const &tag, Context &ctx, int dataVersion, std::set<Flag> flags) {
  return Impl::From(tag, ctx, dataVersion, flags);
}

std::optional<std::pair<Pos3i, CompoundTagPtr>> Entity::ToTileEntityBlock(CompoundTag const &c) {
  return Impl::ToTileEntityBlock(c);
}

CompoundTagPtr Entity::ToTileEntityData(CompoundTag const &c, Context &ctx, int dataVersion) {
  return Impl::ToTileEntityData(c, ctx, dataVersion);
}

CompoundTagPtr Entity::ToItemFrameTileEntityData(CompoundTag const &c, Context &ctx, std::u8string const &name, int dataVersion) {
  return Impl::ToItemFrameTileEntityData(c, ctx, name, dataVersion);
}

bool Entity::IsTileEntity(CompoundTag const &tag) {
  return Impl::IsTileEntity(tag);
}

std::optional<Entity::LocalPlayerResult> Entity::LocalPlayer(CompoundTag const &tag, Context &ctx, int dataVersion, std::set<Flag> flags) {
  return Impl::LocalPlayer(tag, ctx, dataVersion, flags);
}

} // namespace je2be::java
