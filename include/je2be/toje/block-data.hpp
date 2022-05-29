#pragma once

namespace je2be::toje {

class BlockData {
  using String = std::string;
  using Props = std::map<std::string, std::string>;
  using Converter = std::function<String(String const &bName, CompoundTag const &s, Props &p)>;

public:
  static std::shared_ptr<mcfile::je::Block const> From(mcfile::be::Block const &b) {
    using namespace std;
    using namespace mcfile;
    static unique_ptr<unordered_map<string, Converter> const> const sTable(CreateTable());

    auto found = sTable->find(b.fName);
    if (found == sTable->end()) {
      return Identity(b);
    } else {
      static CompoundTag const sEmpty;
      CompoundTag const &states = b.fStates ? *b.fStates : sEmpty;
      map<string, string> props;
      auto name = found->second(b.fName, states, props);
      return make_shared<mcfile::je::Block const>(name, props);
    }
  }

  static std::shared_ptr<mcfile::je::Block const> Identity(mcfile::be::Block const &b) {
    return std::make_shared<mcfile::je::Block const>(b.fName);
  }

private:
  BlockData() = delete;

#pragma region Converters : A
  static String Anvil(String const &bName, CompoundTag const &s, Props &p) {
    auto damage = s.string("damage", "undamaged");
    std::string name = "anvil";
    if (damage == "slightly_damaged") {
      name = "chipped_anvil";
    } else if (damage == "very_damaged") {
      name = "damaged_anvil";
    } else {
      // undamaged
      name = "anvil";
    }
    Facing4FromDirectionA(s, p);
    return Ns() + name;
  }

  static String AzaleaLeavesFlowered(String const &bName, CompoundTag const &s, Props &p) {
    PersistentFromPersistentBit(s, p);
    Submergible(s, p);
    return Ns() + "flowering_azalea_leaves";
  }
#pragma endregion

#pragma region Converters : B
  static String Bamboo(String const &bName, CompoundTag const &s, Props &p) {
    auto leafSize = s.string("bamboo_leaf_size", "large_leaves");
    std::string leaves = "none";
    if (leafSize == "no_leaves") {
      leaves = "none";
    } else if (leafSize == "large_leaves") {
      leaves = "large";
    } else if (leafSize == "small_leaves") {
      leaves = "small";
    }
    p["leaves"] = leaves;

    auto stalkThickness = s.string("bamboo_stalk_thickness", "thin");
    int age = 0; // thin
    if (stalkThickness == "thick") {
      age = 1;
    }
    p["age"] = Int(age);

    auto ageBit = s.boolean("age_bit", false);
    p["stage"] = ageBit ? "1" : "0";
    return bName;
  }

  static String Barrel(String const &bName, CompoundTag const &s, Props &p) {
    OpenFromOpenBit(s, p);
    Facing6FromFacingDirectionA(s, p);
    return bName;
  }

  static String Beehive(String const &bName, CompoundTag const &s, Props &p) {
    Facing4FromDirectionA(s, p);
    auto honeyLevel = s.int32("honey_level", 0);
    p["honey_level"] = Int(honeyLevel);
    return bName;
  }

  static String Beetroot(String const &bName, CompoundTag const &s, Props &p) {
    AgeFromGrowthNonLinear(s, p);
    return Ns() + "beetroots";
  }

  static String Bed(String const &bName, CompoundTag const &s, Props &p) {
    Facing4FromDirectionA(s, p);
    auto headPiece = s.boolean("head_piece_bit", false);
    auto occupied = s.boolean("occupied_bit", false);
    p["part"] = headPiece ? "head" : "foot";
    p["occupied"] = Bool(occupied);
    return Ns() + "white_bed";
  }

  static String Bell(String const &bName, CompoundTag const &s, Props &p) {
    auto attachment = s.string("attachment", "floor");
    auto direction = s.int32("direction", 0);
    auto toggle = s.boolean("toggle_bit", false);
    std::string facing;
    switch (direction) {
    case 1:
      facing = "east";
      break;
    case 2:
      facing = "south";
      break;
    case 3:
      facing = "west";
      break;
    case 0:
    default:
      facing = "north";
      break;
    }
    std::string a;
    if (attachment == "standing") {
      a = "floor";
    } else if (attachment == "hanging") {
      a = "ceiling";
    } else if (attachment == "multiple") {
      a = "double_wall";
    } else if (attachment == "side") {
      a = "single_wall";
    } else {
      a = attachment;
    }
    p["facing"] = facing;
    p["attachment"] = a;
    p["powered"] = Bool(toggle);
    return bName;
  }

  static String BigDripleaf(String const &bName, CompoundTag const &s, Props &p) {
    auto head = s.boolean("big_dripleaf_head", false);
    Facing4FromDirectionA(s, p);
    std::string name;
    if (head) {
      // none,partial_tilt,unstable,full_tilt => none,partial,unstable,full
      auto tilt = s.string("big_dripleaf_tilt", "full_tilt");
      std::string t;
      if (tilt == "partial_tilt") {
        t = "partial";
      } else if (tilt == "full_tilt") {
        t = "full";
      } else {
        t = tilt;
      }
      p["tilt"] = t;
      name = "big_dripleaf";
    } else {
      name = "big_dripleaf_stem";
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

  static String BlockWithFacing4FromFacingDirectionASubmergible(String const &bName, CompoundTag const &s, Props &p) {
    Facing4FromFacingDirectionA(s, p);
    Submergible(s, p);
    return bName;
  }

  static String BlockWithFacing6FromFacingDirection6(String const &bName, CompoundTag const &s, Props &p) {
    Facing6FromFacingDirectionB(s, p);
    return bName;
  }

  static String BlockWithMultiFaceDirectionBitsSubmergible(String const &bName, CompoundTag const &s, Props &p) {
    auto bits = s.int32("multi_face_direction_bits", 0);
    bool down = (bits & 0x1) == 0x1;
    bool up = (bits & 0x2) == 0x2;
    bool north = (bits & 0x10) == 0x10;
    bool south = (bits & 0x4) == 0x4;
    bool west = (bits & 0x8) == 0x8;
    bool east = (bits & 0x20) == 0x20;
    p["down"] = Bool(down);
    p["up"] = Bool(up);
    p["north"] = Bool(north);
    p["south"] = Bool(south);
    p["west"] = Bool(west);
    p["east"] = Bool(east);

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
    p["snowy"] = "false";
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
    auto slotA = s.boolean("brewing_stand_slot_a_bit", false);
    auto slotB = s.boolean("brewing_stand_slot_b_bit", false);
    auto slotC = s.boolean("brewing_stand_slot_c_bit", false);
    p["has_bottle_0"] = Bool(slotA);
    p["has_bottle_1"] = Bool(slotB);
    p["has_bottle_2"] = Bool(slotC);
    return bName;
  }

  static String BrownMushroomBlock(String const &bName, CompoundTag const &s, Props &p) {
    bool stem = MushroomProperties(s, p);
    std::string name;
    if (stem) {
      name = "mushroom_stem";
    } else {
      name = "brown_mushroom_block";
    }
    return Ns() + name;
  }

  static String BubbleColumn(String const &bName, CompoundTag const &s, Props &p) {
    auto dragDown = s.boolean("drag_down", false);
    p["drag"] = Bool(dragDown);
    return bName;
  }

  static String Button(String const &bName, CompoundTag const &s, Props &p) {
    auto buttonPressedBit = s.boolean("button_pressed_bit", false);
    auto facingDirection = s.int32("facing_direction", 0);
    std::string facing = "south";
    std::string face = "ceiling";
    switch (facingDirection) {
    case 0:
      face = "ceiling";
      facing = "south";
      break;
    case 1:
      face = "floor";
      facing = "south";
      break;
    case 2:
      face = "wall";
      facing = "north";
      break;
    case 3:
      face = "wall";
      facing = "south";
      break;
    case 4:
      face = "wall";
      facing = "west";
      break;
    case 5:
      face = "wall";
      facing = "east";
      break;
    default:
      break;
    }
    p["face"] = face;
    p["facing"] = facing;
    p["powered"] = Bool(buttonPressedBit);
    if (bName == "minecraft:wooden_button") {
      return Ns() + "oak_button";
    } else {
      return bName;
    }
  }
#pragma endregion

#pragma region Converters : C
  static String Cake(String const &bName, CompoundTag const &s, Props &p) {
    auto biteCounter = s.int32("bite_counter", 0);
    p["bites"] = Int(biteCounter);
    return bName;
  }

  static String Campfire(String const &bName, CompoundTag const &s, Props &p) {
    Facing4FromDirectionA(s, p);
    Submergible(s, p);
    auto extinguished = s.boolean("extinguished", false);
    p["lit"] = Bool(!extinguished);
    return bName;
  }

  static String Candle(String const &bName, CompoundTag const &s, Props &p) {
    auto candles = s.int32("candles", 0);
    p["candles"] = Int(candles + 1);
    Lit(s, p);
    Submergible(s, p);
    return bName;
  }

  static String CandleCake(String const &bName, CompoundTag const &s, Props &p) {
    Lit(s, p);
    return bName;
  }

  static String Carpet(String const &bName, CompoundTag const &s, Props &p) {
    auto colorB = s.string("color", "white");
    auto colorJ = JavaNameFromColorCodeJava(ColorCodeJavaFromBedrockName(colorB));
    return Ns() + colorJ + "_carpet";
  }

  static String Cauldron(String const &bName, CompoundTag const &s, Props &p) {
    auto liquid = s.string("cauldron_liquid", "water");
    std::string name = "cauldron";
    auto fillLevel = s.int32("fill_level", 0);
    if (fillLevel < 1) {
      name = "cauldron";
    } else if (liquid == "water") {
      name = "water_cauldron";
      p["level"] = Int((std::min)((std::max)((int)ceil(fillLevel * 0.5), 1), 3));
    } else if (liquid == "lava") {
      name = "lava_cauldron";
    } else if (liquid == "powder_snow") {
      name = "powder_snow_cauldron";
      p["level"] = Int((std::min)((std::max)((int)ceil(fillLevel * 0.5), 1), 3));
    }
    return Ns() + name;
  }

  static String CaveVines(String const &bName, CompoundTag const &s, Props &p) {
    auto berries = bName.ends_with("_with_berries");
    auto growingPlantAge = s.int32("growing_plant_age", 1);
    p["age"] = Int(growingPlantAge);
    p["berries"] = Bool(berries);
    return Ns() + "cave_vines";
  }

  static String CaveVinesBody(String const &bName, CompoundTag const &s, Props &p) {
    auto berries = bName.ends_with("_with_berries");
    p["berries"] = Bool(berries);
    return Ns() + "cave_vines_plant";
  }

  static String Chain(String const &bName, CompoundTag const &s, Props &p) {
    AxisFromPillarAxis(s, p);
    Submergible(s, p);
    return bName;
  }

  static String Cocoa(String const &bName, CompoundTag const &s, Props &p) {
    Facing4FromDirectionA(s, p);
    Age(s, p);
    return bName;
  }

  static String CommandBlock(String const &bName, CompoundTag const &s, Props &p) {
    auto conditional = s.boolean("conditional_bit", false);
    Facing6FromFacingDirectionA(s, p);
    p["conditional"] = Bool(conditional);
    return bName;
  }

  static String Comparator(String const &bName, CompoundTag const &s, Props &p) {
    auto powered = bName.starts_with(Ns() + "powered_");
    auto lit = s.boolean("output_lit_bit", false);
    auto subtract = s.boolean("output_subtract_bit", false);
    Facing4FromDirectionA(s, p);
    p["powered"] = Bool(lit);
    p["mode"] = subtract ? "subtract" : "compare";
    return Ns() + "comparator";
  }

  static String Composter(String const &bName, CompoundTag const &s, Props &p) {
    auto level = s.int32("composter_fill_level", 0);
    p["level"] = Int(level);
    return bName;
  }

  static String Concrete(String const &bName, CompoundTag const &s, Props &p) {
    auto colorB = s.string("color", "white");
    auto colorJ = JavaNameFromColorCodeJava(ColorCodeJavaFromBedrockName(colorB));
    return Ns() + colorJ + "_concrete";
  }

  static String ConcretePowder(String const &bName, CompoundTag const &s, Props &p) {
    auto colorB = s.string("color", "white");
    auto colorJ = JavaNameFromColorCodeJava(ColorCodeJavaFromBedrockName(colorB));
    return Ns() + colorJ + "_concrete_powder";
  }

  static String Coral(String const &bName, CompoundTag const &s, Props &p) {
    auto color = s.string("coral_color", "pink");
    auto dead = s.boolean("dead_bit", false);
    auto type = CoralTypeFromCoralColor(color);
    std::string name;
    if (dead) {
      name = "dead_";
    }
    name += type;
    Submergible(s, p);
    return Ns() + name + "_coral";
  }

  static String CoralBlock(String const &bName, CompoundTag const &s, Props &p) {
    auto color = s.string("coral_color", "pink");
    auto dead = s.boolean("dead_bit", false);
    auto type = CoralTypeFromCoralColor(color);
    std::string name;
    if (dead) {
      name = "dead_";
    }
    name += type + "_coral_block";
    return Ns() + name;
  }

  static String CoralFan(String const &bName, CompoundTag const &s, Props &p) {
    auto color = s.string("coral_color", "pink");
    auto dead = bName.ends_with("_dead");
    auto type = CoralTypeFromCoralColor(color);
    std::string name;
    if (dead) {
      name = "dead_";
    }
    name += type + "_coral_fan";
    Submergible(s, p);
    return Ns() + name;
  }

  static String CoralFanHang(String const &bName, CompoundTag const &s, Props &p) {
    auto hangType = s.boolean("coral_hang_type_bit", false);
    auto dead = s.boolean("dead_bit", false);
    std::string name;
    if (dead) {
      name = "dead_";
    }
    if (bName.ends_with("2")) {
      if (hangType) {
        name += "fire";
      } else {
        name += "bubble";
      }
    } else if (bName.ends_with("3")) {
      name += "horn";
    } else {
      if (hangType) {
        name += "brain";
      } else {
        name += "tube";
      }
    }
    CoralDirection(s, p);
    Submergible(s, p);
    return Ns() + name + "_coral_wall_fan";
  }
#pragma endregion

#pragma region Converters : D
  static String DarkoakStandingSign(String const &bName, CompoundTag const &s, Props &p) {
    RotationFromGroundSignDirection(s, p);
    Submergible(s, p);
    return Ns() + "dark_oak_sign";
  }

  static String DarkoakWallSign(String const &bName, CompoundTag const &s, Props &p) {
    Facing4FromFacingDirectionA(s, p);
    Submergible(s, p);
    return Ns() + "dark_oak_wall_sign";
  }

  static String DaylightDetector(String const &bName, CompoundTag const &s, Props &p) {
    auto inverted = bName.ends_with("_inverted");
    p["inverted"] = Bool(inverted);
    PowerFromRedstoneSignal(s, p);
    return Ns() + "daylight_detector";
  }

  static String Dirt(String const &bName, CompoundTag const &s, Props &p) {
    auto type = s.string("dirt_type", "normal");
    std::string prefix;
    if (type != "normal") {
      prefix = type + "_";
    }
    return Ns() + prefix + "dirt";
  }

  static String Door(String const &bName, CompoundTag const &s, Props &p) {
    auto doorHingeBit = s.boolean("door_hinge_bit", false);
    OpenFromOpenBit(s, p);
    FacingCFromDirection(s, p);
    HalfFromUpperBlockBit(s, p);
    p["hinge"] = doorHingeBit ? "right" : "left";
    p["powered"] = "false";
    if (bName == "minecraft:wooden_door") {
      return Ns() + "oak_door";
    } else {
      return bName;
    }
  }

  static String DoublePlant(String const &bName, CompoundTag const &s, Props &p) {
    auto type = s.string("double_plant_type", "rose");
    std::string name;
    if (type == "fern") {
      name = "large_fern";
    } else if (type == "syringa") {
      name = "lilac";
    } else if (type == "paeonia") {
      name = "peony";
    } else if (type == "sunflower") {
      name = "sunflower";
    } else if (type == "rose") {
      name = "rose_bush";
    } else { // "grass"
      name = "tall_grass";
    }
    HalfFromUpperBlockBit(s, p);
    return Ns() + name;
  }

  static Converter DoubleSlab(std::string name) {
    return [name](String const &bName, CompoundTag const &s, Props &p) {
      p["type"] = "double";
      Submergible(s, p);
      return Ns() + name;
    };
  }

  static String DoubleStoneSlab(String const &bName, CompoundTag const &s, Props &p) {
    auto stoneSlabType = s.string("stone_slab_type", "stone");
    p["type"] = "double";
    Submergible(s, p);
    return Ns() + stoneSlabType + "_slab";
  }

  static String DoubleStoneSlab2(String const &bName, CompoundTag const &s, Props &p) {
    auto stoneSlabType = s.string("stone_slab_type_2", "prismarine_dark");
    auto name = StoneTypeFromStone2(stoneSlabType);
    p["type"] = "double";
    Submergible(s, p);
    return Ns() + name + "_slab";
  }

  static String DoubleStoneSlab3(String const &bName, CompoundTag const &s, Props &p) {
    auto stoneSlabType = s.string("stone_slab_type_3", "andesite");
    p["type"] = "double";
    Submergible(s, p);
    return Ns() + stoneSlabType + "_slab";
  }

  static String DoubleStoneSlab4(String const &bName, CompoundTag const &s, Props &p) {
    auto stoneSlabType = s.string("stone_slab_type_4", "stone");
    p["type"] = "double";
    Submergible(s, p);
    return Ns() + stoneSlabType + "_slab";
  }

  static String DoubleWoodenSlab(String const &bName, CompoundTag const &s, Props &p) {
    auto woodType = s.string("wood_type", "oak");
    p["type"] = "double";
    Submergible(s, p);
    return Ns() + woodType + "_slab";
  }

  static String Dispenser(String const &bName, CompoundTag const &s, Props &p) {
    Facing6FromFacingDirectionA(s, p);
    auto triggered = s.boolean("triggered_bit", false);
    p["triggered"] = Bool(triggered);
    return bName;
  }

  static String Dropper(String const &bName, CompoundTag const &s, Props &p) {
    Facing6FromFacingDirectionA(s, p);
    auto triggered = s.boolean("triggered_bit", false);
    p["triggered"] = Bool(triggered);
    return bName;
  }
#pragma endregion

#pragma region Converters : E
  static String EndPortalFrame(String const &bName, CompoundTag const &s, Props &p) {
    Facing4FromDirectionA(s, p);
    auto eye = s.boolean("end_portal_eye_bit", false);
    p["eye"] = Bool(eye);
    return bName;
  }

  static String EndRod(String const &bName, CompoundTag const &s, Props &p) {
    std::string facing;
    switch (s.int32("facing_direction", 0)) {
    case 1:
      facing = "up";
      break;
    case 2:
      facing = "south";
      break;
    case 3:
      facing = "north";
      break;
    case 4:
      facing = "east";
      break;
    case 5:
      facing = "west";
      break;
    case 0:
    default:
      facing = "down";
      break;
    }
    p["facing"] = facing;
    return bName;
  }
#pragma endregion

#pragma region Converter : F
  static String Farmland(String const &bName, CompoundTag const &s, Props &p) {
    auto moisture = s.int32("moisturized_amount", 0);
    p["moisture"] = Int(moisture);
    return bName;
  }

  static String Fence(String const &bName, CompoundTag const &s, Props &p) {
    auto woodType = s.string("wood_type", "oak");
    Submergible(s, p);
    return Ns() + woodType + "_fence";
  }

  static String FenceGate(String const &bName, CompoundTag const &s, Props &p) {
    auto inWall = s.boolean("in_wall_bit", false);
    p["in_wall"] = Bool(inWall);
    p["powered"] = Bool(false);
    Facing4FromDirectionA(s, p);
    OpenFromOpenBit(s, p);
    if (bName.ends_with(":fence_gate")) {
      return Ns() + "oak_fence_gate";
    } else {
      return bName;
    }
  }

  static String FurnaceAndSimilar(String const &bName, CompoundTag const &s, Props &p) {
    Facing4FromFacingDirectionA(s, p);
    auto lit = bName.starts_with("minecraft:lit_");
    std::string name;
    if (lit) {
      name = bName.substr(14);
    } else {
      name = bName.substr(10);
    }
    p["lit"] = Bool(lit);
    return Ns() + name;
  }
#pragma endregion

#pragma region Converters : G
  static String GoldenRail(String const &bName, CompoundTag const &s, Props &p) {
    auto railData = s.boolean("rail_data_bit", false);
    auto railDirection = s.int32("rail_direction", 0);
    p["powered"] = Bool(railData);
    p["shape"] = ShapeFromRailDirection(railDirection);
    Submergible(s, p);
    return Ns() + "powered_rail";
  }

  static String Grass(String const &bName, CompoundTag const &s, Props &p) {
    p["snowy"] = "false";
    return Ns() + "grass_block";
  }

  static String Grindstone(String const &bName, CompoundTag const &s, Props &p) {
    auto attachment = s.string("attachment", "floor");
    std::string face;
    if (attachment == "side") {
      face = "wall";
    } else if (attachment == "hanging") {
      face = "ceiling";
    } else { // standing
      face = "floor";
    }
    p["face"] = face;
    Facing4FromDirectionA(s, p);
    return bName;
  }
#pragma endregion

#pragma region Converters : H
  static String Hopper(String const &bName, CompoundTag const &s, Props &p) {
    Facing6FromFacingDirectionA(s, p);
    auto toggle = s.boolean("toggle_bit", false);
    p["enabled"] = Bool(!toggle);
    return bName;
  }
#pragma endregion

#pragma region Converters : K
  static String Kelp(String const &bName, CompoundTag const &s, Props &p) {
    auto age = s.int32("kelp_age", 0);
    p["age"] = Int(age);
    return bName;
  }
#pragma endregion

#pragma region Converters : L
  static String Lantern(String const &bName, CompoundTag const &s, Props &p) {
    auto hanging = s.boolean("hanging", false);
    p["hanging"] = Bool(hanging);
    Submergible(s, p);
    return bName;
  }

  static String Leaves(String const &bName, CompoundTag const &s, Props &p) {
    auto leafType = s.string("old_leaf_type", "oak");
    PersistentFromPersistentBit(s, p);
    Submergible(s, p);
    return Ns() + leafType + "_leaves";
  }

  static String Leaves2(String const &bName, CompoundTag const &s, Props &p) {
    auto newLeafType = s.string("new_leaf_type", "acacia"); //TODO: acacia?
    PersistentFromPersistentBit(s, p);
    Submergible(s, p);
    return Ns() + newLeafType + "_leaves";
  }

  static String Lectern(String const &bName, CompoundTag const &s, Props &p) {
    auto powered = s.boolean("powered_bit", false);
    p["powered"] = Bool(powered);
    Facing4FromDirectionA(s, p);
    return bName;
  }

  static String Lever(String const &bName, CompoundTag const &s, Props &p) {
    auto direction = s.string("lever_direction", "up_north_south");
    std::string face;
    std::string facing;
    if (direction == "up_east_west") {
      face = "floor";
      facing = "west";
    } else if (direction == "up_north_south") {
      face = "floor";
      facing = "north";
    } else if (direction == "down_east_west") {
      face = "ceiling";
      facing = "west";
    } else if (direction == "down_north_south") {
      face = "ceiling";
      facing = "north";
    } else {
      face = "wall";
      facing = direction;
    }
    p["face"] = face;
    p["facing"] = facing;
    auto open = s.boolean("open_bit", false);
    p["powered"] = Bool(open);
    return bName;
  }

  static String LightBlock(String const &bName, CompoundTag const &s, Props &p) {
    auto level = s.int32("block_light_level", 0);
    p["level"] = Int(level);
    Submergible(s, p);
    return Ns() + "light";
  }

  static String Liquid(String const &bName, CompoundTag const &s, Props &p) {
    auto depth = s.int32("liquid_depth", 0);
    p["level"] = Int(depth);
    return bName;
  }

  static String LiquidFlowing(String const &bName, CompoundTag const &s, Props &p) {
    auto depth = s.int32("liquid_depth", 0);
    p["level"] = Int(depth);
    if (bName.ends_with("lava")) {
      return Ns() + "lava";
    } else {
      return Ns() + "water";
    }
  }

  static String LitPumpkin(String const &bName, CompoundTag const &s, Props &p) {
    Facing4FromDirectionA(s, p);
    return Ns() + "jack_o_lantern";
  }

  static String Log(String const &bName, CompoundTag const &s, Props &p) {
    auto logType = s.string("old_log_type", "oak");
    AxisFromPillarAxis(s, p);
    return Ns() + logType + "_log";
  }

  static String Log2(String const &bName, CompoundTag const &s, Props &p) {
    auto logType = s.string("new_log_type", "acacia"); //TODO: acacia?
    AxisFromPillarAxis(s, p);
    return Ns() + logType + "_log";
  }
#pragma endregion

#pragma region Converters : M
  static String MangrovePropagule(String const &bName, CompoundTag const &s, Props &p) {
    bool hanging = s.boolean("hanging", false);
    int stage = s.int32("propagule_stage", 0);
    p["hanging"] = Bool(hanging);
    p["stage"] = Int(1);
    if (hanging) {
      p["age"] = Int(stage);
    } else {
      p["age"] = Int(4);
    }
    Submergible(s, p);
    return bName;
  }

  static String MelonStem(String const &bName, CompoundTag const &s, Props &p) {
    auto growth = s.int32("growth", 0);
    p["age"] = Int(growth);
    Facing4FromFacingDirectionA(s, p);
    return bName;
  }

  static String MonsterEgg(String const &bName, CompoundTag const &s, Props &p) {
    auto type = s.string("monster_egg_stone_type", "stone");
    std::string name;
    if (type == "cobblestone") {
      name = type;
    } else if (type == "stone_brick") {
      name = "stone_bricks";
    } else if (type == "mossy_stone_brick") {
      name = "mossy_stone_bricks";
    } else if (type == "cracked_stone_brick") {
      name = "cracked_stone_bricks";
    } else if (type == "chiseled_stone_brick") {
      name = "chiseled_stone_bricks";
    } else { // stone
      name = "stone";
    }
    return Ns() + "infested_" + name;
  }
#pragma endregion

#pragma region Converters : N
  static Converter NetherVines(std::string type) {
    return [type](String const &bName, CompoundTag const &s, Props &p) {
      auto age = s.int32(type + "_vines_age", 0);
      p["age"] = Int(age);
      return bName;
    };
  }
#pragma endregion

#pragma region Converters : O
  static String OakWallSign(String const &bName, CompoundTag const &s, Props &p) {
    Facing4FromFacingDirectionA(s, p);
    Submergible(s, p);
    return Ns() + "oak_wall_sign";
  }

  static String Observer(String const &bName, CompoundTag const &s, Props &p) {
    Facing6FromFacingDirectionA(s, p);
    auto powered = s.boolean("powered", false);
    p["powered"] = Bool(powered);
    return bName;
  }
#pragma endregion

#pragma region Converters : P
  static String Planks(String const &bName, CompoundTag const &s, Props &p) {
    auto woodType = s.string("wood_type", "acacia"); //TODO: acacia?
    return Ns() + woodType + "_planks";
  }

  static String PointedDripstone(String const &bName, CompoundTag const &s, Props &p) {
    auto thickness = s.string("dripstone_thickness", "base");
    std::string t = thickness;
    // base, middle, frustum, tip , tip_merge
    if (thickness == "merge") {
      t = "tip_merge";
    }
    p["thickness"] = t;
    auto hanging = s.boolean("hanging", false);
    p["vertical_direction"] = hanging ? "down" : "up";
    Submergible(s, p);
    return bName;
  }

  static String Portal(String const &bName, CompoundTag const &s, Props &p) {
    auto axis = s.string("portal_axis", "y");
    p["axis"] = axis;
    return Ns() + "nether_portal";
  }

  static String PressurePlate(String const &bName, CompoundTag const &s, Props &p) {
    auto redstoneSignal = s.int32("redstone_signal", 0);
    p["powered"] = Bool(redstoneSignal > 0);
    return bName;
  }

  static String Prismarine(String const &bName, CompoundTag const &s, Props &p) {
    auto type = s.string("prismarine_block_type", "default");
    std::string name;
    if (type == "bricks") {
      name = "prismarine_bricks";
    } else if (type == "dark") {
      name = "dark_prismarine";
    } else { // default
      name = "prismarine";
    }
    return Ns() + name;
  }

  static String PumpkinStem(String const &bName, CompoundTag const &s, Props &p) {
    auto growth = s.int32("growth", 0);
    p["age"] = Int(growth);
    Facing4FromFacingDirectionA(s, p);
    return bName;
  }

  static String PurpurBlock(String const &bName, CompoundTag const &s, Props &p) {
    auto chisel = s.string("chisel_type", "default");
    std::string name;
    if (chisel == "lines") {
      name = "purpur_pillar";
      AxisFromPillarAxis(s, p);
    } else { // "default"
      name = "purpur_block";
    }
    return Ns() + name;
  }
#pragma endregion

#pragma region Converters : Q
  static String QuartzBlock(String const &bName, CompoundTag const &s, Props &p) {
    auto type = s.string("chisel_type", "chiseled");
    std::string prefix;
    if (type == "lines") {
      AxisFromPillarAxis(s, p);
      return Ns() + "quartz_pillar";
    } else if (type == "smooth") {
      return Ns() + "smooth_quartz";
    } else if (type == "chiseled") {
      return Ns() + "chiseled_quartz_block";
    } else { // "default";
      return Ns() + "quartz_block";
    }
  }
#pragma endregion

#pragma region Converters : R
  static String Rail(String const &bName, CompoundTag const &s, Props &p) {
    auto railDirection = s.int32("rail_direction", 0);
    p["shape"] = ShapeFromRailDirection(railDirection);
    Submergible(s, p);
    return bName;
  }

  static String RailCanBePowered(String const &bName, CompoundTag const &s, Props &p) {
    auto railData = s.boolean("rail_data_bit", false);
    auto railDirection = s.int32("rail_direction", 0);
    p["powered"] = Bool(railData);
    p["shape"] = ShapeFromRailDirection(railDirection);
    Submergible(s, p);
    return bName;
  }

  static String RedFlower(String const &bName, CompoundTag const &s, Props &p) {
    auto flowerType = s.string("flower_type", "poppy");
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
      return Ns() + "mushroom_stem";
    } else {
      return bName;
    }
  }

  static String RedSandstone(String const &bName, CompoundTag const &s, Props &p) {
    auto type = s.string("sand_stone_type", "default");
    std::string name = "red_sandstone";
    if (type == "heiroglyphs") {
      name = "chiseled_red_sandstone";
    } else if (type == "cut") {
      name = "cut_red_sandstone";
    } else if (type == "smooth") {
      name = "smooth_red_sandstone";
    } else { // "default"
      name = "red_sandstone";
    }
    return Ns() + name;
  }

  static String RedstoneLamp(String const &bName, CompoundTag const &s, Props &p) {
    auto lit = bName == "minecraft:lit_redstone_lamp";
    p["lit"] = Bool(lit);
    return Ns() + "redstone_lamp";
  }

  static String RedstoneOre(String const &bName, CompoundTag const &s, Props &p) {
    auto lit = bName.starts_with("minecraft:lit_");
    std::string name;
    if (lit) {
      name = bName.substr(14);
    } else {
      name = bName.substr(10);
    }
    p["lit"] = Bool(lit);
    return Ns() + name;
  }

  static String Reeds(String const &bName, CompoundTag const &s, Props &p) {
    Age(s, p);
    return Ns() + "sugar_cane";
  }

  static Converter Rename(std::string name) {
    return [name](String const &bName, CompoundTag const &s, Props &p) {
      return Ns() + name;
    };
  }

  static Converter RenameStairs(std::string name) {
    return [name](String const &bName, CompoundTag const &s, Props &p) {
      FacingFromWeirdoDirection(s, p);
      HalfFromUpsideDownBit(s, p);
      Submergible(s, p);
      return Ns() + name;
    };
  }

  static String Repeater(String const &bName, CompoundTag const &s, Props &p) {
    auto powered = bName == "minecraft:powered_repeater";
    p["powered"] = Bool(powered);
    Facing4FromDirectionA(s, p);
    auto delay = s.int32("repeater_delay", 0);
    p["delay"] = Int(delay + 1 + 0);
    return Ns() + "repeater";
  }

  static String RespawnAnchor(String const &bName, CompoundTag const &s, Props &p) {
    auto charges = s.int32("respawn_anchor_charge", 0);
    p["charges"] = Int(charges);
    return bName;
  }
#pragma endregion

#pragma region Converters : S
  static String Same(String const &bName, CompoundTag const &s, Props &p) {
    return bName;
  }

  static String Sand(String const &bName, CompoundTag const &s, Props &p) {
    auto type = s.string("sand_type", "normal");
    if (type == "red") {
      return Ns() + "red_sand";
    } else { // "normal"
      return bName;
    }
  }

  static String Sandstone(String const &bName, CompoundTag const &s, Props &p) {
    auto type = s.string("sand_stone_type", "default");
    std::string name = "sandstone";
    if (type == "heiroglyphs") {
      name = "chiseled_sandstone";
    } else if (type == "cut") {
      name = "cut_sandstone";
    } else if (type == "smooth") {
      name = "smooth_sandstone";
    } else { // "default"
      name = "sandstone";
    }
    return Ns() + name;
  }

  static String Sapling(String const &bName, CompoundTag const &s, Props &p) {
    auto age = s.byte("age_bit", 0);
    auto saplingType = s.string("sapling_type", "acacia");
    p["stage"] = Int(age);
    return Ns() + saplingType + "_sapling";
  }

  static String SculkCatalyst(String const &bName, CompoundTag const &s, Props &p) {
    auto bloom = s.boolean("bloom", false);
    p["bloom"] = Bool(bloom);
    return bName;
  }

  static String SculkShrieker(String const &bName, CompoundTag const &s, Props &p) {
    auto canSummon = s.boolean("can_summon", false);
    auto active = s.boolean("active", false);
    p["can_summon"] = Bool(canSummon);
    p["shrieking"] = Bool(active);
    Submergible(s, p);
    return bName;
  }

  static String Seagrass(String const &bName, CompoundTag const &s, Props &p) {
    auto type = s.string("sea_grass_type", "default");
    std::string name;
    if (type == "double_bot") {
      name = "tall_seagrass";
      p["half"] = "lower";
    } else if (type == "double_top") {
      name = "tall_seagrass";
      p["half"] = "upper";
    } else { // "default"
      name = "seagrass";
    }
    return Ns() + name;
  }

  static String SeaPickle(String const &bName, CompoundTag const &s, Props &p) {
    auto count = s.int32("cluster_count", 0);
    p["pickles"] = Int(count + 1);
    auto dead = s.boolean("dead_bit", false);
    p["waterlogged"] = Bool(!dead);
    return bName;
  }

  static String Scaffolding(String const &bName, CompoundTag const &s, Props &p) {
    auto stability = s.int32("stability", 0);
    p["bottom"] = Bool(stability > 0);
    Submergible(s, p);
    return bName;
  }

  static String ShulkerBox(String const &bName, CompoundTag const &s, Props &p) {
    auto colorB = s.string("color", "white");
    auto colorJ = JavaNameFromColorCodeJava(ColorCodeJavaFromBedrockName(colorB));
    return Ns() + colorJ + "_shulker_box";
  }

  static String SilverGlazedTerracotta(String const &bName, CompoundTag const &s, Props &p) {
    Facing4FromFacingDirectionA(s, p);
    return Ns() + "light_gray_glazed_terracotta";
  }

  static String Slab(String const &bName, CompoundTag const &s, Props &p) {
    Submergible(s, p);
    TypeFromTopSlotBit(s, p);
    return bName;
  }

  static String SmallDripleafBlock(String const &bName, CompoundTag const &s, Props &p) {
    Facing4FromDirectionA(s, p);
    HalfFromUpperBlockBit(s, p);
    Submergible(s, p);
    return Ns() + "small_dripleaf";
  }

  static String SnowLayer(String const &bName, CompoundTag const &s, Props &p) {
    auto height = s.int32("height", 0);
    p["layers"] = Int(height + 1);
    return Ns() + "snow";
  }

  static String Sponge(String const &bName, CompoundTag const &s, Props &p) {
    auto type = s.string("sponge_type", "dry");
    if (type == "wet") {
      return Ns() + "wet_sponge";
    } else { // "dry"
      return Ns() + "sponge";
    }
  }

  static String Stairs(String const &bName, CompoundTag const &s, Props &p) {
    FacingFromWeirdoDirection(s, p);
    HalfFromUpsideDownBit(s, p);
    Submergible(s, p);
    return bName;
  }

  static String StainedGlass(String const &bName, CompoundTag const &s, Props &p) {
    auto colorB = s.string("color", "white");
    auto colorJ = JavaNameFromColorCodeJava(ColorCodeJavaFromBedrockName(colorB));
    return Ns() + colorJ + "_stained_glass";
  }

  static String StainedGlassPane(String const &bName, CompoundTag const &s, Props &p) {
    auto colorB = s.string("color", "white");
    auto colorJ = JavaNameFromColorCodeJava(ColorCodeJavaFromBedrockName(colorB));
    Submergible(s, p);
    return Ns() + colorJ + "_stained_glass_pane";
  }

  static String StainedHardenedClay(String const &bName, CompoundTag const &s, Props &p) {
    auto colorB = s.string("color", "white");
    auto colorJ = JavaNameFromColorCodeJava(ColorCodeJavaFromBedrockName(colorB));
    return Ns() + colorJ + "_terracotta";
  }

  static String StandingSign(String const &bName, CompoundTag const &s, Props &p) {
    std::string name;
    if (bName.ends_with(":standing_sign")) {
      name = "oak_sign";
    } else {
      auto type = VariantFromName(bName, "_standing_sign");
      name = type + "_sign";
    }
    RotationFromGroundSignDirection(s, p);
    Submergible(s, p);
    return Ns() + name;
  }

  static String PistonArmCollision(String const &bName, CompoundTag const &s, Props &p) {
    bool sticky = (bName == "minecraft:stickyPistonArmCollision") || (bName == "minecraft:sticky_piston_arm_collision");
    std::string type = sticky ? "sticky" : "normal";
    auto f6 = Facing6FromBedrockFacingDirectionB(s.int32("facing_direction", 0));
    auto facing = JavaNameFromFacing6(f6);
    p["facing"] = facing;
    p["type"] = type;
    p["short"] = "false"; //?
    return Ns() + "piston_head";
  }

  static String Stone(String const &bName, CompoundTag const &s, Props &p) {
    auto type = s.string("stone_type", "stone");
    std::string name;
    if (type == "diorite_smooth") {
      name = "polished_diorite";
    } else if (type == "andesite_smooth") {
      name = "polished_andesite";
    } else if (type == "granite_smooth") {
      name = "polished_granite";
    } else {
      name = type;
    }
    return Ns() + name;
  }

  static String Stonebrick(String const &bName, CompoundTag const &s, Props &p) {
    auto type = s.string("stone_brick_type", "default");
    std::string prefix;
    if (type != "default") {
      prefix = type + "_";
    }
    return Ns() + prefix + "stone_bricks";
  }

  static String StonecutterBlock(String const &bName, CompoundTag const &s, Props &p) {
    Facing4FromFacingDirectionA(s, p);
    return Ns() + "stonecutter";
  }

  static String StoneSlab(String const &bName, CompoundTag const &s, Props &p) {
    auto stoneSlabType = s.string("stone_slab_type", "stone");
    TypeFromTopSlotBit(s, p);
    Submergible(s, p);
    return Ns() + stoneSlabType + "_slab";
  }

  static String StoneSlab2(String const &bName, CompoundTag const &s, Props &p) {
    auto type = s.string("stone_slab_type_2", "prismarine_dark");
    TypeFromTopSlotBit(s, p);
    Submergible(s, p);
    auto name = StoneTypeFromStone2(type);
    return Ns() + name + "_slab";
  }

  static String StoneSlab3(String const &bName, CompoundTag const &s, Props &p) {
    auto stoneSlabType = s.string("stone_slab_type_3", "andesite");
    TypeFromTopSlotBit(s, p);
    Submergible(s, p);
    return Ns() + stoneSlabType + "_slab";
  }

  static String StoneSlab4(String const &bName, CompoundTag const &s, Props &p) {
    auto type = s.string("stone_slab_type_4", "stone");
    TypeFromTopSlotBit(s, p);
    Submergible(s, p);
    return Ns() + type + "_slab";
  }

  static String StructureBlock(String const &bName, CompoundTag const &s, Props &p) {
    auto type = s.string("structure_block_type", "save");
    p["mode"] = type;
    return bName;
  }

  static String SweetBerryBush(String const &bName, CompoundTag const &s, Props &p) {
    AgeFromGrowthNonLinear(s, p);
    return bName;
  }
#pragma endregion

#pragma region Converters : T
  static String Tallgrass(String const &bName, CompoundTag const &s, Props &p) {
    auto type = s.string("tall_grass_type", "tall");
    if (type == "fern") {
      return Ns() + "fern";
    } else { // "tall"
      return Ns() + "grass";
    }
  }

  static String Target(String const &bName, CompoundTag const &s, Props &p) {
    p["power"] = "0";
    return bName;
  }

  static String Tnt(String const &bName, CompoundTag const &s, Props &p) {
    auto explode = s.boolean("explode_bit", false);
    p["unstable"] = Bool(explode);
    return bName;
  }

  static String Trapdoor(String const &bName, CompoundTag const &s, Props &p) {
    FacingBFromDirection(s, p);
    OpenFromOpenBit(s, p);
    HalfFromUpsideDownBit(s, p);
    Submergible(s, p);
    p["powered"] = "false";
    if (bName.ends_with(":trapdoor")) {
      return Ns() + "oak_trapdoor";
    } else {
      return bName;
    }
  }

  static String Tripwire(String const &bName, CompoundTag const &s, Props &p) {
    auto attached = s.boolean("attached_bit", false);
    auto disarmed = s.boolean("disarmed_bit", false);
    auto powered = s.boolean("powered_bit", false);
    p["attached"] = Bool(attached);
    p["disarmed"] = Bool(disarmed);
    p["powered"] = Bool(powered);
    return Ns() + "tripwire";
  }

  static String TripwireHook(String const &bName, CompoundTag const &s, Props &p) {
    auto attached = s.boolean("attached_bit", false);
    auto powered = s.boolean("powered_bit", false);
    Facing4FromDirectionA(s, p);
    p["attached"] = Bool(attached);
    p["powered"] = Bool(powered);
    return bName;
  }

  static String TurtleEgg(String const &bName, CompoundTag const &s, Props &p) {
    auto eggs = s.string("turtle_egg_count", "one_egg");
    if (eggs == "two_egg") {
      p["eggs"] = "2";
    } else if (eggs == "three_egg") {
      p["eggs"] = "3";
    } else if (eggs == "four_egg") {
      p["eggs"] = "4";
    } else { // "one_egg"
      p["eggs"] = "1";
    }
    auto cracked = s.string("cracked_state", "no_cracks");
    if (cracked == "cracked") {
      p["hatch"] = "1";
    } else if (cracked == "max_cracked") {
      p["hatch"] = "2";
    } else { // "no_cracks"
      p["hatch"] = "0";
    }
    return bName;
  }
#pragma endregion

#pragma region Converters : W
  static String WallWithBlockType(String const &bName, CompoundTag const &s, Props &p) {
    auto type = s.string("wall_block_type", "andesite");
    std::string name = type;
    if (type == "end_brick") {
      name = "end_stone_brick";
    }
    WallProperties(s, p);
    Submergible(s, p);
    return Ns() + name + "_wall";
  }

  static String Wood(String const &bName, CompoundTag const &s, Props &p) {
    auto stripped = s.boolean("stripped_bit", false);
    auto woodType = s.string("wood_type", "oak");
    auto name = stripped ? "stripped_" + woodType + "_wood" : woodType + "_wood";
    AxisFromPillarAxis(s, p);
    return Ns() + name;
  }

  static String WoodenPressurePlate(String const &bName, CompoundTag const &s, Props &p) {
    auto signal = s.int32("restone_signal", 0);
    p["powered"] = Bool(signal > 0);
    return Ns() + "oak_pressure_plate";
  }

  static String WoodenSlab(String const &bName, CompoundTag const &s, Props &p) {
    auto woodType = s.string("wood_type", "acacia");
    TypeFromTopSlotBit(s, p);
    Submergible(s, p);
    return Ns() + woodType + "_slab";
  }

  static String Wool(String const &bName, CompoundTag const &s, Props &p) {
    auto colorB = s.string("color", "white");
    auto colorJ = JavaNameFromColorCodeJava(ColorCodeJavaFromBedrockName(colorB));
    return Ns() + colorJ + "_wool";
  }
#pragma endregion

#pragma region Converters : V
  static String Vine(String const &bName, CompoundTag const &s, Props &p) {
    auto d = s.int32("vine_direction_bits", 0);
    bool east = (d & 0x8) == 0x8;
    bool north = (d & 0x4) == 0x4;
    bool west = (d & 0x2) == 0x2;
    bool south = (d & 0x1) == 0x1;
    p["east"] = Bool(east);
    p["north"] = Bool(north);
    p["west"] = Bool(west);
    p["south"] = Bool(south);
    p["up"] = "false";
    return bName;
  }
#pragma endregion

#pragma region Properties
  static void Age(CompoundTag const &s, Props &p) {
    auto age = s.int32("age", 0);
    p["age"] = Int(age);
  }

  static void AgeFromGrowth(CompoundTag const &s, Props &p) {
    auto age = s.int32("growth", 0);
    p["age"] = Int(age);
  }

  static void AgeFromGrowthNonLinear(CompoundTag const &s, Props &p) {
    auto growth = s.int32("growth", 0);
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
    p["age"] = Int(age);
  }

  static void AxisFromPillarAxis(CompoundTag const &s, Props &props) {
    auto pillarAxis = s.string("pillar_axis", "y");
    props["axis"] = pillarAxis;
  }

  static std::string CoralTypeFromCoralColor(std::string const color) {
    if (color == "blue") {
      return "tube";
    } else if (color == "pink") {
      return "brain";
    } else if (color == "purple") {
      return "bubble";
    } else if (color == "red") {
      return "fire";
    } else if (color == "yellow") {
      return "horn";
    }
    return "brain";
  }

  static void Facing4FromDirectionA(CompoundTag const &s, Props &props) {
    auto direction = s.int32("direction", 0);
    Facing4 f4 = Facing4FromBedrockDirection(direction);
    props["facing"] = JavaNameFromFacing4(f4);
  }

  static void Facing6FromFacingDirectionA(CompoundTag const &s, Props &props) {
    auto facingDirection = s.int32("facing_direction", 0);
    Facing6 f6 = Facing6FromBedrockFacingDirectionA(facingDirection);
    props["facing"] = JavaNameFromFacing6(f6);
  }

  static void Facing4FromFacingDirectionA(CompoundTag const &s, Props &props) {
    auto facingDirection = s.int32("facing_direction", 0);
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
    props["facing"] = JavaNameFromFacing4(f4);
  }

  static void FacingBFromDirection(CompoundTag const &s, Props &props) {
    auto direction = s.int32("direction", 0);
    std::string facing;
    switch (direction) {
    case 2:
      facing = "south";
      break;
    case 1:
      facing = "west";
      break;
    case 3:
      facing = "north";
      break;
    case 0:
    default:
      facing = "east";
      break;
    }
    props["facing"] = facing;
  }

  static void Facing6FromFacingDirectionB(CompoundTag const &s, Props &props) {
    auto direction = s.int32("facing_direction", 0);
    Facing6 f6 = Facing6FromBedrockFacingDirectionB(direction);
    std::string facing = JavaNameFromFacing6(f6);
    props["facing"] = facing;
  }

  static void FacingCFromDirection(CompoundTag const &s, Props &props) {
    auto direction = s.int32("direction", 0);
    std::string facing;
    switch (direction) {
    case 1:
      facing = "south";
      break;
    case 2:
      facing = "west";
      break;
    case 3:
      facing = "north";
      break;
    case 0:
    default:
      facing = "east";
      break;
    }
    props["facing"] = facing;
  }

  static void CoralDirection(CompoundTag const &s, Props &p) {
    auto d = s.int32("coral_direction", 0);
    std::string facing;
    switch (d) {
    case 2:
      facing = "north";
      break;
    case 1:
      facing = "east";
      break;
    case 3:
      facing = "south";
      break;
    case 0:
    default:
      facing = "west";
      break;
    }
    p["facing"] = facing;
  }

  static Converter Torch(std::string prefix) {
    return [prefix](String const &bName, CompoundTag const &s, Props &p) -> String {
      auto t = s.string("torch_facing_direction", "top");
      std::string f;
      if (t == "top" || t == "unknown") {
        // "unknown" can be seen when torch as an item
        return Ns() + prefix + "torch";
      } else {
        if (t == "west") {
          f = "east";
        } else if (t == "east") {
          f = "west";
        } else if (t == "south") {
          f = "north";
        } else { // north
          f = "south";
        }
        p["facing"] = f;
        return Ns() + prefix + "wall_torch";
      }
    };
  }

  static void FacingFromWeirdoDirection(CompoundTag const &s, Props &p) {
    auto weirdoDirection = s.int32("weirdo_direction", 0);
    std::string facing = "east";
    switch (weirdoDirection) {
    case 2:
      facing = "south";
      break;
    case 3:
      facing = "north";
      break;
    case 1:
      facing = "west";
      break;
    case 0:
    default:
      facing = "east";
      break;
    }
    p["facing"] = facing;
  }

  static void HalfFromUpperBlockBit(CompoundTag const &s, Props &props) {
    auto upper = s.boolean("upper_block_bit", false);
    props["half"] = upper ? "upper" : "lower";
  }

  static void HalfFromUpsideDownBit(CompoundTag const &s, Props &props) {
    auto upsideDown = s.boolean("upside_down_bit", false);
    props["half"] = upsideDown ? "top" : "bottom";
  }

  static void Lit(CompoundTag const &s, Props &p) {
    auto lit = s.boolean("lit", false);
    p["lit"] = Bool(lit);
  }

  static void OpenFromOpenBit(CompoundTag const &s, Props &props) {
    auto open = s.boolean("open_bit", false);
    props["open"] = Bool(open);
  }

  static void PersistentFromPersistentBit(CompoundTag const &s, Props &props) {
    auto persistent = s.boolean("persistent_bit", false);
    props["persistent"] = Bool(persistent);
  }

  static void PowerFromRedstoneSignal(CompoundTag const &s, Props &p) {
    auto signal = s.int32("redstone_signal", 0);
    p["power"] = Int(signal);
  }

  static void RotationFromGroundSignDirection(CompoundTag const &s, Props &p) {
    auto groundSignRotation = s.int32("ground_sign_direction", 0);
    p["rotation"] = Int(groundSignRotation);
  }

  static std::string ShapeFromRailDirection(int32_t railDirection) {
    switch (railDirection) {
    case 1:
      return "east_west";
    case 2:
      return "ascending_east";
    case 5:
      return "ascending_south";
    case 3:
      return "ascending_west";
    case 4:
      return "ascending_north";
    case 9:
      return "north_east";
    case 8:
      return "north_west";
    case 6:
      return "south_east";
    case 7:
      return "south_west";
    case 0:
    default:
      return "north_south";
    }
  }

  static std::string StoneTypeFromStone2(std::string const &stoneType2) {
    std::string name;
    if (stoneType2 == "prismarine_dark") {
      name = "dark_prismarine";
    } else if (stoneType2 == "prismarine_rough") {
      name = "prismarine";
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
    p["waterlogged"] = "false";
  }

  static bool MushroomProperties(CompoundTag const &s, Props &p) {
    auto bits = s.int32("huge_mushroom_bits", 0);
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
    p["up"] = Bool(up);
    p["down"] = Bool(down);
    p["north"] = Bool(north);
    p["east"] = Bool(east);
    p["south"] = Bool(south);
    p["west"] = Bool(west);
    return stem;
  }

  static void TypeFromTopSlotBit(CompoundTag const &s, Props &p) {
    auto topSlot = s.boolean("top_slot_bit", false);
    p["type"] = topSlot ? "top" : "bottom";
  }

  static std::string WallConnectionType(std::string const &type) {
    // short,tall,none => low,tall,none
    if (type == "short") {
      return "low";
    } else {
      return type;
    }
  }

  static void WallProperties(CompoundTag const &s, Props &p) {
    auto connectionTypeEast = s.string("wall_connection_type_east", "short");
    auto connectionTypeNorth = s.string("wall_connection_type_north", "short");
    auto connectionTypeSouth = s.string("wall_connection_type_south", "short");
    auto connectionTypeWest = s.string("wall_connection_type_west", "short");
    auto post = s.boolean("wall_post_bit", false);
    p["east"] = WallConnectionType(connectionTypeEast);
    p["north"] = WallConnectionType(connectionTypeNorth);
    p["south"] = WallConnectionType(connectionTypeSouth);
    p["west"] = WallConnectionType(connectionTypeWest);
    p["up"] = Bool(post);
  }
#pragma endregion

  static std::unordered_map<std::string, Converter> *CreateTable() {
    using namespace std;
    auto table = new std::unordered_map<string, Converter>();
#define E(__name, __converter)                               \
  assert(table->find("minecraft:" #__name) == table->end()); \
  table->emplace("minecraft:" #__name, __converter);

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

    E(fence, Fence);
    E(crimson_fence, BlockWithSubmergible);
    E(nether_brick_fence, BlockWithSubmergible);
    E(warped_fence, BlockWithSubmergible);

    E(acacia_fence_gate, FenceGate);
    E(birch_fence_gate, FenceGate);
    E(crimson_fence_gate, FenceGate);
    E(dark_oak_fence_gate, FenceGate);
    E(jungle_fence_gate, FenceGate);
    E(fence_gate, FenceGate);
    E(spruce_fence_gate, FenceGate);
    E(warped_fence_gate, FenceGate);

    E(leaves, Leaves);
    E(leaves2, Leaves2);
    E(log2, Log2);
    E(log, Log);
    E(planks, Planks);

    E(acacia_pressure_plate, PressurePlate);
    E(birch_pressure_plate, PressurePlate);
    E(crimson_pressure_plate, PressurePlate);
    E(dark_oak_pressure_plate, PressurePlate);
    E(jungle_pressure_plate, PressurePlate);
    E(polished_blackstone_pressure_plate, PressurePlate);
    E(spruce_pressure_plate, PressurePlate);
    E(stone_pressure_plate, PressurePlate);
    E(warped_pressure_plate, PressurePlate);

    E(sapling, Sapling);

    E(acacia_standing_sign, StandingSign);
    E(birch_standing_sign, StandingSign);
    E(crimson_standing_sign, StandingSign);
    E(jungle_standing_sign, StandingSign);
    E(standing_sign, StandingSign);
    E(spruce_standing_sign, StandingSign);
    E(warped_standing_sign, StandingSign);

    E(wooden_slab, WoodenSlab);
    E(double_wooden_slab, DoubleWoodenSlab);

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
    E(end_brick_stairs, RenameStairs("end_stone_brick_stairs"));
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
    E(prismarine_bricks_stairs, RenameStairs("prismarine_brick_stairs"));
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
    E(stone_stairs, RenameStairs("cobblestone_stairs"));
    E(normal_stone_stairs, RenameStairs("stone_stairs"));
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

    E(wood, Wood);
    E(activator_rail, RailCanBePowered);
    E(detector_rail, RailCanBePowered);
    E(red_flower, RedFlower);
    E(amethyst_cluster, BlockWithFacing6FromFacingDirectionASubmergible);
    E(stone, Stone);
    E(stone_slab3, StoneSlab3);
    E(double_stone_slab3, DoubleStoneSlab3);

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
    E(blackstone_double_slab, DoubleSlab("blackstone_slab"));
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

    E(carpet, Carpet);
    E(concrete, Concrete);
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

    E(shulker_box, ShulkerBox);
    E(stained_glass, StainedGlass);
    E(stained_glass_pane, StainedGlassPane);
    E(stained_hardened_clay, StainedHardenedClay);
    E(wall_banner, BlockWithFacing4FromFacingDirectionA);
    E(wool, Wool);
    E(blast_furnace, FurnaceAndSimilar);
    E(lit_blast_furnace, FurnaceAndSimilar);
    E(bone_block, BlockWithAxisFromPillarAxis);
    E(coral, Coral);
    E(coral_block, CoralBlock);
    E(coral_fan_hang, CoralFanHang);
    E(coral_fan_hang2, CoralFanHang);
    E(coral_fan_hang3, CoralFanHang);
    E(coral_fan, CoralFan);
    E(coral_fan_dead, CoralFan);
    E(brewing_stand, BrewingStand);
    E(brick_block, Rename("bricks"));
    E(stone_slab, StoneSlab);
    E(double_stone_slab, DoubleStoneSlab);
    E(brown_mushroom_block, BrownMushroomBlock);
    E(bubble_column, BubbleColumn);
    E(cactus, BlockWithAge);
    E(cake, Cake);
    E(campfire, Campfire);
    E(soul_campfire, Campfire);
    E(carrots, BlockWithAgeFromGrowth);
    E(carved_pumpkin, BlockWithFacing4FromDirectionA);
    E(cauldron, Cauldron);
    E(lava_cauldron, Cauldron);
    E(cave_vines, CaveVines);
    E(cave_vines_with_berries, CaveVines);
    E(cave_vines_head_with_berries, CaveVines);
    E(cave_vines_body_with_berries, CaveVinesBody);
    E(chain, Chain);
    E(chain_command_block, CommandBlock);
    E(command_block, CommandBlock);
    E(chest, BlockWithFacing4FromFacingDirectionASubmergible);
    E(quartz_block, QuartzBlock);
    E(red_sandstone, RedSandstone);
    E(sandstone, Sandstone);
    E(stonebrick, Stonebrick);
    E(chorus_flower, BlockWithAge);
    E(chorus_plant, Same);
    E(dirt, Dirt);
    E(cobbled_deepslate_slab, Slab);
    E(cobbled_deepslate_double_slab, DoubleSlab("cobbled_deepslate_slab"));
    E(cobbled_deepslate_wall, BlockWithWallProperties);
    E(web, Rename("cobweb"));
    E(cocoa, Cocoa);
    E(powered_comparator, Comparator);
    E(unpowered_comparator, Comparator);
    E(composter, Composter);
    E(conduit, BlockWithSubmergible);
    E(skull, Same);
    E(crimson_hyphae, BlockWithAxisFromPillarAxis);
    E(crimson_slab, Slab);
    E(crimson_double_slab, DoubleSlab("crimson_slab"));
    E(crimson_stem, BlockWithAxisFromPillarAxis);
    E(cut_copper_slab, Slab);
    E(double_cut_copper_slab, DoubleSlab("cut_copper_slab"));
    E(stone_slab4, StoneSlab4);
    E(double_stone_slab4, DoubleStoneSlab4);
    E(yellow_flower, Rename("dandelion"));
    E(darkoak_standing_sign, DarkoakStandingSign);
    E(darkoak_wall_sign, DarkoakWallSign);
    E(prismarine, Prismarine);
    E(stone_slab2, StoneSlab2);
    E(double_stone_slab2, DoubleStoneSlab2);
    E(daylight_detector, DaylightDetector);
    E(daylight_detector_inverted, DaylightDetector);
    E(deadbush, Rename("dead_bush"));
    E(deepslate, BlockWithAxisFromPillarAxis);
    E(deepslate_brick_slab, Slab);
    E(deepslate_brick_double_slab, DoubleSlab("deepslate_brick_slab"));
    E(deepslate_brick_wall, BlockWithWallProperties);
    E(deepslate_redstone_ore, RedstoneOre);
    E(lit_deepslate_redstone_ore, RedstoneOre);
    E(deepslate_tile_slab, Slab);
    E(deepslate_tile_double_slab, DoubleSlab("deepslate_tile_slab"));
    E(deepslate_tile_wall, BlockWithWallProperties);
    E(grass_path, Rename("dirt_path"));
    E(dispenser, Dispenser);
    E(dropper, Dropper);
    E(ender_chest, BlockWithFacing4FromFacingDirectionASubmergible);
    E(end_portal_frame, EndPortalFrame);
    E(end_rod, EndRod);
    E(end_bricks, Rename("end_stone_bricks"));
    E(exposed_cut_copper_slab, Slab);
    E(exposed_double_cut_copper_slab, DoubleSlab("exposed_cut_copper_slab"));
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
    E(grass, Grass);
    E(grindstone, Grindstone);
    E(hanging_roots, BlockWithSubmergible);
    E(hay_block, BlockWithAxisFromPillarAxis);
    E(heavy_weighted_pressure_plate, BlockWithPowerFromRedstoneSignal);
    E(hopper, Hopper);
    E(monster_egg, MonsterEgg);
    E(infested_deepslate, BlockWithAxisFromPillarAxis);
    E(iron_bars, BlockWithSubmergible);
    E(lit_pumpkin, LitPumpkin);
    E(jigsaw, Same);
    E(jukebox, Same);
    E(kelp, Kelp);
    E(ladder, BlockWithFacing4FromFacingDirectionASubmergible);
    E(lantern, Lantern);
    E(soul_lantern, Lantern);
    E(large_amethyst_bud, BlockWithFacing6FromFacingDirectionASubmergible);
    E(double_plant, DoublePlant);
    E(lava, Liquid);
    E(lectern, Lectern);
    E(lever, Lever);
    E(lightning_rod, BlockWithFacing6FromFacingDirectionASubmergible);
    E(light_block, LightBlock);
    E(light_weighted_pressure_plate, BlockWithPowerFromRedstoneSignal);
    E(waterlily, Rename("lily_pad"));
    E(loom, BlockWithFacing4FromDirectionA);
    E(magma, Rename("magma_block"));
    E(medium_amethyst_bud, BlockWithFacing6FromFacingDirectionASubmergible);
    E(melon_block, Rename("melon"));
    E(movingBlock, Same); // legacy, < 1.18.30
    E(moving_block, Same);
    E(mycelium, BlockWithSnowy); // No "snowy" property in BE
    E(nether_brick, Rename("nether_bricks"));
    E(portal, Portal);
    E(quartz_ore, Rename("nether_quartz_ore"));
    E(nether_wart, BlockWithAge);
    E(noteblock, Rename("note_block"));
    E(wooden_pressure_plate, WoodenPressurePlate);
    E(wall_sign, OakWallSign);
    E(observer, Observer);
    E(oxidized_cut_copper_slab, Slab);
    E(oxidized_double_cut_copper_slab, DoubleSlab("oxidized_cut_copper_slab"));
    E(piston, BlockWithFacing6FromFacingDirection6);
    E(sticky_piston, BlockWithFacing6FromFacingDirection6);
    E(podzol, BlockWithSnowy); // No "snowy" property in BE
    E(pointed_dripstone, PointedDripstone);
    E(polished_basalt, BlockWithAxisFromPillarAxis);
    E(polished_blackstone_brick_slab, Slab);
    E(polished_blackstone_brick_double_slab, DoubleSlab("polished_blackstone_brick_slab"));
    E(polished_blackstone_brick_wall, BlockWithWallProperties);
    E(polished_blackstone_slab, Slab);
    E(polished_blackstone_double_slab, DoubleSlab("polished_blackstone_slab"));
    E(polished_blackstone_wall, BlockWithWallProperties);
    E(polished_deepslate_slab, Slab);
    E(polished_deepslate_double_slab, DoubleSlab("polished_deepslate_slab"));
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
    E(redstone_torch, Torch("redstone_"));
    E(redstone_wire, BlockWithPowerFromRedstoneSignal);
    E(red_mushroom_block, RedMushroomBlock);
    E(red_nether_brick, Rename("red_nether_bricks"));
    E(sand, Sand);
    E(powered_repeater, Repeater);
    E(unpowered_repeater, Repeater);
    E(repeating_command_block, CommandBlock);
    E(respawn_anchor, RespawnAnchor);
    E(dirt_with_roots, Rename("rooted_dirt"));
    E(scaffolding, Scaffolding);
    E(sculk_sensor, Same);
    E(seagrass, Seagrass);
    E(sea_pickle, SeaPickle);
    E(seaLantern, Rename("sea_lantern")); // legacy, < 1.18.30
    E(sea_lantern, Same);
    E(undyed_shulker_box, Rename("shulker_box"));
    E(slime, Rename("slime_block"));
    E(small_amethyst_bud, BlockWithFacing6FromFacingDirectionASubmergible);
    E(small_dripleaf_block, SmallDripleafBlock);
    E(smoker, FurnaceAndSimilar);
    E(lit_smoker, FurnaceAndSimilar);
    E(snow_layer, SnowLayer);
    E(snow, Rename("snow_block"));
    E(soul_torch, Torch("soul_"));
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
    E(reeds, Reeds);
    E(sweet_berry_bush, SweetBerryBush);
    E(target, Target);
    E(hardened_clay, Rename("terracotta"));
    E(tnt, Tnt);
    E(torch, Torch(""));
    E(trapped_chest, BlockWithFacing4FromFacingDirectionASubmergible);
    E(tripWire, Tripwire); // legacy, < 1.18.30
    E(trip_wire, Tripwire);
    E(tripwire_hook, TripwireHook);
    E(turtle_egg, TurtleEgg);
    E(twisting_vines, NetherVines("twisting"));
    E(vine, Vine);
    E(warped_hyphae, BlockWithAxisFromPillarAxis);
    E(warped_slab, Slab);
    E(warped_double_slab, DoubleSlab("warped_slab"));
    E(warped_stem, BlockWithAxisFromPillarAxis);
    E(water, Liquid);
    E(waxed_copper, Rename("waxed_copper_block"));
    E(waxed_cut_copper_slab, Slab);
    E(waxed_double_cut_copper_slab, DoubleSlab("waxed_cut_copper_slab"));
    E(waxed_exposed_cut_copper_slab, Slab);
    E(waxed_exposed_double_cut_copper_slab, DoubleSlab("waxed_exposed_cut_copper_slab"));
    E(waxed_oxidized_cut_copper_slab, Slab);
    E(waxed_oxidized_double_cut_copper_slab, DoubleSlab("waxed_oxidized_cut_copper_slab"));
    E(waxed_weathered_cut_copper_slab, Slab);
    E(waxed_weathered_double_cut_copper_slab, DoubleSlab("waxed_weathered_cut_copper_slab"));
    E(weathered_cut_copper_slab, Slab);
    E(weathered_double_cut_copper_slab, DoubleSlab("weathered_cut_copper_slab"));
    E(weeping_vines, NetherVines("weeping"));
    E(wheat, BlockWithAgeFromGrowth);
    E(mob_spawner, Rename("spawner"));
    E(frame, Rename("air"));
    E(glow_frame, Rename("air"));
    E(pistonArmCollision, PistonArmCollision); // legacy, < 1.18.30
    E(piston_arm_collision, PistonArmCollision);
    E(stickyPistonArmCollision, PistonArmCollision); // legacy, 1.18.30
    E(sticky_piston_arm_collision, PistonArmCollision);
    E(flowing_lava, LiquidFlowing);
    E(flowing_water, LiquidFlowing);

    // 1.19
    E(frog_spawn, Rename("frogspawn"));
    E(mangrove_log, BlockWithAxisFromPillarAxis);
    E(stripped_mangrove_log, BlockWithAxisFromPillarAxis);
    E(mangrove_wood, BlockWithAxisFromPillarAxis);
    E(stripped_mangrove_wood, BlockWithAxisFromPillarAxis);
    E(ochre_froglight, BlockWithAxisFromPillarAxis);
    E(verdant_froglight, BlockWithAxisFromPillarAxis);
    E(pearlescent_froglight, BlockWithAxisFromPillarAxis);
    E(mangrove_slab, Slab);
    E(mangrove_double_slab, DoubleSlab("mangrove_slab"));
    E(mangrove_button, Button);
    E(mangrove_door, Door);
    E(mangrove_fence, BlockWithSubmergible);
    E(mangrove_fence_gate, FenceGate);
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
#undef E

    return table;
  }

#pragma region Utilities
  static std::string Bool(bool b) {
    return b ? "true" : "false";
  }

  static std::string Int(int32_t i) {
    return std::to_string(i);
  }

  static inline std::string Ns() {
    return "minecraft:";
  }

  // Get "acacia" from "minecraft:acacia_pressure_plate" when suffix is "_pressure_plate"
  static inline std::string VariantFromName(std::string const &name, std::string const &suffix) {
    assert(name.starts_with(Ns()) && name.ends_with(suffix));
    return name.substr(Ns().size()).substr(0, name.size() - Ns().size() - suffix.size());
  }
#pragma endregion
};

} // namespace je2be::toje
