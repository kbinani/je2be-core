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
    map<string, string> props;
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

    map<string, string> props;
    props["hinge"] = doorHingeBit ? "right" : "left";
    props["half"] = upperBlockBit ? "upper" : "lower";
    props["open"] = Bool(openBit);
    props["facing"] = FacingCFromDirection(direction);
    props["powered"] = "false";
    return make_shared<Block const>(b.fName, props);
  }

#pragma endregion

#pragma region Converter : F

  static Return Fence(Input const &b) {
    using namespace std;
    using namespace mcfile::je;
    auto const &s = *b.fStates;
    auto woodType = s.string("wood_type", "oak");
    return make_shared<Block const>("minecraft:" + woodType + "_fence");
  }

  static Return FenceGate(Input const &b) {
    using namespace std;
    using namespace mcfile::je;
    auto const &s = *b.fStates;
    auto direction = s.int32("direction", 0);
    auto inWall = s.boolean("in_wall_bit", false);
    auto open = s.boolean("open_bit", false);
    map<string, string> props;
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
    map<string, string> props;
    props["persistent"] = Bool(persistent);
    return make_shared<Block const>("minecraft:" + newLeafType + "_leaves", props);
  }

  static Return Log2(Input const &b) {
    using namespace std;
    using namespace mcfile::je;
    auto const &s = *b.fStates;
    auto newLogType = s.string("new_log_type", "acacia"); //TODO: acacia?
    auto pillarAxis = s.string("pillar_axis", "y");
    map<string, string> props;
    props["axis"] = pillarAxis;
    return make_shared<Block const>("minecraft:" + newLogType + "_log", props);
  }

#pragma endregion

#pragma region : P

  static Return Planks(Input const &b) {
    using namespace std;
    using namespace mcfile::je;
    auto const &s = *b.fStates;
    auto woodType = s.string("wood_type", "acacia"); //TODO: acacia?
    return make_shared<Block const>("minecraft:" + woodType + "_planks");
  }

#pragma endregion

  /*
  static Return ***(Input const &b) {
    using namespace std;
    using namespace mcfile::je;
    auto const &s = *b.fStates;
  }
*/

#pragma region Properties

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

#pragma endregion

#pragma region Utilities

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

#undef E

    return table;
  }

  static std::string Bool(bool b) {
    return b ? "true" : "false";
  }
#pragma endregion
};

} // namespace je2be::toje
