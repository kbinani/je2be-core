#include "bedrock/_block-data.hpp"

#include <je2be/nbt.hpp>
#include <je2be/strings.hpp>

#include "_closed-range.hpp"
#include "_namespace.hpp"
#include "bedrock/_legacy-block.hpp"
#include "block/_trial-spawner.hpp"
#include "enums/_color-code-java.hpp"
#include "enums/_facing4.hpp"
#include "enums/_facing6.hpp"
#include "enums/_red-flower.hpp"

namespace je2be::bedrock {

class BlockData::Impl {
  using String = std::u8string;
  using Props = std::map<std::u8string, std::u8string>;
  using Converter = std::function<String(String const &bName, CompoundTag const &s, Props &p)>;
  using Behavior = std::function<void(CompoundTag const &s, Props &p)>;

  struct C {
    template <class... Arg>
    C(Converter base, Arg... args) : fBase(base), fBehaviors(std::initializer_list<Behavior>{args...}) {}

    String operator()(String const &bName, CompoundTag const &input, Props &p) const {
      auto name = fBase(bName, input, p);
      for (auto const &b : fBehaviors) {
        b(input, p);
      }
      return name;
    }

  private:
    Converter fBase;
    std::vector<Behavior> fBehaviors;
  };

public:
  static std::shared_ptr<mcfile::je::Block const> From(mcfile::be::Block const &b, int dataVersion) {
    using namespace std;
    using namespace mcfile;
    static unique_ptr<unordered_map<u8string_view, Converter> const> const sTable(CreateTable());

    mcfile::be::Block migrated = b;
    LegacyBlock::Migrate(migrated);

    u8string_view key(migrated.fName);
    auto found = sTable->find(Namespace::Remove(key));
    if (found == sTable->end()) {
      return Identity(migrated, dataVersion);
    } else {
      static CompoundTag const sEmpty;
      CompoundTag const &states = migrated.fStates ? *migrated.fStates : sEmpty;
      map<u8string, u8string> props;
      auto name = found->second(migrated.fName, states, props);
      return mcfile::je::Block::FromNameAndProperties(name, dataVersion, props);
    }
  }

  static std::shared_ptr<mcfile::je::Block const> Identity(mcfile::be::Block const &b, int dataVersion) {
    return mcfile::je::Block::FromName(b.fName, dataVersion);
  }

private:
  Impl() = delete;

#pragma region Converters : A
  static String Anvil(String const &bName, CompoundTag const &s, Props &p) {
    auto damage = s.string(u8"damage", u8"undamaged");
    std::u8string name = u8"anvil";
    if (damage == u8"slightly_damaged") {
      name = u8"chipped_anvil";
    } else if (damage == u8"very_damaged") {
      name = u8"damaged_anvil";
    } else {
      // undamaged
      name = u8"anvil";
    }
    Facing4FromCardinalDirectionMigratingDirectionA(s, p);
    return Ns() + name;
  }

  static String AzaleaLeavesFlowered(String const &bName, CompoundTag const &s, Props &p) {
    PersistentFromPersistentBit(s, p);
    Submergible(s, p);
    return Ns() + u8"flowering_azalea_leaves";
  }
#pragma endregion

#pragma region Converters : B
  static String Bamboo(String const &bName, CompoundTag const &s, Props &p) {
    auto leafSize = s.string(u8"bamboo_leaf_size", u8"large_leaves");
    std::u8string leaves = u8"none";
    if (leafSize == u8"no_leaves") {
      leaves = u8"none";
    } else if (leafSize == u8"large_leaves") {
      leaves = u8"large";
    } else if (leafSize == u8"small_leaves") {
      leaves = u8"small";
    }
    p[u8"leaves"] = leaves;

    auto stalkThickness = s.string(u8"bamboo_stalk_thickness", u8"thin");
    int age = 0; // thin
    if (stalkThickness == u8"thick") {
      age = 1;
    }
    p[u8"age"] = Int(age);

    auto ageBit = s.boolean(u8"age_bit", false);
    p[u8"stage"] = ageBit ? u8"1" : u8"0";
    return bName;
  }

  static String Barrel(String const &bName, CompoundTag const &s, Props &p) {
    OpenFromOpenBit(s, p);
    Facing6FromFacingDirectionA(s, p);
    return bName;
  }

  static String Beehive(String const &bName, CompoundTag const &s, Props &p) {
    Facing4FromDirectionA(s, p);
    auto honeyLevel = s.int32(u8"honey_level", 0);
    p[u8"honey_level"] = Int(honeyLevel);
    return bName;
  }

  static String Beetroot(String const &bName, CompoundTag const &s, Props &p) {
    AgeFromGrowthNonLinear(s, p);
    return Ns() + u8"beetroots";
  }

  static String Bed(String const &bName, CompoundTag const &s, Props &p) {
    Facing4FromDirectionA(s, p);
    auto headPiece = s.boolean(u8"head_piece_bit", false);
    auto occupied = s.boolean(u8"occupied_bit", false);
    p[u8"part"] = headPiece ? u8"head" : u8"foot";
    p[u8"occupied"] = Bool(occupied);
    return Ns() + u8"white_bed";
  }

  static String Bell(String const &bName, CompoundTag const &s, Props &p) {
    auto attachment = s.string(u8"attachment", u8"floor");
    auto direction = s.int32(u8"direction", 0);
    auto toggle = s.boolean(u8"toggle_bit", false);
    std::u8string facing;
    switch (direction) {
    case 1:
      facing = u8"east";
      break;
    case 2:
      facing = u8"south";
      break;
    case 3:
      facing = u8"west";
      break;
    case 0:
    default:
      facing = u8"north";
      break;
    }
    std::u8string a;
    if (attachment == u8"standing") {
      a = u8"floor";
    } else if (attachment == u8"hanging") {
      a = u8"ceiling";
    } else if (attachment == u8"multiple") {
      a = u8"double_wall";
    } else if (attachment == u8"side") {
      a = u8"single_wall";
    } else {
      a = attachment;
    }
    p[u8"facing"] = facing;
    p[u8"attachment"] = a;
    p[u8"powered"] = Bool(toggle);
    return bName;
  }

  static String BigDripleaf(String const &bName, CompoundTag const &s, Props &p) {
    auto head = s.boolean(u8"big_dripleaf_head", false);
    Facing4FromCardinalDirectionMigratingDirectionA(s, p);
    std::u8string name;
    if (head) {
      // none,partial_tilt,unstable,full_tilt => none,partial,unstable,full
      auto tilt = s.string(u8"big_dripleaf_tilt", u8"full_tilt");
      std::u8string t;
      if (tilt == u8"partial_tilt") {
        t = u8"partial";
      } else if (tilt == u8"full_tilt") {
        t = u8"full";
      } else {
        t = tilt;
      }
      p[u8"tilt"] = t;
      name = u8"big_dripleaf";
    } else {
      name = u8"big_dripleaf_stem";
    }
    Submergible(s, p);
    return Ns() + name;
  }

  static String BlockWithAge(String const &bName, CompoundTag const &s, Props &p) {
    Age(s, p);
    return bName;
  }

  static String BlockWithAgeFromGrowth(String const &bName, CompoundTag const &s, Props &p) {
    AgeFromGrowth(s, p);
    return bName;
  }

  static String BlockWithAxisFromPillarAxis(String const &bName, CompoundTag const &s, Props &p) {
    AxisFromPillarAxis(s, p);
    return bName;
  }

  static String BlockWithFacing4FromDirectionA(String const &bName, CompoundTag const &s, Props &p) {
    Facing4FromDirectionA(s, p);
    return bName;
  }

  static String BlockWithFacing4FromFacingDirectionA(String const &bName, CompoundTag const &s, Props &p) {
    Facing4FromFacingDirectionA(s, p);
    return bName;
  }

  static String BlockWithFacing6FromFacingDirectionA(String const &bName, CompoundTag const &s, Props &p) {
    Facing6FromFacingDirectionA(s, p);
    return bName;
  }

  static String BlockWithFacing6FromFacingDirectionASubmergible(String const &bName, CompoundTag const &s, Props &p) {
    Facing6FromFacingDirectionA(s, p);
    Submergible(s, p);
    return bName;
  }

  static String BlockWithFacing6FromBlockFaceMigratingFacingDirectionASubmergible(String const &bName, CompoundTag const &s, Props &p) {
    Facing6FromBlockFaceMigratingFacingDirectionA(s, p);
    Submergible(s, p);
    return bName;
  }

  static String BlockWithFacing4FromFacingDirectionASubmergible(String const &bName, CompoundTag const &s, Props &p) {
    Facing4FromFacingDirectionA(s, p);
    Submergible(s, p);
    return bName;
  }

  static String BlockWithFacing4FromCardinalDirectionMigratingFacingDirectionASubmergible(String const &bName, CompoundTag const &s, Props &p) {
    Facing4FromCardinalDirectionMigratingFacingDirectionA(s, p);
    Submergible(s, p);
    return bName;
  }

  static String BlockWithFacing6FromFacingDirection6(String const &bName, CompoundTag const &s, Props &p) {
    Facing6FromFacingDirectionB(s, p);
    return bName;
  }

  static String BlockWithMultiFaceDirectionBitsSubmergible(String const &bName, CompoundTag const &s, Props &p) {
    auto bits = s.int32(u8"multi_face_direction_bits", 0);
    bool down = (bits & 0x1) == 0x1;
    bool up = (bits & 0x2) == 0x2;
    bool north = (bits & 0x10) == 0x10;
    bool south = (bits & 0x4) == 0x4;
    bool west = (bits & 0x8) == 0x8;
    bool east = (bits & 0x20) == 0x20;
    p[u8"down"] = Bool(down);
    p[u8"up"] = Bool(up);
    p[u8"north"] = Bool(north);
    p[u8"south"] = Bool(south);
    p[u8"west"] = Bool(west);
    p[u8"east"] = Bool(east);

    Submergible(s, p);

    return bName;
  }

  static String BlockWithPersistentFromPersistentBitSubmergible(String const &bName, CompoundTag const &s, Props &p) {
    PersistentFromPersistentBit(s, p);
    Submergible(s, p);
    return bName;
  }

  static String BlockWithPowerFromRedstoneSignal(String const &bName, CompoundTag const &s, Props &p) {
    PowerFromRedstoneSignal(s, p);
    return bName;
  }

  static String BlockWithRotationFromGroundSignDirection(String const &bName, CompoundTag const &s, Props &p) {
    RotationFromGroundSignDirection(s, p);
    return bName;
  }

  static String BlockWithSnowy(String const &bName, CompoundTag const &s, Props &p) {
    p[u8"snowy"] = u8"false";
    return bName;
  }

  static String BlockWithSubmergible(String const &bName, CompoundTag const &s, Props &p) {
    Submergible(s, p);
    return bName;
  }

  static String BlockWithWallProperties(String const &bName, CompoundTag const &s, Props &p) {
    WallProperties(s, p);
    Submergible(s, p);
    return bName;
  }

  static String BrewingStand(String const &bName, CompoundTag const &s, Props &p) {
    auto slotA = s.boolean(u8"brewing_stand_slot_a_bit", false);
    auto slotB = s.boolean(u8"brewing_stand_slot_b_bit", false);
    auto slotC = s.boolean(u8"brewing_stand_slot_c_bit", false);
    p[u8"has_bottle_0"] = Bool(slotA);
    p[u8"has_bottle_1"] = Bool(slotB);
    p[u8"has_bottle_2"] = Bool(slotC);
    return bName;
  }

  static String BrownMushroomBlock(String const &bName, CompoundTag const &s, Props &p) {
    bool stem = MushroomProperties(s, p);
    std::u8string name;
    if (stem) {
      name = u8"mushroom_stem";
    } else {
      name = u8"brown_mushroom_block";
    }
    return Ns() + name;
  }

  static String BubbleColumn(String const &bName, CompoundTag const &s, Props &p) {
    auto dragDown = s.boolean(u8"drag_down", false);
    p[u8"drag"] = Bool(dragDown);
    return bName;
  }

  static String Button(String const &bName, CompoundTag const &s, Props &p) {
    auto buttonPressedBit = s.boolean(u8"button_pressed_bit", false);
    auto facingDirection = s.int32(u8"facing_direction", 0);
    std::u8string facing = u8"south";
    std::u8string face = u8"ceiling";
    switch (facingDirection) {
    case 0:
      face = u8"ceiling";
      facing = u8"south";
      break;
    case 1:
      face = u8"floor";
      facing = u8"south";
      break;
    case 2:
      face = u8"wall";
      facing = u8"north";
      break;
    case 3:
      face = u8"wall";
      facing = u8"south";
      break;
    case 4:
      face = u8"wall";
      facing = u8"west";
      break;
    case 5:
      face = u8"wall";
      facing = u8"east";
      break;
    default:
      break;
    }
    p[u8"face"] = face;
    p[u8"facing"] = facing;
    p[u8"powered"] = Bool(buttonPressedBit);
    if (bName == u8"minecraft:wooden_button") {
      return Ns() + u8"oak_button";
    } else {
      return bName;
    }
  }
#pragma endregion

#pragma region Converters : C
  static String Cake(String const &bName, CompoundTag const &s, Props &p) {
    auto biteCounter = s.int32(u8"bite_counter", 0);
    p[u8"bites"] = Int(biteCounter);
    return bName;
  }

  static String Campfire(String const &bName, CompoundTag const &s, Props &p) {
    Facing4FromCardinalDirectionMigratingDirectionA(s, p);
    Submergible(s, p);
    auto extinguished = s.boolean(u8"extinguished", false);
    p[u8"lit"] = Bool(!extinguished);
    return bName;
  }

  static String Candle(String const &bName, CompoundTag const &s, Props &p) {
    auto candles = s.int32(u8"candles", 0);
    p[u8"candles"] = Int(candles + 1);
    Lit(s, p);
    Submergible(s, p);
    return bName;
  }

  static String CandleCake(String const &bName, CompoundTag const &s, Props &p) {
    Lit(s, p);
    return bName;
  }

  static String CarpetLegacy(String const &bName, CompoundTag const &s, Props &p) {
    auto colorB = s.string(u8"color", u8"white");
    auto colorJ = JavaNameFromColorCodeJava(ColorCodeJavaFromBedrockName(colorB));
    return Ns() + colorJ + u8"_carpet";
  }

  static String Cauldron(String const &bName, CompoundTag const &s, Props &p) {
    auto liquid = s.string(u8"cauldron_liquid", u8"water");
    std::u8string name = u8"cauldron";
    auto fillLevel = s.int32(u8"fill_level", 0);
    if (fillLevel < 1) {
      name = u8"cauldron";
    } else if (liquid == u8"water") {
      name = u8"water_cauldron";
      p[u8"level"] = Int((std::min)((std::max)((int)ceil(fillLevel * 0.5), 1), 3));
    } else if (liquid == u8"lava") {
      name = u8"lava_cauldron";
    } else if (liquid == u8"powder_snow") {
      name = u8"powder_snow_cauldron";
      p[u8"level"] = Int((std::min)((std::max)((int)ceil(fillLevel * 0.5), 1), 3));
    }
    return Ns() + name;
  }

  static String CaveVines(String const &bName, CompoundTag const &s, Props &p) {
    auto berries = bName.ends_with(u8"_with_berries");
    auto growingPlantAge = s.int32(u8"growing_plant_age", 1);
    p[u8"age"] = Int(growingPlantAge);
    p[u8"berries"] = Bool(berries);
    return Ns() + u8"cave_vines";
  }

  static String CaveVinesBody(String const &bName, CompoundTag const &s, Props &p) {
    auto berries = bName.ends_with(u8"_with_berries");
    p[u8"berries"] = Bool(berries);
    return Ns() + u8"cave_vines_plant";
  }

  static String Chain(String const &bName, CompoundTag const &s, Props &p) {
    AxisFromPillarAxis(s, p);
    Submergible(s, p);
    return bName;
  }

  static String ChiseledBookshelf(String const &bName, CompoundTag const &s, Props &p) {
    Facing4FromDirectionA(s, p);
    auto stored = s.int32(u8"books_stored", 0);
    for (int i = 0; i < 6; i++) {
      bool occupied = (uint8_t(stored) & (uint8_t(1) << i)) == ((uint8_t(1) << i));
      auto key = u8"slot_" + Int(i) + u8"_occupied";
      p[key] = Bool(occupied);
    }
    return bName;
  }

  static String Cocoa(String const &bName, CompoundTag const &s, Props &p) {
    Facing4FromDirectionA(s, p);
    Age(s, p);
    return bName;
  }

  static String CommandBlock(String const &bName, CompoundTag const &s, Props &p) {
    auto conditional = s.boolean(u8"conditional_bit", false);
    Facing6FromFacingDirectionA(s, p);
    p[u8"conditional"] = Bool(conditional);
    return bName;
  }

  static String Comparator(String const &bName, CompoundTag const &s, Props &p) {
    // auto powered = bName.starts_with(Ns() + "powered_");
    auto lit = s.boolean(u8"output_lit_bit", false);
    auto subtract = s.boolean(u8"output_subtract_bit", false);
    Facing4FromCardinalDirectionMigratingDirectionA(s, p);
    p[u8"powered"] = Bool(lit);
    p[u8"mode"] = subtract ? u8"subtract" : u8"compare";
    return Ns() + u8"comparator";
  }

  static String Composter(String const &bName, CompoundTag const &s, Props &p) {
    auto level = s.int32(u8"composter_fill_level", 0);
    p[u8"level"] = Int(level);
    return bName;
  }

  static String ConcreteLegacy(String const &bName, CompoundTag const &s, Props &p) {
    auto colorB = s.string(u8"color", u8"white");
    auto colorJ = JavaNameFromColorCodeJava(ColorCodeJavaFromBedrockName(colorB));
    return Ns() + colorJ + u8"_concrete";
  }

  static String ConcretePowder(String const &bName, CompoundTag const &s, Props &p) {
    auto colorB = s.string(u8"color", u8"white");
    auto colorJ = JavaNameFromColorCodeJava(ColorCodeJavaFromBedrockName(colorB));
    return Ns() + colorJ + u8"_concrete_powder";
  }

  static String CopperBulb(String const &bName, CompoundTag const &s, Props &p) {
    auto lit = s.boolean(u8"lit", false);
    auto poweredBit = s.boolean(u8"powered_bit", false);
    p[u8"lit"] = Bool(lit);
    p[u8"powered"] = Bool(poweredBit);
    return bName;
  }

  static String CoralLegacy(String const &bName, CompoundTag const &s, Props &p) {
    auto color = s.string(u8"coral_color", u8"pink");
    auto dead = s.boolean(u8"dead_bit", false);
    auto type = CoralTypeFromCoralColor(color);
    std::u8string name;
    if (dead) {
      name = u8"dead_";
    }
    name += type;
    Submergible(s, p);
    return Ns() + name + u8"_coral";
  }

  static String CoralBlock(String const &bName, CompoundTag const &s, Props &p) {
    auto color = s.string(u8"coral_color", u8"pink");
    auto dead = s.boolean(u8"dead_bit", false);
    auto type = CoralTypeFromCoralColor(color);
    std::u8string name;
    if (dead) {
      name = u8"dead_";
    }
    name += type + u8"_coral_block";
    return Ns() + name;
  }

  static String CoralFan(String const &bName, CompoundTag const &s, Props &p) {
    // coral_fan_direction always 1
    // auto direction = s.int32(u8"coral_fan_direction", 1);
    Submergible(s, p);
    return bName;
  }

  static String CoralFanLegacy(String const &bName, CompoundTag const &s, Props &p) {
    auto color = s.string(u8"coral_color", u8"pink");
    auto dead = bName.ends_with(u8"_dead");
    auto type = CoralTypeFromCoralColor(color);
    std::u8string name;
    if (dead) {
      name = u8"dead_";
    }
    name += type + u8"_coral_fan";
    Submergible(s, p);
    return Ns() + name;
  }

  static String CoralFanHang(String const &bName, CompoundTag const &s, Props &p) {
    auto hangType = s.boolean(u8"coral_hang_type_bit", false);
    auto dead = s.boolean(u8"dead_bit", false);
    std::u8string name;
    if (dead) {
      name = u8"dead_";
    }
    if (bName.ends_with(u8"2")) {
      if (hangType) {
        name += u8"fire";
      } else {
        name += u8"bubble";
      }
    } else if (bName.ends_with(u8"3")) {
      name += u8"horn";
    } else {
      if (hangType) {
        name += u8"brain";
      } else {
        name += u8"tube";
      }
    }
    CoralDirection(s, p);
    Submergible(s, p);
    return Ns() + name + u8"_coral_wall_fan";
  }

  static String Crafter(String const &bName, CompoundTag const &s, Props &p) {
    auto crafting = s.boolean(u8"crafting", false);
    auto orientation = s.string(u8"orientation", u8"north_up");
    auto triggered = s.boolean(u8"triggered_bit", false);
    p[u8"crafting"] = Bool(crafting);
    p[u8"orientation"] = String(orientation);
    p[u8"triggered"] = Bool(triggered);
    return bName;
  }
#pragma endregion

#pragma region Converters : D
  static String DarkoakStandingSign(String const &bName, CompoundTag const &s, Props &p) {
    RotationFromGroundSignDirection(s, p);
    Submergible(s, p);
    return Ns() + u8"dark_oak_sign";
  }

  static String DarkoakWallSign(String const &bName, CompoundTag const &s, Props &p) {
    Facing4FromFacingDirectionA(s, p);
    Submergible(s, p);
    return Ns() + u8"dark_oak_wall_sign";
  }

  static String DaylightDetector(String const &bName, CompoundTag const &s, Props &p) {
    auto inverted = bName.ends_with(u8"_inverted");
    p[u8"inverted"] = Bool(inverted);
    PowerFromRedstoneSignal(s, p);
    return Ns() + u8"daylight_detector";
  }

  static String DecoratedPot(String const &bName, CompoundTag const &s, Props &p) {
    Submergible(s, p);
    Facing4FromDirectionNorth0East1South2West3(s, p);
    p[u8"cracked"] = u8"false";
    return bName;
  }

  static String Dirt(String const &bName, CompoundTag const &s, Props &p) {
    auto type = s.string(u8"dirt_type", u8"normal");
    std::u8string prefix;
    if (type != u8"normal") {
      prefix = type + u8"_";
    }
    return Ns() + prefix + u8"dirt";
  }

  static String Door(String const &bName, CompoundTag const &s, Props &p) {
    auto doorHingeBit = s.boolean(u8"door_hinge_bit", false);
    OpenFromOpenBit(s, p);
    FacingCFromDirection(s, p);
    HalfFromUpperBlockBit(s, p);
    p[u8"hinge"] = doorHingeBit ? u8"right" : u8"left";
    p[u8"powered"] = u8"false";
    if (bName == u8"minecraft:wooden_door") {
      return Ns() + u8"oak_door";
    } else {
      return bName;
    }
  }

  static String DoublePlantLegacy(String const &bName, CompoundTag const &s, Props &p) {
    auto type = s.string(u8"double_plant_type", u8"rose");
    std::u8string name;
    if (type == u8"fern") {
      name = u8"large_fern";
    } else if (type == u8"syringa") {
      name = u8"lilac";
    } else if (type == u8"paeonia") {
      name = u8"peony";
    } else if (type == u8"sunflower") {
      name = u8"sunflower";
    } else if (type == u8"rose") {
      name = u8"rose_bush";
    } else { // "grass"
      name = u8"tall_grass";
    }
    HalfFromUpperBlockBit(s, p);
    return Ns() + name;
  }

  static Converter DoubleSlab(std::u8string name) {
    return [name](String const &bName, CompoundTag const &s, Props &p) {
      p[u8"type"] = u8"double";
      Submergible(s, p);
      return Ns() + name;
    };
  }

  static String DoubleStoneSlab(String const &bName, CompoundTag const &s, Props &p) {
    auto stoneSlabType = s.string(u8"stone_slab_type", u8"stone");
    p[u8"type"] = u8"double";
    Submergible(s, p);
    return Ns() + stoneSlabType + u8"_slab";
  }

  static String DoubleStoneSlab2(String const &bName, CompoundTag const &s, Props &p) {
    auto stoneSlabType = s.string(u8"stone_slab_type_2", u8"prismarine_dark");
    auto name = StoneTypeFromStone2(stoneSlabType);
    p[u8"type"] = u8"double";
    Submergible(s, p);
    return Ns() + name + u8"_slab";
  }

  static String DoubleStoneSlab3(String const &bName, CompoundTag const &s, Props &p) {
    auto stoneSlabType = s.string(u8"stone_slab_type_3", u8"andesite");
    p[u8"type"] = u8"double";
    Submergible(s, p);
    return Ns() + stoneSlabType + u8"_slab";
  }

  static String DoubleStoneSlab4(String const &bName, CompoundTag const &s, Props &p) {
    auto stoneSlabType = s.string(u8"stone_slab_type_4", u8"stone");
    p[u8"type"] = u8"double";
    Submergible(s, p);
    return Ns() + stoneSlabType + u8"_slab";
  }

  static String DoubleWoodenSlab(String const &bName, CompoundTag const &s, Props &p) {
    auto woodType = s.string(u8"wood_type", u8"oak");
    p[u8"type"] = u8"double";
    Submergible(s, p);
    return Ns() + woodType + u8"_slab";
  }

  static String Dispenser(String const &bName, CompoundTag const &s, Props &p) {
    Facing6FromFacingDirectionA(s, p);
    auto triggered = s.boolean(u8"triggered_bit", false);
    p[u8"triggered"] = Bool(triggered);
    return bName;
  }

  static String Dropper(String const &bName, CompoundTag const &s, Props &p) {
    Facing6FromFacingDirectionA(s, p);
    auto triggered = s.boolean(u8"triggered_bit", false);
    p[u8"triggered"] = Bool(triggered);
    return bName;
  }
#pragma endregion

#pragma region Converters : E
  static String EndPortalFrame(String const &bName, CompoundTag const &s, Props &p) {
    Facing4FromCardinalDirectionMigratingDirectionA(s, p);
    auto eye = s.boolean(u8"end_portal_eye_bit", false);
    p[u8"eye"] = Bool(eye);
    return bName;
  }

  static String EndRod(String const &bName, CompoundTag const &s, Props &p) {
    std::u8string facing;
    switch (s.int32(u8"facing_direction", 0)) {
    case 1:
      facing = u8"up";
      break;
    case 2:
      facing = u8"south";
      break;
    case 3:
      facing = u8"north";
      break;
    case 4:
      facing = u8"east";
      break;
    case 5:
      facing = u8"west";
      break;
    case 0:
    default:
      facing = u8"down";
      break;
    }
    p[u8"facing"] = facing;
    return bName;
  }
#pragma endregion

#pragma region Converter : F
  static String Farmland(String const &bName, CompoundTag const &s, Props &p) {
    auto moisture = s.int32(u8"moisturized_amount", 0);
    p[u8"moisture"] = Int(moisture);
    return bName;
  }

  static String FenceLegacy(String const &bName, CompoundTag const &s, Props &p) {
    auto woodType = s.string(u8"wood_type", u8"oak");
    Submergible(s, p);
    return Ns() + woodType + u8"_fence";
  }

  static String BlockWithDirection(String const &bName, CompoundTag const &s, Props &p) {
    auto inWall = s.boolean(u8"in_wall_bit", false);
    p[u8"in_wall"] = Bool(inWall);
    p[u8"powered"] = Bool(false);
    Facing4FromDirectionA(s, p);
    OpenFromOpenBit(s, p);
    if (bName.ends_with(u8":fence_gate")) {
      return Ns() + u8"oak_fence_gate";
    } else {
      return bName;
    }
  }

  static String FurnaceAndSimilar(String const &bName, CompoundTag const &s, Props &p) {
    Facing4FromCardinalDirectionMigratingFacingDirectionA(s, p);
    auto lit = bName.starts_with(u8"minecraft:lit_");
    std::u8string name;
    if (lit) {
      name = bName.substr(14);
    } else {
      name = bName.substr(10);
    }
    p[u8"lit"] = Bool(lit);
    return Ns() + name;
  }
#pragma endregion

#pragma region Converters : G
  static String GoldenRail(String const &bName, CompoundTag const &s, Props &p) {
    auto railData = s.boolean(u8"rail_data_bit", false);
    auto railDirection = s.int32(u8"rail_direction", 0);
    p[u8"powered"] = Bool(railData);
    p[u8"shape"] = ShapeFromRailDirection(railDirection);
    Submergible(s, p);
    return Ns() + u8"powered_rail";
  }

  static String GrassBlock(String const &bName, CompoundTag const &s, Props &p) {
    p[u8"snowy"] = u8"false";
    return Ns() + u8"grass_block";
  }

  static String Grindstone(String const &bName, CompoundTag const &s, Props &p) {
    auto attachment = s.string(u8"attachment", u8"floor");
    std::u8string face;
    if (attachment == u8"side") {
      face = u8"wall";
    } else if (attachment == u8"hanging") {
      face = u8"ceiling";
    } else { // standing
      face = u8"floor";
    }
    p[u8"face"] = face;
    Facing4FromDirectionA(s, p);
    return bName;
  }
#pragma endregion

#pragma region Converters : H
  static String HangingSign(String const &bName, CompoundTag const &s, Props &p) {
    Submergible(s, p);

    bool hanging = s.boolean(u8"hanging", false);
    if (hanging) {
      auto attached = s.boolean(u8"attached_bit", false);
      p[u8"attached"] = Bool(attached);
      if (attached) {
        p[u8"rotation"] = Int(s.int32(u8"ground_sign_direction", 0));
      } else {
        int rotation;
        switch (s.int32(u8"facing_direction", 2)) {
        case 3:
          rotation = 0;
          break;
        case 4:
          rotation = 4;
          break;
        case 5:
          rotation = 12;
          break;
        case 2:
        default:
          rotation = 8;
          break;
        }
        p[u8"rotation"] = Int(rotation);
      }
      return bName;
    } else {
      std::u8string facing = u8"north";
      switch (s.int32(u8"facing_direction", 2)) {
      case 2:
        facing = u8"north";
        break;
      case 3:
        facing = u8"south";
        break;
      case 4:
        facing = u8"west";
        break;
      case 5:
        facing = u8"east";
        break;
      }
      p[u8"facing"] = facing;
      return strings::Replace(bName, u8"_hanging_sign", u8"_wall_hanging_sign");
    }
  }

  static String Hopper(String const &bName, CompoundTag const &s, Props &p) {
    Facing6FromFacingDirectionA(s, p);
    auto toggle = s.boolean(u8"toggle_bit", false);
    p[u8"enabled"] = Bool(!toggle);
    return bName;
  }
#pragma endregion

#pragma region Converters : J
  static String Jigsaw(String const &bName, CompoundTag const &s, Props &p) {
    auto facingDirection = s.int32(u8"facing_direction", 0);
    auto rotation = s.int32(u8"rotation", 0);
    Facing6 f6 = Facing6FromBedrockFacingDirectionA(facingDirection);
    std::u8string orientation = JavaNameFromFacing6(f6);
    switch (f6) {
    case Facing6::Up:
    case Facing6::Down: {
      Facing4 f4 = Facing4FromNorth0East1South2West3(rotation);
      orientation += u8"_" + JavaNameFromFacing4(f4);
      break;
    }
    default:
      orientation += u8"_up";
      break;
    }
    p[u8"orientation"] = orientation;
    return bName;
  }
#pragma endregion

#pragma region Converters : K
  static String Kelp(String const &bName, CompoundTag const &s, Props &p) {
    auto age = s.int32(u8"kelp_age", 0);
    p[u8"age"] = Int(age);
    return bName;
  }
#pragma endregion

#pragma region Converters : L
  static String Lantern(String const &bName, CompoundTag const &s, Props &p) {
    auto hanging = s.boolean(u8"hanging", false);
    p[u8"hanging"] = Bool(hanging);
    Submergible(s, p);
    return bName;
  }

  static String LeavesLegacy(String const &bName, CompoundTag const &s, Props &p) {
    auto leafType = s.string(u8"old_leaf_type", u8"oak");
    PersistentFromPersistentBit(s, p);
    Submergible(s, p);
    return Ns() + leafType + u8"_leaves";
  }

  static String Leaves2(String const &bName, CompoundTag const &s, Props &p) {
    auto newLeafType = s.string(u8"new_leaf_type", u8"acacia"); // TODO: acacia?
    PersistentFromPersistentBit(s, p);
    Submergible(s, p);
    return Ns() + newLeafType + u8"_leaves";
  }

  static String Lectern(String const &bName, CompoundTag const &s, Props &p) {
    auto powered = s.boolean(u8"powered_bit", false);
    p[u8"powered"] = Bool(powered);
    Facing4FromCardinalDirectionMigratingDirectionA(s, p);
    return bName;
  }

  static String Lever(String const &bName, CompoundTag const &s, Props &p) {
    auto direction = s.string(u8"lever_direction", u8"up_north_south");
    std::u8string face;
    std::u8string facing;
    if (direction == u8"up_east_west") {
      face = u8"floor";
      facing = u8"west";
    } else if (direction == u8"up_north_south") {
      face = u8"floor";
      facing = u8"north";
    } else if (direction == u8"down_east_west") {
      face = u8"ceiling";
      facing = u8"west";
    } else if (direction == u8"down_north_south") {
      face = u8"ceiling";
      facing = u8"north";
    } else {
      face = u8"wall";
      facing = direction;
    }
    p[u8"face"] = face;
    p[u8"facing"] = facing;
    auto open = s.boolean(u8"open_bit", false);
    p[u8"powered"] = Bool(open);
    return bName;
  }

  static String LightBlock(String const &bName, CompoundTag const &s, Props &p) {
    auto level = s.int32(u8"block_light_level", 0);
    p[u8"level"] = Int(level);
    Submergible(s, p);
    return Ns() + u8"light";
  }

  static String Liquid(String const &bName, CompoundTag const &s, Props &p) {
    auto depth = s.int32(u8"liquid_depth", 0);
    p[u8"level"] = Int(depth);
    return bName;
  }

  static String LiquidFlowing(String const &bName, CompoundTag const &s, Props &p) {
    auto depth = s.int32(u8"liquid_depth", 0);
    p[u8"level"] = Int(depth);
    if (bName.ends_with(u8"lava")) {
      return Ns() + u8"lava";
    } else {
      return Ns() + u8"water";
    }
  }

  static String LitPumpkin(String const &bName, CompoundTag const &s, Props &p) {
    if (!FacingFromCardinalDirection(s, p)) {
      // legacy
      Facing4FromDirectionA(s, p);
    }
    return Ns() + u8"jack_o_lantern";
  }

  static String LogLegacy(String const &bName, CompoundTag const &s, Props &p) {
    auto logType = s.string(u8"old_log_type", u8"oak");
    AxisFromPillarAxis(s, p);
    return Ns() + logType + u8"_log";
  }

  static String Log2Legacy(String const &bName, CompoundTag const &s, Props &p) {
    auto logType = s.string(u8"new_log_type", u8"acacia"); // TODO: acacia?
    AxisFromPillarAxis(s, p);
    return Ns() + logType + u8"_log";
  }
#pragma endregion

#pragma region Converters : M
  static String MangrovePropagule(String const &bName, CompoundTag const &s, Props &p) {
    bool hanging = s.boolean(u8"hanging", false);
    int stage = s.int32(u8"propagule_stage", 0);
    p[u8"hanging"] = Bool(hanging);
    p[u8"stage"] = Int(1);
    p[u8"age"] = Int(stage);
    Submergible(s, p);
    return bName;
  }

  static String MelonStem(String const &bName, CompoundTag const &s, Props &p) {
    auto growth = s.int32(u8"growth", 0);
    p[u8"age"] = Int(growth);
    Facing4FromFacingDirectionA(s, p);
    return bName;
  }

  static String MonsterEgg(String const &bName, CompoundTag const &s, Props &p) {
    auto type = s.string(u8"monster_egg_stone_type", u8"stone");
    std::u8string name;
    if (type == u8"cobblestone") {
      name = type;
    } else if (type == u8"stone_brick") {
      name = u8"stone_bricks";
    } else if (type == u8"mossy_stone_brick") {
      name = u8"mossy_stone_bricks";
    } else if (type == u8"cracked_stone_brick") {
      name = u8"cracked_stone_bricks";
    } else if (type == u8"chiseled_stone_brick") {
      name = u8"chiseled_stone_bricks";
    } else { // stone
      name = u8"stone";
    }
    return Ns() + u8"infested_" + name;
  }
#pragma endregion

#pragma region Converters : N
  static Converter NetherVines(std::u8string type) {
    return [type](String const &bName, CompoundTag const &s, Props &p) {
      auto age = s.int32(type + u8"_vines_age", 0);
      p[u8"age"] = Int(age);
      return bName;
    };
  }
#pragma endregion

#pragma region Converters : O
  static String OakWallSign(String const &bName, CompoundTag const &s, Props &p) {
    Facing4FromFacingDirectionA(s, p);
    Submergible(s, p);
    return Ns() + u8"oak_wall_sign";
  }

  static String Observer(String const &bName, CompoundTag const &s, Props &p) {
    auto facingDirection = s.string(u8"minecraft:facing_direction");
    if (facingDirection) {
      p[u8"facing"] = String(*facingDirection);
    } else {
      // legacy
      Facing6FromFacingDirectionA(s, p);
    }
    auto powered = s.boolean(u8"powered", false);
    p[u8"powered"] = Bool(powered);
    return bName;
  }
#pragma endregion

#pragma region Converters : P
  static String PaleMossCarpet(String const &bName, CompoundTag const &s, Props &p) {
    auto bottom = !s.boolean(u8"upper_block_bit", false);
    p[u8"bottom"] = Bool(bottom);
    for (auto f : {Facing4::North, Facing4::East, Facing4::South, Facing4::West}) {
      auto name = JavaNameFromFacing4(f);
      auto typeB = s.string(u8"pale_moss_carpet_side_" + name, u8"none");
      std::u8string typeJ = u8"none";
      if (typeB == u8"short") {
        typeJ = u8"low";
      } else if (typeB == u8"tall") {
        typeJ = u8"tall";
      }
      p[name] = typeJ;
    }
    return bName;
  }

  static String PinkPetals(String const &bName, CompoundTag const &s, Props &p) {
    i32 growth = s.int32(u8"growth", 0);
    i32 flowerCount = ClosedRange<i32>::Clamp(growth + 1, 1, 4);
    p[u8"flower_amount"] = mcfile::String::ToString(flowerCount);
    FacingFromCardinalDirectionMigratingDirectionNorth2East3South0West1(s, p);
    return bName;
  }

  static String PlanksLegacy(String const &bName, CompoundTag const &s, Props &p) {
    auto woodType = s.string(u8"wood_type", u8"acacia"); // TODO: acacia?
    return Ns() + woodType + u8"_planks";
  }

  static String PointedDripstone(String const &bName, CompoundTag const &s, Props &p) {
    auto thickness = s.string(u8"dripstone_thickness", u8"base");
    std::u8string t = thickness;
    // base, middle, frustum, tip , tip_merge
    if (thickness == u8"merge") {
      t = u8"tip_merge";
    }
    p[u8"thickness"] = t;
    auto hanging = s.boolean(u8"hanging", false);
    p[u8"vertical_direction"] = hanging ? u8"down" : u8"up";
    Submergible(s, p);
    return bName;
  }

  static String Portal(String const &bName, CompoundTag const &s, Props &p) {
    auto axis = s.string(u8"portal_axis", u8"y");
    p[u8"axis"] = axis;
    return Ns() + u8"nether_portal";
  }

  static String PressurePlate(String const &bName, CompoundTag const &s, Props &p) {
    auto redstoneSignal = s.int32(u8"redstone_signal", 0);
    p[u8"powered"] = Bool(redstoneSignal > 0);
    return bName;
  }

  static String Prismarine(String const &bName, CompoundTag const &s, Props &p) {
    auto type = s.string(u8"prismarine_block_type", u8"default");
    std::u8string name;
    if (type == u8"bricks") {
      name = u8"prismarine_bricks";
    } else if (type == u8"dark") {
      name = u8"dark_prismarine";
    } else { // default
      name = u8"prismarine";
    }
    return Ns() + name;
  }

  static String CarvedPumpkin(String const &bName, CompoundTag const &s, Props &p) {
    if (!FacingFromCardinalDirection(s, p)) {
      // legacy
      Facing4FromDirectionA(s, p);
    }
    return bName;
  }

  static String PumpkinStem(String const &bName, CompoundTag const &s, Props &p) {
    auto growth = s.int32(u8"growth", 0);
    p[u8"age"] = Int(growth);
    Facing4FromFacingDirectionA(s, p);
    return bName;
  }

  static String PurpurBlock(String const &bName, CompoundTag const &s, Props &p) {
    auto chisel = s.string(u8"chisel_type", u8"default");
    std::u8string name;
    if (chisel == u8"lines") {
      name = u8"purpur_pillar";
      AxisFromPillarAxis(s, p);
    } else { // "default"
      name = u8"purpur_block";
    }
    return Ns() + name;
  }
#pragma endregion

#pragma region Converters : Q
  static String QuartzBlock(String const &bName, CompoundTag const &s, Props &p) {
    auto type = s.string(u8"chisel_type", u8"chiseled");
    if (type == u8"lines") {
      AxisFromPillarAxis(s, p);
      return Ns() + u8"quartz_pillar";
    } else if (type == u8"smooth") {
      return Ns() + u8"smooth_quartz";
    } else if (type == u8"chiseled") {
      return Ns() + u8"chiseled_quartz_block";
    } else { // "default";
      return Ns() + u8"quartz_block";
    }
  }
#pragma endregion

#pragma region Converters : R
  static String Rail(String const &bName, CompoundTag const &s, Props &p) {
    auto railDirection = s.int32(u8"rail_direction", 0);
    p[u8"shape"] = ShapeFromRailDirection(railDirection);
    Submergible(s, p);
    return bName;
  }

  static String RailCanBePowered(String const &bName, CompoundTag const &s, Props &p) {
    auto railData = s.boolean(u8"rail_data_bit", false);
    auto railDirection = s.int32(u8"rail_direction", 0);
    p[u8"powered"] = Bool(railData);
    p[u8"shape"] = ShapeFromRailDirection(railDirection);
    Submergible(s, p);
    return bName;
  }

  static String RedFlower(String const &bName, CompoundTag const &s, Props &p) {
    auto flowerType = s.string(u8"flower_type", u8"poppy");
    auto rf = RedFlowerFromBedrockName(flowerType);
    if (!rf) {
      return bName;
    }
    auto name = JavaNameFromRedFlower(*rf);
    return Ns() + name;
  }

  static String RedMushroomBlock(String const &bName, CompoundTag const &s, Props &p) {
    bool stem = MushroomProperties(s, p);
    if (stem) {
      return Ns() + u8"mushroom_stem";
    } else {
      return bName;
    }
  }

  static String RedSandstone(String const &bName, CompoundTag const &s, Props &p) {
    auto type = s.string(u8"sand_stone_type", u8"default");
    std::u8string name = u8"red_sandstone";
    if (type == u8"heiroglyphs") {
      name = u8"chiseled_red_sandstone";
    } else if (type == u8"cut") {
      name = u8"cut_red_sandstone";
    } else if (type == u8"smooth") {
      name = u8"smooth_red_sandstone";
    } else { // "default"
      name = u8"red_sandstone";
    }
    return Ns() + name;
  }

  static String RedstoneLamp(String const &bName, CompoundTag const &s, Props &p) {
    auto lit = bName == u8"minecraft:lit_redstone_lamp";
    p[u8"lit"] = Bool(lit);
    return Ns() + u8"redstone_lamp";
  }

  static String RedstoneTorch(String const &bName, CompoundTag const &s, Props &p) {
    auto name = Torch(u8"redstone_")(bName, s, p);
    assert(name.starts_with(u8"minecraft:"));
    auto lit = bName != u8"minecraft:unlit_redstone_torch";
    p[u8"lit"] = Bool(lit);
    return name;
  }

  static String RedstoneOre(String const &bName, CompoundTag const &s, Props &p) {
    auto lit = bName.starts_with(u8"minecraft:lit_");
    std::u8string name;
    if (lit) {
      name = bName.substr(14);
    } else {
      name = bName.substr(10);
    }
    p[u8"lit"] = Bool(lit);
    return Ns() + name;
  }

  static String SugarCane(String const &bName, CompoundTag const &s, Props &p) {
    Age(s, p);
    return Ns() + u8"sugar_cane";
  }

  static Converter Rename(std::u8string name) {
    return [name](String const &bName, CompoundTag const &s, Props &p) {
      return Ns() + name;
    };
  }

  static Converter RenameStairs(std::u8string name) {
    return [name](String const &bName, CompoundTag const &s, Props &p) {
      FacingFromWeirdoDirection(s, p);
      HalfFromUpsideDownBit(s, p);
      Submergible(s, p);
      return Ns() + name;
    };
  }

  static String Repeater(String const &bName, CompoundTag const &s, Props &p) {
    auto powered = bName == u8"minecraft:powered_repeater";
    p[u8"powered"] = Bool(powered);
    Facing4FromCardinalDirectionMigratingDirectionA(s, p);
    auto delay = s.int32(u8"repeater_delay", 0);
    p[u8"delay"] = Int(delay + 1 + 0);
    return Ns() + u8"repeater";
  }

  static String RespawnAnchor(String const &bName, CompoundTag const &s, Props &p) {
    auto charges = s.int32(u8"respawn_anchor_charge", 0);
    p[u8"charges"] = Int(charges);
    return bName;
  }
#pragma endregion

#pragma region Converters : S
  static String Same(String const &bName, CompoundTag const &s, Props &p) {
    return bName;
  }

  static String Sand(String const &bName, CompoundTag const &s, Props &p) {
    auto type = s.string(u8"sand_type", u8"normal");
    if (type == u8"red") {
      return Ns() + u8"red_sand";
    } else { // "normal"
      return bName;
    }
  }

  static String Sandstone(String const &bName, CompoundTag const &s, Props &p) {
    auto type = s.string(u8"sand_stone_type", u8"default");
    std::u8string name = u8"sandstone";
    if (type == u8"heiroglyphs") {
      name = u8"chiseled_sandstone";
    } else if (type == u8"cut") {
      name = u8"cut_sandstone";
    } else if (type == u8"smooth") {
      name = u8"smooth_sandstone";
    } else { // "default"
      name = u8"sandstone";
    }
    return Ns() + name;
  }

  static String SaplingLegacy(String const &bName, CompoundTag const &s, Props &p) {
    auto saplingType = s.string(u8"sapling_type", u8"acacia");
    StageFromAgeBit(s, p);
    return Ns() + saplingType + u8"_sapling";
  }

  static String SculkCatalyst(String const &bName, CompoundTag const &s, Props &p) {
    auto bloom = s.boolean(u8"bloom", false);
    p[u8"bloom"] = Bool(bloom);
    return bName;
  }

  static String SculkShrieker(String const &bName, CompoundTag const &s, Props &p) {
    auto canSummon = s.boolean(u8"can_summon", false);
    auto active = s.boolean(u8"active", false);
    p[u8"can_summon"] = Bool(canSummon);
    p[u8"shrieking"] = Bool(active);
    Submergible(s, p);
    return bName;
  }

  static String Seagrass(String const &bName, CompoundTag const &s, Props &p) {
    auto type = s.string(u8"sea_grass_type", u8"default");
    std::u8string name;
    if (type == u8"double_bot") {
      name = u8"tall_seagrass";
      p[u8"half"] = u8"lower";
    } else if (type == u8"double_top") {
      name = u8"tall_seagrass";
      p[u8"half"] = u8"upper";
    } else { // "default"
      name = u8"seagrass";
    }
    return Ns() + name;
  }

  static String SeaPickle(String const &bName, CompoundTag const &s, Props &p) {
    auto count = s.int32(u8"cluster_count", 0);
    p[u8"pickles"] = Int(count + 1);
    auto dead = s.boolean(u8"dead_bit", false);
    p[u8"waterlogged"] = Bool(!dead);
    return bName;
  }

  static String Scaffolding(String const &bName, CompoundTag const &s, Props &p) {
    auto stability = s.int32(u8"stability", 0);
    auto stabilityCheck = s.boolean(u8"stability_check", false);
    p[u8"bottom"] = Bool(stabilityCheck);
    p[u8"distance"] = Int(stability);
    Submergible(s, p);
    return bName;
  }

  static String ShulkerBoxLegacy(String const &bName, CompoundTag const &s, Props &p) {
    auto colorB = s.string(u8"color", u8"white");
    auto colorJ = JavaNameFromColorCodeJava(ColorCodeJavaFromBedrockName(colorB));
    return Ns() + colorJ + u8"_shulker_box";
  }

  static String SilverGlazedTerracotta(String const &bName, CompoundTag const &s, Props &p) {
    Facing4FromFacingDirectionA(s, p);
    return Ns() + u8"light_gray_glazed_terracotta";
  }

  static String Slab(String const &bName, CompoundTag const &s, Props &p) {
    Submergible(s, p);
    TypeFromVerticalHalfMigratingTopSlotBit(s, p);
    return bName;
  }

  static String SmallDripleafBlock(String const &bName, CompoundTag const &s, Props &p) {
    Facing4FromCardinalDirectionMigratingDirectionA(s, p);
    HalfFromUpperBlockBit(s, p);
    Submergible(s, p);
    return Ns() + u8"small_dripleaf";
  }

  static String SnifferEgg(String const &bName, CompoundTag const &s, Props &p) {
    auto crackedState = s.string(u8"cracked_state", u8"no_cracks");
    i32 hatch = 0;
    if (crackedState == u8"cracked") {
      hatch = 1;
    } else if (crackedState == u8"max_cracked") {
      hatch = 2;
    }
    p[u8"hatch"] = Int(hatch);
    return bName;
  }

  static String SnowLayer(String const &bName, CompoundTag const &s, Props &p) {
    auto height = s.int32(u8"height", 0);
    p[u8"layers"] = Int(height + 1);
    return Ns() + u8"snow";
  }

  static String Sponge(String const &bName, CompoundTag const &s, Props &p) {
    auto type = s.string(u8"sponge_type", u8"dry");
    if (type == u8"wet") {
      return Ns() + u8"wet_sponge";
    } else { // "dry"
      return Ns() + u8"sponge";
    }
  }

  static String Stairs(String const &bName, CompoundTag const &s, Props &p) {
    FacingFromWeirdoDirection(s, p);
    HalfFromUpsideDownBit(s, p);
    Submergible(s, p);
    return bName;
  }

  static String StainedGlassLegacy(String const &bName, CompoundTag const &s, Props &p) {
    auto colorB = s.string(u8"color", u8"white");
    auto colorJ = JavaNameFromColorCodeJava(ColorCodeJavaFromBedrockName(colorB));
    return Ns() + colorJ + u8"_stained_glass";
  }

  static String StainedGlassPaneLegacy(String const &bName, CompoundTag const &s, Props &p) {
    auto colorB = s.string(u8"color", u8"white");
    auto colorJ = JavaNameFromColorCodeJava(ColorCodeJavaFromBedrockName(colorB));
    Submergible(s, p);
    return Ns() + colorJ + u8"_stained_glass_pane";
  }

  static String StainedHardenedClay(String const &bName, CompoundTag const &s, Props &p) {
    auto colorB = s.string(u8"color", u8"white");
    auto colorJ = JavaNameFromColorCodeJava(ColorCodeJavaFromBedrockName(colorB));
    return Ns() + colorJ + u8"_terracotta";
  }

  static String StandingSign(String const &bName, CompoundTag const &s, Props &p) {
    std::u8string name;
    if (bName.ends_with(u8":standing_sign")) {
      name = u8"oak_sign";
    } else {
      auto type = VariantFromName(bName, u8"_standing_sign");
      name = type + u8"_sign";
    }
    RotationFromGroundSignDirection(s, p);
    Submergible(s, p);
    return Ns() + name;
  }

  static String PistonArmCollision(String const &bName, CompoundTag const &s, Props &p) {
    bool sticky = (bName == u8"minecraft:stickyPistonArmCollision") || (bName == u8"minecraft:sticky_piston_arm_collision");
    std::u8string type = sticky ? u8"sticky" : u8"normal";
    auto f6 = Facing6FromBedrockFacingDirectionB(s.int32(u8"facing_direction", 0));
    auto facing = JavaNameFromFacing6(f6);
    p[u8"facing"] = facing;
    p[u8"type"] = type;
    p[u8"short"] = u8"false"; //?
    return Ns() + u8"piston_head";
  }

  static String Stone(String const &bName, CompoundTag const &s, Props &p) {
    auto type = s.string(u8"stone_type");
    if (type) {
      // legacy, < 1.20.5x
      std::u8string name;
      if (type == u8"diorite_smooth") {
        name = u8"polished_diorite";
      } else if (type == u8"andesite_smooth") {
        name = u8"polished_andesite";
      } else if (type == u8"granite_smooth") {
        name = u8"polished_granite";
      } else {
        name = *type;
      }
      return Ns() + name;
    } else {
      return Ns() + u8"stone";
    }
  }

  static String Stonebrick(String const &bName, CompoundTag const &s, Props &p) {
    auto type = s.string(u8"stone_brick_type", u8"default");
    std::u8string prefix;
    if (type != u8"default") {
      prefix = type + u8"_";
    }
    return Ns() + prefix + u8"stone_bricks";
  }

  static String StonecutterBlock(String const &bName, CompoundTag const &s, Props &p) {
    Facing4FromCardinalDirectionMigratingFacingDirectionA(s, p);
    return Ns() + u8"stonecutter";
  }

  static String StoneSlab(String const &bName, CompoundTag const &s, Props &p) {
    auto stoneSlabType = s.string(u8"stone_slab_type", u8"stone");
    TypeFromVerticalHalfMigratingTopSlotBit(s, p);
    Submergible(s, p);
    return Ns() + stoneSlabType + u8"_slab";
  }

  static String StoneSlab2(String const &bName, CompoundTag const &s, Props &p) {
    auto type = s.string(u8"stone_slab_type_2", u8"prismarine_dark");
    TypeFromVerticalHalfMigratingTopSlotBit(s, p);
    Submergible(s, p);
    auto name = StoneTypeFromStone2(type);
    return Ns() + name + u8"_slab";
  }

  static String StoneSlab3(String const &bName, CompoundTag const &s, Props &p) {
    auto stoneSlabType = s.string(u8"stone_slab_type_3", u8"andesite");
    TypeFromVerticalHalfMigratingTopSlotBit(s, p);
    Submergible(s, p);
    return Ns() + stoneSlabType + u8"_slab";
  }

  static String StoneSlab4(String const &bName, CompoundTag const &s, Props &p) {
    auto type = s.string(u8"stone_slab_type_4", u8"stone");
    TypeFromVerticalHalfMigratingTopSlotBit(s, p);
    Submergible(s, p);
    return Ns() + type + u8"_slab";
  }

  static String StructureBlock(String const &bName, CompoundTag const &s, Props &p) {
    auto type = s.string(u8"structure_block_type", u8"save");
    p[u8"mode"] = type;
    return bName;
  }

  static String SweetBerryBush(String const &bName, CompoundTag const &s, Props &p) {
    AgeFromGrowth(s, p);
    return bName;
  }
#pragma endregion

#pragma region Converters : T
  static String Tallgrass(String const &bName, CompoundTag const &s, Props &p) {
    auto type = s.string(u8"tall_grass_type", u8"tall");
    if (type == u8"fern") {
      return Ns() + u8"fern";
    } else { // "tall"
      return Ns() + u8"short_grass";
    }
  }

  static String Target(String const &bName, CompoundTag const &s, Props &p) {
    p[u8"power"] = u8"0";
    return bName;
  }

  static String Tnt(String const &bName, CompoundTag const &s, Props &p) {
    auto explode = s.boolean(u8"explode_bit", false);
    p[u8"unstable"] = Bool(explode);
    return bName;
  }

  static String Trapdoor(String const &bName, CompoundTag const &s, Props &p) {
    FacingBFromDirection(s, p);
    OpenFromOpenBit(s, p);
    HalfFromUpsideDownBit(s, p);
    Submergible(s, p);
    p[u8"powered"] = u8"false";
    if (bName.ends_with(u8":trapdoor")) {
      return Ns() + u8"oak_trapdoor";
    } else {
      return bName;
    }
  }

  static String TrialSpawner(String const &bName, CompoundTag const &s, Props &p) {
    auto stateB = s.int32(u8"trial_spawner_state", 0);
    auto stateJ = TrialSpawner::JavaTrialSpawnerStateFromBedrock(stateB);
    p[u8"trial_spawner_state"] = stateJ;
    if (auto ominous = s.boolean(u8"ominous"); ominous) {
      p[u8"ominous"] = Bool(*ominous);
    }
    return bName;
  }

  static String Tripwire(String const &bName, CompoundTag const &s, Props &p) {
    auto attached = s.boolean(u8"attached_bit", false);
    auto disarmed = s.boolean(u8"disarmed_bit", false);
    auto powered = s.boolean(u8"powered_bit", false);
    p[u8"attached"] = Bool(attached);
    p[u8"disarmed"] = Bool(disarmed);
    p[u8"powered"] = Bool(powered);
    return Ns() + u8"tripwire";
  }

  static String TripwireHook(String const &bName, CompoundTag const &s, Props &p) {
    auto attached = s.boolean(u8"attached_bit", false);
    auto powered = s.boolean(u8"powered_bit", false);
    Facing4FromDirectionA(s, p);
    p[u8"attached"] = Bool(attached);
    p[u8"powered"] = Bool(powered);
    return bName;
  }

  static String TurtleEgg(String const &bName, CompoundTag const &s, Props &p) {
    auto eggs = s.string(u8"turtle_egg_count", u8"one_egg");
    if (eggs == u8"two_egg") {
      p[u8"eggs"] = u8"2";
    } else if (eggs == u8"three_egg") {
      p[u8"eggs"] = u8"3";
    } else if (eggs == u8"four_egg") {
      p[u8"eggs"] = u8"4";
    } else { // "one_egg"
      p[u8"eggs"] = u8"1";
    }
    auto cracked = s.string(u8"cracked_state", u8"no_cracks");
    if (cracked == u8"cracked") {
      p[u8"hatch"] = u8"1";
    } else if (cracked == u8"max_cracked") {
      p[u8"hatch"] = u8"2";
    } else { // "no_cracks"
      p[u8"hatch"] = u8"0";
    }
    return bName;
  }
#pragma endregion

#pragma region Converters : V
  static String Vault(String const &bName, CompoundTag const &s, Props &p) {
    FacingFromCardinalDirection(s, p);
    auto ominous = s.boolean(u8"ominous");
    if (ominous) {
      p[u8"ominous"] = Bool(*ominous);
    }
    auto vaultState = s.string(u8"vault_state", u8"inactive");
    p[u8"vault_state"] = vaultState;
    return bName;
  }
#pragma endregion

#pragma region Converters : W
  static String WallWithBlockType(String const &bName, CompoundTag const &s, Props &p) {
    auto type = s.string(u8"wall_block_type", u8"andesite");
    std::u8string name = type;
    if (type == u8"end_brick") {
      name = u8"end_stone_brick";
    }
    WallProperties(s, p);
    Submergible(s, p);
    return Ns() + name + u8"_wall";
  }

  static String WoodLegacy(String const &bName, CompoundTag const &s, Props &p) {
    auto stripped = s.boolean(u8"stripped_bit", false);
    auto woodType = s.string(u8"wood_type", u8"oak");
    auto name = stripped ? u8"stripped_" + woodType + u8"_wood" : woodType + u8"_wood";
    AxisFromPillarAxis(s, p);
    return Ns() + name;
  }

  static String WoodenPressurePlate(String const &bName, CompoundTag const &s, Props &p) {
    auto signal = s.int32(u8"redstone_signal", 0);
    p[u8"powered"] = Bool(signal > 0);
    return Ns() + u8"oak_pressure_plate";
  }

  static String WoodenSlab(String const &bName, CompoundTag const &s, Props &p) {
    auto woodType = s.string(u8"wood_type", u8"acacia");
    TypeFromVerticalHalfMigratingTopSlotBit(s, p);
    Submergible(s, p);
    return Ns() + woodType + u8"_slab";
  }

  static String LegacyWool(String const &bName, CompoundTag const &s, Props &p) {
    auto colorB = s.string(u8"color", u8"white");
    auto colorJ = JavaNameFromColorCodeJava(ColorCodeJavaFromBedrockName(colorB));
    return Ns() + colorJ + u8"_wool";
  }
#pragma endregion

#pragma region Converters : V
  static String Vine(String const &bName, CompoundTag const &s, Props &p) {
    auto d = s.int32(u8"vine_direction_bits", 0);
    bool east = (d & 0x8) == 0x8;
    bool north = (d & 0x4) == 0x4;
    bool west = (d & 0x2) == 0x2;
    bool south = (d & 0x1) == 0x1;
    p[u8"east"] = Bool(east);
    p[u8"north"] = Bool(north);
    p[u8"west"] = Bool(west);
    p[u8"south"] = Bool(south);
    p[u8"up"] = u8"false";
    return bName;
  }
#pragma endregion

#pragma region Properties
  static void Age(CompoundTag const &s, Props &p) {
    auto age = s.int32(u8"age", 0);
    p[u8"age"] = Int(age);
  }

  static void AgeFromGrowth(CompoundTag const &s, Props &p) {
    auto age = s.int32(u8"growth", 0);
    p[u8"age"] = Int(age);
  }

  static void AgeFromGrowthNonLinear(CompoundTag const &s, Props &p) {
    auto growth = s.int32(u8"growth", 0);
    int age = 0;
    if (growth < 2) {
      age = 0;
    } else if (growth < 4) {
      age = 1;
    } else if (growth < 7) {
      age = 2;
    } else {
      age = 3;
    }
    p[u8"age"] = Int(age);
  }

  static void AxisFromPillarAxis(CompoundTag const &s, Props &props) {
    auto pillarAxis = s.string(u8"pillar_axis", u8"y");
    props[u8"axis"] = pillarAxis;
  }

  static std::u8string CoralTypeFromCoralColor(std::u8string const color) {
    if (color == u8"blue") {
      return u8"tube";
    } else if (color == u8"pink") {
      return u8"brain";
    } else if (color == u8"purple") {
      return u8"bubble";
    } else if (color == u8"red") {
      return u8"fire";
    } else if (color == u8"yellow") {
      return u8"horn";
    }
    return u8"brain";
  }

  static void Facing4FromDirectionA(CompoundTag const &s, Props &props) {
    auto direction = s.int32(u8"direction", 0);
    Facing4 f4 = Facing4FromBedrockDirection(direction);
    props[u8"facing"] = JavaNameFromFacing4(f4);
  }

  static void Facing4FromCardinalDirectionMigratingDirectionA(CompoundTag const &s, Props &props) {
    if (auto cd = s.string(u8"minecraft:cardinal_direction"); cd) {
      props[u8"facing"] = *cd;
    } else {
      Facing4FromDirectionA(s, props);
    }
  }

  static void FacingFromCardinalDirectionMigratingDirectionNorth2East3South0West1(CompoundTag const &s, Props &props) {
    if (auto cd = s.string(u8"minecraft:cardinal_direction"); cd) {
      props[u8"facing"] = *cd;
    } else {
      auto direction = s.int32(u8"direction", 0);
      Facing4 f4 = Facing4FromNorth2East3South0West1(direction);
      props[u8"facing"] = JavaNameFromFacing4(f4);
    }
  }

  static void Facing6FromFacingDirectionA(CompoundTag const &s, Props &props) {
    auto facingDirection = s.int32(u8"facing_direction", 0);
    Facing6 f6 = Facing6FromBedrockFacingDirectionA(facingDirection);
    props[u8"facing"] = JavaNameFromFacing6(f6);
  }

  static void Facing6FromBlockFaceMigratingFacingDirectionA(CompoundTag const &s, Props &props) {
    if (auto bf = s.string(u8"minecraft:block_face"); bf) {
      props[u8"facing"] = *bf;
    } else {
      auto facingDirection = s.int32(u8"facing_direction", 0);
      Facing6 f6 = Facing6FromBedrockFacingDirectionA(facingDirection);
      props[u8"facing"] = JavaNameFromFacing6(f6);
    }
  }

  static void Facing4FromFacingDirectionA(CompoundTag const &s, Props &props) {
    auto facingDirection = s.int32(u8"facing_direction", 0);
    Facing6 f6 = Facing6FromBedrockFacingDirectionA(facingDirection);
    Facing4 f4;
    switch (f6) {
    case Facing6::North:
      f4 = Facing4::North;
      break;
    case Facing6::East:
      f4 = Facing4::East;
      break;
    case Facing6::West:
      f4 = Facing4::West;
      break;
    case Facing6::South:
    default:
      f4 = Facing4::South;
      break;
    }
    props[u8"facing"] = JavaNameFromFacing4(f4);
  }

  static void Facing4FromCardinalDirectionMigratingFacingDirectionA(CompoundTag const &s, Props &props) {
    if (auto cd = s.string(u8"minecraft:cardinal_direction"); cd) {
      props[u8"facing"] = *cd;
    } else {
      Facing4FromFacingDirectionA(s, props);
    }
  }

  static void FacingBFromDirection(CompoundTag const &s, Props &props) {
    auto direction = s.int32(u8"direction", 0);
    std::u8string facing;
    switch (direction) {
    case 2:
      facing = u8"south";
      break;
    case 1:
      facing = u8"west";
      break;
    case 3:
      facing = u8"north";
      break;
    case 0:
    default:
      facing = u8"east";
      break;
    }
    props[u8"facing"] = facing;
  }

  static void Facing4FromDirectionNorth0East1South2West3(CompoundTag const &s, Props &props) {
    auto direction = s.int32(u8"direction", 0);
    std::u8string facing;
    switch (direction) {
    case 1:
      facing = u8"east";
      break;
    case 2:
      facing = u8"south";
      break;
    case 3:
      facing = u8"west";
      break;
    case 0:
    default:
      facing = u8"north";
      break;
    }
    props[u8"facing"] = facing;
  }

  static void Facing6FromFacingDirectionB(CompoundTag const &s, Props &props) {
    auto direction = s.int32(u8"facing_direction", 0);
    Facing6 f6 = Facing6FromBedrockFacingDirectionB(direction);
    std::u8string facing = JavaNameFromFacing6(f6);
    props[u8"facing"] = facing;
  }

  static void FacingCFromDirection(CompoundTag const &s, Props &props) {
    auto direction = s.int32(u8"direction", 0);
    std::u8string facing;
    switch (direction) {
    case 1:
      facing = u8"south";
      break;
    case 2:
      facing = u8"west";
      break;
    case 3:
      facing = u8"north";
      break;
    case 0:
    default:
      facing = u8"east";
      break;
    }
    props[u8"facing"] = facing;
  }

  static void CoralDirection(CompoundTag const &s, Props &p) {
    auto d = s.int32(u8"coral_direction", 0);
    std::u8string facing;
    switch (d) {
    case 2:
      facing = u8"north";
      break;
    case 1:
      facing = u8"east";
      break;
    case 3:
      facing = u8"south";
      break;
    case 0:
    default:
      facing = u8"west";
      break;
    }
    p[u8"facing"] = facing;
  }

  static Converter Torch(std::u8string prefix) {
    return [prefix](String const &bName, CompoundTag const &s, Props &p) -> String {
      auto t = s.string(u8"torch_facing_direction", u8"top");
      std::u8string f;
      if (t == u8"top" || t == u8"unknown") {
        // "unknown" can be seen when torch as an item
        return Ns() + prefix + u8"torch";
      } else {
        if (t == u8"west") {
          f = u8"east";
        } else if (t == u8"east") {
          f = u8"west";
        } else if (t == u8"south") {
          f = u8"north";
        } else { // north
          f = u8"south";
        }
        p[u8"facing"] = f;
        return Ns() + prefix + u8"wall_torch";
      }
    };
  }

  static void FacingFromWeirdoDirection(CompoundTag const &s, Props &p) {
    auto weirdoDirection = s.int32(u8"weirdo_direction", 0);
    std::u8string facing = u8"east";
    switch (weirdoDirection) {
    case 2:
      facing = u8"south";
      break;
    case 3:
      facing = u8"north";
      break;
    case 1:
      facing = u8"west";
      break;
    case 0:
    default:
      facing = u8"east";
      break;
    }
    p[u8"facing"] = facing;
  }

  static bool FacingFromCardinalDirection(CompoundTag const &s, Props &p) {
    if (auto cardinalDirection = s.string(u8"minecraft:cardinal_direction"); cardinalDirection) {
      p[u8"facing"] = *cardinalDirection;
      return true;
    } else {
      return false;
    }
  }

  static void HalfFromUpperBlockBit(CompoundTag const &s, Props &props) {
    auto upper = s.boolean(u8"upper_block_bit", false);
    props[u8"half"] = upper ? u8"upper" : u8"lower";
  }

  static void HalfFromUpsideDownBit(CompoundTag const &s, Props &props) {
    auto upsideDown = s.boolean(u8"upside_down_bit", false);
    props[u8"half"] = upsideDown ? u8"top" : u8"bottom";
  }

  static void Lit(CompoundTag const &s, Props &p) {
    auto lit = s.boolean(u8"lit", false);
    p[u8"lit"] = Bool(lit);
  }

  static void OpenFromOpenBit(CompoundTag const &s, Props &props) {
    auto open = s.boolean(u8"open_bit", false);
    props[u8"open"] = Bool(open);
  }

  static void PersistentFromPersistentBit(CompoundTag const &s, Props &props) {
    auto persistent = s.boolean(u8"persistent_bit", false);
    props[u8"persistent"] = Bool(persistent);
  }

  static void PowerFromRedstoneSignal(CompoundTag const &s, Props &p) {
    auto signal = s.int32(u8"redstone_signal", 0);
    p[u8"power"] = Int(signal);
  }

  static void RotationFromGroundSignDirection(CompoundTag const &s, Props &p) {
    auto groundSignRotation = s.int32(u8"ground_sign_direction", 0);
    p[u8"rotation"] = Int(groundSignRotation);
  }

  static std::u8string ShapeFromRailDirection(i32 railDirection) {
    switch (railDirection) {
    case 1:
      return u8"east_west";
    case 2:
      return u8"ascending_east";
    case 5:
      return u8"ascending_south";
    case 3:
      return u8"ascending_west";
    case 4:
      return u8"ascending_north";
    case 9:
      return u8"north_east";
    case 8:
      return u8"north_west";
    case 6:
      return u8"south_east";
    case 7:
      return u8"south_west";
    case 0:
    default:
      return u8"north_south";
    }
  }

  static void StageFromAgeBit(CompoundTag const &s, Props &p) {
    auto age = s.byte(u8"age_bit", 0);
    p[u8"stage"] = Int(age);
  }

  static std::u8string StoneTypeFromStone2(std::u8string const &stoneType2) {
    std::u8string name;
    if (stoneType2 == u8"prismarine_dark") {
      name = u8"dark_prismarine";
    } else if (stoneType2 == u8"prismarine_rough") {
      name = u8"prismarine";
    } else {
      // smooth_sandstone
      // red_sandstone
      // mossy_cobblestone
      // prismarine_brick
      // purpur
      // red_nether_brick
      name = stoneType2;
    }
    return name;
  }

  static void Submergible(CompoundTag const &s, Props &p) {
    p[u8"waterlogged"] = u8"false";
  }

  static bool MushroomProperties(CompoundTag const &s, Props &p) {
    auto bits = s.int32(u8"huge_mushroom_bits", 0);
    bool up = false;
    bool down = false;
    bool north = false;
    bool east = false;
    bool south = false;
    bool west = false;
    bool stem = false;
    switch (bits) {
    case 0:
      up = false;
      down = false;
      north = false;
      east = false;
      south = false;
      west = false;
      break;
    case 1:
      up = true;
      west = true;
      north = true;
      down = false;
      east = false;
      south = false;
      break;
    case 2:
      up = true;
      north = true;
      down = false;
      east = false;
      south = false;
      west = false;
      break;
    case 3:
      up = true;
      north = true;
      east = true;
      down = false;
      south = false;
      west = false;
      break;
    case 4:
      up = true;
      west = true;
      down = false;
      north = false;
      east = false;
      south = false;
      break;
    case 5:
      up = true;
      down = false;
      north = false;
      east = false;
      south = false;
      west = false;
      break;
    case 6:
      up = true;
      east = true;
      down = false;
      north = false;
      south = false;
      west = false;
      break;
    case 7:
      up = true;
      south = true;
      west = true;
      down = false;
      north = false;
      east = false;
      break;
    case 8:
      up = true;
      south = true;
      down = false;
      north = false;
      east = false;
      west = false;
      break;
    case 9:
      up = true;
      east = true;
      south = true;
      down = false;
      north = false;
      west = false;
      break;
    case 10:
      north = true;
      east = true;
      south = true;
      west = true;
      up = false;
      down = false;
      stem = true;
      break;
    case 15:
      stem = true;
      // fallthrough
    case 14:
      up = true;
      down = true;
      north = true;
      east = true;
      south = true;
      west = true;
      break;
    }
    p[u8"up"] = Bool(up);
    p[u8"down"] = Bool(down);
    p[u8"north"] = Bool(north);
    p[u8"east"] = Bool(east);
    p[u8"south"] = Bool(south);
    p[u8"west"] = Bool(west);
    return stem;
  }

  static void TypeFromVerticalHalfMigratingTopSlotBit(CompoundTag const &s, Props &p) {
    if (auto vf = s.string(u8"minecraft:vertical_half"); vf) {
      if (*vf == u8"double") {
        p[u8"type"] = u8"bottom";
      } else {
        p[u8"type"] = *vf;
      }
    } else {
      auto topSlot = s.boolean(u8"top_slot_bit", false);
      p[u8"type"] = topSlot ? u8"top" : u8"bottom";
    }
  }

  static std::u8string WallConnectionType(std::u8string const &type) {
    // short,tall,none => low,tall,none
    if (type == u8"short") {
      return u8"low";
    } else {
      return type;
    }
  }

  static void WallProperties(CompoundTag const &s, Props &p) {
    auto connectionTypeEast = s.string(u8"wall_connection_type_east", u8"short");
    auto connectionTypeNorth = s.string(u8"wall_connection_type_north", u8"short");
    auto connectionTypeSouth = s.string(u8"wall_connection_type_south", u8"short");
    auto connectionTypeWest = s.string(u8"wall_connection_type_west", u8"short");
    auto post = s.boolean(u8"wall_post_bit", false);
    p[u8"east"] = WallConnectionType(connectionTypeEast);
    p[u8"north"] = WallConnectionType(connectionTypeNorth);
    p[u8"south"] = WallConnectionType(connectionTypeSouth);
    p[u8"west"] = WallConnectionType(connectionTypeWest);
    p[u8"up"] = Bool(post);
  }

  static void SculkSensorPhase(CompoundTag const &s, Props &p) {
    auto phaseB = s.int32(u8"sculk_sensor_phase", 0);
    std::u8string phaseJ = u8"inactive";
    switch (phaseB) {
    case 1:
      phaseJ = u8"active";
      break;
    case 2:
      phaseJ = u8"cooldown";
      break;
    }
    p[u8"sculk_sensor_phase"] = phaseJ;
  }
#pragma endregion

  static std::unordered_map<std::u8string_view, Converter> *CreateTable() {
    using namespace std;
    auto table = new std::unordered_map<u8string_view, Converter>();
#define E(__name, __converter)                       \
  assert(table->find(u8"" #__name) == table->end()); \
  if (string(#__converter) != "Same") {              \
    table->try_emplace(u8"" #__name, __converter);   \
  }

    E(acacia_button, Button);
    E(wooden_button, Button);
    E(birch_button, Button);
    E(jungle_button, Button);
    E(dark_oak_button, Button);
    E(stone_button, Button);
    E(crimson_button, Button);
    E(warped_button, Button);
    E(polished_blackstone_button, Button);
    E(spruce_button, Button);

    E(wooden_door, Door);
    E(iron_door, Door);
    E(spruce_door, Door);
    E(birch_door, Door);
    E(jungle_door, Door);
    E(acacia_door, Door);
    E(dark_oak_door, Door);
    E(crimson_door, Door);
    E(warped_door, Door);

    E(fence, FenceLegacy); // legacy
    E(oak_fence, BlockWithSubmergible);
    E(spruce_fence, BlockWithSubmergible);
    E(birch_fence, BlockWithSubmergible);
    E(jungle_fence, BlockWithSubmergible);
    E(acacia_fence, BlockWithSubmergible);
    E(dark_oak_fence, BlockWithSubmergible);
    E(crimson_fence, BlockWithSubmergible);
    E(nether_brick_fence, BlockWithSubmergible);
    E(warped_fence, BlockWithSubmergible);

    E(acacia_fence_gate, BlockWithDirection);
    E(birch_fence_gate, BlockWithDirection);
    E(crimson_fence_gate, BlockWithDirection);
    E(dark_oak_fence_gate, BlockWithDirection);
    E(jungle_fence_gate, BlockWithDirection);
    E(fence_gate, BlockWithDirection);
    E(spruce_fence_gate, BlockWithDirection);
    E(warped_fence_gate, BlockWithDirection);

    E(leaves, LeavesLegacy);
    E(leaves2, Leaves2);
    E(oak_leaves, BlockWithPersistentFromPersistentBitSubmergible);
    E(birch_leaves, BlockWithPersistentFromPersistentBitSubmergible);
    E(spruce_leaves, BlockWithPersistentFromPersistentBitSubmergible);
    E(acacia_leaves, BlockWithPersistentFromPersistentBitSubmergible);
    E(jungle_leaves, BlockWithPersistentFromPersistentBitSubmergible);
    E(dark_oak_leaves, BlockWithPersistentFromPersistentBitSubmergible);
    E(log2, Log2Legacy);
    E(log, LogLegacy);
    E(oak_log, BlockWithAxisFromPillarAxis);
    E(spruce_log, BlockWithAxisFromPillarAxis);
    E(birch_log, BlockWithAxisFromPillarAxis);
    E(jungle_log, BlockWithAxisFromPillarAxis);
    E(acacia_log, BlockWithAxisFromPillarAxis);
    E(dark_oak_log, BlockWithAxisFromPillarAxis);
    E(planks, PlanksLegacy);

    E(acacia_pressure_plate, PressurePlate);
    E(birch_pressure_plate, PressurePlate);
    E(crimson_pressure_plate, PressurePlate);
    E(dark_oak_pressure_plate, PressurePlate);
    E(jungle_pressure_plate, PressurePlate);
    E(polished_blackstone_pressure_plate, PressurePlate);
    E(spruce_pressure_plate, PressurePlate);
    E(stone_pressure_plate, PressurePlate);
    E(warped_pressure_plate, PressurePlate);

    E(sapling, SaplingLegacy);
    C sapling(Same, StageFromAgeBit);
    E(oak_sapling, sapling);
    E(birch_sapling, sapling);
    E(spruce_sapling, sapling);
    E(acacia_sapling, sapling);
    E(jungle_sapling, sapling);
    E(dark_oak_sapling, sapling);

    E(acacia_standing_sign, StandingSign);
    E(birch_standing_sign, StandingSign);
    E(crimson_standing_sign, StandingSign);
    E(jungle_standing_sign, StandingSign);
    E(standing_sign, StandingSign);
    E(spruce_standing_sign, StandingSign);
    E(warped_standing_sign, StandingSign);

    E(wooden_slab, WoodenSlab); // legacy
    E(oak_slab, Slab);
    E(birch_slab, Slab);
    E(spruce_slab, Slab);
    E(acacia_slab, Slab);
    E(jungle_slab, Slab);
    E(dark_oak_slab, Slab);
    E(double_wooden_slab, DoubleWoodenSlab);
    E(oak_double_slab, DoubleSlab(u8"oak_slab"));
    E(birch_double_slab, DoubleSlab(u8"birch_slab"));
    E(spruce_double_slab, DoubleSlab(u8"spruce_slab"));
    E(acacia_double_slab, DoubleSlab(u8"acacia_slab"));
    E(jungle_double_slab, DoubleSlab(u8"jungle_slab"));
    E(dark_oak_double_slab, DoubleSlab(u8"dark_oak_slab"));

    E(acacia_stairs, Stairs);
    E(andesite_stairs, Stairs);
    E(birch_stairs, Stairs);
    E(blackstone_stairs, Stairs);
    E(brick_stairs, Stairs);
    E(cobbled_deepslate_stairs, Stairs);
    E(crimson_stairs, Stairs);
    E(cut_copper_stairs, Stairs);
    E(dark_oak_stairs, Stairs);
    E(dark_prismarine_stairs, Stairs);
    E(deepslate_brick_stairs, Stairs);
    E(deepslate_tile_stairs, Stairs);
    E(diorite_stairs, Stairs);
    E(end_brick_stairs, RenameStairs(u8"end_stone_brick_stairs"));
    E(exposed_cut_copper_stairs, Stairs);
    E(granite_stairs, Stairs);
    E(jungle_stairs, Stairs);
    E(mossy_cobblestone_stairs, Stairs);
    E(mossy_stone_brick_stairs, Stairs);
    E(nether_brick_stairs, Stairs);
    E(oak_stairs, Stairs);
    E(oxidized_cut_copper_stairs, Stairs);
    E(polished_andesite_stairs, Stairs);
    E(polished_blackstone_brick_stairs, Stairs);
    E(polished_blackstone_stairs, Stairs);
    E(polished_deepslate_stairs, Stairs);
    E(polished_diorite_stairs, Stairs);
    E(polished_granite_stairs, Stairs);
    E(prismarine_bricks_stairs, RenameStairs(u8"prismarine_brick_stairs"));
    E(prismarine_stairs, Stairs);
    E(purpur_stairs, Stairs);
    E(quartz_stairs, Stairs);
    E(red_nether_brick_stairs, Stairs);
    E(red_sandstone_stairs, Stairs);
    E(sandstone_stairs, Stairs);
    E(smooth_quartz_stairs, Stairs);
    E(smooth_red_sandstone_stairs, Stairs);
    E(smooth_sandstone_stairs, Stairs);
    E(spruce_stairs, Stairs);
    E(stone_brick_stairs, Stairs);
    E(stone_stairs, RenameStairs(u8"cobblestone_stairs"));
    E(normal_stone_stairs, RenameStairs(u8"stone_stairs"));
    E(warped_stairs, Stairs);
    E(waxed_cut_copper_stairs, Stairs);
    E(waxed_exposed_cut_copper_stairs, Stairs);
    E(waxed_oxidized_cut_copper_stairs, Stairs);
    E(waxed_weathered_cut_copper_stairs, Stairs);
    E(weathered_cut_copper_stairs, Stairs);

    E(acacia_trapdoor, Trapdoor);
    E(birch_trapdoor, Trapdoor);
    E(crimson_trapdoor, Trapdoor);
    E(dark_oak_trapdoor, Trapdoor);
    E(iron_trapdoor, Trapdoor);
    E(jungle_trapdoor, Trapdoor);
    E(trapdoor, Trapdoor);
    E(spruce_trapdoor, Trapdoor);
    E(warped_trapdoor, Trapdoor);

    E(acacia_wall_sign, BlockWithFacing4FromFacingDirectionASubmergible);
    E(birch_wall_sign, BlockWithFacing4FromFacingDirectionASubmergible);
    E(crimson_wall_sign, BlockWithFacing4FromFacingDirectionASubmergible);
    E(jungle_wall_sign, BlockWithFacing4FromFacingDirectionASubmergible);
    E(spruce_wall_sign, BlockWithFacing4FromFacingDirectionASubmergible);
    E(warped_wall_sign, BlockWithFacing4FromFacingDirectionASubmergible);

    E(wood, WoodLegacy);
    E(oak_wood, BlockWithAxisFromPillarAxis);
    E(birch_wood, BlockWithAxisFromPillarAxis);
    E(spruce_wood, BlockWithAxisFromPillarAxis);
    E(acacia_wood, BlockWithAxisFromPillarAxis);
    E(jungle_wood, BlockWithAxisFromPillarAxis);
    E(dark_oak_wood, BlockWithAxisFromPillarAxis);
    E(stripped_oak_wood, BlockWithAxisFromPillarAxis);
    E(stripped_birch_wood, BlockWithAxisFromPillarAxis);
    E(stripped_spruce_wood, BlockWithAxisFromPillarAxis);
    E(stripped_acacia_wood, BlockWithAxisFromPillarAxis);
    E(stripped_jungle_wood, BlockWithAxisFromPillarAxis);
    E(stripped_dark_oak_wood, BlockWithAxisFromPillarAxis);

    E(activator_rail, RailCanBePowered);
    E(detector_rail, RailCanBePowered);
    E(red_flower, RedFlower);
    E(amethyst_cluster, BlockWithFacing6FromBlockFaceMigratingFacingDirectionASubmergible);
    E(stone, Stone);
    E(stone_slab3, StoneSlab3); // legacy, < 1.19
    E(stone_block_slab3, StoneSlab3);
    E(double_stone_slab3, DoubleStoneSlab3); // legacy, < 1.19
    E(double_stone_block_slab3, DoubleStoneSlab3);

    E(cobblestone_wall, WallWithBlockType);
    E(blackstone_wall, BlockWithWallProperties);

    E(anvil, Anvil);
    E(melon_stem, MelonStem);
    E(pumpkin_stem, PumpkinStem);
    E(azalea_leaves, BlockWithPersistentFromPersistentBitSubmergible);
    E(bamboo, Bamboo);
    E(bamboo_sapling, Same);
    E(barrel, Barrel);
    E(basalt, BlockWithAxisFromPillarAxis);
    E(beehive, Beehive);
    E(beetroot, Beetroot);
    E(bee_nest, Beehive);
    E(bell, Bell);
    E(big_dripleaf, BigDripleaf);
    E(blackstone_slab, Slab);
    E(blackstone_double_slab, DoubleSlab(u8"blackstone_slab"));
    E(standing_banner, BlockWithRotationFromGroundSignDirection);
    E(bed, Bed);

    E(black_candle, Candle);
    E(blue_candle, Candle);
    E(brown_candle, Candle);
    E(candle, Candle);
    E(cyan_candle, Candle);
    E(gray_candle, Candle);
    E(green_candle, Candle);
    E(light_blue_candle, Candle);
    E(light_gray_candle, Candle);
    E(lime_candle, Candle);
    E(magenta_candle, Candle);
    E(orange_candle, Candle);
    E(pink_candle, Candle);
    E(purple_candle, Candle);
    E(red_candle, Candle);
    E(white_candle, Candle);
    E(yellow_candle, Candle);

    E(black_candle_cake, CandleCake);
    E(blue_candle_cake, CandleCake);
    E(brown_candle_cake, CandleCake);
    E(candle_cake, CandleCake);
    E(cyan_candle_cake, CandleCake);
    E(gray_candle_cake, CandleCake);
    E(green_candle_cake, CandleCake);
    E(light_blue_candle_cake, CandleCake);
    E(light_gray_candle_cake, CandleCake);
    E(lime_candle_cake, CandleCake);
    E(magenta_candle_cake, CandleCake);
    E(orange_candle_cake, CandleCake);
    E(pink_candle_cake, CandleCake);
    E(purple_candle_cake, CandleCake);
    E(red_candle_cake, CandleCake);
    E(white_candle_cake, CandleCake);
    E(yellow_candle_cake, CandleCake);

    E(carpet, CarpetLegacy);
    E(concrete, ConcreteLegacy);
    E(concretePowder, ConcretePowder); // legacy, < 1.18.30
    E(concrete_powder, ConcretePowder);

    E(black_glazed_terracotta, BlockWithFacing4FromFacingDirectionA);
    E(blue_glazed_terracotta, BlockWithFacing4FromFacingDirectionA);
    E(brown_glazed_terracotta, BlockWithFacing4FromFacingDirectionA);
    E(cyan_glazed_terracotta, BlockWithFacing4FromFacingDirectionA);
    E(gray_glazed_terracotta, BlockWithFacing4FromFacingDirectionA);
    E(green_glazed_terracotta, BlockWithFacing4FromFacingDirectionA);
    E(light_blue_glazed_terracotta, BlockWithFacing4FromFacingDirectionA);
    E(silver_glazed_terracotta, SilverGlazedTerracotta);
    E(lime_glazed_terracotta, BlockWithFacing4FromFacingDirectionA);
    E(magenta_glazed_terracotta, BlockWithFacing4FromFacingDirectionA);
    E(orange_glazed_terracotta, BlockWithFacing4FromFacingDirectionA);
    E(pink_glazed_terracotta, BlockWithFacing4FromFacingDirectionA);
    E(purple_glazed_terracotta, BlockWithFacing4FromFacingDirectionA);
    E(red_glazed_terracotta, BlockWithFacing4FromFacingDirectionA);
    E(white_glazed_terracotta, BlockWithFacing4FromFacingDirectionA);
    E(yellow_glazed_terracotta, BlockWithFacing4FromFacingDirectionA);

    E(shulker_box, ShulkerBoxLegacy);
    E(white_shulker_box, Same);
    E(orange_shulker_box, Same);
    E(magenta_shulker_box, Same);
    E(light_blue_shulker_box, Same);
    E(yellow_shulker_box, Same);
    E(lime_shulker_box, Same);
    E(pink_shulker_box, Same);
    E(gray_shulker_box, Same);
    E(light_gray_shulker_box, Same);
    E(cyan_shulker_box, Same);
    E(purple_shulker_box, Same);
    E(blue_shulker_box, Same);
    E(brown_shulker_box, Same);
    E(green_shulker_box, Same);
    E(red_shulker_box, Same);
    E(black_shulker_box, Same);
    E(undyed_shulker_box, Rename(u8"shulker_box"));

    E(stained_glass, StainedGlassLegacy);
    E(white_stained_glass, Same);
    E(orange_stained_glass, Same);
    E(magenta_stained_glass, Same);
    E(light_blue_stained_glass, Same);
    E(yellow_stained_glass, Same);
    E(lime_stained_glass, Same);
    E(pink_stained_glass, Same);
    E(gray_stained_glass, Same);
    E(light_gray_stained_glass, Same);
    E(cyan_stained_glass, Same);
    E(purple_stained_glass, Same);
    E(blue_stained_glass, Same);
    E(brown_stained_glass, Same);
    E(green_stained_glass, Same);
    E(red_stained_glass, Same);
    E(black_stained_glass, Same);
    E(stained_glass_pane, StainedGlassPaneLegacy);
    E(white_stained_glass_pane, BlockWithSubmergible);
    E(orange_stained_glass_pane, BlockWithSubmergible);
    E(magenta_stained_glass_pane, BlockWithSubmergible);
    E(light_blue_stained_glass_pane, BlockWithSubmergible);
    E(yellow_stained_glass_pane, BlockWithSubmergible);
    E(lime_stained_glass_pane, BlockWithSubmergible);
    E(pink_stained_glass_pane, BlockWithSubmergible);
    E(gray_stained_glass_pane, BlockWithSubmergible);
    E(light_gray_stained_glass_pane, BlockWithSubmergible);
    E(cyan_stained_glass_pane, BlockWithSubmergible);
    E(purple_stained_glass_pane, BlockWithSubmergible);
    E(blue_stained_glass_pane, BlockWithSubmergible);
    E(brown_stained_glass_pane, BlockWithSubmergible);
    E(green_stained_glass_pane, BlockWithSubmergible);
    E(red_stained_glass_pane, BlockWithSubmergible);
    E(black_stained_glass_pane, BlockWithSubmergible);
    E(stained_hardened_clay, StainedHardenedClay);
    E(wall_banner, BlockWithFacing4FromFacingDirectionA);
    E(wool, LegacyWool); // legacy, < preview 1.19.70.26
    E(blast_furnace, FurnaceAndSimilar);
    E(lit_blast_furnace, FurnaceAndSimilar);
    E(bone_block, BlockWithAxisFromPillarAxis);
    E(coral, CoralLegacy);
    E(tube_coral, BlockWithSubmergible);
    E(brain_coral, BlockWithSubmergible);
    E(bubble_coral, BlockWithSubmergible);
    E(fire_coral, BlockWithSubmergible);
    E(horn_coral, BlockWithSubmergible);
    E(dead_tube_coral, BlockWithSubmergible);
    E(dead_brain_coral, BlockWithSubmergible);
    E(dead_bubble_coral, BlockWithSubmergible);
    E(dead_fire_coral, BlockWithSubmergible);
    E(dead_horn_coral, BlockWithSubmergible);
    E(coral_block, CoralBlock); // legacy < 1.21
    E(tube_coral_block, Same);
    E(brain_coral_block, Same);
    E(bubble_coral_block, Same);
    E(fire_coral_block, Same);
    E(horn_coral_block, Same);
    E(dead_tube_coral_block, Same);
    E(dead_brain_coral_block, Same);
    E(dead_bubble_coral_block, Same);
    E(dead_fire_coral_block, Same);
    E(dead_horn_coral_block, Same);

    E(coral_fan_hang, CoralFanHang);
    E(coral_fan_hang2, CoralFanHang);
    E(coral_fan_hang3, CoralFanHang);
    E(coral_fan, CoralFanLegacy);      // legacy
    E(coral_fan_dead, CoralFanLegacy); // legacy
    E(tube_coral_fan, CoralFan);
    E(brain_coral_fan, CoralFan);
    E(bubble_coral_fan, CoralFan);
    E(fire_coral_fan, CoralFan);
    E(horn_coral_fan, CoralFan);
    E(dead_tube_coral_fan, CoralFan);
    E(dead_brain_coral_fan, CoralFan);
    E(dead_bubble_coral_fan, CoralFan);
    E(dead_fire_coral_fan, CoralFan);
    E(dead_horn_coral_fan, CoralFan);

    E(brewing_stand, BrewingStand);
    E(brick_block, Rename(u8"bricks"));
    E(stone_slab, StoneSlab); // legacy, < 1.19
    E(stone_block_slab, StoneSlab);
    E(double_stone_slab, DoubleStoneSlab); // legacy, < 1.19
    E(double_stone_block_slab, DoubleStoneSlab);
    E(brown_mushroom_block, BrownMushroomBlock);
    E(bubble_column, BubbleColumn);
    E(cactus, BlockWithAge);
    E(cake, Cake);
    E(campfire, Campfire);
    E(soul_campfire, Campfire);
    E(carrots, BlockWithAgeFromGrowth);
    E(carved_pumpkin, CarvedPumpkin);
    E(cauldron, Cauldron);
    E(lava_cauldron, Cauldron);
    E(cave_vines, CaveVines);
    E(cave_vines_with_berries, CaveVines);
    E(cave_vines_head_with_berries, CaveVines);
    E(cave_vines_body_with_berries, CaveVinesBody);
    E(chain, Chain);
    E(chain_command_block, CommandBlock);
    E(command_block, CommandBlock);
    E(chest, BlockWithFacing4FromCardinalDirectionMigratingFacingDirectionASubmergible);
    E(quartz_block, QuartzBlock);
    E(red_sandstone, RedSandstone);
    E(sandstone, Sandstone);
    E(stonebrick, Stonebrick);
    E(chorus_flower, BlockWithAge);
    E(chorus_plant, Same);
    E(dirt, Dirt);
    E(cobbled_deepslate_slab, Slab);
    E(cobbled_deepslate_double_slab, DoubleSlab(u8"cobbled_deepslate_slab"));
    E(cobbled_deepslate_wall, BlockWithWallProperties);
    E(web, Rename(u8"cobweb"));
    E(cocoa, Cocoa);
    E(powered_comparator, Comparator);
    E(unpowered_comparator, Comparator);
    E(composter, Composter);
    E(conduit, BlockWithSubmergible);
    E(skull, Same);
    E(crimson_hyphae, BlockWithAxisFromPillarAxis);
    E(crimson_slab, Slab);
    E(crimson_double_slab, DoubleSlab(u8"crimson_slab"));
    E(crimson_stem, BlockWithAxisFromPillarAxis);
    E(cut_copper_slab, Slab);
    E(double_cut_copper_slab, DoubleSlab(u8"cut_copper_slab"));
    E(stone_slab4, StoneSlab4); // legacy, < 1.19
    E(stone_block_slab4, StoneSlab4);
    E(double_stone_slab4, DoubleStoneSlab4); // legacy, < 1.19
    E(double_stone_block_slab4, DoubleStoneSlab4);
    E(yellow_flower, Rename(u8"dandelion"));
    E(darkoak_standing_sign, DarkoakStandingSign);
    E(darkoak_wall_sign, DarkoakWallSign);
    E(prismarine, Prismarine);
    E(stone_slab2, StoneSlab2); // legacy, < 1.19
    E(stone_block_slab2, StoneSlab2);
    E(double_stone_slab2, DoubleStoneSlab2); // legacy, < 1.19
    E(double_stone_block_slab2, DoubleStoneSlab2);
    E(daylight_detector, DaylightDetector);
    E(daylight_detector_inverted, DaylightDetector);
    E(deadbush, Rename(u8"dead_bush"));
    E(deepslate, BlockWithAxisFromPillarAxis);
    E(deepslate_brick_slab, Slab);
    E(deepslate_brick_double_slab, DoubleSlab(u8"deepslate_brick_slab"));
    E(deepslate_brick_wall, BlockWithWallProperties);
    E(deepslate_redstone_ore, RedstoneOre);
    E(lit_deepslate_redstone_ore, RedstoneOre);
    E(deepslate_tile_slab, Slab);
    E(deepslate_tile_double_slab, DoubleSlab(u8"deepslate_tile_slab"));
    E(deepslate_tile_wall, BlockWithWallProperties);
    E(grass_path, Rename(u8"dirt_path"));
    E(dispenser, Dispenser);
    E(dropper, Dropper);
    E(ender_chest, BlockWithFacing4FromCardinalDirectionMigratingFacingDirectionASubmergible);
    E(end_portal_frame, EndPortalFrame);
    E(end_rod, EndRod);
    E(end_bricks, Rename(u8"end_stone_bricks"));
    E(exposed_cut_copper_slab, Slab);
    E(exposed_double_cut_copper_slab, DoubleSlab(u8"exposed_cut_copper_slab"));
    E(farmland, Farmland);
    E(tallgrass, Tallgrass);
    E(fire, BlockWithAge);
    E(azalea_leaves_flowered, AzaleaLeavesFlowered);
    E(flower_pot, Same);
    E(frosted_ice, BlockWithAge);
    E(furnace, FurnaceAndSimilar);
    E(lit_furnace, FurnaceAndSimilar);
    E(glass_pane, BlockWithSubmergible);
    E(glow_lichen, BlockWithMultiFaceDirectionBitsSubmergible);
    E(grass, GrassBlock); // legacy
    E(grass_block, GrassBlock);

    E(grindstone, Grindstone);
    E(hanging_roots, BlockWithSubmergible);
    E(hay_block, BlockWithAxisFromPillarAxis);
    E(heavy_weighted_pressure_plate, BlockWithPowerFromRedstoneSignal);
    E(hopper, Hopper);
    E(monster_egg, MonsterEgg);
    E(infested_deepslate, BlockWithAxisFromPillarAxis);
    E(iron_bars, BlockWithSubmergible);
    E(lit_pumpkin, LitPumpkin);
    E(jukebox, Same);
    E(kelp, Kelp);
    E(ladder, BlockWithFacing4FromFacingDirectionASubmergible);
    E(lantern, Lantern);
    E(soul_lantern, Lantern);
    E(large_amethyst_bud, BlockWithFacing6FromBlockFaceMigratingFacingDirectionASubmergible);
    E(double_plant, DoublePlantLegacy);
    E(lava, Liquid);
    E(lectern, Lectern);
    E(lever, Lever);
    E(lightning_rod, BlockWithFacing6FromFacingDirectionASubmergible);
    E(light_block, LightBlock);
    E(light_weighted_pressure_plate, BlockWithPowerFromRedstoneSignal);
    E(waterlily, Rename(u8"lily_pad"));
    E(loom, BlockWithFacing4FromDirectionA);
    E(magma, Rename(u8"magma_block"));
    E(medium_amethyst_bud, BlockWithFacing6FromBlockFaceMigratingFacingDirectionASubmergible);
    E(melon_block, Rename(u8"melon"));
    E(movingBlock, Same); // legacy, < 1.18.30
    E(moving_block, Same);
    E(mycelium, BlockWithSnowy); // No "snowy" property in BE
    E(nether_brick, Rename(u8"nether_bricks"));
    E(portal, Portal);
    E(quartz_ore, Rename(u8"nether_quartz_ore"));
    E(nether_wart, BlockWithAge);
    E(noteblock, Rename(u8"note_block"));
    E(wooden_pressure_plate, WoodenPressurePlate);
    E(wall_sign, OakWallSign);
    E(observer, Observer);
    E(oxidized_cut_copper_slab, Slab);
    E(oxidized_double_cut_copper_slab, DoubleSlab(u8"oxidized_cut_copper_slab"));
    E(piston, BlockWithFacing6FromFacingDirection6);
    E(sticky_piston, BlockWithFacing6FromFacingDirection6);
    E(podzol, BlockWithSnowy); // No "snowy" property in BE
    E(pointed_dripstone, PointedDripstone);
    E(polished_basalt, BlockWithAxisFromPillarAxis);
    E(polished_blackstone_brick_slab, Slab);
    E(polished_blackstone_brick_double_slab, DoubleSlab(u8"polished_blackstone_brick_slab"));
    E(polished_blackstone_brick_wall, BlockWithWallProperties);
    E(polished_blackstone_slab, Slab);
    E(polished_blackstone_double_slab, DoubleSlab(u8"polished_blackstone_slab"));
    E(polished_blackstone_wall, BlockWithWallProperties);
    E(polished_deepslate_slab, Slab);
    E(polished_deepslate_double_slab, DoubleSlab(u8"polished_deepslate_slab"));
    E(polished_deepslate_wall, BlockWithWallProperties);
    E(potatoes, BlockWithAgeFromGrowth);
    E(golden_rail, GoldenRail);
    E(pumpkin, Same);
    E(purpur_block, PurpurBlock);
    E(rail, Rail);
    E(redstone_lamp, RedstoneLamp);
    E(lit_redstone_lamp, RedstoneLamp);
    E(redstone_ore, RedstoneOre);
    E(lit_redstone_ore, RedstoneOre);
    E(redstone_torch, RedstoneTorch);
    E(unlit_redstone_torch, RedstoneTorch);
    E(redstone_wire, BlockWithPowerFromRedstoneSignal);
    E(red_mushroom_block, RedMushroomBlock);
    E(red_nether_brick, Rename(u8"red_nether_bricks"));
    E(sand, Sand);
    E(powered_repeater, Repeater);
    E(unpowered_repeater, Repeater);
    E(repeating_command_block, CommandBlock);
    E(respawn_anchor, RespawnAnchor);
    E(dirt_with_roots, Rename(u8"rooted_dirt"));
    E(scaffolding, Scaffolding);
    E(sculk_sensor, C(Same, SculkSensorPhase, Submergible));
    E(seagrass, Seagrass);
    E(sea_pickle, SeaPickle);
    E(seaLantern, Rename(u8"sea_lantern")); // legacy, < 1.18.30
    E(sea_lantern, Same);
    E(slime, Rename(u8"slime_block"));
    E(small_amethyst_bud, BlockWithFacing6FromBlockFaceMigratingFacingDirectionASubmergible);
    E(small_dripleaf_block, SmallDripleafBlock);
    E(smoker, FurnaceAndSimilar);
    E(lit_smoker, FurnaceAndSimilar);
    E(snow_layer, SnowLayer);
    E(snow, Rename(u8"snow_block"));
    E(soul_torch, Torch(u8"soul_"));
    E(sponge, Sponge);
    E(stonecutter_block, StonecutterBlock);
    E(stripped_acacia_log, BlockWithAxisFromPillarAxis);
    E(stripped_birch_log, BlockWithAxisFromPillarAxis);
    E(stripped_crimson_hyphae, BlockWithAxisFromPillarAxis);
    E(stripped_crimson_stem, BlockWithAxisFromPillarAxis);
    E(stripped_dark_oak_log, BlockWithAxisFromPillarAxis);
    E(stripped_jungle_log, BlockWithAxisFromPillarAxis);
    E(stripped_oak_log, BlockWithAxisFromPillarAxis);
    E(stripped_spruce_log, BlockWithAxisFromPillarAxis);
    E(stripped_warped_hyphae, BlockWithAxisFromPillarAxis);
    E(stripped_warped_stem, BlockWithAxisFromPillarAxis);
    E(structure_block, StructureBlock);
    E(structure_void, Same);
    E(reeds, SugarCane); // legacy
    E(sugar_cane, SugarCane);
    E(sweet_berry_bush, SweetBerryBush);
    E(target, Target);
    E(hardened_clay, Rename(u8"terracotta"));
    E(tnt, Tnt);
    E(torch, Torch(u8""));
    E(trapped_chest, BlockWithFacing4FromCardinalDirectionMigratingFacingDirectionASubmergible);
    E(tripWire, Tripwire); // legacy, < 1.18.30
    E(trip_wire, Tripwire);
    E(tripwire_hook, TripwireHook);
    E(turtle_egg, TurtleEgg);
    E(twisting_vines, NetherVines(u8"twisting"));
    E(vine, Vine);
    E(warped_hyphae, BlockWithAxisFromPillarAxis);
    E(warped_slab, Slab);
    E(warped_double_slab, DoubleSlab(u8"warped_slab"));
    E(warped_stem, BlockWithAxisFromPillarAxis);
    E(water, Liquid);
    E(waxed_copper, Rename(u8"waxed_copper_block"));
    E(waxed_cut_copper_slab, Slab);
    E(waxed_double_cut_copper_slab, DoubleSlab(u8"waxed_cut_copper_slab"));
    E(waxed_exposed_cut_copper_slab, Slab);
    E(waxed_exposed_double_cut_copper_slab, DoubleSlab(u8"waxed_exposed_cut_copper_slab"));
    E(waxed_oxidized_cut_copper_slab, Slab);
    E(waxed_oxidized_double_cut_copper_slab, DoubleSlab(u8"waxed_oxidized_cut_copper_slab"));
    E(waxed_weathered_cut_copper_slab, Slab);
    E(waxed_weathered_double_cut_copper_slab, DoubleSlab(u8"waxed_weathered_cut_copper_slab"));
    E(weathered_cut_copper_slab, Slab);
    E(weathered_double_cut_copper_slab, DoubleSlab(u8"weathered_cut_copper_slab"));
    E(weeping_vines, NetherVines(u8"weeping"));
    E(wheat, BlockWithAgeFromGrowth);
    E(mob_spawner, Rename(u8"spawner"));
    E(frame, Rename(u8"air"));
    E(glow_frame, Rename(u8"air"));
    E(pistonArmCollision, PistonArmCollision); // legacy, < 1.18.30
    E(piston_arm_collision, PistonArmCollision);
    E(stickyPistonArmCollision, PistonArmCollision); // legacy, 1.18.30
    E(sticky_piston_arm_collision, PistonArmCollision);
    E(flowing_lava, LiquidFlowing);
    E(flowing_water, LiquidFlowing);
    E(jigsaw, Jigsaw);

    // 1.19
    E(frog_spawn, Rename(u8"frogspawn"));
    E(mangrove_log, BlockWithAxisFromPillarAxis);
    E(stripped_mangrove_log, BlockWithAxisFromPillarAxis);
    E(mangrove_wood, BlockWithAxisFromPillarAxis);
    E(stripped_mangrove_wood, BlockWithAxisFromPillarAxis);
    E(ochre_froglight, BlockWithAxisFromPillarAxis);
    E(verdant_froglight, BlockWithAxisFromPillarAxis);
    E(pearlescent_froglight, BlockWithAxisFromPillarAxis);
    E(mangrove_slab, Slab);
    E(mangrove_double_slab, DoubleSlab(u8"mangrove_slab"));
    E(mangrove_button, Button);
    E(mangrove_door, Door);
    E(mangrove_fence, BlockWithSubmergible);
    E(mangrove_fence_gate, BlockWithDirection);
    E(mangrove_pressure_plate, PressurePlate);
    E(mangrove_standing_sign, StandingSign);
    E(mangrove_stairs, Stairs);
    E(mud_brick_stairs, Stairs);
    E(mangrove_trapdoor, Trapdoor);
    E(mangrove_wall_sign, BlockWithFacing4FromFacingDirectionASubmergible);
    E(mud_brick_wall, BlockWithWallProperties);
    E(sculk_vein, BlockWithMultiFaceDirectionBitsSubmergible);
    E(mangrove_propagule, MangrovePropagule);
    E(mangrove_leaves, BlockWithPersistentFromPersistentBitSubmergible);
    E(mangrove_roots, BlockWithSubmergible);
    E(sculk_shrieker, SculkShrieker);
    E(sculk_catalyst, SculkCatalyst);
    E(mud_brick_slab, Slab);
    E(mud_brick_double_slab, DoubleSlab(u8"mud_brick_slab"));
    E(muddy_mangrove_roots, BlockWithAxisFromPillarAxis);

    // 1.19.3
    E(acacia_hanging_sign, HangingSign);
    E(bamboo_button, Button);
    E(bamboo_door, Door);
    E(bamboo_fence, BlockWithSubmergible);
    E(bamboo_fence_gate, BlockWithDirection);
    E(bamboo_hanging_sign, HangingSign);
    E(bamboo_mosaic_slab, Slab);
    E(bamboo_mosaic_double_slab, DoubleSlab(u8"bamboo_mosaic_slab"));
    E(bamboo_mosaic_stairs, Stairs);
    E(bamboo_pressure_plate, PressurePlate);
    E(bamboo_standing_sign, StandingSign);
    E(bamboo_slab, Slab);
    E(bamboo_double_slab, DoubleSlab(u8"bamboo_slab"));
    E(bamboo_stairs, Stairs);
    E(bamboo_trapdoor, Trapdoor);
    E(bamboo_wall_sign, BlockWithFacing4FromFacingDirectionASubmergible);
    E(chiseled_bookshelf, ChiseledBookshelf);
    E(birch_hanging_sign, HangingSign);
    E(crimson_hanging_sign, HangingSign);
    E(dark_oak_hanging_sign, HangingSign);
    E(jungle_hanging_sign, HangingSign);
    E(mangrove_hanging_sign, HangingSign);
    E(oak_hanging_sign, HangingSign);
    E(spruce_hanging_sign, HangingSign);
    E(warped_hanging_sign, HangingSign);

    // 1.20
    E(bamboo_block, BlockWithAxisFromPillarAxis);
    E(stripped_bamboo_block, BlockWithAxisFromPillarAxis);
    E(decorated_pot, DecoratedPot);
    E(torchflower_crop, C(Same, AgeFromGrowthNonLinear));
    E(cherry_slab, Slab);
    E(cherry_double_slab, DoubleSlab(u8"cherry_slab"));
    E(pink_petals, PinkPetals);
    E(cherry_wood, BlockWithAxisFromPillarAxis);
    E(stripped_cherry_wood, BlockWithAxisFromPillarAxis);
    E(cherry_log, BlockWithAxisFromPillarAxis);
    E(stripped_cherry_log, BlockWithAxisFromPillarAxis);
    E(cherry_stairs, Stairs);
    E(cherry_button, Button);
    E(cherry_hanging_sign, HangingSign);
    E(cherry_standing_sign, StandingSign);
    E(cherry_wall_sign, BlockWithFacing4FromFacingDirectionASubmergible);
    E(cherry_door, Door);
    E(cherry_fence_gate, BlockWithDirection);
    E(cherry_fence, BlockWithSubmergible);
    E(cherry_trapdoor, Trapdoor);
    E(cherry_sapling, sapling);
    E(cherry_pressure_plate, PressurePlate);
    E(cherry_leaves, BlockWithPersistentFromPersistentBitSubmergible);
    E(pitcher_crop, C(Same, AgeFromGrowth, HalfFromUpperBlockBit));
    E(pitcher_plant, C(Same, HalfFromUpperBlockBit));
    E(sniffer_egg, SnifferEgg);
    E(calibrated_sculk_sensor, C(Same, SculkSensorPhase, FacingFromCardinalDirectionMigratingDirectionNorth2East3South0West1, Submergible));

    E(barrier, C(Same, Submergible));

    // 1.20.3
    E(tuff_stairs, Stairs);
    E(tuff_slab, Slab);
    E(tuff_double_slab, DoubleSlab(u8"tuff_slab"));
    E(tuff_wall, BlockWithWallProperties);
    E(polished_tuff_slab, Slab);
    E(polished_tuff_stairs, Stairs);
    E(polished_tuff_double_slab, DoubleSlab(u8"polished_tuff_slab"));
    E(polished_tuff_wall, BlockWithWallProperties);
    E(tuff_brick_slab, Slab);
    E(tuff_brick_double_slab, DoubleSlab(u8"tuff_brick_slab"));
    E(tuff_brick_stairs, Stairs);
    E(tuff_brick_wall, BlockWithWallProperties);
    E(copper_grate, BlockWithSubmergible);
    E(exposed_copper_grate, BlockWithSubmergible);
    E(weathered_copper_grate, BlockWithSubmergible);
    E(oxidized_copper_grate, BlockWithSubmergible);
    E(waxed_copper_grate, BlockWithSubmergible);
    E(waxed_exposed_copper_grate, BlockWithSubmergible);
    E(waxed_weathered_copper_grate, BlockWithSubmergible);
    E(waxed_oxidized_copper_grate, BlockWithSubmergible);
    E(copper_door, Door);
    E(exposed_copper_door, Door);
    E(weathered_copper_door, Door);
    E(oxidized_copper_door, Door);
    E(waxed_copper_door, Door);
    E(waxed_exposed_copper_door, Door);
    E(waxed_weathered_copper_door, Door);
    E(waxed_oxidized_copper_door, Door);
    E(copper_trapdoor, Trapdoor);
    E(exposed_copper_trapdoor, Trapdoor);
    E(weathered_copper_trapdoor, Trapdoor);
    E(oxidized_copper_trapdoor, Trapdoor);
    E(waxed_copper_trapdoor, Trapdoor);
    E(waxed_exposed_copper_trapdoor, Trapdoor);
    E(waxed_weathered_copper_trapdoor, Trapdoor);
    E(waxed_oxidized_copper_trapdoor, Trapdoor);
    E(copper_bulb, CopperBulb);
    E(exposed_copper_bulb, CopperBulb);
    E(weathered_copper_bulb, CopperBulb);
    E(oxidized_copper_bulb, CopperBulb);
    E(waxed_copper_bulb, CopperBulb);
    E(waxed_exposed_copper_bulb, CopperBulb);
    E(waxed_weathered_copper_bulb, CopperBulb);
    E(waxed_oxidized_copper_bulb, CopperBulb);
    E(crafter, Crafter);

    // 1.21
    E(vault, Vault);
    E(heavy_core, BlockWithSubmergible);
    E(trial_spawner, TrialSpawner);
    E(smooth_stone_slab, Slab);
    E(cobblestone_slab, Slab);
    E(stone_brick_slab, Slab);
    E(sandstone_slab, Slab);
    E(brick_slab, Slab);
    E(nether_brick_slab, Slab);
    E(quartz_slab, Slab);
    C doublePlant(Same, HalfFromUpperBlockBit);
    E(large_fern, doublePlant);
    E(tall_grass, doublePlant);
    E(sunflower, doublePlant);
    E(lilac, doublePlant);
    E(rose_bush, doublePlant);
    E(peony, doublePlant);

    // 1.21.4
    E(pale_oak_leaves, BlockWithPersistentFromPersistentBitSubmergible);
    E(pale_oak_slab, Slab);
    E(pale_oak_double_slab, DoubleSlab(u8"pale_oak_slab"));
    E(pale_oak_fence, BlockWithSubmergible);
    E(pale_oak_fence_gate, BlockWithDirection);
    E(pale_oak_door, Door);
    E(pale_oak_trapdoor, Trapdoor);
    E(pale_oak_pressure_plate, PressurePlate);
    E(pale_oak_button, Button);
    E(pale_oak_stairs, Stairs);
    E(pale_moss_carpet, PaleMossCarpet);
    E(resin_clump, BlockWithMultiFaceDirectionBitsSubmergible);
    E(resin_brick_stairs, Stairs);
    E(resin_brick_slab, Slab);
    E(resin_brick_double_slab, DoubleSlab(u8"resin_brick_slab"));
    E(resin_brick_wall, BlockWithWallProperties);
#undef E

    return table;
  }

#pragma region Utilities
  static std::u8string Bool(bool b) {
    return b ? u8"true" : u8"false";
  }

  static std::u8string Int(i32 i) {
    return mcfile::String::ToString(i);
  }

  static inline std::u8string Ns() {
    return u8"minecraft:";
  }

  // Get "acacia" from "minecraft:acacia_pressure_plate" when suffix is "_pressure_plate"
  static inline std::u8string VariantFromName(std::u8string const &name, std::u8string const &suffix) {
    assert(name.starts_with(Ns()) && name.ends_with(suffix));
    return name.substr(Ns().size()).substr(0, name.size() - Ns().size() - suffix.size());
  }
#pragma endregion
};

std::shared_ptr<mcfile::je::Block const> BlockData::From(mcfile::be::Block const &b, int dataVersion) {
  return Impl::From(b, dataVersion);
}

std::shared_ptr<mcfile::je::Block const> BlockData::Identity(mcfile::be::Block const &b, int dataVersion) {
  return Impl::Identity(b, dataVersion);
}

} // namespace je2be::bedrock
