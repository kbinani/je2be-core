#include "tobe/_block-data.hpp"

#include <je2be/strings.hpp>

#include "_closed-range.hpp"
#include "_namespace.hpp"
#include "_optional.hpp"
#include "_props.hpp"
#include "enums/_facing4.hpp"
#include "enums/_facing6.hpp"
#include "tobe/_versions.hpp"

namespace je2be::tobe {

class BlockData::Impl {
public:
  Impl() = delete;

  using Block = mcfile::je::Block;

  using PropertyType = std::shared_ptr<Tag>;

  using NamingFunction = std::function<std::string(Block const &)>;
  using PropertyPickupFunction = std::function<void(CompoundTagPtr const &, Block const &)>;

  using AnyConverter = std::function<CompoundTagPtr(Block const &, CompoundTagConstPtr const &)>;

  class Converter {
  public:
    template <class... Arg>
    Converter(NamingFunction name, Arg... args) : fName(name), fProperties(std::initializer_list<PropertyPickupFunction>{args...}) {}

    CompoundTagPtr operator()(Block const &block, CompoundTagConstPtr const &tile) const {
      using namespace std;
      string name = fName(block);
      auto tag = New(name, true);
      auto states = States();
      for (auto const &p : fProperties) {
        p(states, block);
      }
      tag->set("states", states);
      return tag;
    }

  private:
    NamingFunction const fName;
    std::vector<PropertyPickupFunction> const fProperties;
  };

  static CompoundTagPtr Identity(mcfile::je::Block const &block, CompoundTagConstPtr const &) {
    auto tag = New(block.fName, true);
    auto states = States();
    tag->set("states", states);
    return tag;
  }

  static NamingFunction Name(std::string const &name) {
    return [=](Block const &) { return "minecraft:" + name; };
  }

  static std::string Same(Block const &block) { return block.name(); }

  static NamingFunction SlabName(std::string const &name, std::string const &tail) {
    return [=](Block const &block) {
      auto t = block.property("type", "bottom");
      return t == "double" ? ("minecraft:double_" + name + "_slab" + tail) : ("minecraft:" + name + "_slab" + tail);
    };
  }

  static NamingFunction ChangeWhenDoubleType(std::string const &doubleName) {
    return [=](Block const &block) {
      auto t = block.property("type", "bottom");
      return t == "double" ? "minecraft:" + doubleName : block.name();
    };
  }

  static PropertyPickupFunction AddStringProperty(std::string const &name, std::string const &value) {
    return [=](CompoundTagPtr const &s, Block const &b) { s->set(name, String(value)); };
  }

  static PropertyPickupFunction AddBoolProperty(std::string const &name, bool v) {
    return [=](CompoundTagPtr const &s, Block const &b) { s->set(name, Bool(v)); };
  }

  static PropertyPickupFunction AddIntProperty(std::string const &name, i32 v) {
    return [=](CompoundTagPtr const &s, Block const &b) { s->set(name, Int(v)); };
  }

  static PropertyPickupFunction AddByteProperty(std::string const &name, i8 v) {
    return [=](CompoundTagPtr const &s, Block const &b) { s->set(name, Byte(v)); };
  }

  static void AxisToPillarAxis(CompoundTagPtr const &s, Block const &block) {
    auto v = block.property("axis", "y");
    s->set("pillar_axis", String(v));
  }

  static void PersistentAndDistanceToPersistentBitAndUpdateBit(CompoundTagPtr const &s, Block const &block) {
    auto persistent = block.property("persistent", "false") == "true";
    auto distance = Wrap(strings::ToI32(block.property("distance", "7")), 7);
    s->set("persistent_bit", Bool(persistent));
    // NOTE:
    //  Java: leaves decay when distance > 6
    //  Bedrock: leaves decay when distance > 4
    // Set update_bit to false for leaves with distance = 5, 6 not to decay as far as possible after conversion
    s->set("update_bit", Bool(distance > 6 && !persistent));
  }

  static void TypeToTopSlotBit(CompoundTagPtr const &s, Block const &block) {
    auto t = block.property("type", "bottom");
    s->set("top_slot_bit", Bool(t == "top"));
  }

  static PropertyPickupFunction AddStoneSlabType(std::string const &number, std::string const &type) {
    auto typeKey = number.empty() ? "stone_slab_type" : "stone_slab_type_" + number;
    return [=](CompoundTagPtr const &s, Block const &b) { s->set(typeKey, String(type)); };
  }

  static void UpperBlockBitToHalf(CompoundTagPtr const &s, Block const &block) {
    auto half = block.property("half", "lower");
    s->set("upper_block_bit", Bool(half == "upper"));
  }

  static void WaterloggedToDeadBit(CompoundTagPtr const &s, Block const &block) {
    auto waterlogged = block.property("waterlogged", "false");
    s->set("dead_bit", Bool(waterlogged == "false"));
  }

  static void PicklesToClusterCount(CompoundTagPtr const &s, Block const &block) {
    auto pickles = block.property("pickles", "1");
    auto cluster = Wrap(strings::ToI32(pickles), 1) - 1;
    s->set("cluster_count", Int(cluster));
  }

  static void HalfToSeagrassType(CompoundTagPtr const &s, Block const &block) {
    auto half = block.property("half", "lower");
    auto type = half == "lower" ? "double_bot" : "double_top";
    s->set("sea_grass_type", String(type));
  }

  static PropertyType Age(Block const &block) {
    auto age = Wrap(strings::ToI32(block.property("age", "0")), 0);
    return Int(age);
  }

  static PropertyPickupFunction Name(std::function<PropertyType(Block const &)> func, std::string const &name) {
    return [=](CompoundTagPtr const &s, Block const &block) { s->set(name, func(block)); };
  }

  static PropertyType Level(Block const &block) {
    auto level = Wrap(strings::ToI32(block.property("level", "0")), 0);
    return Int(level);
  }

  static void VineDirectionBits(CompoundTagPtr const &s, Block const &block) {
    auto east = block.property("east", "false") == "true";
    auto north = block.property("north", "false") == "true";
    auto south = block.property("south", "false") == "true";
    // auto up = block.property("up", "false") == "true";
    auto west = block.property("west", "false") == "true";
    i32 direction = 0;
    if (east) {
      direction |= 0x8;
    }
    if (north) {
      direction |= 0x4;
    }
    if (west) {
      direction |= 0x2;
    }
    if (south) {
      direction |= 0x1;
    }
    s->set("vine_direction_bits", Int(direction));
  }

  static void MultiFaceDirectionBits(CompoundTagPtr const &s, Block const &block) {
    auto down = block.property("down", "false") == "true";
    auto up = block.property("up", "false") == "true";
    auto east = block.property("east", "false") == "true";
    auto west = block.property("west", "false") == "true";
    auto north = block.property("north", "false") == "true";
    auto south = block.property("south", "false") == "true";
    i32 bits = 0;
    if (down) {
      bits |= 0x1;
    }
    if (up) {
      bits |= 0x2;
    }
    if (north) {
      bits |= 0x10;
    }
    if (south) {
      bits |= 0x4;
    }
    if (west) {
      bits |= 0x8;
    }
    if (east) {
      bits |= 0x20;
    }
    s->set("multi_face_direction_bits", Int(bits));
  }

  static Converter Subtype(std::string const &name, std::string const &subtypeTitle, std::string const &subtype) { return Converter(Name(name), AddStringProperty(subtypeTitle, subtype)); }

  static Converter Stone(std::string const &stoneType) { return Subtype("stone", "stone_type", stoneType); }

  static Converter Dirt(std::string const &dirtType) { return Subtype("dirt", "dirt_type", dirtType); }

  static Converter Log(std::string const &type) { return Converter(Name("log"), AddStringProperty("old_log_type", type), AxisToPillarAxis); }

  static Converter Log2(std::string const &type) { return Converter(Name("log2"), AddStringProperty("new_log_type", type), AxisToPillarAxis); }

  static Converter Wood(std::string const &type, bool stripped) { return Converter(Name("wood"), AxisToPillarAxis, AddStringProperty("wood_type", type), AddBoolProperty("stripped_bit", stripped)); }

  static Converter Leaves(std::string const &type) { return Converter(Name("leaves"), AddStringProperty("old_leaf_type", type), PersistentAndDistanceToPersistentBitAndUpdateBit); }

  static Converter Leaves2(std::string const &type) { return Converter(Name("leaves2"), AddStringProperty("new_leaf_type", type), PersistentAndDistanceToPersistentBitAndUpdateBit); }

  static Converter WoodenSlab(std::string const &type) { return Converter(SlabName("wooden", ""), TypeToTopSlotBit, AddStringProperty("wood_type", type)); }

  static Converter StoneSlab(std::string const &type) { return StoneSlabNumbered("", type); }

  static Converter StoneSlab2(std::string const &type) { return StoneSlabNumbered("2", type); }

  static Converter StoneSlab3(std::string const &type) { return StoneSlabNumbered("3", type); }

  static Converter StoneSlab4(std::string const &type) { return StoneSlabNumbered("4", type); }

  static Converter StoneSlabNumbered(std::string const &number, std::string const &type) { return Converter(SlabName("stone_block", number), TypeToTopSlotBit, AddStoneSlabType(number, type)); }

  static Converter Slab(std::string const &doubledName) { return Converter(ChangeWhenDoubleType(doubledName), TypeToTopSlotBit); }

  static Converter TallGrass(std::string const &type) { return Converter(Name("tallgrass"), AddStringProperty("tall_grass_type", type)); }

  static Converter DoublePlant(std::string const &type) { return Converter(Name("double_plant"), AddStringProperty("double_plant_type", type), UpperBlockBitToHalf); }

  static Converter Rename(std::string const &name) { return Converter(Name(name)); }

  static Converter SeaPickle() { return Converter(Name("sea_pickle"), WaterloggedToDeadBit, PicklesToClusterCount); }

  static Converter RedFlower(std::string const &type) { return Converter(Name("red_flower"), AddStringProperty("flower_type", type)); }

  static Converter Kelp(std::optional<i32> age = std::nullopt) {
    if (age) {
      return Converter(Name("kelp"), AddIntProperty("kelp_age", *age));
    } else {
      return Converter(Name("kelp"), Name(Age, "kelp_age"));
    }
  }

  static Converter Liquid(std::string const &type) { return Converter(Same, Name(Level, "liquid_depth")); }

  static Converter NetherVines(std::string const &type, std::optional<int> age = std::nullopt) {
    if (age) {
      return Converter(Name(type + "_vines"), AddIntProperty(type + "_vines_age", *age));
    } else {
      return Converter(Name(type + "_vines"), Name(Age, type + "_vines_age"));
    }
  }

  static CompoundTagPtr Null(Block const &block, CompoundTagConstPtr const &) {
    // not present in bedrock
    return New("air");
  }

  static void StairsDirectionFromFacing(CompoundTagPtr const &s, Block const &block) {
    auto facing = block.property("facing", "north");
    i32 direction = 0;
    if (facing == "east") {
      direction = 0;
    } else if (facing == "south") {
      direction = 2;
    } else if (facing == "north") {
      direction = 3;
    } else if (facing == "west") {
      direction = 1;
    }
    s->set("weirdo_direction", Int(direction));
  }

  static void HalfToUpsideDownBit(CompoundTagPtr const &s, Block const &block) {
    auto half = block.property("half", "bottom");
    s->set("upside_down_bit", Bool(half == "top"));
  }

  static Converter Stairs(std::optional<std::string> name = std::nullopt) {
    NamingFunction naming = name ? Name(*name) : Same;
    return Converter(naming, HalfToUpsideDownBit, StairsDirectionFromFacing);
  }

  static Converter Sponge(std::string const &type) { return Subtype("sponge", "sponge_type", type); }

  static Converter Wool(std::string const &color) { return Subtype("wool", "color", color); }

  static Converter RedSandstone(std::string const &type) { return Subtype("red_sandstone", "sand_stone_type", type); }

  static Converter Sandstone(std::string const &type) { return Subtype("sandstone", "sand_stone_type", type); }

  static Converter Sand(std::string const &type) { return Subtype("sand", "sand_type", type); }

  static Converter QuartzBlock(std::string const &type) { return Converter(Name("quartz_block"), AxisToPillarAxis, AddStringProperty("chisel_type", type)); }

  static Converter Planks(std::string const &type) { return Subtype("planks", "wood_type", type); }

  static PropertyType FacingA(Block const &block) {
    auto facing = block.property("facing");
    Facing4 f4 = Facing4FromJavaName(facing);
    return Int(BedrockDirectionFromFacing4(f4));
  }

  static PropertyType FacingB(Block const &block) {
    auto facing = block.property("facing");
    i32 direction = 0;
    if (facing == "south") {
      direction = 2;
    } else if (facing == "east") {
      direction = 0;
    } else if (facing == "west") {
      direction = 1;
    } else if (facing == "north") {
      direction = 3;
    }
    return Int(direction);
  }

  static PropertyType FacingC(Block const &b) {
    auto facing = b.property("facing");
    i32 direction = 0;
    if (facing == "north") {
      direction = 3;
    } else if (facing == "east") {
      direction = 0;
    } else if (facing == "south") {
      direction = 1;
    } else if (facing == "west") {
      direction = 2;
    }
    return Int(direction);
  }

  static PropertyType FacingD(Block const &b) {
    auto facing = b.property("facing");
    i32 direction = 0;
    if (facing == "north") {
      direction = 2;
    } else if (facing == "east") {
      direction = 1;
    } else if (facing == "south") {
      direction = 3;
    } else if (facing == "west") {
      direction = 0;
    }
    return Int(direction);
  }

  static void DirectionNorth0East1South2West3FromFacing(CompoundTagPtr const &s, Block const &block) {
    auto f4 = Facing4FromJavaName(block.property("facing"));
    i32 direction = 0;
    switch (f4) {
    case Facing4::North:
      direction = 0;
      break;
    case Facing4::East:
      direction = 1;
      break;
    case Facing4::South:
      direction = 2;
      break;
    case Facing4::West:
      direction = 3;
      break;
    }
    s->set("direction", Int(direction));
  }

  static void DirectionNorth2East3South0West1FromFacing(CompoundTagPtr const &s, Block const &block) {
    auto f4 = Facing4FromJavaName(block.property("facing"));
    s->set("direction", Int(North2East3South0West1FromFacing4(f4)));
  }

  static void DirectionFromFacingA(CompoundTagPtr const &s, Block const &block) {
    s->set("direction", FacingA(block));
  }

  static void DirectionFromFacingB(CompoundTagPtr const &s, Block const &block) {
    s->set("direction", FacingB(block));
  }

  static void DirectionFromFacingC(CompoundTagPtr const &s, Block const &block) {
    s->set("direction", FacingC(block));
  }

  static Converter LitPumpkin() { return Converter(Name("lit_pumpkin"), DirectionFromFacingA); }

  static Converter StainedGlass(std::string const &color) { return Subtype("stained_glass", "color", color); }

  static Converter Prismarine(std::string const &type) { return Subtype("prismarine", "prismarine_block_type", type); }

  static Converter Terracotta(std::string const &color) { return Subtype("stained_hardened_clay", "color", color); }

  static Converter Concrete(std::string const &color) { return Subtype("concrete", "color", color); }

  static Converter ConcretePowder(std::string const &color) { return Subtype("concrete_powder", "color", color); }

  static Converter CoralBlock(std::string const &color, bool dead) { return Converter(Name("coral_block"), AddStringProperty("coral_color", color), AddBoolProperty("dead_bit", dead)); }

  static void StageToAgeBit(CompoundTagPtr const &s, Block const &block) {
    auto stage = block.property("stage", "0");
    s->set("age_bit", Bool(stage == "1"));
  }

  static Converter Sapling(std::string const &type) { return Converter(Name("sapling"), AddStringProperty("sapling_type", type), StageToAgeBit); }

  static Converter StoneBrick(std::string const &type) { return Subtype("stonebrick", "stone_brick_type", type); }

  static void LayersToHeight(CompoundTagPtr const &s, Block const &block) {
    auto layers = Wrap(strings::ToI32(block.property("layers", "1")), 1);
    s->set("height", Int(layers - 1));
  }

  static CompoundTagPtr FlowerPot(Block const &b, CompoundTagConstPtr const &tile) {
    bool update = false;
    if (tile) {
      if (auto item = tile->int32("Item"); item) {
        update = true;
      }
    }
    auto d = New("flower_pot");
    auto s = States();
    AddBoolProperty("update_bit", update)(s, b);
    return AttachStates(d, s);
  }

  static CompoundTagPtr SnowLayer(Block const &b, CompoundTagConstPtr const &) {
    auto d = New("snow_layer");
    auto s = States();
    LayersToHeight(s, b);
    AddBoolProperty("covered_bit", false)(s, b);
    return AttachStates(d, s);
  }

  static CompoundTagPtr MangrovePropagule(Block const &b, CompoundTagConstPtr const &) {
    auto d = New("mangrove_propagule");
    auto s = States();
    int age = Wrap(strings::ToI32(b.property("age", "0")), 0);
    bool hanging = b.property("hanging") == "true";
    s->set("hanging", Bool(hanging));
    s->set("propagule_stage", Int(age));
    return AttachStates(d, s);
  }

  static CompoundTagPtr SculkCatalyst(Block const &b, CompoundTagConstPtr const &) {
    auto d = New("sculk_catalyst");
    auto s = States();
    auto bloom = b.property("bloom") == "true";
    s->set("bloom", Bool(bloom));
    return AttachStates(d, s);
  }

  static CompoundTagPtr SculkShrieker(Block const &b, CompoundTagConstPtr const &) {
    auto d = New("sculk_shrieker");
    auto s = States();
    auto canSummon = b.property("can_summon") == "true";
    auto shrieking = b.property("shrieking") == "true";
    s->set("can_summon", Bool(canSummon));
    s->set("active", Bool(shrieking));
    return AttachStates(d, s);
  }

  static void EndRodFacingDirectionFromFacing(CompoundTagPtr const &s, Block const &block) {
    auto facing = block.property("facing", "up");
    i32 direction = 1;
    if (facing == "up") {
      direction = 1;
    } else if (facing == "east") {
      direction = 4;
    } else if (facing == "south") {
      direction = 2;
    } else if (facing == "north") {
      direction = 3;
    } else if (facing == "down") {
      direction = 0;
    } else if (facing == "west") {
      direction = 5;
    }
    s->set("facing_direction", Int(direction));
  }

  static Converter AnyTorch(std::string const &prefix) { return Converter(Name(prefix + "torch"), AddStringProperty("torch_facing_direction", "top")); }

  static std::string GetTorchFacingDirectionFromFacing(std::string_view const &facing) {
    if (facing == "east") {
      return "west";
    } else if (facing == "west") {
      return "east";
    } else if (facing == "north") {
      return "south";
    } else {
      return "north";
    }
  }

  static void TorchFacingDirectionFromFacing(CompoundTagPtr const &s, Block const &block) {
    auto facing = block.property("facing", "north");
    auto direction = GetTorchFacingDirectionFromFacing(facing);
    s->set("torch_facing_direction", String(direction));
  }

  static Converter AnyWallTorch(std::string const &prefix) { return Converter(Name(prefix + "torch"), TorchFacingDirectionFromFacing); }

  static Converter InfestedStone(std::string const &type) { return Subtype("monster_egg", "monster_egg_stone_type", type); }

  static Converter Fence(std::string const &type) { return Subtype("fence", "wood_type", type); }

  static PropertyType Moisture(Block const &block) {
    auto v = Wrap(strings::ToI32(block.property("moisture", "0")), 0);
    return Int(v);
  }

  static bool On(std::string_view const &actual) {
    if (actual.empty()) {
      return true;
    }
    return actual == "true";
  }

  static bool Off(std::string_view const &actual) {
    if (actual.empty()) {
      return true;
    }
    return actual == "false";
  }

  static PropertyPickupFunction HugeMushroomBits(bool stem) {
    return [=](CompoundTagPtr const &s, Block const &block) {
      auto up = block.property("up");
      auto down = block.property("down");
      auto north = block.property("north");
      auto east = block.property("east");
      auto south = block.property("south");
      auto west = block.property("west");

      i32 bits = stem ? 15 : 14;
      if (up.empty() && down.empty() && north.empty() && east.empty() && south.empty() && west.empty() && stem) {
        bits = 15;
      } else if (Off(up) && Off(down) && Off(north) && Off(east) && Off(south) && Off(west)) {
        bits = 0;
      } else if (On(up) && Off(down) && On(north) && Off(east) && Off(south) && On(west)) {
        bits = 1;
      } else if (On(up) && Off(down) && On(north) && Off(east) && Off(south) && Off(west)) {
        bits = 2;
      } else if (On(up) && Off(down) && On(north) && On(east) && Off(south) && Off(west)) {
        bits = 3;
      } else if (On(up) && Off(down) && Off(north) && Off(east) && Off(south) && On(west)) {
        bits = 4;
      } else if (On(up) && Off(down) && Off(north) && Off(east) && Off(south) && Off(west)) {
        bits = 5;
      } else if (On(up) && Off(down) && Off(north) && On(east) && Off(south) && Off(west)) {
        bits = 6;
      } else if (On(up) && Off(down) && Off(north) && Off(east) && On(south) && On(west)) {
        bits = 7;
      } else if (On(up) && Off(down) && Off(north) && Off(east) && On(south) && Off(west)) {
        bits = 8;
      } else if (On(up) && Off(down) && Off(north) && On(east) && On(south) && Off(west)) {
        bits = 9;
      } else if (Off(up) && Off(down) && On(north) && On(east) && On(south) && On(west) && stem) {
        bits = 10;
      } else if (On(up) && On(down) && On(north) && On(east) && On(south) && On(west)) {
        if (stem) {
          bits = 15;
        } else {
          bits = 14;
        }
      }
      s->set("huge_mushroom_bits", Int(bits));
    };
  }

  static Converter AnyMushroomBlock(std::string const &name, bool stem) { return Converter(Name(name), HugeMushroomBits(stem)); }

  static Converter ShulkerBox(std::string const &color) { return Subtype("shulker_box", "color", color); }

  static void EyeToEndPortalEyeBit(CompoundTagPtr const &s, Block const &block) {
    auto eye = block.property("eye", "false") == "true";
    s->set("end_portal_eye_bit", Bool(eye));
  }

  static void FacingDirectionAFromFacing(CompoundTagPtr const &s, Block const &block) {
    i32 direction = GetFacingDirectionAFromFacing(block);
    s->set("facing_direction", Int(direction));
  }

  static void PistonFacingDirectionFromFacing6(CompoundTagPtr const &s, Block const &block) {
    auto facing = block.property("facing", "");
    Facing6 f6 = Facing6FromJavaName(facing);
    i32 direction = BedrockFacingDirectionBFromFacing6(f6);
    s->set("facing_direction", Int(direction));
  }

  static std::string GetWallConnectionType(std::string_view const &type) {
    if (type == "low") {
      return "short";
    } else if (type == "tall") {
      return "tall";
    } else {
      return "none";
    }
  }

  static PropertyPickupFunction WallConnectionType(std::string const &direction) {
    return [=](CompoundTagPtr const &s, Block const &b) {
      std::string beName = "wall_connection_type_" + direction;
      auto v = b.property(direction, "none");
      if (v == "true" || v == "false") {
        if (v == "true") {
          s->set(beName, String("short"));
        } else {
          s->set(beName, String("none"));
        }
      } else {
        auto type = GetWallConnectionType(v);
        s->set(beName, String(type));
      }
    };
  }

  static void WallPostBit(CompoundTagPtr const &s, Block const &b) {
    auto up = b.property("up", "false") == "true";
    s->set("wall_post_bit", Bool(up));
  }

  static Converter Wall(std::string const &type) { return Converter(Name("cobblestone_wall"), WallPostBit, AddStringProperty("wall_block_type", type), WallConnectionType("east"), WallConnectionType("north"), WallConnectionType("south"), WallConnectionType("west")); }

  static Converter Carpet(std::string const &color) { return Subtype("carpet", "color", color); }

  static Converter StainedGlassPane(std::string const &color) { return Subtype("stained_glass_pane", "color", color); }

  static Converter Anvil(std::string const &damage) { return Converter(Name("anvil"), AddStringProperty("damage", damage), DirectionFromFacingA); }

  static Converter Coral(std::string const &type, bool dead) { return Converter(Name("coral"), AddStringProperty("coral_color", type), AddBoolProperty("dead_bit", dead)); }

  static Converter CoralFan(std::string const &type, bool dead) { return Converter(Name(dead ? "coral_fan_dead" : "coral_fan"), AddStringProperty("coral_color", type), AddIntProperty("coral_fan_direction", 0)); }

  static Converter CoralWallFan(std::string const &tail, bool dead, i8 type) { return Converter(Name("coral_fan_hang" + tail), AddByteProperty("coral_hang_type_bit", type), Name(FacingD, "coral_direction"), AddBoolProperty("dead_bit", dead)); }

  static Converter WallSign(std::optional<std::string> prefix = std::nullopt) {
    std::string name = prefix ? *prefix + "_wall_sign" : "wall_sign";
    return Converter(Name(name), FacingDirectionAFromFacing);
  }

  static PropertyType Rotation(Block const &block) {
    auto r = Wrap(strings::ToI32(block.property("rotation", "0")), 0);
    return Int(r);
  }

  static Converter Sign(std::optional<std::string> prefix = std::nullopt) {
    std::string name = prefix ? *prefix + "_standing_sign" : "standing_sign";
    return Converter(Name(name), Name(Rotation, "ground_sign_direction"));
  }

  static void PartToHeadPieceBit(CompoundTagPtr const &s, Block const &block) {
    auto head = block.property("part", "foot") == "head";
    s->set("head_piece_bit", Bool(head));
  }

  static void OccupiedToOccupiedBit(CompoundTagPtr const &s, Block const &block) {
    auto occupied = block.property("occupied", "false") == "true";
    s->set("occupied_bit", Bool(occupied));
  }

  static PropertyType Open(Block const &block) { return Bool(block.property("open", "false") == "true"); }

  static void GrindstoneFaceToAttachment(CompoundTagPtr const &s, Block const &block) {
    auto face = block.property("face", "wall");
    std::string attachment;
    if (face == "wall") {
      attachment = "side";
    } else if (face == "floor") {
      attachment = "standing";
    } else {
      attachment = "hanging";
    }
    s->set("attachment", String(attachment));
  }

  static PropertyType Hanging(Block const &block) {
    auto hanging = block.property("hanging", "false") == "true";
    return Bool(hanging);
  }

  static void BellAttachmentFromAttachment(CompoundTagPtr const &s, Block const &block) {
    auto attachment = block.property("attachment", "floor");
    s->set("attachment", String(GetAttachment(attachment)));
  }

  static void BellDirectionFromFacing(CompoundTagPtr const &s, Block const &block) {
    auto facing = block.property("facing", "north");
    i32 direction = 0;
    if (facing == "north") {
      direction = 0;
    } else if (facing == "east") {
      direction = 1;
    } else if (facing == "south") {
      direction = 2;
    } else {
      direction = 3;
    }
    s->set("direction", Int(direction));
  }

  static PropertyType Powered(Block const &block) {
    auto p = block.property("powered", "false") == "true";
    return Bool(p);
  }

  static void RedstoneSignalFromPowered(CompoundTagPtr const &s, Block const &block) {
    auto powered = block.property("powered", "false") == "true";
    s->set("redstone_signal", Int(powered ? 1 : 0));
  }

  static PropertyType Lit(Block const &block) {
    auto l = block.property("lit", "flase") == "true";
    return Bool(l);
  }

  static void LitToExtinguished(CompoundTagPtr const &s, Block const &block) {
    auto l = block.property("lit", "flase") == "true";
    s->set("extinguished", Bool(!l));
  }

  static std::vector<AnyConverter> *CreateConverterTable() {
    using namespace std;
    Converter axisToPillarAxis(Same, AxisToPillarAxis);
    Converter directionFromFacing(Same, DirectionFromFacingA);
    Converter facingDirectionFromFacingA(Same, FacingDirectionAFromFacing);
    Converter facingDirectionFromFacingB(Same, PistonFacingDirectionFromFacing6);

    auto table = new vector<AnyConverter>(mcfile::blocks::minecraft::minecraft_max_block_id);
#define E(__name, __func)                                                                               \
  assert(!(*table)[static_cast<u32>(mcfile::blocks::minecraft::__name)] && "converter is already set"); \
  (*table)[static_cast<u32>(mcfile::blocks::minecraft::__name)] = __func;

    E(stone, Stone("stone"));
    E(granite, Stone("granite"));
    E(polished_granite, Stone("granite_smooth"));
    E(andesite, Stone("andesite"));
    E(polished_andesite, Stone("andesite_smooth"));
    E(diorite, Stone("diorite"));
    E(polished_diorite, Stone("diorite_smooth"));
    E(dirt, Dirt("normal"));
    E(coarse_dirt, Dirt("coarse"));
    E(grass_block, Rename("grass"));
    E(oak_log, Log("oak"));
    E(spruce_log, Log("spruce"));
    E(birch_log, Log("birch"));
    E(jungle_log, Log("jungle"));
    E(acacia_log, Log2("acacia"));
    E(dark_oak_log, Log2("dark_oak"));
    E(crimson_stem, axisToPillarAxis);
    E(warped_stem, axisToPillarAxis);
    E(mangrove_log, axisToPillarAxis);

    E(stripped_oak_log, axisToPillarAxis);
    E(stripped_spruce_log, axisToPillarAxis);
    E(stripped_birch_log, axisToPillarAxis);
    E(stripped_acacia_log, axisToPillarAxis);
    E(stripped_jungle_log, axisToPillarAxis);
    E(stripped_dark_oak_log, axisToPillarAxis);
    E(stripped_mangrove_log, axisToPillarAxis);

    E(oak_wood, Wood("oak", false));
    E(spruce_wood, Wood("spruce", false));
    E(birch_wood, Wood("birch", false));
    E(acacia_wood, Wood("acacia", false));
    E(jungle_wood, Wood("jungle", false));
    E(dark_oak_wood, Wood("dark_oak", false));
    E(mangrove_wood, axisToPillarAxis);
    E(stripped_oak_wood, Wood("oak", true));
    E(stripped_spruce_wood, Wood("spruce", true));
    E(stripped_birch_wood, Wood("birch", true));
    E(stripped_acacia_wood, Wood("acacia", true));
    E(stripped_jungle_wood, Wood("jungle", true));
    E(stripped_dark_oak_wood, Wood("dark_oak", true));
    E(stripped_mangrove_wood, axisToPillarAxis);
    E(oak_leaves, Leaves("oak"));
    E(spruce_leaves, Leaves("spruce"));
    E(birch_leaves, Leaves("birch"));
    E(jungle_leaves, Leaves("jungle"));
    E(acacia_leaves, Leaves2("acacia"));
    E(dark_oak_leaves, Leaves2("dark_oak"));
    E(crimson_hyphae, axisToPillarAxis);
    E(warped_hyphae, axisToPillarAxis);
    E(stripped_crimson_hyphae, axisToPillarAxis);
    E(stripped_warped_hyphae, axisToPillarAxis);
    E(stripped_crimson_stem, axisToPillarAxis);
    E(stripped_warped_stem, axisToPillarAxis);
    E(oak_slab, WoodenSlab("oak"));
    E(birch_slab, WoodenSlab("birch"));
    E(spruce_slab, WoodenSlab("spruce"));
    E(jungle_slab, WoodenSlab("jungle"));
    E(acacia_slab, WoodenSlab("acacia"));
    E(dark_oak_slab, WoodenSlab("dark_oak"));
    E(petrified_oak_slab, WoodenSlab("oak"));
    E(stone_slab, StoneSlab4("stone"));
    E(granite_slab, StoneSlab3("granite"));
    E(andesite_slab, StoneSlab3("andesite"));
    E(diorite_slab, StoneSlab3("diorite"));
    E(cobblestone_slab, StoneSlab("cobblestone"));
    E(stone_brick_slab, StoneSlab("stone_brick"));
    E(brick_slab, StoneSlab("brick"));
    E(sandstone_slab, StoneSlab("sandstone"));
    E(smooth_sandstone_slab, StoneSlab2("smooth_sandstone"));
    E(smooth_stone_slab, StoneSlab("smooth_stone"));
    E(nether_brick_slab, StoneSlab("nether_brick"));
    E(quartz_slab, StoneSlab("quartz"));
    E(smooth_quartz_slab, StoneSlab4("smooth_quartz"));
    E(red_sandstone_slab, StoneSlab2("red_sandstone"));
    E(smooth_red_sandstone_slab, StoneSlab3("smooth_red_sandstone"));
    E(cut_red_sandstone_slab, StoneSlab4("cut_red_sandstone"));
    E(mossy_cobblestone_slab, StoneSlab2("mossy_cobblestone"));
    E(polished_diorite_slab, StoneSlab3("polished_diorite"));
    E(mossy_stone_brick_slab, StoneSlab4("mossy_stone_brick"));
    E(polished_granite_slab, StoneSlab3("polished_granite"));
    E(dark_prismarine_slab, StoneSlab2("prismarine_dark"));
    E(prismarine_brick_slab, StoneSlab2("prismarine_brick"));
    E(prismarine_slab, StoneSlab2("prismarine_rough"));
    E(purpur_slab, StoneSlab2("purpur"));
    E(cut_sandstone_slab, StoneSlab4("cut_sandstone"));
    E(polished_blackstone_brick_slab, Slab("polished_blackstone_brick_double_slab"));
    E(polished_blackstone_slab, Slab("polished_blackstone_double_slab"));
    E(blackstone_slab, Slab("blackstone_double_slab"));
    E(polished_andesite_slab, StoneSlab3("polished_andesite"));
    E(red_nether_brick_slab, StoneSlab2("red_nether_brick"));
    E(end_stone_brick_slab, StoneSlab3("end_stone_brick"));
    E(warped_slab, Slab("warped_double_slab"));
    E(crimson_slab, Slab("crimson_double_slab"));
    E(mangrove_slab, Slab("mangrove_double_slab"));
    E(grass, TallGrass("tall"));
    E(tall_grass, DoublePlant("grass"));
    E(large_fern, DoublePlant("fern"));
    E(fern, TallGrass("fern"));
    E(lilac, DoublePlant("syringa"));
    E(rose_bush, DoublePlant("rose"));
    E(peony, DoublePlant("paeonia"));
    E(sunflower, DoublePlant("sunflower"));
    E(dead_bush, Rename("deadbush"));
    E(sea_pickle, SeaPickle());
    E(dandelion, Rename("yellow_flower"));
    E(poppy, RedFlower("poppy"));
    E(blue_orchid, RedFlower("orchid"));
    E(allium, RedFlower("allium"));
    E(azure_bluet, RedFlower("houstonia"));
    E(red_tulip, RedFlower("tulip_red"));
    E(orange_tulip, RedFlower("tulip_orange"));
    E(white_tulip, RedFlower("tulip_white"));
    E(pink_tulip, RedFlower("tulip_pink"));
    E(oxeye_daisy, RedFlower("oxeye"));
    E(cornflower, RedFlower("cornflower"));
    E(lily_of_the_valley, RedFlower("lily_of_the_valley"));
    E(seagrass, Converter(Name("seagrass"), AddStringProperty("sea_grass_type", "default")));
    E(tall_seagrass, Converter(Name("seagrass"), HalfToSeagrassType));
    E(kelp, Kelp());
    E(kelp_plant, Kelp(16));
    E(water, Liquid("water"));
    E(lava, Liquid("lava"));
    E(weeping_vines_plant, NetherVines("weeping", 25)); // TODO(kbinani): is 25 correct?
    E(weeping_vines, NetherVines("weeping"));
    E(twisting_vines_plant, NetherVines("twisting", 25)); // TODO(kbinani): is 25 correct?
    E(twisting_vines, NetherVines("twisting"));
    E(vine, Converter(Same, VineDirectionBits));
    E(glow_lichen, Converter(Same, MultiFaceDirectionBits));
    E(sculk_vein, Converter(Same, MultiFaceDirectionBits));
    E(cocoa, Converter(Same, Name(Age, "age"), DirectionFromFacingA));
    E(nether_wart, Converter(Same, Name(Age, "age")));
    E(cobblestone_stairs, Stairs("stone_stairs"));
    E(stone_stairs, Stairs("normal_stone_stairs"));
    E(end_stone_brick_stairs, Stairs("end_brick_stairs"));
    E(prismarine_brick_stairs, Stairs("prismarine_bricks_stairs"));
    E(oak_stairs, Stairs());
    E(spruce_stairs, Stairs());
    E(birch_stairs, Stairs());
    E(jungle_stairs, Stairs());
    E(acacia_stairs, Stairs());
    E(dark_oak_stairs, Stairs());
    E(crimson_stairs, Stairs());
    E(warped_stairs, Stairs());
    E(granite_stairs, Stairs());
    E(polished_granite_stairs, Stairs());
    E(diorite_stairs, Stairs());
    E(polished_diorite_stairs, Stairs());
    E(andesite_stairs, Stairs());
    E(polished_andesite_stairs, Stairs());
    E(mossy_cobblestone_stairs, Stairs());
    E(stone_brick_stairs, Stairs());
    E(mossy_stone_brick_stairs, Stairs());
    E(brick_stairs, Stairs());
    E(nether_brick_stairs, Stairs());
    E(red_nether_brick_stairs, Stairs());
    E(sandstone_stairs, Stairs());
    E(smooth_sandstone_stairs, Stairs());
    E(red_sandstone_stairs, Stairs());
    E(smooth_red_sandstone_stairs, Stairs());
    E(quartz_stairs, Stairs());
    E(smooth_quartz_stairs, Stairs());
    E(purpur_stairs, Stairs());
    E(prismarine_stairs, Stairs());
    E(dark_prismarine_stairs, Stairs());
    E(blackstone_stairs, Stairs());
    E(polished_blackstone_stairs, Stairs());
    E(polished_blackstone_brick_stairs, Stairs());
    E(bamboo_stairs, Stairs());
    E(bamboo_mosaic_stairs, Stairs());
    E(sponge, Sponge("dry"));
    E(wet_sponge, Sponge("wet"));
    E(sandstone, Sandstone("default"));
    E(chiseled_sandstone, Sandstone("heiroglyphs"));
    E(cut_sandstone, Sandstone("cut"));
    E(smooth_sandstone, Sandstone("smooth"));
    E(red_sandstone, RedSandstone("default"));
    E(chiseled_red_sandstone, RedSandstone("heiroglyphs"));
    E(cut_red_sandstone, RedSandstone("cut"));
    E(smooth_red_sandstone, RedSandstone("smooth"));
    E(white_wool, Wool("white"));
    E(orange_wool, Wool("orange"));
    E(magenta_wool, Wool("magenta"));
    E(light_blue_wool, Wool("light_blue"));
    E(yellow_wool, Wool("yellow"));
    E(lime_wool, Wool("lime"));
    E(pink_wool, Wool("pink"));
    E(gray_wool, Wool("gray"));
    E(light_gray_wool, Wool("silver"));
    E(cyan_wool, Wool("cyan"));
    E(purple_wool, Wool("purple"));
    E(blue_wool, Wool("blue"));
    E(brown_wool, Wool("brown"));
    E(green_wool, Wool("green"));
    E(red_wool, Wool("red"));
    E(black_wool, Wool("black"));
    E(snow_block, Rename("snow"));
    E(quartz_block, QuartzBlock("default"));
    E(smooth_quartz, QuartzBlock("smooth"));
    E(quartz_pillar, QuartzBlock("lines"));
    E(chiseled_quartz_block, QuartzBlock("chiseled"));
    E(bricks, Rename("brick_block"));
    E(sand, Sand("normal"));
    E(red_sand, Sand("red"));
    E(oak_planks, Planks("oak"));
    E(spruce_planks, Planks("spruce"));
    E(birch_planks, Planks("birch"));
    E(jungle_planks, Planks("jungle"));
    E(acacia_planks, Planks("acacia"));
    E(dark_oak_planks, Planks("dark_oak"));
    E(purpur_block, Converter(Same, AddStringProperty("chisel_type", "default"), AddStringProperty("pillar_axis", "y")));
    E(purpur_pillar, Converter(Name("purpur_block"), AddStringProperty("chisel_type", "lines"), AxisToPillarAxis));
    E(jack_o_lantern, LitPumpkin());
    E(carved_pumpkin, directionFromFacing);
    E(white_stained_glass, StainedGlass("white"));
    E(orange_stained_glass, StainedGlass("orange"));
    E(magenta_stained_glass, StainedGlass("magenta"));
    E(light_blue_stained_glass, StainedGlass("light_blue"));
    E(yellow_stained_glass, StainedGlass("yellow"));
    E(lime_stained_glass, StainedGlass("lime"));
    E(pink_stained_glass, StainedGlass("pink"));
    E(gray_stained_glass, StainedGlass("gray"));
    E(light_gray_stained_glass, StainedGlass("silver"));
    E(cyan_stained_glass, StainedGlass("cyan"));
    E(purple_stained_glass, StainedGlass("purple"));
    E(blue_stained_glass, StainedGlass("blue"));
    E(brown_stained_glass, StainedGlass("brown"));
    E(green_stained_glass, StainedGlass("green"));
    E(red_stained_glass, StainedGlass("red"));
    E(black_stained_glass, StainedGlass("black"));
    E(white_concrete_powder, ConcretePowder("white"));
    E(orange_concrete_powder, ConcretePowder("orange"));
    E(magenta_concrete_powder, ConcretePowder("magenta"));
    E(light_blue_concrete_powder, ConcretePowder("light_blue"));
    E(yellow_concrete_powder, ConcretePowder("yellow"));
    E(lime_concrete_powder, ConcretePowder("lime"));
    E(pink_concrete_powder, ConcretePowder("pink"));
    E(gray_concrete_powder, ConcretePowder("gray"));
    E(light_gray_concrete_powder, ConcretePowder("silver"));
    E(cyan_concrete_powder, ConcretePowder("cyan"));
    E(purple_concrete_powder, ConcretePowder("purple"));
    E(blue_concrete_powder, ConcretePowder("blue"));
    E(brown_concrete_powder, ConcretePowder("brown"));
    E(green_concrete_powder, ConcretePowder("green"));
    E(red_concrete_powder, ConcretePowder("red"));
    E(black_concrete_powder, ConcretePowder("black"));
    E(white_concrete, Concrete("white"));
    E(orange_concrete, Concrete("orange"));
    E(magenta_concrete, Concrete("magenta"));
    E(light_blue_concrete, Concrete("light_blue"));
    E(yellow_concrete, Concrete("yellow"));
    E(lime_concrete, Concrete("lime"));
    E(pink_concrete, Concrete("pink"));
    E(gray_concrete, Concrete("gray"));
    E(light_gray_concrete, Concrete("silver"));
    E(cyan_concrete, Concrete("cyan"));
    E(purple_concrete, Concrete("purple"));
    E(blue_concrete, Concrete("blue"));
    E(brown_concrete, Concrete("brown"));
    E(green_concrete, Concrete("green"));
    E(red_concrete, Concrete("red"));
    E(black_concrete, Concrete("black"));
    E(white_terracotta, Terracotta("white"));
    E(orange_terracotta, Terracotta("orange"));
    E(magenta_terracotta, Terracotta("magenta"));
    E(light_blue_terracotta, Terracotta("light_blue"));
    E(yellow_terracotta, Terracotta("yellow"));
    E(lime_terracotta, Terracotta("lime"));
    E(pink_terracotta, Terracotta("pink"));
    E(gray_terracotta, Terracotta("gray"));
    E(light_gray_terracotta, Terracotta("silver"));
    E(cyan_terracotta, Terracotta("cyan"));
    E(purple_terracotta, Terracotta("purple"));
    E(blue_terracotta, Terracotta("blue"));
    E(brown_terracotta, Terracotta("brown"));
    E(green_terracotta, Terracotta("green"));
    E(red_terracotta, Terracotta("red"));
    E(black_terracotta, Terracotta("black"));
    E(nether_quartz_ore, Rename("quartz_ore"));
    E(red_nether_bricks, Rename("red_nether_brick"));
    E(magma_block, Rename("magma"));
    E(sea_lantern, Identity);
    E(prismarine_bricks, Prismarine("bricks"));
    E(dark_prismarine, Prismarine("dark"));
    E(prismarine, Prismarine("default"));
    E(terracotta, Rename("hardened_clay"));
    E(end_stone_bricks, Rename("end_bricks"));
    E(melon, Rename("melon_block"));
    E(chiseled_stone_bricks, StoneBrick("chiseled"));
    E(cracked_stone_bricks, StoneBrick("cracked"));
    E(mossy_stone_bricks, StoneBrick("mossy"));
    E(stone_bricks, StoneBrick("default"));
    E(oak_sapling, Sapling("oak"));
    E(birch_sapling, Sapling("birch"));
    E(jungle_sapling, Sapling("jungle"));
    E(acacia_sapling, Sapling("acacia"));
    E(spruce_sapling, Sapling("spruce"));
    E(dark_oak_sapling, Sapling("dark_oak"));
    E(tube_coral_block, CoralBlock("blue", false));
    E(brain_coral_block, CoralBlock("pink", false));
    E(bubble_coral_block, CoralBlock("purple", false));
    E(fire_coral_block, CoralBlock("red", false));
    E(horn_coral_block, CoralBlock("yellow", false));
    E(dead_tube_coral_block, CoralBlock("blue", true));
    E(dead_brain_coral_block, CoralBlock("pink", true));
    E(dead_bubble_coral_block, CoralBlock("purple", true));
    E(dead_fire_coral_block, CoralBlock("red", true));
    E(dead_horn_coral_block, CoralBlock("yellow", true));
    E(snow, SnowLayer);
    E(sugar_cane, Converter(Name("reeds"), Name(Age, "age")));
    E(end_rod, Converter(Same, EndRodFacingDirectionFromFacing));
    E(oak_fence, Fence("oak"));
    E(spruce_fence, Fence("spruce"));
    E(birch_fence, Fence("birch"));
    E(jungle_fence, Fence("jungle"));
    E(acacia_fence, Fence("acacia"));
    E(dark_oak_fence, Fence("dark_oak"));
    E(bamboo_fence, Identity);
    E(ladder, facingDirectionFromFacingA);
    E(chest, facingDirectionFromFacingA);
    E(furnace, Converter(PrefixLit, FacingDirectionAFromFacing));
    E(nether_bricks, Rename("nether_brick"));
    E(infested_stone, InfestedStone("stone"));
    E(infested_cobblestone, InfestedStone("cobblestone"));
    E(infested_stone_bricks, InfestedStone("stone_brick"));
    E(infested_mossy_stone_bricks, InfestedStone("mossy_stone_brick"));
    E(infested_cracked_stone_bricks, InfestedStone("cracked_stone_brick"));
    E(infested_chiseled_stone_bricks, InfestedStone("chiseled_stone_brick"));
    E(torch, AnyTorch(""));
    E(wall_torch, AnyWallTorch(""));
    E(soul_torch, AnyTorch("soul_"));
    E(soul_wall_torch, AnyWallTorch("soul_"));
    E(redstone_torch, Converter(PrefixUnlit("redstone_torch"), AddStringProperty("torch_facing_direction", "top")));
    E(redstone_wall_torch, Converter(PrefixUnlit("redstone_torch"), TorchFacingDirectionFromFacing));
    E(farmland, Converter(Same, Name(Moisture, "moisturized_amount")));
    E(red_mushroom_block, AnyMushroomBlock("red_mushroom_block", false));
    E(brown_mushroom_block, AnyMushroomBlock("brown_mushroom_block", false));
    E(mushroom_stem, AnyMushroomBlock("brown_mushroom_block", true));
    E(end_portal_frame, Converter(Same, DirectionFromFacingA, EyeToEndPortalEyeBit));
    E(white_shulker_box, ShulkerBox("white"));
    E(orange_shulker_box, ShulkerBox("orange"));
    E(magenta_shulker_box, ShulkerBox("magenta"));
    E(light_blue_shulker_box, ShulkerBox("light_blue"));
    E(yellow_shulker_box, ShulkerBox("yellow"));
    E(lime_shulker_box, ShulkerBox("lime"));
    E(pink_shulker_box, ShulkerBox("pink"));
    E(gray_shulker_box, ShulkerBox("gray"));
    E(light_gray_shulker_box, ShulkerBox("silver"));
    E(cyan_shulker_box, ShulkerBox("cyan"));
    E(purple_shulker_box, ShulkerBox("purple"));
    E(blue_shulker_box, ShulkerBox("blue"));
    E(brown_shulker_box, ShulkerBox("brown"));
    E(green_shulker_box, ShulkerBox("green"));
    E(red_shulker_box, ShulkerBox("red"));
    E(black_shulker_box, ShulkerBox("black"));
    E(shulker_box, Rename("undyed_shulker_box"));
    E(cobblestone_wall, Wall("cobblestone"));
    E(mossy_cobblestone_wall, Wall("mossy_cobblestone"));
    E(brick_wall, Wall("brick"));
    E(prismarine_wall, Wall("prismarine"));
    E(red_sandstone_wall, Wall("red_sandstone"));
    E(mossy_stone_brick_wall, Wall("mossy_stone_brick"));
    E(granite_wall, Wall("granite"));
    E(andesite_wall, Wall("andesite"));
    E(diorite_wall, Wall("diorite"));
    E(stone_brick_wall, Wall("stone_brick"));
    E(nether_brick_wall, Wall("nether_brick"));
    E(red_nether_brick_wall, Wall("red_nether_brick"));
    E(sandstone_wall, Wall("sandstone"));
    E(end_stone_brick_wall, Wall("end_brick"));
    Converter wall(Same, WallPostBit, WallConnectionType("east"), WallConnectionType("north"), WallConnectionType("south"), WallConnectionType("west"));
    E(blackstone_wall, wall);
    E(polished_blackstone_wall, wall);
    E(polished_blackstone_brick_wall, wall);
    E(white_carpet, Carpet("white"));
    E(orange_carpet, Carpet("orange"));
    E(magenta_carpet, Carpet("magenta"));
    E(light_blue_carpet, Carpet("light_blue"));
    E(yellow_carpet, Carpet("yellow"));
    E(lime_carpet, Carpet("lime"));
    E(pink_carpet, Carpet("pink"));
    E(gray_carpet, Carpet("gray"));
    E(light_gray_carpet, Carpet("silver"));
    E(cyan_carpet, Carpet("cyan"));
    E(purple_carpet, Carpet("purple"));
    E(blue_carpet, Carpet("blue"));
    E(brown_carpet, Carpet("brown"));
    E(green_carpet, Carpet("green"));
    E(red_carpet, Carpet("red"));
    E(black_carpet, Carpet("black"));
    E(white_stained_glass_pane, StainedGlassPane("white"));
    E(orange_stained_glass_pane, StainedGlassPane("orange"));
    E(magenta_stained_glass_pane, StainedGlassPane("magenta"));
    E(light_blue_stained_glass_pane, StainedGlassPane("light_blue"));
    E(yellow_stained_glass_pane, StainedGlassPane("yellow"));
    E(lime_stained_glass_pane, StainedGlassPane("lime"));
    E(pink_stained_glass_pane, StainedGlassPane("pink"));
    E(gray_stained_glass_pane, StainedGlassPane("gray"));
    E(light_gray_stained_glass_pane, StainedGlassPane("silver"));
    E(cyan_stained_glass_pane, StainedGlassPane("cyan"));
    E(purple_stained_glass_pane, StainedGlassPane("purple"));
    E(blue_stained_glass_pane, StainedGlassPane("blue"));
    E(brown_stained_glass_pane, StainedGlassPane("brown"));
    E(green_stained_glass_pane, StainedGlassPane("green"));
    E(red_stained_glass_pane, StainedGlassPane("red"));
    E(black_stained_glass_pane, StainedGlassPane("black"));
    E(slime_block, Rename("slime"));
    E(anvil, Anvil("undamaged"));
    E(chipped_anvil, Anvil("slightly_damaged"));
    E(damaged_anvil, Anvil("very_damaged"));
    E(white_glazed_terracotta, facingDirectionFromFacingA);
    E(orange_glazed_terracotta, facingDirectionFromFacingA);
    E(magenta_glazed_terracotta, facingDirectionFromFacingA);
    E(light_blue_glazed_terracotta, facingDirectionFromFacingA);
    E(yellow_glazed_terracotta, facingDirectionFromFacingA);
    E(lime_glazed_terracotta, facingDirectionFromFacingA);
    E(pink_glazed_terracotta, facingDirectionFromFacingA);
    E(gray_glazed_terracotta, facingDirectionFromFacingA);
    E(light_gray_glazed_terracotta, Converter(Name("silver_glazed_terracotta"), FacingDirectionAFromFacing));
    E(cyan_glazed_terracotta, facingDirectionFromFacingA);
    E(purple_glazed_terracotta, facingDirectionFromFacingA);
    E(blue_glazed_terracotta, facingDirectionFromFacingA);
    E(brown_glazed_terracotta, facingDirectionFromFacingA);
    E(green_glazed_terracotta, facingDirectionFromFacingA);
    E(red_glazed_terracotta, facingDirectionFromFacingA);
    E(black_glazed_terracotta, facingDirectionFromFacingA);

    E(tube_coral, Coral("blue", false));
    E(brain_coral, Coral("pink", false));
    E(bubble_coral, Coral("purple", false));
    E(fire_coral, Coral("red", false));
    E(horn_coral, Coral("yellow", false));

    E(dead_tube_coral, Coral("blue", true));
    E(dead_brain_coral, Coral("pink", true));
    E(dead_bubble_coral, Coral("purple", true));
    E(dead_fire_coral, Coral("red", true));
    E(dead_horn_coral, Coral("yellow", true));

    E(tube_coral_fan, CoralFan("blue", false));
    E(brain_coral_fan, CoralFan("pink", false));
    E(bubble_coral_fan, CoralFan("purple", false));
    E(fire_coral_fan, CoralFan("red", false));
    E(horn_coral_fan, CoralFan("yellow", false));

    E(dead_tube_coral_fan, CoralFan("blue", true));
    E(dead_brain_coral_fan, CoralFan("pink", true));
    E(dead_bubble_coral_fan, CoralFan("purple", true));
    E(dead_fire_coral_fan, CoralFan("red", true));
    E(dead_horn_coral_fan, CoralFan("yellow", true));

    E(tube_coral_wall_fan, CoralWallFan("", false, 0));
    E(brain_coral_wall_fan, CoralWallFan("", false, 1));
    E(bubble_coral_wall_fan, CoralWallFan("2", false, 0));
    E(fire_coral_wall_fan, CoralWallFan("2", false, 1));
    E(horn_coral_wall_fan, CoralWallFan("3", false, 0));

    E(dead_tube_coral_wall_fan, CoralWallFan("", true, 0));
    E(dead_brain_coral_wall_fan, CoralWallFan("", true, 1));
    E(dead_bubble_coral_wall_fan, CoralWallFan("2", true, 0));
    E(dead_fire_coral_wall_fan, CoralWallFan("2", true, 1));
    E(dead_horn_coral_wall_fan, CoralWallFan("3", true, 0));

    E(oak_sign, Sign());
    E(spruce_sign, Sign("spruce"));
    E(birch_sign, Sign("birch"));
    E(jungle_sign, Sign("jungle"));
    E(acacia_sign, Sign("acacia"));
    E(dark_oak_sign, Sign("darkoak"));
    E(crimson_sign, Sign("crimson"));
    E(warped_sign, Sign("warped"));
    E(mangrove_sign, Sign("mangrove"));
    E(bamboo_sign, Sign("bamboo"));

    E(oak_wall_sign, WallSign());
    E(spruce_wall_sign, WallSign("spruce"));
    E(birch_wall_sign, WallSign("birch"));
    E(jungle_wall_sign, WallSign("jungle"));
    E(acacia_wall_sign, WallSign("acacia"));
    E(dark_oak_wall_sign, WallSign("darkoak"));
    E(crimson_wall_sign, WallSign("crimson"));
    E(warped_wall_sign, WallSign("warped"));
    E(mangrove_wall_sign, WallSign("mangrove"));
    E(bamboo_wall_sign, WallSign("bamboo"));

    Converter bed(Name("bed"), DirectionFromFacingA, PartToHeadPieceBit, OccupiedToOccupiedBit);
    E(white_bed, bed);
    E(orange_bed, bed);
    E(magenta_bed, bed);
    E(light_blue_bed, bed);
    E(yellow_bed, bed);
    E(lime_bed, bed);
    E(pink_bed, bed);
    E(gray_bed, bed);
    E(light_gray_bed, bed);
    E(cyan_bed, bed);
    E(purple_bed, bed);
    E(blue_bed, bed);
    E(brown_bed, bed);
    E(green_bed, bed);
    E(red_bed, bed);
    E(black_bed, bed);

    E(flower_pot, FlowerPot);

    Converter pottedFlowerPot(Name("flower_pot"), AddBoolProperty("update_bit", true));
    E(potted_oak_sapling, pottedFlowerPot);
    E(potted_spruce_sapling, pottedFlowerPot);
    E(potted_birch_sapling, pottedFlowerPot);
    E(potted_jungle_sapling, pottedFlowerPot);
    E(potted_acacia_sapling, pottedFlowerPot);
    E(potted_dark_oak_sapling, pottedFlowerPot);
    E(potted_fern, pottedFlowerPot);
    E(potted_dead_bush, pottedFlowerPot);
    E(potted_dandelion, pottedFlowerPot);
    E(potted_poppy, pottedFlowerPot);
    E(potted_blue_orchid, pottedFlowerPot);
    E(potted_allium, pottedFlowerPot);
    E(potted_azure_bluet, pottedFlowerPot);
    E(potted_red_tulip, pottedFlowerPot);
    E(potted_orange_tulip, pottedFlowerPot);
    E(potted_white_tulip, pottedFlowerPot);
    E(potted_pink_tulip, pottedFlowerPot);
    E(potted_oxeye_daisy, pottedFlowerPot);
    E(potted_cornflower, pottedFlowerPot);
    E(potted_lily_of_the_valley, pottedFlowerPot);
    E(potted_wither_rose, pottedFlowerPot);
    E(potted_brown_mushroom, pottedFlowerPot);
    E(potted_red_mushroom, pottedFlowerPot);
    E(potted_crimson_fungus, pottedFlowerPot);
    E(potted_warped_fungus, pottedFlowerPot);
    E(potted_crimson_roots, pottedFlowerPot);
    E(potted_warped_roots, pottedFlowerPot);
    E(potted_bamboo, pottedFlowerPot);
    E(potted_cactus, pottedFlowerPot);
    E(potted_azalea_bush, pottedFlowerPot);
    E(potted_flowering_azalea_bush, pottedFlowerPot);
    E(potted_mangrove_propagule, pottedFlowerPot);

    Converter skull(Name("skull"), AddIntProperty("facing_direction", 1), AddBoolProperty("no_drop_bit", false));
    E(skeleton_skull, skull);
    E(wither_skeleton_skull, skull);
    E(player_head, skull);
    E(zombie_head, skull);
    E(creeper_head, skull);
    E(dragon_head, skull);
    E(piglin_head, skull);

    Converter wallSkull(Name("skull"), AddBoolProperty("no_drop_bit", false), WallSkullFacingDirection);
    E(skeleton_wall_skull, wallSkull);
    E(wither_skeleton_wall_skull, wallSkull);
    E(player_wall_head, wallSkull);
    E(zombie_wall_head, wallSkull);
    E(creeper_wall_head, wallSkull);
    E(dragon_wall_head, wallSkull);
    E(piglin_wall_head, wallSkull);

    Converter banner(Name("standing_banner"), Name(Rotation, "ground_sign_direction"));
    E(white_banner, banner);
    E(orange_banner, banner);
    E(magenta_banner, banner);
    E(light_blue_banner, banner);
    E(yellow_banner, banner);
    E(lime_banner, banner);
    E(pink_banner, banner);
    E(gray_banner, banner);
    E(light_gray_banner, banner);
    E(cyan_banner, banner);
    E(purple_banner, banner);
    E(blue_banner, banner);
    E(brown_banner, banner);
    E(green_banner, banner);
    E(red_banner, banner);
    E(black_banner, banner);

    Converter wallBanner(Name("wall_banner"), FacingDirectionAFromFacing);
    E(white_wall_banner, wallBanner);
    E(orange_wall_banner, wallBanner);
    E(magenta_wall_banner, wallBanner);
    E(light_blue_wall_banner, wallBanner);
    E(yellow_wall_banner, wallBanner);
    E(lime_wall_banner, wallBanner);
    E(pink_wall_banner, wallBanner);
    E(gray_wall_banner, wallBanner);
    E(light_gray_wall_banner, wallBanner);
    E(cyan_wall_banner, wallBanner);
    E(purple_wall_banner, wallBanner);
    E(blue_wall_banner, wallBanner);
    E(brown_wall_banner, wallBanner);
    E(green_wall_banner, wallBanner);
    E(red_wall_banner, wallBanner);
    E(black_wall_banner, wallBanner);

    E(stonecutter, Converter(Name("stonecutter_block"), FacingDirectionAFromFacing));
    E(loom, directionFromFacing);
    E(grindstone, Converter(Name("grindstone"), DirectionFromFacingA, GrindstoneFaceToAttachment));
    E(smoker, Converter(PrefixLit, FacingDirectionAFromFacing));
    E(blast_furnace, Converter(PrefixLit, FacingDirectionAFromFacing));
    E(barrel, Converter(Name("barrel"), FacingDirectionAFromFacing, Name(Open, "open_bit")));
    Converter lantern(Same, Name(Hanging, "hanging"));
    E(lantern, lantern);
    E(soul_lantern, lantern);
    E(bell, Converter(Name("bell"), BellDirectionFromFacing, BellAttachmentFromAttachment, Name(Powered, "toggle_bit")));
    Converter campfire(Same, DirectionFromFacingA, LitToExtinguished);
    E(campfire, campfire);
    E(soul_campfire, campfire);
    E(piston, facingDirectionFromFacingB);
    E(sticky_piston, facingDirectionFromFacingB);
    E(piston_head, PistonHead);
    E(moving_piston, MovingPiston);
    E(note_block, Rename("noteblock"));
    E(dispenser, Converter(Same, FacingDirectionAFromFacing, Name(Triggered, "triggered_bit")));
    E(lever, Converter(Same, LeverDirection, Name(Powered, "open_bit")));

    Converter fenceGate(Same, DirectionFromFacingA, Name(InWall, "in_wall_bit"), Name(Open, "open_bit"));
    E(oak_fence_gate, Converter(Name("fence_gate"), DirectionFromFacingA, Name(InWall, "in_wall_bit"), Name(Open, "open_bit")));
    E(spruce_fence_gate, fenceGate);
    E(birch_fence_gate, fenceGate);
    E(jungle_fence_gate, fenceGate);
    E(acacia_fence_gate, fenceGate);
    E(dark_oak_fence_gate, fenceGate);
    E(crimson_fence_gate, fenceGate);
    E(warped_fence_gate, fenceGate);
    E(mangrove_fence_gate, fenceGate);
    E(bamboo_fence_gate, fenceGate);

    Converter pressurePlate(Same, RedstoneSignalFromPowered);
    E(oak_pressure_plate, Converter(Name("wooden_pressure_plate"), RedstoneSignalFromPowered));
    E(spruce_pressure_plate, pressurePlate);
    E(birch_pressure_plate, pressurePlate);
    E(jungle_pressure_plate, pressurePlate);
    E(acacia_pressure_plate, pressurePlate);
    E(dark_oak_pressure_plate, pressurePlate);
    E(crimson_pressure_plate, pressurePlate);
    E(warped_pressure_plate, pressurePlate);
    E(stone_pressure_plate, pressurePlate);
    E(light_weighted_pressure_plate, pressurePlate);
    E(heavy_weighted_pressure_plate, pressurePlate);
    E(polished_blackstone_pressure_plate, pressurePlate);

    Converter trapdoor(Same, DirectionFromFacingB, Name(Open, "open_bit"), HalfToUpsideDownBit);
    E(oak_trapdoor, Converter(Name("trapdoor"), DirectionFromFacingB, Name(Open, "open_bit"), HalfToUpsideDownBit));
    E(spruce_trapdoor, trapdoor);
    E(birch_trapdoor, trapdoor);
    E(jungle_trapdoor, trapdoor);
    E(acacia_trapdoor, trapdoor);
    E(dark_oak_trapdoor, trapdoor);
    E(crimson_trapdoor, trapdoor);
    E(warped_trapdoor, trapdoor);
    E(iron_trapdoor, trapdoor);
    E(mangrove_trapdoor, trapdoor);
    E(bamboo_trapdoor, trapdoor);

    E(lily_pad, Rename("waterlily"));

    Converter button(Same, ButtonFacingDirection, Name(Powered, "button_pressed_bit"));
    E(oak_button, Converter(Name("wooden_button"), ButtonFacingDirection, Name(Powered, "button_pressed_bit")));
    E(spruce_button, button);
    E(birch_button, button);
    E(jungle_button, button);
    E(acacia_button, button);
    E(dark_oak_button, button);
    E(crimson_button, button);
    E(warped_button, button);
    E(stone_button, button);
    E(polished_blackstone_button, button);
    E(mangrove_button, button);
    E(bamboo_button, button);

    E(tripwire_hook, Converter(Same, DirectionFromFacingA, Name(Attached, "attached_bit"), Name(Powered, "powered_bit")));
    E(trapped_chest, facingDirectionFromFacingA);
    E(daylight_detector, Converter(DaylightDetectorName, Name(Power, "redstone_signal")));
    E(hopper, Converter(Same, FacingDirectionAFromFacing, ToggleBitFromEnabled));
    E(dropper, Converter(Same, FacingDirectionAFromFacing, Name(Triggered, "triggered_bit")));
    E(observer, Converter(Same, FacingDirectionAFromFacing, Name(Powered, "powered_bit")));

    Converter door(Same, DirectionFromFacingC, Name(Open, "open_bit"), UpperBlockBitToHalf, DoorHingeBitFromHinge);
    E(oak_door, Converter(Name("wooden_door"), DirectionFromFacingC, Name(Open, "open_bit"), UpperBlockBitToHalf, DoorHingeBitFromHinge));
    E(iron_door, door);
    E(spruce_door, door);
    E(birch_door, door);
    E(jungle_door, door);
    E(acacia_door, door);
    E(dark_oak_door, door);
    E(crimson_door, door);
    E(warped_door, door);
    E(mangrove_door, door);
    E(bamboo_door, door);

    E(repeater, Converter(RepeaterName, Name(Delay, "repeater_delay"), DirectionFromFacingA));
    E(comparator, Converter(ComparatorName, DirectionFromFacingA, OutputSubtractBitFromMode, Name(Powered, "output_lit_bit")));
    E(powered_rail, Converter(Name("golden_rail"), RailDirectionFromShape, Name(Powered, "rail_data_bit")));
    E(detector_rail, Converter(Same, RailDirectionFromShape, Name(Powered, "rail_data_bit")));
    E(activator_rail, Converter(Same, RailDirectionFromShape, Name(Powered, "rail_data_bit")));
    E(rail, Converter(Same, RailDirectionFromShape));
    E(nether_portal, Converter(Name("portal"), Name(Axis, "portal_axis")));

    E(bamboo, Bamboo);
    E(sweet_berry_bush, Converter(Same, GrowthFromAge));
    E(bubble_column, Converter(Same, DragDownFromDrag));
    E(cake, Converter(Same, BiteCounterFromBites));
    E(beetroots, Converter(Name("beetroot"), GrowthFromAge));
    E(potatoes, Converter(Same, Name(Age, "growth")));
    E(carrots, Converter(Same, Name(Age, "growth")));
    E(pumpkin_stem, Converter(Same, Name(Age, "growth"), AddIntProperty("facing_direction", 0)));
    E(melon_stem, Converter(Same, Name(Age, "growth"), AddIntProperty("facing_direction", 0)));
    E(attached_pumpkin_stem, Converter(Name("pumpkin_stem"), AddIntProperty("growth", 7), FacingDirectionAFromFacing));
    E(attached_melon_stem, Converter(Name("melon_stem"), AddIntProperty("growth", 7), FacingDirectionAFromFacing));
    E(wheat, Converter(Same, Name(Age, "growth")));

    E(cobweb, Converter(Name("web")));
    E(lectern, Converter(Same, DirectionFromFacingA, Name(Powered, "powered_bit")));
    E(ender_chest, Converter(Same, FacingDirectionAFromFacing));
    E(bone_block, Converter(Same, AxisToPillarAxis, AddIntProperty("deprecated", 0)));
    E(water_cauldron, Converter(Name("cauldron"), CauldronFillLevelFromLevel, AddStringProperty("cauldron_liquid", "water")));
    E(lava_cauldron, Converter(Name("lava_cauldron"), AddIntProperty("fill_level", 6), AddStringProperty("cauldron_liquid", "lava")));
    E(powder_snow_cauldron, Converter(Name("cauldron"), CauldronFillLevelFromLevel, AddStringProperty("cauldron_liquid", "powder_snow")));
    E(hay_block, Converter(Same, AxisToPillarAxis, AddIntProperty("deprecated", 0)));
    E(composter, Converter(Same, Name(Level, "composter_fill_level")));
    E(cave_air, Rename("air"));
    E(void_air, Rename("air"));
    E(turtle_egg, Converter(Same, TurtleEggCount, TurtleEggCrackedState));

    Converter beehive(Same, Name(FacingA, "direction"), HoneyLevel);
    E(bee_nest, beehive);
    E(beehive, beehive);

    E(chorus_flower, Converter(Same, Name(Age, "age")));

    Converter commandBlock(Same, Conditional, FacingDirectionAFromFacing);
    E(command_block, commandBlock);
    E(chain_command_block, commandBlock);
    E(repeating_command_block, commandBlock);

    E(dirt_path, Rename("grass_path"));
    E(waxed_copper_block, Rename("waxed_copper"));
    E(rooted_dirt, Rename("dirt_with_roots"));

    Converter leaves(Same, PersistentAndDistanceToPersistentBitAndUpdateBit);
    E(azalea_leaves, leaves);
    E(flowering_azalea_leaves, Converter(Name("azalea_leaves_flowered"), PersistentAndDistanceToPersistentBitAndUpdateBit));

    E(big_dripleaf, BigDripleaf);
    E(big_dripleaf_stem, Converter(Name("big_dripleaf"), AddByteProperty("big_dripleaf_head", false), DirectionFromFacingA, AddStringProperty("big_dripleaf_tilt", "none")));
    E(small_dripleaf, Converter(Name("small_dripleaf_block"), DirectionFromFacingA, UpperBlockBitToHalf));

    Converter candle(Same, Name(Lit, "lit"), Name(Candles, "candles"));
    E(candle, candle);
    E(white_candle, candle);
    E(orange_candle, candle);
    E(magenta_candle, candle);
    E(light_blue_candle, candle);
    E(yellow_candle, candle);
    E(lime_candle, candle);
    E(pink_candle, candle);
    E(gray_candle, candle);
    E(light_gray_candle, candle);
    E(cyan_candle, candle);
    E(purple_candle, candle);
    E(blue_candle, candle);
    E(brown_candle, candle);
    E(green_candle, candle);
    E(red_candle, candle);
    E(black_candle, candle);

    Converter candleCake(Same, Name(Lit, "lit"));
    E(candle_cake, candleCake);
    E(white_candle_cake, candleCake);
    E(orange_candle_cake, candleCake);
    E(magenta_candle_cake, candleCake);
    E(light_blue_candle_cake, candleCake);
    E(yellow_candle_cake, candleCake);
    E(lime_candle_cake, candleCake);
    E(pink_candle_cake, candleCake);
    E(gray_candle_cake, candleCake);
    E(light_gray_candle_cake, candleCake);
    E(cyan_candle_cake, candleCake);
    E(purple_candle_cake, candleCake);
    E(blue_candle_cake, candleCake);
    E(brown_candle_cake, candleCake);
    E(green_candle_cake, candleCake);
    E(red_candle_cake, candleCake);
    E(black_candle_cake, candleCake);

    E(cut_copper_stairs, Stairs());
    E(exposed_cut_copper_stairs, Stairs());
    E(weathered_cut_copper_stairs, Stairs());
    E(oxidized_cut_copper_stairs, Stairs());

    E(waxed_cut_copper_stairs, Stairs());
    E(waxed_exposed_cut_copper_stairs, Stairs());
    E(waxed_weathered_cut_copper_stairs, Stairs());
    E(waxed_oxidized_cut_copper_stairs, Stairs());

    E(cobbled_deepslate_stairs, Stairs());
    E(polished_deepslate_stairs, Stairs());
    E(deepslate_brick_stairs, Stairs());
    E(deepslate_tile_stairs, Stairs());

    E(mangrove_stairs, Stairs());
    E(mud_brick_stairs, Stairs());

    E(cut_copper_slab, Slab("double_cut_copper_slab"));
    E(exposed_cut_copper_slab, Slab("exposed_double_cut_copper_slab"));
    E(weathered_cut_copper_slab, Slab("weathered_double_cut_copper_slab"));
    E(oxidized_cut_copper_slab, Slab("oxidized_double_cut_copper_slab"));

    E(waxed_cut_copper_slab, Slab("waxed_double_cut_copper_slab"));
    E(waxed_exposed_cut_copper_slab, Slab("waxed_exposed_double_cut_copper_slab"));
    E(waxed_weathered_cut_copper_slab, Slab("waxed_weathered_double_cut_copper_slab"));
    E(waxed_oxidized_cut_copper_slab, Slab("waxed_oxidized_double_cut_copper_slab"));

    E(cobbled_deepslate_slab, Slab("cobbled_deepslate_double_slab"));
    E(polished_deepslate_slab, Slab("polished_deepslate_double_slab"));
    E(deepslate_brick_slab, Slab("deepslate_brick_double_slab"));
    E(deepslate_tile_slab, Slab("deepslate_tile_double_slab"));

    E(cobbled_deepslate_wall, wall);
    E(polished_deepslate_wall, wall);
    E(deepslate_brick_wall, wall);
    E(deepslate_tile_wall, wall);

    E(lightning_rod, facingDirectionFromFacingA);

    E(small_amethyst_bud, facingDirectionFromFacingA);
    E(medium_amethyst_bud, facingDirectionFromFacingA);
    E(large_amethyst_bud, facingDirectionFromFacingA);
    E(amethyst_cluster, facingDirectionFromFacingA);

    E(pointed_dripstone, PointedDripstone);
    E(light, Light);
    E(cave_vines, CaveVines);
    E(cave_vines_plant, CaveVinesPlant);
    E(chain, axisToPillarAxis);

    E(bamboo_sapling, Converter(Same, AddBoolProperty("age_bit", false), AddStringProperty("sapling_type", "oak")));
    E(brewing_stand, BrewingStand);
    E(cactus, Converter(Same, Name(Age, "age")));
    E(fire, Converter(Same, Name(Age, "age")));
    E(frosted_ice, Converter(Same, Name(Age, "age")));
    E(pumpkin, Converter(Same, AddIntProperty("direction", 0)));
    E(redstone_wire, Converter(Same, Name(Power, "redstone_signal")));
    E(scaffolding, Scaffolding);
    E(structure_block, StructureBlock);
    E(structure_void, Converter(Same, AddStringProperty("structure_void_type", "void")));
    E(tnt, Tnt);
    E(tripwire, Tripwire);
    E(basalt, axisToPillarAxis);
    E(polished_basalt, axisToPillarAxis);
    E(respawn_anchor, RespawnAnchor);
    E(deepslate, axisToPillarAxis);
    E(infested_deepslate, axisToPillarAxis);
    E(redstone_lamp, Converter(PrefixLit));
    E(redstone_ore, Converter(PrefixLit));
    E(deepslate_redstone_ore, Converter(PrefixLit));

    E(ochre_froglight, axisToPillarAxis);
    E(verdant_froglight, axisToPillarAxis);
    E(pearlescent_froglight, axisToPillarAxis);
    E(mangrove_propagule, MangrovePropagule);
    E(frogspawn, Rename("frog_spawn"));
    E(mangrove_leaves, leaves);
    E(sculk_shrieker, SculkShrieker);
    E(mud_brick_wall, wall);
    E(sculk_catalyst, SculkCatalyst);
    E(mud_brick_slab, Slab("mud_brick_double_slab"));
    E(muddy_mangrove_roots, axisToPillarAxis);

    E(spawner, Null); // spawner doesn't exist as block in BE.

    E(bamboo_block, axisToPillarAxis);
    E(stripped_bamboo_block, axisToPillarAxis);

    E(acacia_hanging_sign, HangingSign);
    E(acacia_wall_hanging_sign, WallHangingSign);
    E(bamboo_hanging_sign, HangingSign);
    E(bamboo_wall_hanging_sign, WallHangingSign);
    E(birch_hanging_sign, HangingSign);
    E(birch_wall_hanging_sign, WallHangingSign);
    E(crimson_hanging_sign, HangingSign);
    E(crimson_wall_hanging_sign, WallHangingSign);
    E(dark_oak_hanging_sign, HangingSign);
    E(dark_oak_wall_hanging_sign, WallHangingSign);
    E(jungle_hanging_sign, HangingSign);
    E(jungle_wall_hanging_sign, WallHangingSign);
    E(mangrove_hanging_sign, HangingSign);
    E(mangrove_wall_hanging_sign, WallHangingSign);
    E(oak_hanging_sign, HangingSign);
    E(oak_wall_hanging_sign, WallHangingSign);
    E(spruce_hanging_sign, HangingSign);
    E(spruce_wall_hanging_sign, WallHangingSign);
    E(warped_hanging_sign, HangingSign);
    E(warped_wall_hanging_sign, WallHangingSign);

    E(chiseled_bookshelf, ChiseledBookshelf);

    E(bamboo_slab, Slab("bamboo_double_slab"));
    E(bamboo_mosaic_slab, Slab("bamboo_mosaic_double_slab"));
    E(decorated_pot, Converter(Same, DirectionNorth0East1South2West3FromFacing));
    E(torchflower_crop, Converter(Same, GrowthFromAge));
    E(jigsaw, Jigsaw);
    E(cherry_slab, Slab("cherry_double_slab"));
    E(pink_petals, PinkPetals);
    E(cherry_wood, axisToPillarAxis);
    E(stripped_cherry_wood, axisToPillarAxis);
    E(cherry_log, axisToPillarAxis);
    E(stripped_cherry_log, axisToPillarAxis);
    E(cherry_stairs, Stairs());
    E(cherry_button, button);
    E(cherry_hanging_sign, HangingSign);
    E(cherry_wall_hanging_sign, WallHangingSign);
    E(cherry_sign, Sign("cherry"));
    E(cherry_wall_sign, WallSign("cherry"));
    E(cherry_door, door);
    E(cherry_fence_gate, fenceGate);
    E(cherry_fence, Identity);
    E(cherry_trapdoor, trapdoor);
    E(cherry_sapling, Converter(Same, StageToAgeBit));
    E(cherry_pressure_plate, pressurePlate);
    E(cherry_leaves, leaves);
#undef E

    return table;
  }

  static CompoundTagPtr PinkPetals(Block const &block, CompoundTagConstPtr const &tile) {
    auto c = New(block.fName, true);
    auto s = States();

    DirectionNorth2East3South0West1FromFacing(s, block);

    i32 flowerCount = Wrap(strings::ToI32(block.property("flower_amount", "1")), 1);
    i32 growth = ClosedRange<i32>::Clamp(flowerCount - 1, 0, 3);
    s->set("growth", Int(growth));

    return AttachStates(c, s);
  }

  static CompoundTagPtr Jigsaw(Block const &block, CompoundTagConstPtr const &tile) {
    auto c = New(block.fName, true);
    auto s = States();
    auto orientation = block.property("orientation", "north_up");
    // orientation(J) facing_direction(B) rotation(B)
    // up_south       1                   2
    // up_west        1                   3
    // up_north       1                   0
    // up_east        1                   1
    // south_up       3                   0
    // east_up        5                   0
    // north_up       2                   0
    // west_up        4                   0
    // down_south     0                   2
    // down_north     0                   0
    auto tokens = mcfile::String::Split(std::string(orientation), '_');
    if (tokens.size() == 2) {
      Facing6 f6 = Facing6FromJavaName(tokens[0]);
      int facingDirection = BedrockFacingDirectionAFromFacing6(f6);
      Facing4 f4 = Facing4FromJavaName(tokens[1]);
      int rotation = North0East1South2West3FromFacing4(f4);
      if (tokens[1] == "up" || tokens[1] == "down") {
        rotation = 0;
      }
      s->set("facing_direction", Int(facingDirection));
      s->set("rotation", Int(rotation));
    }
    return AttachStates(c, s);
  }

  static CompoundTagPtr ChiseledBookshelf(Block const &block, CompoundTagConstPtr const &tile) {
    auto c = New(block.fName, true);
    auto s = States();
    int stored = 0;
    for (int i = 0; i < 6; i++) {
      bool occupied = block.property("slot_" + std::to_string(i) + "_occupied", "false") == "true";
      if (occupied) {
        stored++;
      }
    }
    s->set("books_stored", Int(stored));
    int direction = BedrockDirectionFromFacing4(Facing4FromJavaName(block.property("facing", "north")));
    s->set("direction", Int(direction));

    int lastInteractedSlot = 0;
    if (tile) {
      lastInteractedSlot = tile->int32("last_interacted_slot", 0);
    }
    s->set("last_interacted_slot", Int(lastInteractedSlot));

    return AttachStates(c, s);
  }

  static CompoundTagPtr WallHangingSign(Block const &block, CompoundTagConstPtr const &) {
    auto c = New(strings::Replace(block.fName, "_wall_", "_"), true);
    Facing4 f4 = Facing4FromJavaName(block.property("facing", "north"));
    int direction = 0;
    int facingDirection = 3;
    switch (f4) {
    case Facing4::South:
      direction = 0;
      facingDirection = 3;
      break;
    case Facing4::West:
      direction = 4;
      facingDirection = 0;
      break;
    case Facing4::North:
      direction = 8;
      facingDirection = 2;
      break;
    case Facing4::East:
      direction = 12;
      facingDirection = 1;
      break;
    }
    auto s = States();
    s->set("ground_sign_direction", Int(direction));
    s->set("facing_direction", Int(facingDirection));
    s->set("hanging", Bool(false));
    s->set("attach_bit", Bool(false));
    return AttachStates(c, s);
  }

  static CompoundTagPtr HangingSign(Block const &block, CompoundTagConstPtr const &) {
    auto c = New(block.fName, true);
    auto s = States();
    s->set("attached_bit", Bool(block.property("attached", "false") == "true"));
    s->set("ground_sign_direction", Int(Wrap(strings::ToI32(block.property("rotation", "0")), 0)));
    s->set("facing_direction", Int(0));
    s->set("hanging", Bool(true));
    return AttachStates(c, s);
  }

  static CompoundTagPtr Bamboo(Block const &block, CompoundTagConstPtr const &) {
    auto c = New(block.fName, true);
    auto s = States();
    BambooLeafSizeFromLeaves(s, block);
    auto stage = Wrap(strings::ToI32(block.property("stage", "0")), 0);
    s->set("age_bit", Bool(stage > 0));
    BambooStalkThicknessFromAge(s, block);
    return AttachStates(c, s);
  }

  static CompoundTagPtr FlowingLiquid(Block const &block) {
    auto c = New(block.fName, true);
    auto s = States();
    s->set("liquid_depth", Int(8));
    return AttachStates(c, s);
  }

  static CompoundTagPtr PistonArmCollision(Block const &block) {
    auto c = New("piston_arm_collision");
    auto s = States();
    auto direction = strings::ToI32(block.property("facing_direction", "0"));
    s->set("facing_direction", Int(Wrap(direction, 0)));
    return AttachStates(c, s);
  }

  static CompoundTagPtr StickyPistonArmCollision(Block const &block) {
    auto c = New("sticky_piston_arm_collision");
    auto s = States();
    auto direction = strings::ToI32(block.property("facing_direction", "0"));
    s->set("facing_direction", Int(Wrap(direction, 0)));
    return AttachStates(c, s);
  }

  static CompoundTagPtr MovingPiston(Block const &block, CompoundTagConstPtr const &) {
    auto c = New("moving_block");
    auto s = States();
    return AttachStates(c, s);
  }

  static CompoundTagPtr PistonHead(Block const &block, CompoundTagConstPtr const &) {
    auto type = block.property("type", "normal");
    auto c = New(type == "normal" ? "piston_arm_collision" : "sticky_piston_arm_collision");
    auto f6 = Facing6FromJavaName(block.property("facing", ""));
    auto direction = BedrockFacingDirectionBFromFacing6(f6);
    auto s = States();
    s->set("facing_direction", Int(direction));
    return AttachStates(c, s);
  }

  static CompoundTagPtr BigDripleaf(Block const &block, CompoundTagConstPtr const &) {
    auto c = New("big_dripleaf");
    auto s = States();
    s->set("big_dripleaf_head", Bool(true));
    DirectionFromFacingA(s, block);
    auto tilt = block.property("tilt", "none");
    if (tilt == "none") {
      //nop
    } else if (tilt == "partial") {
      tilt = "partial_tilt";
    } else if (tilt == "full") {
      tilt = "full_tilt";
    } else if (tilt == "unstable") {
      //nop
    }
    s->set("big_dripleaf_tilt", String(tilt));
    return AttachStates(c, s);
  }

  static CompoundTagPtr RespawnAnchor(Block const &block, CompoundTagConstPtr const &) {
    auto c = New("respawn_anchor");
    auto s = States();
    auto charges = strings::ToI32(block.property("charges", "0"));
    i32 charge = 0;
    if (charges) {
      charge = *charges;
    }
    s->set("respawn_anchor_charge", Int(charge));
    return AttachStates(c, s);
  }

  static CompoundTagPtr Tripwire(Block const &block, CompoundTagConstPtr const &) {
    auto c = New("trip_wire");
    auto s = States();
    auto attached = block.property("attached", "false") == "true";
    auto disarmed = block.property("disarmed", "false") == "true";
    auto powered = block.property("powered", "false") == "true";
    s->set("attached_bit", Bool(attached));
    s->set("disarmed_bit", Bool(disarmed));
    s->set("powered_bit", Bool(powered));
    s->set("suspended_bit", Bool(true));
    return AttachStates(c, s);
  }

  static CompoundTagPtr Tnt(Block const &block, CompoundTagConstPtr const &) {
    auto c = New("tnt");
    auto s = States();
    auto unstable = block.property("unstable", "false") == "true";
    s->set("explode_bit", Bool(unstable));
    s->set("allow_underwater_bit", Bool(false));
    return AttachStates(c, s);
  }

  static CompoundTagPtr StructureBlock(Block const &block, CompoundTagConstPtr const &) {
    auto c = New("structure_block");
    auto s = States();
    auto mode = block.property("mode", "save");
    s->set("structure_block_type", String(mode));
    return AttachStates(c, s);
  }

  static CompoundTagPtr Scaffolding(Block const &block, CompoundTagConstPtr const &) {
    auto c = New("scaffolding");
    auto s = States();
    auto distance = strings::ToI32(block.property("distance", "0"));
    i32 stability = 0;
    if (distance) {
      stability = *distance;
    }
    s->set("stability", Int(stability));
    s->set("stability_check", Bool(true));
    return AttachStates(c, s);
  }

  static CompoundTagPtr BrewingStand(Block const &block, CompoundTagConstPtr const &) {
    auto c = New("brewing_stand");
    auto has0 = block.property("has_bottle_0", "false") == "true";
    auto has1 = block.property("has_bottle_1", "false") == "true";
    auto has2 = block.property("has_bottle_2", "false") == "true";
    auto s = States();
    s->set("brewing_stand_slot_a_bit", Bool(has0));
    s->set("brewing_stand_slot_b_bit", Bool(has1));
    s->set("brewing_stand_slot_c_bit", Bool(has2));
    return AttachStates(c, s);
  }

  static CompoundTagPtr CaveVines(Block const &block, CompoundTagConstPtr const &) {
    bool berries = block.property("berries", "false") == "true";
    auto age = Wrap(strings::ToI32(block.property("age", "1")), 1);
    auto c = New(berries ? "cave_vines_head_with_berries" : "cave_vines");
    auto s = States();
    s->set("growing_plant_age", Int(age));
    return AttachStates(c, s);
  }

  static CompoundTagPtr CaveVinesPlant(Block const &block, CompoundTagConstPtr const &) {
    bool berries = block.property("berries", "false") == "true";
    auto c = New(berries ? "cave_vines_body_with_berries" : "cave_vines");
    auto s = States();
    s->set("growing_plant_age", Int(24));
    return AttachStates(c, s);
  }

  static PropertyType Candles(Block const &block) {
    auto candles = block.property("candles", "1");
    auto num = strings::ToI32(candles);
    int i = 0;
    if (num) {
      i = std::clamp(*num, 1, 4) - 1;
    }
    return Int(i);
  }

  static CompoundTagPtr Light(Block const &b, CompoundTagConstPtr const &) {
    auto c = New("light_block");
    auto s = States();
    auto level = strings::ToI32(b.property("level", "15"));
    s->set("block_light_level", Int(level ? *level : 15));
    return AttachStates(c, s);
  }

  static void Debug(CompoundTagPtr const &s, Block const &b) {
    (void)s;
  }

  static CompoundTagPtr PointedDripstone(Block const &b, CompoundTagConstPtr const &) {
    auto c = New("pointed_dripstone");
    auto s = States();
    auto thickness = b.property("thickness", "tip");
    if (thickness == "tip_merge") {
      thickness = "merge";
    }
    auto direction = b.property("vertical_direction", "down");
    s->set("dripstone_thickness", String(thickness));
    s->set("hanging", Bool(direction == "down"));
    return AttachStates(c, s);
  }

  static void Conditional(CompoundTagPtr const &s, Block const &b) {
    auto conditional = b.property("conditional", "false");
    s->set("conditional_bit", Bool(conditional == "true"));
  }

  static void BambooStalkThicknessFromAge(CompoundTagPtr const &s, Block const &b) {
    auto age = b.property("age", "0");
    if (age == "0") {
      s->set("bamboo_stalk_thickness", String("thin"));
    } else {
      s->set("bamboo_stalk_thickness", String("thick"));
    }
  }

  static void HoneyLevel(CompoundTagPtr const &s, Block const &b) {
    auto v = strings::ToI32(b.property("honey_level", "0"));
    i32 level = v ? *v : 0;
    s->set("honey_level", Int(level));
  }

  static void TurtleEggCount(CompoundTagPtr const &s, Block const &b) {
    auto eggs = b.property("eggs", "1");
    std::string eggCount = "one_egg";
    if (eggs == "1") {
      eggCount = "one_egg";
    } else if (eggs == "2") {
      eggCount = "two_egg";
    } else if (eggs == "3") {
      eggCount = "three_egg";
    } else if (eggs == "4") {
      eggCount = "four_egg";
    }
    s->set("turtle_egg_count", String(eggCount));
  }

  static void TurtleEggCrackedState(CompoundTagPtr const &s, Block const &b) {
    auto hatch = b.property("hatch", "0");
    std::string state = "no_cracks";
    if (hatch == "0") {
      state = "no_cracks";
    } else if (hatch == "1") {
      state = "cracked";
    } else if (hatch == "2") {
      state = "max_cracked";
    }
    s->set("cracked_state", String(state));
  }

  static void WallSkullFacingDirection(CompoundTagPtr const &s, Block const &b) {
    auto facing = b.property("facing", "");
    i32 direction = 1;
    if (facing == "south") {
      direction = 3;
    } else if (facing == "east") {
      direction = 5;
    } else if (facing == "north") {
      direction = 2;
    } else if (facing == "west") {
      direction = 4;
    }
    s->set("facing_direction", Int(direction));
  }

  static void CauldronFillLevelFromLevel(CompoundTagPtr const &s, Block const &b) {
    auto level = Wrap(strings::ToI32(b.property("level", "0")), 0);
    s->set("fill_level", Int(level * 2));
  }

  static void BiteCounterFromBites(CompoundTagPtr const &s, Block const &b) {
    auto bites = Wrap(strings::ToI32(b.property("bites", "0")), 0);
    s->set("bite_counter", Int(bites));
  }

  static void DragDownFromDrag(CompoundTagPtr const &s, Block const &b) {
    auto drag = b.property("drag", "true") == "true";
    s->set("drag_down", Bool(drag));
  }

  static void GrowthFromAge(CompoundTagPtr const &s, Block const &b) {
    auto age = Wrap(strings::ToI32(b.property("age", "0")), 0);
    i32 growth = 0;
    switch (age) {
    case 0:
      growth = 0;
      break;
    case 1:
      growth = 2;
      break;
    case 2:
      growth = 4;
      break;
    case 3:
      growth = 7;
      break;
    }
    s->set("growth", Int(growth));
  }

  static void AgeBitFromAge(CompoundTagPtr const &s, Block const &b) {
    auto age = b.property("age", "0");
    s->set("age", Bool(age == "1"));
  }

  static void BambooLeafSizeFromLeaves(CompoundTagPtr const &s, Block const &b) {
    auto leaves = b.property("leaves", "none");
    std::string size = "no_leaves";
    if (leaves == "none") {
      size = "no_leaves";
    } else if (leaves == "large") {
      size = "large_leaves";
    } else if (leaves == "small") {
      size = "small_leaves";
    }
    s->set("bamboo_leaf_size", String(size));
  }

  static PropertyType Axis(Block const &b) {
    auto axis = b.property("axis", "x");
    return String(axis);
  }

  static void RailDirectionFromShape(CompoundTagPtr const &s, Block const &b) {
    auto shape = b.property("shape", "north_south");
    i32 direction = 0;
    if (shape == "north_south") {
      direction = 0;
    } else if (shape == "east_west") {
      direction = 1;
    } else if (shape == "ascending_east") {
      direction = 2;
    } else if (shape == "ascending_south") {
      direction = 5;
    } else if (shape == "ascending_west") {
      direction = 3;
    } else if (shape == "ascending_north") {
      direction = 4;
    } else if (shape == "north_east") {
      direction = 9;
    } else if (shape == "north_west") {
      direction = 8;
    } else if (shape == "south_east") {
      direction = 6;
    } else if (shape == "south_west") {
      direction = 7;
    }
    s->set("rail_direction", Int(direction));
  }

  static void OutputSubtractBitFromMode(CompoundTagPtr const &s, Block const &b) {
    auto mode = b.property("mode", "compare");
    s->set("output_subtract_bit", Bool(mode == "subtract"));
  }

  static PropertyType Delay(Block const &b) {
    auto delay = Wrap(strings::ToI32(b.property("delay", "1")), 1);
    return Int(delay - 1);
  }

  static std::string ComparatorName(Block const &b) {
    auto powered = b.property("powered", "false") == "true";
    if (powered) {
      return "minecraft:powered_comparator";
    } else {
      return "minecraft:unpowered_comparator";
    }
  }

  static std::string RepeaterName(Block const &b) {
    auto powered = b.property("powered", "false") == "true";
    if (powered) {
      return "minecraft:powered_repeater";
    } else {
      return "minecraft:unpowered_repeater";
    }
  }

  static std::string PrefixLit(Block const &b) {
    auto lit = b.property("lit", "false") == "true";
    if (lit) {
      auto name = Namespace::Remove(b.fName);
      return "minecraft:lit_" + std::string(name);
    } else {
      return b.name();
    }
  }

  static NamingFunction PrefixUnlit(std::string name) {
    return [name](Block const &b) {
      auto lit = b.property("lit", "false") == "true";
      if (lit) {
        return "minecraft:" + name;
      } else {
        return "minecraft:unlit_" + name;
      }
    };
  }

  static void DoorHingeBitFromHinge(CompoundTagPtr const &s, Block const &b) {
    auto hinge = b.property("hinge", "left");
    s->set("door_hinge_bit", Bool(hinge == "right"));
  }

  static void ToggleBitFromEnabled(CompoundTagPtr const &s, Block const &b) {
    auto enabled = b.property("enabled", "true") == "true";
    s->set("toggle_bit", Bool(!enabled));
  }

  static std::string DaylightDetectorName(Block const &b) {
    auto inverted = b.property("inverted", "false") == "true";
    if (inverted) {
      return "minecraft:daylight_detector_inverted";
    } else {
      return "minecraft:daylight_detector";
    }
  }

  static PropertyType Attached(Block const &b) { return Bool(b.property("attached", "false") == "true"); }

  static void ButtonFacingDirection(CompoundTagPtr const &s, Block const &b) {
    auto face = b.property("face", "wall");
    auto facing = b.property("facing", "north");
    i32 direction = 0;
    if (face == "floor") {
      direction = 1;
    } else if (face == "ceiling") {
      direction = 0;
    } else {
      if (facing == "south") {
        direction = 3;
      } else if (facing == "north") {
        direction = 2;
      } else if (facing == "east") {
        direction = 5;
      } else if (facing == "west") {
        direction = 4;
      }
    }
    s->set("facing_direction", Int(direction));
  }

  static PropertyType Power(Block const &b) { return Int(Wrap(strings::ToI32(b.property("power", "0")), 0)); }

  static PropertyType InWall(Block const &b) { return Bool(b.property("in_wall", "false") == "true"); }

  static void LeverDirection(CompoundTagPtr const &s, Block const &b) {
    auto face = b.property("face", "wall");
    auto facing = b.property("facing", "north");
    std::string result;
    if (face == "floor") {
      if (facing == "west" || facing == "east") {
        result = "up_east_west";
      } else {
        result = "up_north_south";
      }
    } else if (face == "ceiling") {
      if (facing == "west" || facing == "east") {
        result = "down_east_west";
      } else {
        result = "down_north_south";
      }
    } else {
      result = facing;
    }
    s->set("lever_direction", String(result));
  }

  static PropertyType Triggered(Block const &b) {
    bool v = b.property("triggered", "false") == "true";
    return Bool(v);
  }

  static std::string GetAttachment(std::string_view const &attachment) {
    if (attachment == "floor") {
      return "standing";
    } else if (attachment == "ceiling") {
      return "hanging";
    } else if (attachment == "double_wall") {
      return "multiple";
    } else if (attachment == "single_wall") {
      return "side";
    }
    return std::string(attachment);
  }

  static CompoundTagPtr New(std::string_view const &name, bool nameIsFull = false) {
    using namespace std;
    auto tag = Compound();
    string fullName = nameIsFull ? string(name) : "minecraft:"s + string(name);
    tag->set("name", String(fullName));
    tag->set("version", Int(kBlockDataVersion));
    return tag;
  }

  static CompoundTagPtr States() { return Compound(); }

  static CompoundTagPtr AttachStates(CompoundTagPtr const &data, CompoundTagPtr const &s) {
    data->set("states", s);
    return data;
  }
};

CompoundTagPtr BlockData::From(std::shared_ptr<mcfile::je::Block const> const &block, CompoundTagConstPtr const &tile) {
  using namespace std;
  static unique_ptr<vector<Impl::AnyConverter> const> const table(Impl::CreateConverterTable());
  if (!block) {
    return Air();
  }
  u32 index = static_cast<u32>(block->fId);
  if (index < table->size()) {
    Impl::AnyConverter func = (*table)[index];
    if (func) {
      return func(*block, tile);
    }
  }
  // "j2b:sticky_piston_arm_collision" is created at Converter::PreprocessChunk
  if (block->fName == "j2b:sticky_piston_arm_collision") {
    return Impl::StickyPistonArmCollision(*block);
  } else if (block->fName == "j2b:piston_arm_collision") {
    return Impl::PistonArmCollision(*block);
  } else if (block->fName == "minecraft:flowing_water" || block->fName == "minecraft:flowing_lava") {
    return Impl::FlowingLiquid(*block);
  } else {
    return Impl::Identity(*block, nullptr);
  }
}

CompoundTagPtr BlockData::Air() {
  static CompoundTagPtr const air = Make("air");
  return air;
}

CompoundTagPtr BlockData::Make(std::string const &name) {
  auto tag = Impl::New(name);
  auto states = Impl::States();
  tag->set("states", states);
  return tag;
}

i32 BlockData::GetFacingDirectionAFromFacing(mcfile::je::Block const &block) {
  auto facing = block.property("facing", "north");
  auto f6 = Facing6FromJavaName(facing);
  return BedrockFacingDirectionAFromFacing6(f6);
}

} // namespace je2be::tobe
