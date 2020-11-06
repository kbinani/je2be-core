#pragma once

namespace j2b {

class Entity {
private:
  using CompoundTag = mcfile::nbt::CompoundTag;
  using EntityData = std::shared_ptr<mcfile::nbt::CompoundTag>;
  using Converter = std::function<EntityData(
      CompoundTag const &, std::vector<EntityData> &passengers,
      JavaEditionMap const &mapInfo, DimensionDataFragment &ddf)>;

  using Behavior =
      std::function<EntityData(EntityData const &, CompoundTag const &input)>;

  struct Convert {
    template <class... Arg>
    Convert(Converter base, Arg... args)
        : fBase(base), fBehaviors(std::initializer_list<Behavior>{args...}) {}

    EntityData operator()(CompoundTag const &input,
                          std::vector<EntityData> &passengers,
                          JavaEditionMap const &mapInfo,
                          DimensionDataFragment &ddf) const {
      auto c = fBase(input, passengers, mapInfo, ddf);
      if (!c)
        return nullptr;
      for (auto const &b : fBehaviors) {
        auto next = b(c, input);
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
  explicit Entity(int64_t uid)
      : fMotion(0, 0, 0), fPos(0, 0, 0), fRotation(0, 0), fUniqueId(uid) {}

  static std::vector<EntityData> From(mcfile::nbt::CompoundTag const &tag,
                                      JavaEditionMap const &mapInfo,
                                      DimensionDataFragment &ddf) {
    using namespace props;
    auto id = GetString(tag, "id");
    if (!id)
      return std::vector<EntityData>();
    static std::unique_ptr<
        std::unordered_map<std::string, Converter> const> const
        table(CreateEntityTable());
    auto found = table->find(*id);
    std::vector<EntityData> ret;
    if (found == table->end()) {
      auto converted = Default(tag);
      if (converted) {
        ret.push_back(converted);
      }
      return ret;
    }
    auto converted = found->second(tag, ret, mapInfo, ddf);
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

  static bool RotAlmostEquals(Rotation const &rot, float yaw, float pitch) {
    return DegAlmostEquals(rot.fYaw, yaw) && DegAlmostEquals(rot.fPitch, pitch);
  }

  static std::tuple<Pos, std::shared_ptr<CompoundTag>, std::string>
  ToTileEntityBlock(CompoundTag const &c) {
    using namespace std;
    using namespace props;
    auto id = GetString(c, "id");
    assert(id);
    if (*id == "minecraft:item_frame") {
      auto tileX = GetInt(c, "TileX");
      auto tileY = GetInt(c, "TileY");
      auto tileZ = GetInt(c, "TileZ");

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
      states->fValue = {
          {"facing_direction", Int(facing)},
          {"item_frame_map_bit", Bool(map)},
      };
      string key = "minecraft:frame[facing_direction=" + to_string(facing) +
                   ",item_frame_map_bit=" + (map ? "true" : "false") + "]";
      b->fValue = {
          {"name", String("minecraft:frame")},
          {"version", Int(BlockData::kBlockDataVersion)},
          {"states", states},
      };
      Pos pos(*tileX, *tileY, *tileZ);
      return make_tuple(pos, b, key);
    }
  }

  static TileEntity::TileEntityData
  ToTileEntityData(CompoundTag const &c, JavaEditionMap const &mapInfo,
                   DimensionDataFragment &ddf) {
    using namespace props;
    auto id = GetString(c, "id");
    assert(id);
    if (*id == "minecraft:item_frame") {
      auto tag = std::make_shared<CompoundTag>();
      auto tileX = GetInt(c, "TileX");
      auto tileY = GetInt(c, "TileY");
      auto tileZ = GetInt(c, "TileZ");
      if (!tileX || !tileY || !tileZ)
        return nullptr;
      tag->fValue = {
          {"id", String("ItemFrame")}, {"isMovable", Bool(true)},
          {"x", Int(*tileX)},          {"y", Int(*tileY)},
          {"z", Int(*tileZ)},
      };
      auto itemRotation = GetByteOrDefault(c, "ItemRotation", 0);
      auto itemDropChange = GetFloatOrDefault(c, "ItemDropChange", 1);
      auto found = c.fValue.find("Item");
      if (found != c.fValue.end() &&
          found->second->id() == mcfile::nbt::Tag::TAG_Compound) {
        auto item = std::dynamic_pointer_cast<CompoundTag>(found->second);
        auto m = Item::From(item, mapInfo, ddf);
        if (m) {
          tag->fValue.insert(make_pair("Item", m));
          tag->fValue.insert(make_pair("ItemRotation", Float(itemRotation)));
          tag->fValue.insert(
              make_pair("ItemDropChange", Float(itemDropChange)));
        }
      }
      return tag;
    }
  }

  static bool IsTileEntity(CompoundTag const &tag) {
    auto id = props::GetString(tag, "id");
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
      definitions->fValue.push_back(String(d));
    }
    tag->fValue = {
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
    };
    if (fCustomName) {
      tag->fValue["CustomName"] = String(*fCustomName);
      tag->fValue["CustomNameVisible"] = Bool(fCustomNameVisible);
    }
    return tag;
  }

private:
  static std::unordered_map<std::string, Converter> *CreateEntityTable() {
    auto table = new std::unordered_map<std::string, Converter>();
#define E(__name, __func)                                                      \
  table->insert(std::make_pair("minecraft:" __name, __func))
#define A(__name) table->insert(std::make_pair("minecraft:" __name, Animal))
#define M(__name) table->insert(std::make_pair("minecraft:" __name, Monster))

    E("painting", Painting);
    E("end_crystal", EndCrystal);

    E("bat", Convert(Mob, Bat));
    A("bee");
    M("blaze");
    E("cat", Convert(Animal, AgeableA("cat"), Tameable("cat"), Sittable,
                     CollarColorable, Cat));
    M("cave_spider");
    A("chicken");
    A("cod");

    E("cow", Convert(Animal, AgeableA("cow")));
    E("creeper", Convert(Monster, Creeper));
    A("dolphin");
    A("donkey");
    M("drownd");
    M("elder_guardian");
    M("enderman");
    M("endermite");
    E("evoker", Convert(Monster, Rename("evocation_illager")));

    A("fox");
    M("ghast");
    M("guardian");
    M("hoglin");
    E("horse", Convert(Animal, AgeableA("horse"), Horse));
    E("husk", Convert(Monster, AgeableA("husk")));
    E("llama", Convert(Animal, AgeableA("llama"), Llama));
    E("magma_cube", Convert(Monster, Slime));
    E("mooshroom", Convert(Animal, AgeableA("mooshroom"), Mooshroom));

    A("mule");
    A("ocelot");
    E("panda", Convert(Animal, AgeableA("panda")));
    E("parrot", Convert(Animal, Parrot));
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
    E("sheep", Convert(Animal, AgeableA("sheep"), Colorable("sheep"),
                       Definitions("+minecraft:sheep_dyeable",
                                   "+minecraft:rideable_wooly",
                                   "+minecraft:loot_wooly")));
    E("shulker", Convert(Monster, Shulker));
    M("silverfish");
    M("skeleton"); // lefty skeleton does not exist in Bedrock?

    A("skeleton_horse");
    E("slime", Convert(Monster, Slime));
    M("spider");
    A("squid");
    M("stray");
    A("strider");
    E("trader_llama",
      Convert(Animal, Rename("llama"), AgeableA("llama"), Llama, TraderLlama));
    E("tropical_fish", Convert(Animal, Rename("tropicalfish"), TropicalFish));
    A("turtle");

    M("vex");
    E("villager", Convert(Animal, Rename("villager_v2")));
    M("vindicator");
    A("wandering_trader");
    M("witch");
    M("wither_skeleton");
    E("wolf", Convert(Animal, Tameable("wolf"), Sittable, CollarColorable));
    M("zoglin");
    E("zombie", Convert(Monster, AgeableB("zombie")));

    M("zombie_horse");
    E("zombie_villager", Convert(Animal, Rename("zombie_villager_v2")));
    E("zombified_piglin",
      Convert(Monster, Rename("zombie_pigman"), AgeableB("pig_zombie")));

    E("boat", Convert(Vehicle, Boat));
    E("minecart",
      Convert(Vehicle, Minecart, Definitions("+minecraft:minecart")));
    E("armor_stand", LivingEntity);
    E("hopper_minecart", Convert(StorageMinecart, Minecart,
                                 Definitions("+minecraft:hopper_minecart")));
    E("chest_minecart", Convert(StorageMinecart, Minecart,
                                Definitions("+minecraft:chest_minecart")));
    E("tnt_minecart",
      Convert(Vehicle, Minecart,
              Definitions("+minecraft:tnt_minecart", "+minecraft:inactive")));
    E("snow_golem", Mob);
    E("iron_golem", Mob);

    E("item", Item);
#undef A
#undef M
#undef E
    return table;
  }

  static EntityData Item(CompoundTag const &tag,
                         std::vector<EntityData> &passengers,
                         JavaEditionMap const &mapInfo,
                         DimensionDataFragment &ddf) {
    using namespace props;
    auto e = BaseProperties(tag);
    if (!e)
      return nullptr;

    auto item = tag.compound("Item");
    if (!item)
      return nullptr;
    auto beItem = Item::From(item, mapInfo, ddf);
    if (!beItem)
      return nullptr;

    auto ret = e->toCompoundTag();
    ret->fValue["Item"] = beItem;

    auto thrower = GetUUID(tag, {.fIntArray = "Thrower"});
    int64_t owner = -1;
    if (thrower) {
      owner = *thrower;
    }
    ret->fValue["OwnerID"] = Long(owner);
    ret->fValue["OwnerNew"] = Long(owner);

    return ret;
  }

  static EntityData Horse(EntityData const &c, CompoundTag const &tag) {
    auto variant = props::GetIntOrDefault(tag, "Variant", 0);
    auto baseColor = 0xf & variant;
    auto markings = 0xf & (variant >> 8);
    c->fValue["Variant"] = props::Int(baseColor);
    c->fValue["MarkVariant"] = props::Int(markings);
    return c;
  }

  static EntityData StorageMinecart(CompoundTag const &tag,
                                    std::vector<EntityData> &passengers,
                                    JavaEditionMap const &mapInfo,
                                    DimensionDataFragment &ddf) {
    using namespace mcfile::nbt;

    auto e = BaseProperties(tag);
    if (!e)
      return nullptr;
    auto c = e->toCompoundTag();

    auto chestItems = std::make_shared<ListTag>();
    chestItems->fType = Tag::TAG_Compound;

    auto items = props::GetList(tag, "Items");
    if (items) {
      for (auto const &it : items->fValue) {
        if (it->id() != Tag::TAG_Compound)
          continue;
        auto item = std::dynamic_pointer_cast<CompoundTag>(it);
        if (!item)
          continue;
        auto converted = Item::From(item, mapInfo, ddf);
        if (!converted)
          continue;
        auto slot = props::GetByte(*item, "Slot");
        if (!slot)
          continue;
        converted->fValue["Slot"] = props::Byte(*slot);
        chestItems->fValue.push_back(converted);
      }
    }

    c->fValue["ChestItems"] = chestItems;

    return c;
  }

  static EntityData Parrot(EntityData const &c, CompoundTag const &tag) {
    auto variant = props::GetIntOrDefault(tag, "Variant", 0);
    c->fValue["Variant"] = props::Int(variant);
    return c;
  }

  static EntityData Minecart(EntityData const &c, CompoundTag const &tag) {
    auto pos = props::GetVec(tag, "Pos");
    auto onGround = props::GetBool(tag, "OnGround");
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
      c->fValue["Pos"] = pos->toListTag();
    }
    return c;
  }

  static EntityData TropicalFish(EntityData const &c, CompoundTag const &tag) {
    auto variant = props::GetIntOrDefault(tag, "Variant", 0);
    auto small = (0xf & variant) == 0;
    auto pattern = 0xf & (variant >> 8);
    auto bodyColor = 0xf & (variant >> 16);
    auto patternColor = 0xf & (variant >> 24);

    c->fValue["Variant"] = props::Int(small ? 0 : 1);
    c->fValue["MarkVariant"] = props::Int(pattern);
    c->fValue["Color"] = props::Byte(bodyColor);
    c->fValue["Color2"] = props::Byte(patternColor);
    return c;
  }

  static EntityData TraderLlama(EntityData const &c, CompoundTag const &tag) {
    using namespace mcfile::nbt;
    AddDefinition(c, "+minecraft:llama_wandering_trader");
    AddDefinition(c, "-minecraft:llama_wild");
    AddDefinition(c, "+minecraft:llama_tamed");
    AddDefinition(c, "+minecraft:strength_3");
    AddDefinition(c, "+minecraft:llama_unchested");
    c->fValue["InventoryVersion"] = props::String("1.16.40");
    auto chestItems = std::make_shared<ListTag>();
    chestItems->fType = Tag::TAG_Compound;
    for (int i = 0; i < 16; i++) {
      auto item = Item::Empty();
      item->fValue["Slot"] = props::Byte(i);
      chestItems->fValue.push_back(item);
    }
    c->fValue["ChestItems"] = chestItems;
    c->fValue["MarkVariant"] = props::Int(1);
    c->fValue["IsTamed"] = props::Bool(true);
    return c;
  }

  static EntityData Llama(EntityData const &c, CompoundTag const &tag) {
    auto variant = props::GetIntOrDefault(tag, "Variant", 0);
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
    c->fValue["Variant"] = props::Int(variant);
    AddDefinition(c, "+minecraft:llama_" + color);
    return c;
  }

  static EntityData Shulker(EntityData const &c, CompoundTag const &tag) {
    auto color = props::GetByteOrDefault(tag, "Color", 16);
    if (0 <= color && color <= 15) {
      c->fValue["Color"] = props::Byte(color);
      AddDefinition(c, "+minecraft:shulker_" +
                           BedrockNameFromColorCodeJava((ColorCodeJava)color));
    } else {
      AddDefinition(c, "+minecraft:shulker_undyed");
    }
    c->fValue["Variant"] = props::Int(color);
    return c;
  }

  template <class... Arg> static Behavior Definitions(Arg... defs) {
    return [=](EntityData const &c, CompoundTag const &tag) {
      for (std::string const &def :
           std::initializer_list<std::string>{defs...}) {
        AddDefinition(c, def);
      }
      return c;
    };
  }

  static Behavior Colorable(std::string const &definitionKey) {
    return [=](EntityData const &c, CompoundTag const &tag) {
      auto color = props::GetByteOrDefault(tag, "Color", 0);
      c->fValue["Color"] = props::Byte(color);
      AddDefinition(c, "+minecraft:" + definitionKey + "_" +
                           BedrockNameFromColorCodeJava((ColorCodeJava)color));
      return c;
    };
  }

  static EntityData Rabbit(EntityData const &c, CompoundTag const &tag) {
    auto type = props::GetIntOrDefault(tag, "RabbitType", 0);
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
      c->fValue["CustomName"] = props::String("The Killer Bunny");
    }
    c->fValue["Variant"] = props::Int(variant);
    AddDefinition(c, "+coat_" + coat);
    return c;
  }

  static EntityData Debug(EntityData const &c, CompoundTag const &tag) {
    return c;
  }

  static EntityData CollarColorable(EntityData const &c,
                                    CompoundTag const &tag) {
    auto collarColor = props::GetByte(tag, "CollarColor");
    if (collarColor && GetOwnerUUID(tag)) {
      c->fValue["Color"] = props::Byte(*collarColor);
    }
    return c;
  }

  static Behavior Steerable(std::string const &definitionKey) {
    return [=](EntityData const &c, CompoundTag const &tag) {
      auto saddle = props::GetBoolOrDefault(tag, "Saddle", false);
      if (saddle) {
        AddDefinition(c, "-minecraft:" + definitionKey + "_unsaddled");
        AddDefinition(c, "+minecraft:" + definitionKey + "_saddled");
      } else {
        AddDefinition(c, "+minecraft:" + definitionKey + "_unsaddled");
      }
      c->fValue["Saddled"] = props::Bool(saddle);
      return c;
    };
  }

  static EntityData Mooshroom(EntityData const &c, CompoundTag const &tag) {
    auto type = props::GetStringOrDefault(tag, "Type", "red");
    int32_t variant = 0;
    if (type == "brown") {
      variant = 1;
      AddDefinition(c, "+minecraft:mooshroom_brown");
    } else {
      AddDefinition(c, "+minecraft:mooshroom_red");
    }
    c->fValue["Variant"] = props::Int(variant);
    return c;
  }

  static EntityData Slime(EntityData const &c, CompoundTag const &tag) {
    auto size = props::GetInt(tag, "Size");
    int8_t variant = 0;
    if (size) {
      variant = int8_t(*size) + 1;
    }
    c->fValue["Variant"] = props::Int(variant);
    c->fValue["Size"] = props::Byte(variant);
    return c;
  }

  static EntityData Vehicle(CompoundTag const &tag,
                            std::vector<EntityData> &out,
                            JavaEditionMap const &mapInfo,
                            DimensionDataFragment &ddf) {
    using namespace mcfile::nbt;

    auto e = BaseProperties(tag);
    if (!e)
      return nullptr;

    auto links = std::make_shared<ListTag>();
    links->fType = Tag::TAG_Compound;

    auto passengers = tag.query("Passengers")->asList();
    if (passengers) {
      for (int i = 0; i < passengers->fValue.size(); i++) {
        auto const &p = passengers->fValue[i];
        auto comp = p->asCompound();
        if (!comp)
          continue;

        auto entities = From(*comp, mapInfo, ddf);
        if (entities.empty())
          continue;

        auto const &passenger = entities[0];
        auto uid = props::GetLong(*passenger, "UniqueID");
        if (!uid)
          continue;
        auto link = std::make_shared<CompoundTag>();
        link->fValue["entityID"] = props::Long(*uid);
        link->fValue["linkID"] = props::Int(i);
        links->fValue.push_back(link);

        out.push_back(passenger);
      }
    }

    auto c = e->toCompoundTag();
    c->fValue["LinksTag"] = links;
    return c;
  }

  static EntityData Boat(EntityData const &c, CompoundTag const &tag) {
    using namespace mcfile::nbt;

    AddDefinition(c, "+minecraft:boat");

    auto type = props::GetStringOrDefault(tag, "Type", "oak");
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
    c->fValue["Variant"] = props::Int(variant);

    auto rotation = props::GetRotation(*c, "Rotation");
    if (rotation) {
      Rotation rot(rotation->fYaw + 90, rotation->fPitch);
      c->fValue["Rotation"] = rot.toListTag();
    }

    auto pos = props::GetVec(*c, "Pos");
    auto onGround = props::GetBoolOrDefault(*c, "OnGround", false);
    if (pos && onGround) {
      int iy = (int)floor(pos->fY);
      pos->fY = iy + 0.35f;
      c->fValue["Pos"] = pos->toListTag();
    }

    return c;
  }

  static Behavior Rename(std::string const &name) {
    return [=](EntityData const &c, CompoundTag const &tag) {
      auto id = props::GetString(tag, "id");
      if (!id)
        return c;
      RemoveDefinition(c, "+" + *id);
      AddDefinition(c, "+minecraft:" + name);
      c->fValue["identifier"] = props::String("minecraft:" + name);
      return c;
    };
  }

  static EntityData Creeper(EntityData const &c, CompoundTag const &tag) {
    using namespace props;
    auto powered = GetBoolOrDefault(tag, "powered", false);
    if (powered) {
      AddDefinition(c, "+minecraft:charged_creeper");
      AddDefinition(c, "+minecraft:exploding");
    }
    return c;
  }

  static EntityData Monster(CompoundTag const &tag,
                            std::vector<EntityData> &passengers,
                            JavaEditionMap const &mapInfo,
                            DimensionDataFragment &ddf) {
    auto c = Mob(tag, passengers, mapInfo, ddf);
    c->fValue["SpawnedByNight"] = props::Bool(false);
    auto persistenceRequired =
        props::GetBoolOrDefault(tag, "PersistenceRequired", true);
    bool persistent = false;
    if (persistenceRequired && props::GetString(tag, "CustomName")) {
      persistent = true;
    }
    c->fValue["Persistent"] = props::Bool(persistent);
    return c;
  }

  static EntityData Animal(CompoundTag const &tag,
                           std::vector<EntityData> &passengers,
                           JavaEditionMap const &mapInfo,
                           DimensionDataFragment &ddf) {
    auto c = Mob(tag, passengers, mapInfo, ddf);
    c->fValue["Persistent"] = props::Bool(true);

    auto leash = props::GetCompound(tag, "Leash");
    if (leash) {
      auto x = props::GetInt(*leash, "X");
      auto y = props::GetInt(*leash, "Y");
      auto z = props::GetInt(*leash, "Z");
      auto uuid = GetEntityUUID(tag);
      if (x && y && z && uuid) {
        int64_t targetUUID = *uuid;
        int64_t leasherId = XXHash::Digest(&targetUUID, sizeof(targetUUID));

        Entity e(leasherId);
        e.fPos = Vec(*x + 0.5f, *y + 0.25f, *z + 0.5f);
        e.fIdentifier = "minecraft:leash_knot";
        auto leashEntityData = e.toCompoundTag();
        passengers.push_back(leashEntityData);

        c->fValue["LeasherID"] = props::Long(leasherId);
      }
    }

    return c;
  }

  static EntityData Bat(EntityData const &c, CompoundTag const &tag) {
    using namespace mcfile::nbt;
    auto batFlags = props::GetBoolOrDefault(tag, "BatFlags", false);
    c->fValue["BatFlags"] = props::Bool(batFlags);
    AddDefinition(c, "+minecraft:bat");
    return c;
  }

  static std::optional<int64_t> GetOwnerUUID(CompoundTag const &tag) {
    return props::GetUUID(tag,
                          {.fIntArray = "Owner", .fHexString = "OwnerUUID"});
  }

  static EntityData Cat(EntityData const &c, CompoundTag const &tag) {
    using namespace mcfile::nbt;
    using namespace std;
    auto catType = props::GetInt(tag, "CatType");
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
      c->fValue["Variant"] = props::Int(variant);
    }
    c->fValue["DwellingUniqueID"] =
        props::String("00000000-0000-0000-0000-000000000000");
    c->fValue["RewardPlayersOnFirstFounding"] = props::Bool(true);
    return c;
  }

  static Behavior AgeableA(std::string const &definitionKey) {
    return [=](EntityData const &c, CompoundTag const &tag) {
      auto age = props::GetIntOrDefault(tag, "Age", 0);
      if (age < 0) {
        AddDefinition(c, "+minecraft:" + definitionKey + "_baby");
        c->fValue["Age"] = props::Int(age);
      } else {
        AddDefinition(c, "+minecraft:" + definitionKey + "_adult");
        c->fValue.erase("Age");
      }
      c->fValue["IsBaby"] = props::Bool(age < 0);
      return c;
    };
  }

  static Behavior AgeableB(std::string const &definitionKey) {
    return [=](EntityData const &c, CompoundTag const &tag) {
      auto baby = props::GetBoolOrDefault(tag, "IsBaby", false);
      if (baby) {
        AddDefinition(c, "+minecraft:" + definitionKey + "_baby");
      } else {
        AddDefinition(c, "+minecraft:" + definitionKey + "_adult");
      }
      c->fValue["IsBaby"] = props::Bool(baby);
      return c;
    };
  }

  static EntityData AgeableC(EntityData const &c, CompoundTag const &tag) {
    auto age = props::GetIntOrDefault(tag, "Age", 0);
    if (age < 0) {
      AddDefinition(c, "+baby");
      c->fValue["Age"] = props::Int(age);
    } else {
      AddDefinition(c, "+adult");
      c->fValue.erase("Age");
    }
    c->fValue["IsBaby"] = props::Bool(age < 0);
    return c;
  }

  static Behavior Tameable(std::string const &definitionKey) {
    return [=](EntityData const &c, CompoundTag const &tag) {
      auto owner = GetOwnerUUID(tag);
      if (owner) {
        c->fValue["OwnerNew"] = props::Long(*owner);
        AddDefinition(c, "+minecraft:" + definitionKey + "_tame");
        c->fValue["IsTamed"] = props::Bool(true);
      }
      AddDefinition(c, "+minecraft:" + definitionKey + "_wild");
      return c;
    };
  }

  static void AddDefinition(EntityData const &c,
                            std::string const &definition) {
    using namespace mcfile::nbt;

    auto found = c->fValue.find("definitions");
    auto d = std::make_shared<ListTag>();
    d->fType = Tag::TAG_String;
    if (found != c->fValue.end()) {
      auto current = found->second->asList();
      if (current && current->fType == Tag::TAG_String) {
        for (auto c : current->fValue) {
          if (c->asString()) {
            d->fValue.push_back(props::String(c->asString()->fValue));
          }
        }
      }
    }
    d->fValue.push_back(props::String(definition));
    c->fValue["definitions"] = d;
  }

  static void RemoveDefinition(EntityData const &c,
                               std::string const &definition) {
    using namespace mcfile::nbt;

    auto found = c->fValue.find("definitions");
    auto d = std::make_shared<ListTag>();
    d->fType = Tag::TAG_String;
    if (found != c->fValue.end()) {
      auto current = found->second->asList();
      if (current && current->fType == Tag::TAG_String) {
        for (auto c : current->fValue) {
          if (c->asString() && c->asString()->fValue != definition) {
            d->fValue.push_back(props::String(c->asString()->fValue));
          }
        }
      }
    }
    c->fValue["definitions"] = d;
  }

  static EntityData Sittable(EntityData const &c, CompoundTag const &tag) {
    auto sitting = props::GetBoolOrDefault(tag, "Sitting", false);
    c->fValue["Sitting"] = props::Bool(sitting);
    return c;
  }

  static EntityData LivingEntity(CompoundTag const &tag,
                                 std::vector<EntityData> &passengers,
                                 JavaEditionMap const &mapInfo,
                                 DimensionDataFragment &ddf) {
    using namespace props;
    auto e = BaseProperties(tag);
    if (!e)
      return nullptr;
    auto ret = e->toCompoundTag();
    auto &c = *ret;
    auto air = GetShortOrDefault(tag, "Air", 300);
    auto armor = GetArmor(tag, mapInfo, ddf);
    auto mainhand = GetMainhand(tag, mapInfo, ddf);
    auto offhand = GetOffhand(tag, mapInfo, ddf);
    auto canPickupLoot = GetBoolOrDefault(tag, "CanPickUpLoot", false);
    auto deathTime = GetShortOrDefault(tag, "DeathTime", 0);
    auto hurtTime = GetShortOrDefault(tag, "HurtTime", 0);
    auto inLove = GetBoolOrDefault(tag, "InLove", false);
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
    ret->fValue.erase("Motion");
    ret->fValue.erase("Dir");
    return ret;
  }

  static EntityData Mob(CompoundTag const &tag,
                        std::vector<EntityData> &passengers,
                        JavaEditionMap const &mapInfo,
                        DimensionDataFragment &ddf) {
    using namespace props;
    auto ret = LivingEntity(tag, passengers, mapInfo, ddf);
    if (!ret)
      return ret;

    auto id = GetString(tag, "id");
    if (id) {
      if (*id == "minecraft:horse" || *id == "minecraft:donkey" ||
          *id == "minecraft:mule" || *id == "minecraft:skeleton_horse" ||
          *id == "minecraft:zombie_horse") {
        auto attributes = EntityAttributes::AnyHorse(tag);
        if (attributes) {
          ret->fValue["Attributes"] = attributes;
        }
      } else {
        auto attributes = EntityAttributes::Mob(*id);
        if (attributes) {
          ret->fValue["Attributes"] = attributes;
        }
      }
    }

    return ret;
  }

  static std::shared_ptr<mcfile::nbt::ListTag>
  GetArmor(CompoundTag const &tag, JavaEditionMap const &mapInfo,
           DimensionDataFragment &ddf) {
    using namespace mcfile::nbt;
    auto armors = std::make_shared<ListTag>();
    armors->fType = Tag::TAG_Compound;

    if (props::GetStringOrDefault(tag, "id", "") == "minecraft:armor_stand") {
      int a = 0;
    }

    auto found = tag.fValue.find("ArmorItems");
    if (found != tag.fValue.end()) {
      auto list = found->second->asList();
      if (list && list->fType == Tag::TAG_Compound) {
        for (int i = 0; i < 4 && i < list->fValue.size(); i++) {
          auto item = std::dynamic_pointer_cast<CompoundTag>(list->fValue[i]);
          if (item) {
            auto converted = Item::From(item, mapInfo, ddf);
            armors->fValue.push_back(converted);
          } else {
            armors->fValue.push_back(Item::Empty());
          }
        }
      }
    }
    for (int i = 0; i < 4; i++) {
      if (armors->fValue.size() < i + 1) {
        armors->fValue.push_back(Item::Empty());
      } else {
        if (!armors->fValue[i]) {
          armors->fValue[i] = Item::Empty();
        }
      }
    }

    auto ret = std::make_shared<ListTag>();
    ret->fType = Tag::TAG_Compound;
    ret->fValue.push_back(armors->fValue[3]);
    ret->fValue.push_back(armors->fValue[2]);
    ret->fValue.push_back(armors->fValue[1]);
    ret->fValue.push_back(armors->fValue[0]);

    return ret;
  }

  static std::shared_ptr<mcfile::nbt::ListTag>
  GetMainhand(CompoundTag const &input, JavaEditionMap const &mapInfo,
              DimensionDataFragment &ddf) {
    return HandItem<0>(input, mapInfo, ddf);
  }

  static std::shared_ptr<mcfile::nbt::ListTag>
  GetOffhand(CompoundTag const &input, JavaEditionMap const &mapInfo,
             DimensionDataFragment &ddf) {
    return HandItem<1>(input, mapInfo, ddf);
  }

  template <size_t index>
  static std::shared_ptr<mcfile::nbt::ListTag>
  HandItem(CompoundTag const &input, JavaEditionMap const &mapInfo,
           DimensionDataFragment &ddf) {
    using namespace mcfile::nbt;
    auto ret = std::make_shared<ListTag>();
    ret->fType = Tag::TAG_Compound;

    auto mainHand = props::GetList(input, "HandItems");
    std::shared_ptr<CompoundTag> item;

    if (mainHand && mainHand->fType == Tag::TAG_Compound &&
        index < mainHand->fValue.size()) {
      auto inItem =
          std::dynamic_pointer_cast<CompoundTag>(mainHand->fValue[index]);
      if (inItem) {
        item = Item::From(inItem, mapInfo, ddf);
      }
    }
    if (!item) {
      item = Item::Empty();
    }
    ret->fValue.push_back(item);

    return ret;
  }

  static std::optional<int64_t> GetEntityUUID(CompoundTag const &tag) {
    return props::GetUUID(tag,
                          {.fLeastAndMostPrefix = "UUID", .fIntArray = "UUID"});
  }

  static std::optional<Entity> BaseProperties(CompoundTag const &tag) {
    using namespace props;
    using namespace mcfile::nbt;
    using namespace std;

    auto fallDistance = GetFloat(tag, "FallDistance");
    auto fire = GetShort(tag, "Fire");
    auto invulnerable = GetBool(tag, "Invulnerable");
    auto onGround = GetBool(tag, "OnGround");
    auto portalCooldown = GetInt(tag, "PortalCooldown");
    auto motion = GetVec(tag, "Motion");
    auto pos = GetVec(tag, "Pos");
    auto rotation = GetRotation(tag, "Rotation");
    auto uuid = GetEntityUUID(tag);
    auto id = GetString(tag, "id");
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
    if (customName && (*customName)["text"].is_string()) {
      auto text = (*customName)["text"].get<std::string>();
      e.fCustomName = text;
      e.fCustomNameVisible = true;
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

  static EntityData EndCrystal(CompoundTag const &tag,
                               std::vector<EntityData> &passengers,
                               JavaEditionMap const &mapInfo,
                               DimensionDataFragment &ddf) {
    auto e = BaseProperties(tag);
    if (!e)
      return nullptr;
    e->fIdentifier = "minecraft:ender_crystal";
    auto c = e->toCompoundTag();
    return c;
  }

  static EntityData Painting(CompoundTag const &tag, std::vector<EntityData> &,
                             JavaEditionMap const &mapInfo,
                             DimensionDataFragment &ddf) {
    using namespace props;
    using namespace mcfile::nbt;
    using namespace std;

    auto facing = GetByte(tag, "Facing");
    auto motive = GetString(tag, "Motive");
    auto beMotive = PaintingMotive(*motive);
    auto size = PaintingSize(*motive);
    if (!beMotive || !size)
      return nullptr;

    auto tileX = GetInt(tag, "TileX");
    auto tileY = GetInt(tag, "TileY");
    auto tileZ = GetInt(tag, "TileZ");
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
    c->fValue.emplace("Motive", String(*beMotive));
    c->fValue.emplace("Direction", Byte(*facing));
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
