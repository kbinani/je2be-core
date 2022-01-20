#pragma once

namespace je2be::toje {

class BlockData {
  using Input = mcfile::be::Block;
  using Return = std::shared_ptr<mcfile::je::Block const>;
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

#pragma region Converters : B

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

#pragma region Converter : D

  static Return Door(Input const &b) {
    using namespace std;
    using namespace mcfile::je;

    auto const &s = *b.fStates;

    auto direction = s.int32("direction", 0);
    auto doorHingeBit = s.boolean("door_hinge_bit", false);
    auto openBit = s.boolean("open_bit", false);
    auto upperBlockBit = s.boolean("upper_block_bit", false);

    auto props = Empty();
    props["hinge"] = doorHingeBit ? "right" : "left";
    props["half"] = upperBlockBit ? "upper" : "lower";
    props["open"] = Bool(openBit);
    props["facing"] = FacingCFromDirection(direction);
    props["powered"] = "false";
    return make_shared<Block const>(b.fName, props);
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
    auto direction = s.int32("direction", 0);
    auto inWall = s.boolean("in_wall_bit", false);
    auto open = s.boolean("open_bit", false);
    auto props = Empty();
    props["in_wall"] = Bool(inWall);
    props["open"] = Bool(open);
    props["powered"] = Bool(false);
    props["facing"] = FacingAFromDirection(direction);
    return make_shared<Block const>(b.fName, props);
  }

#pragma endregion

#pragma region Converter : L

  static Return Leaves2(Input const &b) {
    using namespace std;
    using namespace mcfile::je;
    auto const &s = *b.fStates;
    auto newLeafType = s.string("new_leaf_type", "acacia"); //TODO: acacia?
    auto persistent = s.boolean("persistent_bit", false);
    auto props = Empty();
    props["persistent"] = Bool(persistent);
    return make_shared<Block const>(Ns() + newLeafType + "_leaves", props);
  }

  static Return Log2(Input const &b) {
    using namespace std;
    using namespace mcfile::je;
    auto const &s = *b.fStates;
    auto newLogType = s.string("new_log_type", "acacia"); //TODO: acacia?
    auto pillarAxis = s.string("pillar_axis", "y");
    auto props = Empty();
    props["axis"] = pillarAxis;
    return make_shared<Block const>(Ns() + newLogType + "_log", props);
  }

#pragma endregion

#pragma region Converter : P

  static Return PressurePlate(Input const &b) {
    using namespace std;
    using namespace mcfile::je;
    auto const &s = *b.fStates;
    auto redstoneSignal = s.int32("redstone_signal", 0);
    auto props = Empty();
    props["powered"] = Bool(redstoneSignal > 0);
    return make_shared<Block const>(b.fName, props);
  }

  static Return Planks(Input const &b) {
    using namespace std;
    using namespace mcfile::je;
    auto const &s = *b.fStates;
    auto woodType = s.string("wood_type", "acacia"); //TODO: acacia?
    return make_shared<Block const>(Ns() + woodType + "_planks");
  }

#pragma endregion

#pragma region Converter : S

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

  static Return Stairs(Input const &b) {
    using namespace std;
    using namespace mcfile::je;
    auto const &s = *b.fStates;
    auto upsideDown = s.boolean("upside_down_bit", false);
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
    auto props = Submergible();
    props["facing"] = facing;
    props["half"] = upsideDown ? "top" : "bottom";
    return make_shared<Block const>(b.fName, props);
  }

  static Return StandingSign(Input const &b) {
    using namespace std;
    using namespace mcfile::je;
    auto const &s = *b.fStates;
    auto type = VariantFromName(b.fName, "_standing_sign");
    auto groundSignRotation = s.int32("ground_sign_direction", 0);
    auto props = Submergible();
    props["rotation"] = to_string(groundSignRotation);
    return make_shared<Block const>(Ns() + type + "_sign");
  }

#pragma endregion

#pragma region Converters : T

  static Return Trapdoor(Input const &b) {
    using namespace std;
    using namespace mcfile::je;
    auto const &s = *b.fStates;
    auto direction = s.int32("direction", 0);
    auto open = s.boolean("open_bit", false);
    auto upsideDown = s.boolean("upside_down_bit", false);
    auto props = Submergible();
    props["facing"] = FacingBFromDirection(direction);
    props["half"] = upsideDown ? "top" : "bottom";
    props["open"] = Bool(open);
    props["powered"] = "false";
    return make_shared<Block const>(b.fName, props);
  }

#pragma endregion

#pragma region Converters : W

  static Return WallSign(Input const &b) {
    using namespace std;
    using namespace mcfile::je;
    auto const &s = *b.fStates;
    auto facingDirection = s.int32("facing_direction", 0);
    auto props = Submergible();
    props["facing"] = FacingDirectionFromFacingA(facingDirection);
    return make_shared<Block const>(b.fName, props);
  }

  static Return Wood(Input const &b) {
    using namespace std;
    using namespace mcfile::je;
    auto const &s = *b.fStates;
    auto pillarAxis = s.string("pillar_axis", "y");
    auto stripped = s.boolean("stripped_bit", false);
    auto woodType = s.string("wood_type", "oak");
    auto name = stripped ? "stripped_" + woodType + "_wood" : woodType + "_wood";
    auto props = Empty();
    props["axis"] = pillarAxis;
    return make_shared<Block const>(Ns() + name, props);
  }

  static Return WoodenSlab(Input const &b) {
    using namespace std;
    using namespace mcfile::je;
    auto const &s = *b.fStates;
    auto topSlot = s.boolean("top_slot_bit", false);
    auto woodType = s.string("wood_type", "acacia");
    auto props = Submergible();
    props["type"] = topSlot ? "top" : "bottom";
    return make_shared<Block const>(Ns() + woodType + "_slab", props);
  }

#pragma endregion

  /*
  static Return _(Input const &b) {
    using namespace std;
    using namespace mcfile::je;
    auto const &s = *b.fStates;
    return make_shared<Block const>();
  }
*/

#pragma region Properties

  static std::string FacingAFromDirection(int32_t direction) {
    switch (direction) {
    case 2:
      return "north";
    case 3:
      return "east";
    case 1:
      return "west";
    case 0:
    default:
      return "south";
    }
  }

  static std::string FacingBFromDirection(int32_t direction) {
    switch (direction) {
    case 2:
      return "south";
    case 1:
      return "west";
    case 3:
      return "north";
    case 0:
    default:
      return "east";
    }
  }

  static std::string FacingCFromDirection(int32_t direction) {
    switch (direction) {
    case 1:
      return "south";
    case 2:
      return "west";
    case 3:
      return "north";
    case 0:
    default:
      return "east";
    }
  }

  static std::string FacingDirectionFromFacingA(int32_t facingDirection) {
    // 102534
    switch (facingDirection) {
    case 5:
      return "east";
    case 3:
      return "south";
    case 4:
      return "west";
    case 2:
      return "north";
    case 1:
      return "up";
    case 0:
    default:
      return "down";
    }
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
    E(leaves2, Leaves2);
    E(log2, Log2);
    E(planks, Planks);
    E(acacia_pressure_plate, PressurePlate);
    E(sapling, Sapling);
    E(acacia_standing_sign, StandingSign);
    E(wooden_slab, WoodenSlab);
    E(double_wooden_slab, DoubleWoodenSlab);
    E(acacia_stairs, Stairs);
    E(acacia_trapdoor, Trapdoor);
    E(acacia_wall_sign, WallSign);
    E(wood, Wood);

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

  static inline std::map<std::string, std::string> Submergible() {
    static std::map<std::string, std::string> const sP({{"waterlogged", "false"}});
    return sP;
  }

  // Get "acacia" from "minecraft:acacia_pressure_plate" when suffix is "_pressure_plate"
  static inline std::string VariantFromName(std::string const &name, std::string const &suffix) {
    auto idx = name.find(suffix);
    return name.substr(0, idx).substr(Ns().size());
  }

#pragma endregion
};

} // namespace je2be::toje
