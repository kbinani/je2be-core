#pragma once

namespace je2be::toje {

class BlockData {
  using Input = mcfile::be::Block;
  using Return = std::shared_ptr<mcfile::je::Block const>;
  using States = mcfile::nbt::CompoundTag;
  using Props = std::map<std::string, std::string>;
  using Converter = std::function<Return(Input const &)>;

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
      return found->second(b);
    }
  }

  static std::shared_ptr<mcfile::je::Block const> Identity(mcfile::be::Block const &b) {
    return std::make_shared<mcfile::je::Block const>(b.fName);
  }

private:
  BlockData() = delete;

#pragma region Converters : A

  static Return Anvil(Input const &b) {
    using namespace std;
    using namespace mcfile::je;
    auto const &s = *b.fStates;
    auto damage = s.string("damage", "undamaged");
    string name = "anvil";
    if (damage == "slightly_damaged") {
      name = "chipped_anvil";
    } else if (damage == "very_damaged") {
      name = "damaged_anvil";
    } else {
      // undamaged
      name = "anvil";
    }
    auto props = Empty();
    FacingAFromDirection(s, props);
    return make_shared<Block const>(Ns() + name, props);
  }

  static Return AzaleaLeaves(Input const &b) {
    using namespace std;
    using namespace mcfile::je;
    auto const &s = *b.fStates;
    auto props = Empty();
    PersistentFromPersistentBit(s, props);
    return make_shared<Block const>(b.fName, props);
  }

#pragma endregion

#pragma region Converters : B

  static Return Bamboo(Input const &b) {
    auto const &s = *b.fStates;
    auto p = Empty();

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
    p["age"] = std::to_string(age);

    auto ageBit = s.boolean("age_bit", false);
    p["stage"] = ageBit ? "1" : "0";
    return std::make_shared<mcfile::je::Block const>(b.fName, p);
  }

  static Return Barrel(Input const &b) {
    auto const &s = *b.fStates;
    auto p = Empty();
    OpenFromOpenBit(s, p);
    FacingAFromFacingDirection(s, p);
    return std::make_shared<mcfile::je::Block const>(b.fName, p);
  }

  static Return Beehive(Input const &b) {
    auto const &s = *b.fStates;
    auto p = Empty();
    FacingAFromDirection(s, p);
    auto honeyLevel = s.int32("honey_level", 0);
    p["honey_level"] = std::to_string(honeyLevel);
    return std::make_shared<mcfile::je::Block const>(b.fName, p);
  }

  static Return Beetroot(Input const &b) {
    auto const &s = *b.fStates;
    auto p = Empty();
    AgeFromGrowth(s, p);
    return std::make_shared<mcfile::je::Block const>(Ns() + "beetroots", p);
  }

  static Return Bed(Input const &b) {
    auto const &s = *b.fStates;
    auto p = Empty();
    FacingAFromDirection(s, p);
    auto headPiece = s.boolean("head_piece_bit", false);
    auto occupied = s.boolean("occupied_bit", false);
    p["part"] = headPiece ? "head" : "foot";
    p["occupied"] = Bool(occupied);
    return std::make_shared<mcfile::je::Block const>(Ns() + "white_bed", p);
  }

  static Return Bell(Input const &b) {
    auto const &s = *b.fStates;
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
    auto p = Empty();
    p["facing"] = facing;
    p["attachment"] = a;
    p["powered"] = Bool(toggle);
    return std::make_shared<mcfile::je::Block const>(b.fName, p);
  }

  static Return BigDripleaf(Input const &b) {
    auto const &s = *b.fStates;
    auto head = s.boolean("big_dripleaf_head", false);
    auto p = Empty();
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
    return std::make_shared<mcfile::je::Block const>(Ns() + name, p);
  }

  static Return BlackstoneDoubleSlab(Input const &b) {
    auto const &s = *b.fStates;
    auto p = Empty();
    p["type"] = "double";
    return std::make_shared<mcfile::je::Block const>(Ns() + "blackstone_slab", p);
  }

  static Return BlockWithAgeFromGrowth(Input const &b) {
    auto const &s = *b.fStates;
    auto p = Empty();
    AgeFromGrowth(s, p);
    return std::make_shared<mcfile::je::Block const>(b.fName, p);
  }

  static Return BlockWithAxisFromPillarAxis(Input const &b) {
    auto const &s = *b.fStates;
    auto p = Empty();
    AxisFromPillarAxis(s, p);
    return std::make_shared<mcfile::je::Block const>(b.fName, p);
  }

  static Return BlockWithFacingAFromFacingDirection(Input const &b) {
    using namespace std;
    using namespace mcfile::je;
    auto const &s = *b.fStates;
    auto props = Empty();
    FacingAFromFacingDirection(s, props);
    return make_shared<Block const>(b.fName, props);
  }

  static Return BlockWithRotationFromGroundSignDirection(Input const &b) {
    auto const &s = *b.fStates;
    auto p = Empty();
    RotationFromGroundSignDirection(s, p);
    return std::make_shared<mcfile::je::Block const>(b.fName, p);
  }

  static Return BlockWithTypeFromTopSlotBit(Input const &b) {
    using namespace std;
    using namespace mcfile::je;
    auto const &s = *b.fStates;
    auto props = Empty();
    TypeFromTopSlotBit(s, props);
    return make_shared<Block const>(b.fName, props);
  }

  static Return BlockWithWallProperties(Input const &b) {
    auto const &s = *b.fStates;
    auto p = Empty();
    WallProperties(s, p);
    return std::make_shared<mcfile::je::Block const>(b.fName, p);
  }

  static Return Button(Input const &b) {
    using namespace std;
    auto const &s = *b.fStates;
    auto buttonPressedBit = s.boolean("button_pressed_bit", false);
    auto facingDirection = s.int32("facing_direction", 0);
    string facing = "south";
    string face = "ceiling";
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
    auto props = Empty();
    props["face"] = face;
    props["facing"] = facing;
    props["powered"] = Bool(buttonPressedBit);
    return make_shared<mcfile::je::Block const>(b.fName, props);
  }

#pragma endregion

#pragma region Converters : C
  static Return Candle(Input const &b) {
    auto const &s = *b.fStates;
    auto candles = s.int32("candles", 0);
    auto p = Empty();
    p["candles"] = std::to_string(candles + 1);
    Lit(s, p);
    return std::make_shared<mcfile::je::Block const>(b.fName, p);
  }

  static Return CandleCake(Input const &b) {
    auto const &s = *b.fStates;
    auto p = Empty();
    Lit(s, p);
    return std::make_shared<mcfile::je::Block const>(b.fName, p);
  }

  static Return Carpet(Input const &b) {
    auto const &s = *b.fStates;
    auto color = s.string("color", "white");
    return std::make_shared<mcfile::je::Block const>(Ns() + color + "_carpet");
  }

  static Return Concrete(Input const &b) {
    auto const &s = *b.fStates;
    auto color = s.string("color", "white");
    return std::make_shared<mcfile::je::Block const>(Ns() + color + "_concrete");
  }

  static Return ConcretePowder(Input const &b) {
    auto const &s = *b.fStates;
    auto color = s.string("color", "white");
    return std::make_shared<mcfile::je::Block const>(Ns() + color + "_concrete_powder");
  }

  static Return Coral(Input const &b) {
    auto const &s = *b.fStates;
    auto color = s.string("coral_color", "pink");
    auto dead = s.boolean("dead_bit", false);
    auto type = CoralTypeFromCoralColor(color);
    std::string name;
    if (dead) {
      name = "dead_";
    }
    name += type;
    return std::make_shared<mcfile::je::Block const>(Ns() + name);
  }

  static Return CoralBlock(Input const &b) {
    auto const &s = *b.fStates;
    auto color = s.string("coral_color", "pink");
    auto dead = s.boolean("dead_bit", false);
    auto type = CoralTypeFromCoralColor(color);
    std::string name;
    if (dead) {
      name = "dead_";
    }
    name += type + "_block";
    return std::make_shared<mcfile::je::Block const>(Ns() + name);
  }
#pragma endregion

#pragma region Converters : D

  static Return Door(Input const &b) {
    using namespace std;
    using namespace mcfile::je;

    auto const &s = *b.fStates;

    auto doorHingeBit = s.boolean("door_hinge_bit", false);
    auto upperBlockBit = s.boolean("upper_block_bit", false);

    auto props = Empty();
    OpenFromOpenBit(s, props);
    FacingCFromDirection(s, props);
    props["hinge"] = doorHingeBit ? "right" : "left";
    props["half"] = upperBlockBit ? "upper" : "lower";
    props["powered"] = "false";
    return make_shared<Block const>(b.fName, props);
  }

  static Return DoubleStoneSlab3(Input const &b) {
    using namespace std;
    using namespace mcfile::je;
    auto const &s = *b.fStates;
    auto stoneSlabType = s.string("stone_slab_type3", "andesite");
    auto props = Empty();
    props["type"] = "double";
    return make_shared<Block const>(Ns() + stoneSlabType + "_slab", props);
  }

  static Return DoubleWoodenSlab(Input const &b) {
    using namespace std;
    using namespace mcfile::je;
    auto const &s = *b.fStates;
    auto woodType = s.string("wood_type", "oak");
    auto props = Empty();
    props["type"] = "double";
    props["waterlogged"] = "false";
    return make_shared<Block const>(Ns() + woodType + "_slab", props);
  }

#pragma endregion

#pragma region Converter : F

  static Return Fence(Input const &b) {
    using namespace std;
    using namespace mcfile::je;
    auto const &s = *b.fStates;
    auto woodType = s.string("wood_type", "oak");
    return make_shared<Block const>(Ns() + woodType + "_fence");
  }

  static Return FenceGate(Input const &b) {
    using namespace std;
    using namespace mcfile::je;
    auto const &s = *b.fStates;
    auto inWall = s.boolean("in_wall_bit", false);
    auto props = Empty();
    props["in_wall"] = Bool(inWall);
    props["powered"] = Bool(false);
    FacingAFromDirection(s, props);
    OpenFromOpenBit(s, props);
    return make_shared<Block const>(b.fName, props);
  }

#pragma endregion

#pragma region Converters : L

  static Return Leaves(Input const &b) {
    auto const &s = *b.fStates;
    auto leafType = s.string("old_leaf_type", "oak");
    auto props = Empty();
    PersistentFromPersistentBit(s, props);
    return std::make_shared<mcfile::je::Block const>(Ns() + leafType + "_leaves", props);
  }

  static Return Leaves2(Input const &b) {
    auto const &s = *b.fStates;
    auto newLeafType = s.string("new_leaf_type", "acacia"); //TODO: acacia?
    auto props = Empty();
    PersistentFromPersistentBit(s, props);
    return std::make_shared<mcfile::je::Block const>(Ns() + newLeafType + "_leaves", props);
  }

  static Return Log(Input const &b) {
    auto const &s = *b.fStates;
    auto logType = s.string("old_log_type", "oak");
    auto props = Empty();
    AxisFromPillarAxis(s, props);
    return std::make_shared<mcfile::je::Block const>(Ns() + logType + "_log", props);
  }

  static Return Log2(Input const &b) {
    auto const &s = *b.fStates;
    auto logType = s.string("new_log_type", "acacia"); //TODO: acacia?
    auto props = Empty();
    AxisFromPillarAxis(s, props);
    return std::make_shared<mcfile::je::Block const>(Ns() + logType + "_log", props);
  }

#pragma endregion

#pragma region Converters : M

  static Return MelonStem(Input const &b) {
    using namespace std;
    using namespace mcfile::je;
    auto const &s = *b.fStates;
    auto growth = s.int32("growth", 0);
    auto props = Empty();
    string name;
    if (growth >= 7) {
      name = "attached_melon_stem";
    } else {
      name = "melon_stem";
      props["age"] = to_string(growth);
    }
    FacingAFromDirection(s, props, "facing_direction");
    return make_shared<Block const>(Ns() + name, props);
  }

#pragma endregion

#pragma region Converters : P

  static Return Planks(Input const &b) {
    using namespace std;
    using namespace mcfile::je;
    auto const &s = *b.fStates;
    auto woodType = s.string("wood_type", "acacia"); //TODO: acacia?
    return make_shared<Block const>(Ns() + woodType + "_planks");
  }

  static Return PressurePlate(Input const &b) {
    using namespace std;
    using namespace mcfile::je;
    auto const &s = *b.fStates;
    auto redstoneSignal = s.int32("redstone_signal", 0);
    auto props = Empty();
    props["powered"] = Bool(redstoneSignal > 0);
    return make_shared<Block const>(b.fName, props);
  }

  static Return PumpkinStem(Input const &b) {
    using namespace std;
    using namespace mcfile::je;
    auto const &s = *b.fStates;
    auto growth = s.int32("growth", 0);
    auto props = Empty();
    string name;
    if (growth >= 7) {
      name = "attached_pumpkin_stem";
    } else {
      name = "pumpkin_stem";
      props["age"] = to_string(growth);
    }
    FacingAFromDirection(s, props, "facing_direction");
    return make_shared<Block const>(Ns() + name, props);
  }

#pragma endregion

#pragma region Converters : R

  static Return RailCanBePowered(Input const &b) {
    using namespace std;
    using namespace mcfile::je;
    auto const &s = *b.fStates;
    auto railData = s.boolean("rail_data_bit", false);
    auto railDirection = s.int32("rail_direction", 0);
    auto props = Empty();
    props["powered"] = Bool(railData);
    props["shape"] = ShapeFromRailDirection(railDirection);
    return make_shared<Block const>(b.fName, props);
  }

  static Return RedFlower(Input const &b) {
    using namespace std;
    using namespace mcfile::je;
    auto const &s = *b.fStates;
    auto flowerType = s.string("flower_type", "poppy");
    string name = flowerType;
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
    return make_shared<Block const>(Ns() + name);
  }

#pragma endregion

#pragma region Converters : S

  static Return Sapling(Input const &b) {
    using namespace std;
    using namespace mcfile::je;
    auto const &s = *b.fStates;
    auto age = s.byte("age_bit", 0);
    auto saplingType = s.string("sapling_type", "acacia");
    auto props = Empty();
    props["stage"] = to_string(age);
    return make_shared<Block const>(Ns() + saplingType + "_sapling");
  }

  static Return ShulkerBox(Input const &b) {
    auto const &s = *b.fStates;
    auto color = s.string("color", "white");
    return std::make_shared<mcfile::je::Block const>(Ns() + color + "_shulker_box");
  }

  static Return Stairs(Input const &b) {
    using namespace std;
    using namespace mcfile::je;
    auto const &s = *b.fStates;
    auto weirdoDirection = s.int32("weirdo_direction", 0);
    string facing = "east";
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
    auto props = Empty();
    props["facing"] = facing;
    HalfFromUpsideDownBit(s, props);
    return make_shared<Block const>(b.fName, props);
  }

  static Return StainedGlass(Input const &b) {
    auto const &s = *b.fStates;
    auto color = s.string("color", "white");
    return std::make_shared<mcfile::je::Block const>(Ns() + color + "_stained_glass");
  }

  static Return StainedGlassPane(Input const &b) {
    auto const &s = *b.fStates;
    auto color = s.string("color", "white");
    return std::make_shared<mcfile::je::Block const>(Ns() + color + "_stained_glass_pane");
  }

  static Return StainedHardenedClay(Input const &b) {
    auto const &s = *b.fStates;
    auto color = s.string("color", "white");
    return std::make_shared<mcfile::je::Block const>(Ns() + color + "_terracotta");
  }

  static Return StandingSign(Input const &b) {
    auto const &s = *b.fStates;
    auto type = VariantFromName(b.fName, "_standing_sign");
    auto groundSignRotation = s.int32("ground_sign_direction", 0);
    auto p = Empty();
    RotationFromGroundSignDirection(s, p);
    return std::make_shared<mcfile::je::Block const>(Ns() + type + "_sign", p);
  }

  static Return Stone(Input const &b) {
    using namespace std;
    using namespace mcfile::je;
    auto const &s = *b.fStates;
    auto stoneType = s.string("stone_type");
    string name = stoneType ? *stoneType : "stone";
    return make_shared<Block const>(Ns() + name);
  }

  static Return StoneSlab3(Input const &b) {
    auto const &s = *b.fStates;
    auto stoneSlabType = s.string("stone_slab_type3", "andesite");
    auto p = Empty();
    TypeFromTopSlotBit(s, p);
    return std::make_shared<mcfile::je::Block const>(Ns() + stoneSlabType + "_slab", p);
  }

#pragma endregion

#pragma region Converters : T

  static Return Trapdoor(Input const &b) {
    using namespace std;
    using namespace mcfile::je;
    auto const &s = *b.fStates;
    auto props = Empty();
    FacingBFromDirection(s, props);
    OpenFromOpenBit(s, props);
    HalfFromUpsideDownBit(s, props);
    props["powered"] = "false";
    return make_shared<Block const>(b.fName, props);
  }

#pragma endregion

#pragma region Converters : W

  static Return WallWithBlockType(Input const &b) {
    auto const &s = *b.fStates;
    auto blockType = s.string("wall_block_type", "andesite");
    auto p = Empty();
    WallProperties(s, p);
    return std::make_shared<mcfile::je::Block const>(Ns() + blockType + "_wall", p);
  }

  static Return Wood(Input const &b) {
    using namespace std;
    using namespace mcfile::je;
    auto const &s = *b.fStates;
    auto stripped = s.boolean("stripped_bit", false);
    auto woodType = s.string("wood_type", "oak");
    auto name = stripped ? "stripped_" + woodType + "_wood" : woodType + "_wood";
    auto props = Empty();
    AxisFromPillarAxis(s, props);
    return make_shared<Block const>(Ns() + name, props);
  }

  static Return WoodenSlab(Input const &b) {
    using namespace std;
    using namespace mcfile::je;
    auto const &s = *b.fStates;
    auto woodType = s.string("wood_type", "acacia");
    auto p = Empty();
    TypeFromTopSlotBit(s, p);
    return make_shared<Block const>(Ns() + woodType + "_slab", p);
  }

  static Return Wool(Input const &b) {
    auto const &s = *b.fStates;
    auto color = s.string("color", "white");
    return std::make_shared<mcfile::je::Block const>(Ns() + color + "_wool");
  }

#pragma endregion

  /*
  static Return _(Input const &b) {
    auto const &s = *b.fStates;
    return std::make_shared<mcfile::je::Block const>();
  }
*/

#pragma region Properties

  static void AgeFromGrowth(States const &s, Props &p) {
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
    p["age"] = std::to_string(age);
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
    p["rotation"] = std::to_string(groundSignRotation);
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

    //TODO: remove these "identity" blocks
    E(air, Identity);
    E(amethyst_block, Identity);
    E(ancient_debris, Identity);
    E(azalea, Identity);
    E(barrier, Identity);
    E(beacon, Identity);
    E(bedrock, Identity);
    E(blackstone, Identity);
    E(blue_ice, Identity);
    E(bookshelf, Identity);
    E(brown_mushroom, Identity);
    E(budding_amethyst, Identity);
    E(calcite, Identity);
    E(cartography_table, Identity);
    E(chiseled_deepslate, Identity);
    E(chiseled_nether_bricks, Identity);
    E(chiseled_polished_blackstone, Identity);
    E(clay, Identity);
    E(coal_block, Identity);
    E(coal_ore, Identity);
    E(cobbled_deepslate, Identity);
    E(cobblestone, Identity);
    E(copper_block, Identity);
    E(copper_ore, Identity);
    E(cracked_deepslate_bricks, Identity);
    E(cracked_deepslate_tiles, Identity);
    E(cracked_nether_bricks, Identity);
    E(cracked_polished_blackstone_bricks, Identity);
    E(crafting_table, Identity);
    E(crimson_fungus, Identity);
    E(crimson_nylium, Identity);
    E(crimson_planks, Identity);
    E(crimson_roots, Identity);
    E(crying_obsidian, Identity);
    E(cut_copper, Identity);
    E(deepslate_bricks, Identity);
    E(deepslate_coal_ore, Identity);
    E(deepslate_copper_ore, Identity);
    E(deepslate_diamond_ore, Identity);
    E(deepslate_emerald_ore, Identity);
    E(deepslate_gold_ore, Identity);
    E(deepslate_iron_ore, Identity);
    E(deepslate_lapis_ore, Identity);
    E(deepslate_tiles, Identity);
    E(diamond_block, Identity);
    E(diamond_ore, Identity);
    E(dragon_egg, Identity);
    E(dried_kelp_block, Identity);
    E(dripstone_block, Identity);
    E(emerald_block, Identity);
    E(emerald_ore, Identity);
    E(enchanting_table, Identity);
    E(end_gateway, Identity);
    E(end_portal, Identity);
    E(end_stone, Identity);
    E(exposed_copper, Identity);
    E(exposed_cut_copper, Identity);
    E(fletching_table, Identity);
    E(flowering_azalea, Identity);
    E(gilded_blackstone, Identity);
    E(glass, Identity);
    E(glowstone, Identity);
    E(gold_block, Identity);
    E(gold_ore, Identity);
    E(gravel, Identity);
    E(honeycomb_block, Identity);
    E(honey_block, Identity);
    E(ice, Identity);
    E(iron_block, Identity);
    E(iron_ore, Identity);
    E(lapis_block, Identity);
    E(lapis_ore, Identity);
    E(lodestone, Identity);
    E(mossy_cobblestone, Identity);
    E(moss_block, Identity);
    E(moss_carpet, Identity);
    E(netherite_block, Identity);
    E(netherrack, Identity);
    E(nether_gold_ore, Identity);
    E(nether_sprouts, Identity);
    E(nether_wart_block, Identity);
    E(obsidian, Identity);
    E(oxidized_copper, Identity);
    E(oxidized_cut_copper, Identity);
    E(packed_ice, Identity);
    E(polished_blackstone, Identity);
    E(polished_blackstone_bricks, Identity);
    E(polished_deepslate, Identity);
    E(powder_snow, Identity);
    E(quartz_bricks, Identity);
    E(raw_copper_block, Identity);
    E(raw_gold_block, Identity);
    E(raw_iron_block, Identity);
    E(redstone_block, Identity);
    E(red_mushroom, Identity);
    E(shroomlight, Identity);
    E(smithing_table, Identity);
    E(smooth_basalt, Identity);
    E(smooth_stone, Identity);
    E(soul_fire, Identity);
    E(soul_sand, Identity);
    E(soul_soil, Identity);
    E(spawner, Identity);
    E(spore_blossom, Identity);
    E(tinted_glass, Identity);
    E(tuff, Identity);
    E(warped_fungus, Identity);
    E(warped_nylium, Identity);
    E(warped_planks, Identity);
    E(warped_roots, Identity);
    E(warped_wart_block, Identity);
    E(waxed_cut_copper, Identity);
    E(waxed_exposed_copper, Identity);
    E(waxed_exposed_cut_copper, Identity);
    E(waxed_oxidized_copper, Identity);
    E(waxed_oxidized_cut_copper, Identity);
    E(waxed_weathered_copper, Identity);
    E(waxed_weathered_cut_copper, Identity);
    E(weathered_copper, Identity);
    E(weathered_cut_copper, Identity);
    E(wither_rose, Identity);

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
    E(bamboo_sapling, Identity);
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
    E(black_candle_cake, CandleCake);
    E(blue_candle_cake, CandleCake);
    E(carpet, Carpet);
    E(concrete, Concrete);
    E(concretePowder, ConcretePowder);
    E(black_glazed_terracotta, BlockWithFacingAFromFacingDirection);
    E(blue_glazed_terracotta, BlockWithFacingAFromFacingDirection);
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

#undef E

    return table;
  }

#pragma region Utilities

  static std::string Bool(bool b) {
    return b ? "true" : "false";
  }

  static inline std::string Ns() {
    return "minecraft:";
  }

  static inline std::map<std::string, std::string> Empty() {
    static std::map<std::string, std::string> const sP;
    return sP;
  }

  // Get "acacia" from "minecraft:acacia_pressure_plate" when suffix is "_pressure_plate"
  static inline std::string VariantFromName(std::string const &name, std::string const &suffix) {
    assert(name.starts_with(Ns()) && name.ends_with(suffix));
    return name.substr(Ns().size()).substr(0, name.size() - Ns().size() - suffix.size());
  }

#pragma endregion
};

} // namespace je2be::toje
