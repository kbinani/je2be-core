#pragma once

namespace je2be::toje {

class BlockData {
  using String = std::string;
  using States = mcfile::nbt::CompoundTag;
  using Props = std::map<std::string, std::string>;
  using Converter = std::function<String(String const &bName, States const &s, Props &p)>;

public:
  static std::shared_ptr<mcfile::je::Block const> From(mcfile::be::Block const &b) {
    using namespace std;
    using namespace mcfile;
    using namespace mcfile::nbt;
    static unique_ptr<unordered_map<string, Converter> const> const sTable(CreateTable());

    auto found = sTable->find(b.fName);
    if (found == sTable->end()) {
      return nullptr;
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
  static String Anvil(String const &bName, States const &s, Props &p) {
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
    FacingAFromDirection(s, p);
    return Ns() + name;
  }

  static String AzaleaLeaves(String const &bName, States const &s, Props &p) {
    PersistentFromPersistentBit(s, p);
    return bName;
  }
#pragma endregion

#pragma region Converters : B
  static String Bamboo(String const &bName, States const &s, Props &p) {
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

  static String Barrel(String const &bName, States const &s, Props &p) {
    OpenFromOpenBit(s, p);
    FacingAFromFacingDirection(s, p);
    return bName;
  }

  static String Beehive(String const &bName, States const &s, Props &p) {
    FacingAFromDirection(s, p);
    auto honeyLevel = s.int32("honey_level", 0);
    p["honey_level"] = Int(honeyLevel);
    return bName;
  }

  static String Beetroot(String const &bName, States const &s, Props &p) {
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
    return Ns() + "beetroots";
  }

  static String Bed(String const &bName, States const &s, Props &p) {
    FacingAFromDirection(s, p);
    auto headPiece = s.boolean("head_piece_bit", false);
    auto occupied = s.boolean("occupied_bit", false);
    p["part"] = headPiece ? "head" : "foot";
    p["occupied"] = Bool(occupied);
    return Ns() + "white_bed";
  }

  static String Bell(String const &bName, States const &s, Props &p) {
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

  static String BigDripleaf(String const &bName, States const &s, Props &p) {
    auto head = s.boolean("big_dripleaf_head", false);
    FacingAFromDirection(s, p);
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
    return Ns() + name;
  }

  static String BlackstoneDoubleSlab(String const &bName, States const &s, Props &p) {
    p["type"] = "double";
    return Ns() + "blackstone_slab";
  }

  static String BlockWithAge(String const &bName, States const &s, Props &p) {
    Age(s, p);
    return bName;
  }

  static String BlockWithAgeFromGrowth(String const &bName, States const &s, Props &p) {
    AgeFromGrowth(s, p);
    return bName;
  }

  static String BlockWithAxisFromPillarAxis(String const &bName, States const &s, Props &p) {
    AxisFromPillarAxis(s, p);
    return bName;
  }

  static String BlockWithFacingAFromDirection(String const &bName, States const &s, Props &p) {
    FacingAFromDirection(s, p);
    return bName;
  }

  static String BlockWithFacingAFromFacingDirection(String const &bName, States const &s, Props &p) {
    FacingAFromFacingDirection(s, p);
    return bName;
  }

  static String BlockWithRotationFromGroundSignDirection(String const &bName, States const &s, Props &p) {
    RotationFromGroundSignDirection(s, p);
    return bName;
  }

  static String BlockWithTypeFromTopSlotBit(String const &bName, States const &s, Props &p) {
    TypeFromTopSlotBit(s, p);
    return bName;
  }

  static String BlockWithWallProperties(String const &bName, States const &s, Props &p) {
    WallProperties(s, p);
    return bName;
  }

  static String BrewingStand(String const &bName, States const &s, Props &p) {
    auto slotA = s.boolean("brewing_stand_slot_a_bit", false);
    auto slotB = s.boolean("brewing_stand_slot_b_bit", false);
    auto slotC = s.boolean("brewing_stand_slot_c_bit", false);
    p["has_bottle_0"] = Bool(slotA);
    p["has_bottle_1"] = Bool(slotB);
    p["has_bottle_2"] = Bool(slotC);
    return bName;
  }

  static String BrickBlock(String const &bName, States const &s, Props &p) {
    return Ns() + "bricks";
  }

  static String BrownMushroomBlock(String const &bName, States const &s, Props &p) {
    bool stem = MushroomProperties(s, p);
    std::string name;
    if (stem) {
      name = "mushroom_stem";
    } else {
      name = "brown_mushroom_block";
    }
    return Ns() + name;
  }

  static String BubbleColumn(String const &bName, States const &s, Props &p) {
    auto dragDown = s.boolean("drag_down", false);
    p["drag"] = Bool(dragDown);
    return bName;
  }

  static String Button(String const &bName, States const &s, Props &p) {
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
    return bName;
  }
#pragma endregion

#pragma region Converters : C
  static String Cake(String const &bName, States const &s, Props &p) {
    auto biteCounter = s.int32("bite_counter", 0);
    p["bites"] = Int(biteCounter);
    return bName;
  }

  static String Campfire(String const &bName, States const &s, Props &p) {
    FacingAFromDirection(s, p);
    auto extinguished = s.boolean("extinguished", false);
    p["lit"] = Bool(!extinguished);
    return bName;
  }

  static String Candle(String const &bName, States const &s, Props &p) {
    auto candles = s.int32("candles", 0);
    p["candles"] = Int(candles + 1);
    Lit(s, p);
    return bName;
  }

  static String CandleCake(String const &bName, States const &s, Props &p) {
    Lit(s, p);
    return bName;
  }

  static String Carpet(String const &bName, States const &s, Props &p) {
    auto color = s.string("color", "white");
    return Ns() + color + "_carpet";
  }

  static String Cauldron(String const &bName, States const &s, Props &p) {
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

  static String CaveVines(String const &bName, States const &s, Props &p) {
    auto berries = bName.ends_with("_with_berries");
    auto growingPlantAge = s.int32("growing_plant_age", 1);
    if (growingPlantAge > 24) {
      return Ns() + "cave_vines_plant";
    } else {
      p["age"] = Int(growingPlantAge);
      return Ns() + "cave_vines";
    }
  }

  static String CaveVinesBody(String const &bName, States const &s, Props &p) {
    auto berries = bName.ends_with("_with_berries");
    p["berries"] = Bool(berries);
    return Ns() + "cave_vines_plant";
  }

  static String CobbledDeepslateDoubleSlab(String const &bName, States const &s, Props &p) {
    p["type"] = "double";
    return Ns() + "cobbled_deepslate_slab";
  }

  static String Cocoa(String const &bName, States const &s, Props &p) {
    FacingAFromDirection(s, p);
    Age(s, p);
    return bName;
  }

  static String CommandBlock(String const &bName, States const &s, Props &p) {
    auto conditional = s.boolean("conditional_bit", false);
    FacingAFromFacingDirection(s, p);
    return bName;
  }

  static String Concrete(String const &bName, States const &s, Props &p) {
    auto color = s.string("color", "white");
    return Ns() + color + "_concrete";
  }

  static String ConcretePowder(String const &bName, States const &s, Props &p) {
    auto color = s.string("color", "white");
    return Ns() + color + "_concrete_powder";
  }

  static String Coral(String const &bName, States const &s, Props &p) {
    auto color = s.string("coral_color", "pink");
    auto dead = s.boolean("dead_bit", false);
    auto type = CoralTypeFromCoralColor(color);
    std::string name;
    if (dead) {
      name = "dead_";
    }
    name += type;
    return Ns() + name;
  }

  static String CoralBlock(String const &bName, States const &s, Props &p) {
    auto color = s.string("coral_color", "pink");
    auto dead = s.boolean("dead_bit", false);
    auto type = CoralTypeFromCoralColor(color);
    std::string name;
    if (dead) {
      name = "dead_";
    }
    name += type + "_block";
    return Ns() + name;
  }

  static String CoralFan(String const &bName, States const &s, Props &p) {
    auto color = s.string("coral_color", "pink");
    auto dead = bName.ends_with("_dead");
    auto type = CoralTypeFromCoralColor(color);
    std::string name;
    if (dead) {
      name = "dead_";
    }
    name += type + "_fan";
    return Ns() + name;
  }

  static String CoralFanHang(String const &bName, States const &s, Props &p) {
    auto direction = s.int32("coral_direction", 0);
    auto hangType = s.boolean("coral_hang_type_bit", false);
    auto dead = s.boolean("dead", false);
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
    return Ns() + name + "_wall_fan";
  }
#pragma endregion

#pragma region Converters : D
  static String Dirt(String const &bName, States const &s, Props &p) {
    auto type = s.string("type", "normal");
    std::string prefix;
    if (type != "normal") {
      prefix = type + "_";
    }
    return Ns() + prefix + "dirt";
  }

  static String Door(String const &bName, States const &s, Props &p) {
    auto doorHingeBit = s.boolean("door_hinge_bit", false);
    auto upperBlockBit = s.boolean("upper_block_bit", false);

    OpenFromOpenBit(s, p);
    FacingCFromDirection(s, p);
    p["hinge"] = doorHingeBit ? "right" : "left";
    p["half"] = upperBlockBit ? "upper" : "lower";
    p["powered"] = "false";
    return bName;
  }

  static String DoubleStoneSlab(String const &bName, States const &s, Props &p) {
    auto stoneSlabType = s.string("stone_slab_type", "stone");
    p["type"] = "double";
    return Ns() + stoneSlabType + "_slab";
  }

  static String DoubleStoneSlab3(String const &bName, States const &s, Props &p) {
    auto stoneSlabType = s.string("stone_slab_type3", "andesite");
    p["type"] = "double";
    return Ns() + stoneSlabType + "_slab";
  }

  static String DoubleWoodenSlab(String const &bName, States const &s, Props &p) {
    auto woodType = s.string("wood_type", "oak");
    p["type"] = "double";
    p["waterlogged"] = "false";
    return Ns() + woodType + "_slab";
  }
#pragma endregion

#pragma region Converter : F
  static String Fence(String const &bName, States const &s, Props &p) {
    auto woodType = s.string("wood_type", "oak");
    return Ns() + woodType + "_fence";
  }

  static String FenceGate(String const &bName, States const &s, Props &p) {
    auto inWall = s.boolean("in_wall_bit", false);
    p["in_wall"] = Bool(inWall);
    p["powered"] = Bool(false);
    FacingAFromDirection(s, p);
    OpenFromOpenBit(s, p);
    return bName;
  }
#pragma endregion

#pragma region Converters : L
  static String Leaves(String const &bName, States const &s, Props &p) {
    auto leafType = s.string("old_leaf_type", "oak");
    PersistentFromPersistentBit(s, p);
    return Ns() + leafType + "_leaves";
  }

  static String Leaves2(String const &bName, States const &s, Props &p) {
    auto newLeafType = s.string("new_leaf_type", "acacia"); //TODO: acacia?
    PersistentFromPersistentBit(s, p);
    return Ns() + newLeafType + "_leaves";
  }

  static String Log(String const &bName, States const &s, Props &p) {
    auto logType = s.string("old_log_type", "oak");
    AxisFromPillarAxis(s, p);
    return Ns() + logType + "_log";
  }

  static String Log2(String const &bName, States const &s, Props &p) {
    auto logType = s.string("new_log_type", "acacia"); //TODO: acacia?
    AxisFromPillarAxis(s, p);
    return Ns() + logType + "_log";
  }
#pragma endregion

#pragma region Converters : M
  static String MelonStem(String const &bName, States const &s, Props &p) {
    auto growth = s.int32("growth", 0);
    std::string name;
    if (growth >= 7) {
      name = "attached_melon_stem";
    } else {
      name = "melon_stem";
      p["age"] = Int(growth);
    }
    FacingAFromDirection(s, p, "facing_direction");
    return Ns() + name;
  }
#pragma endregion

#pragma region Converters : P
  static String Planks(String const &bName, States const &s, Props &p) {
    auto woodType = s.string("wood_type", "acacia"); //TODO: acacia?
    return Ns() + woodType + "_planks";
  }

  static String PressurePlate(String const &bName, States const &s, Props &p) {
    auto redstoneSignal = s.int32("redstone_signal", 0);
    p["powered"] = Bool(redstoneSignal > 0);
    return bName;
  }

  static String PumpkinStem(String const &bName, States const &s, Props &p) {
    auto growth = s.int32("growth", 0);
    std::string name;
    if (growth >= 7) {
      name = "attached_pumpkin_stem";
    } else {
      name = "pumpkin_stem";
      p["age"] = Int(growth);
    }
    FacingAFromDirection(s, p, "facing_direction");
    return Ns() + name;
  }
#pragma endregion

#pragma region Converters : Q
  static String QuartzBlock(String const &bName, States const &s, Props &p) {
    auto type = s.string("chisel_type", "chiseled");
    std::string prefix;
    if (type == "lines") {
      AxisFromPillarAxis(s, p);
      return Ns() + "quartz_pillar";
    } else if (type == "smooth") {
      return Ns() + "smooth_quartz_block";
    } else if (type == "chiseled") {
      return Ns() + "chiseled_quartz_block";
    } else { // "default";
      return Ns() + "quartz_block";
    }
  }
#pragma endregion

#pragma region Converters : R
  static String RailCanBePowered(String const &bName, States const &s, Props &p) {
    auto railData = s.boolean("rail_data_bit", false);
    auto railDirection = s.int32("rail_direction", 0);
    p["powered"] = Bool(railData);
    p["shape"] = ShapeFromRailDirection(railDirection);
    return bName;
  }

  static String RedFlower(String const &bName, States const &s, Props &p) {
    auto flowerType = s.string("flower_type", "poppy");
    std::string name = flowerType;
    if (flowerType == "orchid") {
      name = "blue_orchid";
    } else if (flowerType == "houstonia") {
      name = "azure_bluet";
    } else if (flowerType == "tulip_red") {
      name = "red_tulip";
    } else if (flowerType == "tulip_orange") {
      name = "orange_tulip";
    } else if (flowerType == "tulip_white") {
      name = "white_tulip";
    } else if (flowerType == "tulip_pink") {
      name = "pink_tulip";
    } else if (flowerType == "oxeye") {
      name = "oxeye_daisy";
    }
    return Ns() + name;
  }

  static String RedSandstone(String const &bName, States const &s, Props &p) {
    auto type = s.string("sand_stone_type", "default");
    std::string name = "red_sandstone";
    if (type == "hieroglyphs") {
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
#pragma endregion

#pragma region Converters : S
  static String Same(String const &bName, States const &s, Props &p) {
    return bName;
  }

  static String Sandstone(String const &bName, States const &s, Props &p) {
    auto type = s.string("sand_stone_type", "default");
    std::string name = "sandstone";
    if (type == "hieroglyphs") {
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

  static String Sapling(String const &bName, States const &s, Props &p) {
    auto age = s.byte("age_bit", 0);
    auto saplingType = s.string("sapling_type", "acacia");
    p["stage"] = Int(age);
    return Ns() + saplingType + "_sapling";
  }

  static String ShulkerBox(String const &bName, States const &s, Props &p) {
    auto color = s.string("color", "white");
    return Ns() + color + "_shulker_box";
  }

  static String Stairs(String const &bName, States const &s, Props &p) {
    FacingFromWeirdoDirection(s, p);
    HalfFromUpsideDownBit(s, p);
    return bName;
  }

  static String StainedGlass(String const &bName, States const &s, Props &p) {
    auto color = s.string("color", "white");
    return Ns() + color + "_stained_glass";
  }

  static String StainedGlassPane(String const &bName, States const &s, Props &p) {
    auto color = s.string("color", "white");
    return Ns() + color + "_stained_glass_pane";
  }

  static String StainedHardenedClay(String const &bName, States const &s, Props &p) {
    auto color = s.string("color", "white");
    return Ns() + color + "_terracotta";
  }

  static String StandingSign(String const &bName, States const &s, Props &p) {
    auto type = VariantFromName(bName, "_standing_sign");
    auto groundSignRotation = s.int32("ground_sign_direction", 0);
    RotationFromGroundSignDirection(s, p);
    return Ns() + type + "_sign";
  }

  static String Stone(String const &bName, States const &s, Props &p) {
    auto stoneType = s.string("stone_type");
    std::string name = stoneType ? *stoneType : "stone";
    return Ns() + name;
  }

  static String Stonebrick(String const &bName, States const &s, Props &p) {
    auto type = s.string("stone_brick_type", "default");
    std::string prefix;
    if (type != "default") {
      prefix = type + "_";
    }
    return Ns() + prefix + "stone_bricks";
  }

  static String StoneSlab(String const &bName, States const &s, Props &p) {
    auto stoneSlabType = s.string("stone_slab_type", "stone");
    TypeFromTopSlotBit(s, p);
    return Ns() + stoneSlabType + "_slab";
  }

  static String StoneSlab3(String const &bName, States const &s, Props &p) {
    auto stoneSlabType = s.string("stone_slab_type3", "andesite");
    TypeFromTopSlotBit(s, p);
    return Ns() + stoneSlabType + "_slab";
  }

  static String StoneStairs(String const &bName, States const &s, Props &p) {
    FacingFromWeirdoDirection(s, p);
    HalfFromUpsideDownBit(s, p);
    return Ns() + "cobblestone_stairs";
  }
#pragma endregion

#pragma region Converters : T
  static String Trapdoor(String const &bName, States const &s, Props &p) {
    FacingBFromDirection(s, p);
    OpenFromOpenBit(s, p);
    HalfFromUpsideDownBit(s, p);
    p["powered"] = "false";
    return bName;
  }
#pragma endregion

#pragma region Converters : W
  static String WallWithBlockType(String const &bName, States const &s, Props &p) {
    auto blockType = s.string("wall_block_type", "andesite");
    WallProperties(s, p);
    return Ns() + blockType + "_wall";
  }

  static String Web(String const &bName, States const &s, Props &p) {
    return Ns() + "cobweb";
  }

  static String Wood(String const &bName, States const &s, Props &p) {
    auto stripped = s.boolean("stripped_bit", false);
    auto woodType = s.string("wood_type", "oak");
    auto name = stripped ? "stripped_" + woodType + "_wood" : woodType + "_wood";
    AxisFromPillarAxis(s, p);
    return Ns() + name;
  }

  static String WoodenSlab(String const &bName, States const &s, Props &p) {
    auto woodType = s.string("wood_type", "acacia");
    TypeFromTopSlotBit(s, p);
    return Ns() + woodType + "_slab";
  }

  static String Wool(String const &bName, States const &s, Props &p) {
    auto color = s.string("color", "white");
    return Ns() + color + "_wool";
  }
#pragma endregion

  /*
  static String _(String const &bName, States const &s, Props &p) {
    return bName;
  }
*/

#pragma region Properties
  static void Age(States const &s, Props &p) {
    auto age = s.int32("age", 0);
    p["age"] = Int(age);
  }

  static void AgeFromGrowth(States const &s, Props &p) {
    auto age = s.int32("growth", 0);
    p["age"] = Int(age);
  }

  static void AxisFromPillarAxis(States const &s, Props &props) {
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

  static void FacingAFromDirection(States const &s, Props &props, std::string const &key = "direction") {
    auto direction = s.int32(key, 0);
    std::string facing;
    switch (direction) {
    case 2:
      facing = "north";
      break;
    case 3:
      facing = "east";
      break;
    case 1:
      facing = "west";
      break;
    case 0:
    default:
      facing = "south";
      break;
    }
    props["facing"] = facing;
  }

  static void FacingBFromDirection(States const &s, Props &props) {
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

  static void FacingCFromDirection(States const &s, Props &props) {
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

  static void FacingAFromFacingDirection(States const &s, Props &props) {
    auto facingDirection = s.int32("facing_direction", 0);
    std::string facing;
    // 102534
    switch (facingDirection) {
    case 5:
      facing = "east";
      break;
    case 3:
      facing = "south";
      break;
    case 4:
      facing = "west";
      break;
    case 2:
      facing = "north";
      break;
    case 1:
      facing = "up";
      break;
    case 0:
    default:
      facing = "down";
    }
    props["facing"] = facing;
  }

  static void FacingFromWeirdoDirection(States const &s, Props &p) {
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

  static void Lit(States const &s, Props &p) {
    auto lit = s.boolean("lit", false);
    p["lit"] = Bool(lit);
  }

  static void OpenFromOpenBit(States const &s, Props &props) {
    auto open = s.boolean("open_bit", false);
    props["open"] = Bool(open);
  }

  static void PersistentFromPersistentBit(States const &s, Props &props) {
    auto persistent = s.boolean("persistent_bit", false);
    props["persistent"] = Bool(persistent);
  }

  static void RotationFromGroundSignDirection(States const &s, Props &p) {
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

  static void HalfFromUpsideDownBit(States const &s, Props &props) {
    auto upsideDown = s.boolean("upside_down_bit", false);
    props["half"] = upsideDown ? "top" : "bottom";
  }

  static bool MushroomProperties(States const &s, Props &p) {
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
    return stem;
  }

  static void TypeFromTopSlotBit(States const &s, Props &p) {
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

  static void WallProperties(States const &s, Props &p) {
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

    //TODO: remove these "same" blocks
#pragma region Same
    E(air, Same);
    E(amethyst_block, Same);
    E(ancient_debris, Same);
    E(azalea, Same);
    E(barrier, Same);
    E(beacon, Same);
    E(bedrock, Same);
    E(blackstone, Same);
    E(blue_ice, Same);
    E(bookshelf, Same);
    E(brown_mushroom, Same);
    E(budding_amethyst, Same);
    E(calcite, Same);
    E(cartography_table, Same);
    E(chiseled_deepslate, Same);
    E(chiseled_nether_bricks, Same);
    E(chiseled_polished_blackstone, Same);
    E(clay, Same);
    E(coal_block, Same);
    E(coal_ore, Same);
    E(cobbled_deepslate, Same);
    E(cobblestone, Same);
    E(copper_block, Same);
    E(copper_ore, Same);
    E(cracked_deepslate_bricks, Same);
    E(cracked_deepslate_tiles, Same);
    E(cracked_nether_bricks, Same);
    E(cracked_polished_blackstone_bricks, Same);
    E(crafting_table, Same);
    E(crimson_fungus, Same);
    E(crimson_nylium, Same);
    E(crimson_planks, Same);
    E(crimson_roots, Same);
    E(crying_obsidian, Same);
    E(cut_copper, Same);
    E(deepslate_bricks, Same);
    E(deepslate_coal_ore, Same);
    E(deepslate_copper_ore, Same);
    E(deepslate_diamond_ore, Same);
    E(deepslate_emerald_ore, Same);
    E(deepslate_gold_ore, Same);
    E(deepslate_iron_ore, Same);
    E(deepslate_lapis_ore, Same);
    E(deepslate_tiles, Same);
    E(diamond_block, Same);
    E(diamond_ore, Same);
    E(dragon_egg, Same);
    E(dried_kelp_block, Same);
    E(dripstone_block, Same);
    E(emerald_block, Same);
    E(emerald_ore, Same);
    E(enchanting_table, Same);
    E(end_gateway, Same);
    E(end_portal, Same);
    E(end_stone, Same);
    E(exposed_copper, Same);
    E(exposed_cut_copper, Same);
    E(fletching_table, Same);
    E(flowering_azalea, Same);
    E(gilded_blackstone, Same);
    E(glass, Same);
    E(glowstone, Same);
    E(gold_block, Same);
    E(gold_ore, Same);
    E(gravel, Same);
    E(honeycomb_block, Same);
    E(honey_block, Same);
    E(ice, Same);
    E(iron_block, Same);
    E(iron_ore, Same);
    E(lapis_block, Same);
    E(lapis_ore, Same);
    E(lodestone, Same);
    E(mossy_cobblestone, Same);
    E(moss_block, Same);
    E(moss_carpet, Same);
    E(netherite_block, Same);
    E(netherrack, Same);
    E(nether_gold_ore, Same);
    E(nether_sprouts, Same);
    E(nether_wart_block, Same);
    E(obsidian, Same);
    E(oxidized_copper, Same);
    E(oxidized_cut_copper, Same);
    E(packed_ice, Same);
    E(polished_blackstone, Same);
    E(polished_blackstone_bricks, Same);
    E(polished_deepslate, Same);
    E(powder_snow, Same);
    E(quartz_bricks, Same);
    E(raw_copper_block, Same);
    E(raw_gold_block, Same);
    E(raw_iron_block, Same);
    E(redstone_block, Same);
    E(red_mushroom, Same);
    E(shroomlight, Same);
    E(smithing_table, Same);
    E(smooth_basalt, Same);
    E(smooth_stone, Same);
    E(soul_fire, Same);
    E(soul_sand, Same);
    E(soul_soil, Same);
    E(spawner, Same);
    E(spore_blossom, Same);
    E(tinted_glass, Same);
    E(tuff, Same);
    E(warped_fungus, Same);
    E(warped_nylium, Same);
    E(warped_planks, Same);
    E(warped_roots, Same);
    E(warped_wart_block, Same);
    E(waxed_cut_copper, Same);
    E(waxed_exposed_copper, Same);
    E(waxed_exposed_cut_copper, Same);
    E(waxed_oxidized_copper, Same);
    E(waxed_oxidized_cut_copper, Same);
    E(waxed_weathered_copper, Same);
    E(waxed_weathered_cut_copper, Same);
    E(weathered_copper, Same);
    E(weathered_cut_copper, Same);
    E(wither_rose, Same);
#pragma endregion

    E(acacia_button, Button);
    E(wooden_button, Button);
    E(birch_button, Button);
    E(jungle_button, Button);
    E(dark_oak_button, Button);
    E(stone_button, Button);
    E(crimson_button, Button);
    E(warped_button, Button);
    E(polished_blackstone_button, Button);

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

    E(acacia_fence_gate, FenceGate);
    E(birch_fence_gate, FenceGate);

    E(leaves, Leaves);
    E(leaves2, Leaves2);
    E(log2, Log2);
    E(log, Log);
    E(planks, Planks);

    E(acacia_pressure_plate, PressurePlate);
    E(birch_pressure_plate, PressurePlate);

    E(sapling, Sapling);

    E(acacia_standing_sign, StandingSign);
    E(birch_standing_sign, StandingSign);

    E(wooden_slab, WoodenSlab);
    E(double_wooden_slab, DoubleWoodenSlab);

    E(acacia_stairs, Stairs);
    E(andesite_stairs, Stairs);
    E(birch_stairs, Stairs);
    E(blackstone_stairs, Stairs);
    E(brick_stairs, Stairs);
    E(cobbled_deepslate_stairs, Stairs);

    E(acacia_trapdoor, Trapdoor);
    E(birch_trapdoor, Trapdoor);

    E(acacia_wall_sign, BlockWithFacingAFromFacingDirection);
    E(birch_wall_sign, BlockWithFacingAFromFacingDirection);

    E(wood, Wood);
    E(activator_rail, RailCanBePowered);
    E(red_flower, RedFlower);
    E(amethyst_cluster, BlockWithFacingAFromFacingDirection);
    E(stone, Stone);
    E(stone_slab3, StoneSlab3);
    E(double_stone_slab3, DoubleStoneSlab3);

    E(cobblestone_wall, WallWithBlockType);
    E(blackstone_wall, BlockWithWallProperties);

    E(anvil, Anvil);
    E(melon_stem, MelonStem);
    E(pumpkin_stem, PumpkinStem);
    E(azalea_leaves, AzaleaLeaves);
    E(bamboo, Bamboo);
    E(bamboo_sapling, Same);
    E(barrel, Barrel);
    E(basalt, BlockWithAxisFromPillarAxis);
    E(beehive, Beehive);
    E(beetroot, Beetroot);
    E(bee_nest, Beehive);
    E(bell, Bell);
    E(big_dripleaf, BigDripleaf);
    E(blackstone_slab, BlockWithTypeFromTopSlotBit);
    E(blackstone_double_slab, BlackstoneDoubleSlab);
    E(standing_banner, BlockWithRotationFromGroundSignDirection);
    E(bed, Bed);

    E(black_candle, Candle);
    E(blue_candle, Candle);
    E(brown_candle, Candle);
    E(candle, Candle);

    E(black_candle_cake, CandleCake);
    E(blue_candle_cake, CandleCake);
    E(brown_candle_cake, CandleCake);
    E(candle_cake, CandleCake);

    E(carpet, Carpet);
    E(concrete, Concrete);
    E(concretePowder, ConcretePowder);

    E(black_glazed_terracotta, BlockWithFacingAFromFacingDirection);
    E(blue_glazed_terracotta, BlockWithFacingAFromFacingDirection);
    E(brown_glazed_terracotta, BlockWithFacingAFromFacingDirection);

    E(shulker_box, ShulkerBox);
    E(stained_glass, StainedGlass);
    E(stained_glass_pane, StainedGlassPane);
    E(stained_hardened_clay, StainedHardenedClay);
    E(wall_banner, BlockWithFacingAFromFacingDirection);
    E(wool, Wool);
    E(blast_furnace, BlockWithFacingAFromFacingDirection);
    E(bone_block, BlockWithAxisFromPillarAxis);
    E(coral, Coral);
    E(coral_block, CoralBlock);
    E(coral_fan_hang, CoralFanHang);
    E(coral_fan_hang2, CoralFanHang);
    E(coral_fan, CoralFan);
    E(brewing_stand, BrewingStand);
    E(brick_block, BrickBlock);
    E(stone_slab, StoneSlab);
    E(double_stone_slab, DoubleStoneSlab);
    E(brown_mushroom_block, BrownMushroomBlock);
    E(bubble_column, BubbleColumn);
    E(cactus, BlockWithAge);
    E(cake, Cake);
    E(campfire, Campfire);
    E(carrots, BlockWithAgeFromGrowth);
    E(carved_pumpkin, BlockWithFacingAFromDirection);
    E(cauldron, Cauldron);
    E(cave_vines, CaveVines);
    E(cave_vines_with_berries, CaveVines);
    E(cave_vines_head_with_berries, CaveVines);
    E(cave_vines_body_with_berries, CaveVinesBody);
    E(chain, BlockWithAxisFromPillarAxis);
    E(chain_command_block, CommandBlock);
    E(command_block, CommandBlock);
    E(chest, BlockWithFacingAFromFacingDirection);
    E(quartz_block, QuartzBlock);
    E(red_sandstone, RedSandstone);
    E(sandstone, Sandstone);
    E(stonebrick, Stonebrick);
    E(chorus_flower, BlockWithAge);
    E(chorus_plant, Same);
    E(dirt, Dirt);
    E(cobbled_deepslate_slab, BlockWithTypeFromTopSlotBit);
    E(cobbled_deepslate_double_slab, CobbledDeepslateDoubleSlab);
    E(cobbled_deepslate_wall, BlockWithWallProperties);
    E(stone_stairs, StoneStairs);
    E(web, Web);
    E(cocoa, Cocoa);

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
