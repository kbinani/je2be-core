#include "java/_block-data.hpp"

#include <je2be/strings.hpp>

#include "_closed-range.hpp"
#include "_data-version.hpp"
#include "_namespace.hpp"
#include "_optional.hpp"
#include "_props.hpp"
#include "block/_trial-spawner.hpp"
#include "enums/_facing4.hpp"
#include "enums/_facing6.hpp"
#include "java/_versions.hpp"

namespace je2be::java {

class BlockData::Impl {
public:
  Impl() = delete;

  using Block = mcfile::je::Block;

  using PropertyType = std::shared_ptr<Tag>;

  using NamingFunction = std::function<std::u8string(Block const &, Options const &)>;
  using PropertyPickupFunction = std::function<void(CompoundTagPtr const &, Block const &, Options const &)>;

  using AnyConverter = std::function<CompoundTagPtr(Block const &, CompoundTagConstPtr const &, Options const &)>;

  class Converter {
  public:
    template <class... Arg>
    Converter(NamingFunction name, Arg... args) : fName(name), fProperties(std::initializer_list<PropertyPickupFunction>{args...}) {}

    CompoundTagPtr operator()(Block const &block, CompoundTagConstPtr const &tile, Options const &options) const {
      using namespace std;
      u8string name = fName(block, options);
      auto tag = New(name, true);
      auto states = States();
      for (auto const &p : fProperties) {
        p(states, block, options);
      }
      tag->set(u8"states", states);
      return tag;
    }

  private:
    NamingFunction const fName;
    std::vector<PropertyPickupFunction> const fProperties;
  };

  static CompoundTagPtr Identity(mcfile::je::Block const &block, CompoundTagConstPtr const &, Options const &o) {
    auto tag = New(block.fName, true);
    auto states = States();
    tag->set(u8"states", states);
    return tag;
  }

  static NamingFunction Name(std::u8string const &name) {
    return [=](Block const &, Options const &o) { return u8"minecraft:" + name; };
  }

  static std::u8string Same(Block const &block, Options const &o) { return block.name(); }

  static NamingFunction SlabName(std::u8string const &name, std::u8string const &tail) {
    return [=](Block const &block, Options const &o) {
      auto t = block.property(u8"type", u8"bottom");
      return t == u8"double" ? (u8"minecraft:double_" + name + u8"_slab" + tail) : (u8"minecraft:" + name + u8"_slab" + tail);
    };
  }

  static NamingFunction ChangeWhenDoubleType(std::u8string const &doubleName) {
    return [=](Block const &block, Options const &o) {
      auto t = block.property(u8"type", u8"bottom");
      return t == u8"double" ? Namespace::Add(doubleName) : Namespace::Add(strings::Remove(doubleName, u8"double_"));
    };
  }

  static PropertyPickupFunction AddStringProperty(std::u8string const &name, std::u8string const &value) {
    return [=](CompoundTagPtr const &s, Block const &b, Options const &o) { s->set(name, value); };
  }

  static PropertyPickupFunction AddBoolProperty(std::u8string const &name, bool v) {
    return [=](CompoundTagPtr const &s, Block const &b, Options const &o) { s->set(name, Bool(v)); };
  }

  static PropertyPickupFunction AddIntProperty(std::u8string const &name, i32 v) {
    return [=](CompoundTagPtr const &s, Block const &b, Options const &o) { s->set(name, Int(v)); };
  }

  static PropertyPickupFunction AddByteProperty(std::u8string const &name, i8 v) {
    return [=](CompoundTagPtr const &s, Block const &b, Options const &o) { s->set(name, Byte(v)); };
  }

  static void AxisToPillarAxis(CompoundTagPtr const &s, Block const &block, Options const &o) {
    auto v = block.property(u8"axis", u8"y");
    s->set(u8"pillar_axis", std::u8string(v));
  }

  static void PersistentAndDistanceToPersistentBitAndUpdateBit(CompoundTagPtr const &s, Block const &block, Options const &o) {
    auto persistent = block.property(u8"persistent", u8"false") == u8"true";
    auto distance = Wrap(strings::ToI32(block.property(u8"distance", u8"7")), 7);
    s->set(u8"persistent_bit", Bool(persistent));
    if (o.fItem) {
      s->set(u8"update_bit", Bool(false));
    } else {
      // NOTE:
      //  Java: leaves decay when distance > 6
      //  Bedrock: leaves decay when distance > 4
      // Set update_bit to false for leaves with distance = 5, 6 not to decay as far as possible after conversion
      s->set(u8"update_bit", Bool(distance > 6 && !persistent));
    }
  }

  static void TypeToVerticalHalf(CompoundTagPtr const &s, Block const &block, Options const &o) {
    auto t = block.property(u8"type", u8"bottom");
    if (t == u8"double") {
      s->set(u8"minecraft:vertical_half", u8"bottom");
    } else {
      s->set(u8"minecraft:vertical_half", std::u8string(t));
    }
  }

  static PropertyPickupFunction AddStoneSlabType(std::u8string const &number, std::u8string const &type) {
    auto typeKey = number.empty() ? u8"stone_slab_type" : u8"stone_slab_type_" + number;
    return [=](CompoundTagPtr const &s, Block const &b, Options const &o) { s->set(typeKey, type); };
  }

  static void UpperBlockBitToHalf(CompoundTagPtr const &s, Block const &block, Options const &o) {
    auto half = block.property(u8"half", u8"lower");
    s->set(u8"upper_block_bit", Bool(half == u8"upper"));
  }

  static PropertyPickupFunction UpperBlockBitToHalfByItemDefault(bool upperBlockBit) {
    return [=](CompoundTagPtr const &s, Block const &block, Options const &o) {
      if (o.fItem) {
        s->set(u8"upper_block_bit", Bool(upperBlockBit));
      } else {
        auto half = block.property(u8"half", u8"lower");
        s->set(u8"upper_block_bit", Bool(half == u8"upper"));
      }
    };
  }

  static void WaterloggedToDeadBit(CompoundTagPtr const &s, Block const &block, Options const &o) {
    auto waterlogged = block.property(u8"waterlogged", u8"true");
    s->set(u8"dead_bit", Bool(waterlogged == u8"false"));
  }

  static void PicklesToClusterCount(CompoundTagPtr const &s, Block const &block, Options const &o) {
    auto pickles = block.property(u8"pickles", u8"1");
    auto cluster = Wrap(strings::ToI32(pickles), 1) - 1;
    s->set(u8"cluster_count", Int(cluster));
  }

  static void HalfToSeagrassType(CompoundTagPtr const &s, Block const &block, Options const &o) {
    auto half = block.property(u8"half", u8"lower");
    auto type = half == u8"lower" ? u8"double_bot" : u8"double_top";
    s->set(u8"sea_grass_type", type);
  }

  static void SculkSensorPhase(CompoundTagPtr const &s, Block const &block, Options const &o) {
    auto phaseJ = block.property(u8"sculk_sensor_phase", u8"inactive");
    i32 phaseB = 0;
    if (phaseJ == u8"inactive") {
      phaseB = 0;
    } else if (phaseJ == u8"active") {
      phaseB = 1;
    } else if (phaseJ == u8"cooldown") {
      phaseB = 2;
    }
    s->set(u8"sculk_sensor_phase", Int(phaseB));
  }

  static PropertyType Age(Block const &block) {
    auto age = Wrap(strings::ToI32(block.property(u8"age", u8"0")), 0);
    return Int(age);
  }

  static PropertyPickupFunction Name(std::function<PropertyType(Block const &)> func, std::u8string const &name) {
    return [=](CompoundTagPtr const &s, Block const &block, Options const &o) {
      if (auto v = func(block); v) {
        s->set(name, v);
      }
    };
  }

  static PropertyType Level(Block const &block) {
    auto level = Wrap(strings::ToI32(block.property(u8"level", u8"0")), 0);
    return Int(level);
  }

  static void VineDirectionBits(CompoundTagPtr const &s, Block const &block, Options const &o) {
    auto east = block.property(u8"east", u8"false") == u8"true";
    auto north = block.property(u8"north", u8"false") == u8"true";
    auto south = block.property(u8"south", u8"false") == u8"true";
    // auto up = block.property(u8"up", u8"false") == u8"true";
    auto west = block.property(u8"west", u8"false") == u8"true";
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
    s->set(u8"vine_direction_bits", Int(direction));
  }

  static PropertyPickupFunction MultiFaceDirectionBitsByItemDefault(int multiFaceDirectionBits) {
    return [=](CompoundTagPtr const &s, Block const &block, Options const &o) {
      if (o.fItem) {
        s->set(u8"multi_face_direction_bits", Int(multiFaceDirectionBits));
        return;
      }
      auto down = block.property(u8"down", u8"false") == u8"true";
      auto up = block.property(u8"up", u8"false") == u8"true";
      auto east = block.property(u8"east", u8"false") == u8"true";
      auto west = block.property(u8"west", u8"false") == u8"true";
      auto north = block.property(u8"north", u8"false") == u8"true";
      auto south = block.property(u8"south", u8"false") == u8"true";
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
      s->set(u8"multi_face_direction_bits", Int(bits));
    };
  }

  static Converter Subtype(std::u8string const &name, std::u8string const &subtypeTitle, std::u8string const &subtype) { return Converter(Name(name), AddStringProperty(subtypeTitle, subtype)); }

  // static Converter StoneLegacy(std::u8string const &stoneType) { return Subtype(u8"stone", u8"stone_type", stoneType); }

  static Converter Dirt(std::u8string const &dirtType) { return Subtype(u8"dirt", u8"dirt_type", dirtType); }

  // static Converter LogLegacy(std::u8string const &type) { return Converter(Name(u8"log"), AddStringProperty(u8"old_log_type", type), AxisToPillarAxis); }

  // static Converter Log2Legacy(std::u8string const &type) { return Converter(Name(u8"log2"), AddStringProperty(u8"new_log_type", type), AxisToPillarAxis); }

  static Converter WoodLegacy(std::u8string const &type, bool stripped) { return Converter(Name(u8"wood"), AxisToPillarAxis, AddStringProperty(u8"wood_type", type), AddBoolProperty(u8"stripped_bit", stripped)); }

  static Converter LeavesLegacy(std::u8string const &type) { return Converter(Name(u8"leaves"), AddStringProperty(u8"old_leaf_type", type), PersistentAndDistanceToPersistentBitAndUpdateBit); }

  static Converter Leaves2Legacy(std::u8string const &type) { return Converter(Name(u8"leaves2"), AddStringProperty(u8"new_leaf_type", type), PersistentAndDistanceToPersistentBitAndUpdateBit); }

  static Converter WoodenSlabLegacy(std::u8string const &type) { return Converter(SlabName(u8"wooden", u8""), TypeToVerticalHalf, AddStringProperty(u8"wood_type", type)); }

  static Converter StoneSlab(std::u8string const &type) { return StoneSlabNumbered(u8"", type); }

  static Converter StoneSlab2(std::u8string const &type) { return StoneSlabNumbered(u8"2", type); }

  static Converter StoneSlab3(std::u8string const &type) { return StoneSlabNumbered(u8"3", type); }

  static Converter StoneSlab4(std::u8string const &type) { return StoneSlabNumbered(u8"4", type); }

  static Converter StoneSlabNumbered(std::u8string const &number, std::u8string const &type) { return Converter(SlabName(u8"stone_block", number), TypeToVerticalHalf, AddStoneSlabType(number, type)); }

  static Converter Slab(std::u8string const &doubleName) {
    return Converter(ChangeWhenDoubleType(doubleName), TypeToVerticalHalf);
  }

  static AnyConverter SlabWithStoneTypeWhenDouble(std::u8string const &normalName, std::u8string const &doubleName, std::u8string const &stoneSlabType) {
    return [=](Block const &block, CompoundTagConstPtr const &tile, Options const &o) {
      auto type = block.property(u8"type", u8"bottom");
      CompoundTagPtr d;
      auto s = States();
      if (type == u8"double") {
        d = New(doubleName);
        s->set(u8"stone_slab_type", String(stoneSlabType));
      } else {
        d = New(normalName);
      }
      TypeToVerticalHalf(s, block, o);
      return AttachStates(d, s);
    };
  }

  static Converter ShortGrass(std::u8string const &type) { return Converter(Name(u8"short_grass"), AddStringProperty(u8"tall_grass_type", type)); }

  static CompoundTagPtr TallGrass(Block const &b, CompoundTagConstPtr const &tile, Options const &o) {
    auto d = New(u8"tall_grass");
    auto s = States();
    UpperBlockBitToHalf(s, b, o);
    return AttachStates(d, s);
  }

  static Converter DoublePlant(std::u8string const &type) { return Converter(Name(u8"double_plant"), AddStringProperty(u8"double_plant_type", type), UpperBlockBitToHalf); }

  static Converter Rename(std::u8string const &name) { return Converter(Name(name)); }

  static Converter SeaPickle() { return Converter(Name(u8"sea_pickle"), WaterloggedToDeadBit, PicklesToClusterCount); }

  static Converter RedFlowerLegacy(std::u8string const &type) { return Converter(Name(u8"red_flower"), AddStringProperty(u8"flower_type", type)); }

  static Converter Kelp(std::optional<i32> age = std::nullopt) {
    if (age) {
      return Converter(Name(u8"kelp"), AddIntProperty(u8"kelp_age", *age));
    } else {
      return Converter(Name(u8"kelp"), Name(Age, u8"kelp_age"));
    }
  }

  static Converter Liquid(std::u8string const &type) { return Converter(Same, Name(Level, u8"liquid_depth")); }

  static Converter NetherVines(std::u8string const &type, std::optional<int> age = std::nullopt) {
    if (age) {
      return Converter(Name(type + u8"_vines"), AddIntProperty(type + u8"_vines_age", *age));
    } else {
      return Converter(Name(type + u8"_vines"), Name(Age, type + u8"_vines_age"));
    }
  }

  static CompoundTagPtr Null(Block const &block, CompoundTagConstPtr const &, Options const &o) {
    // not present in bedrock
    return New(u8"air");
  }

  static void StairsDirectionFromFacing(CompoundTagPtr const &s, Block const &block, Options const &o) {
    auto facing = block.property(u8"facing", u8"east");
    i32 direction = 0;
    if (facing == u8"east") {
      direction = 0;
    } else if (facing == u8"south") {
      direction = 2;
    } else if (facing == u8"north") {
      direction = 3;
    } else if (facing == u8"west") {
      direction = 1;
    }
    s->set(u8"weirdo_direction", Int(direction));
  }

  static void HalfToUpsideDownBit(CompoundTagPtr const &s, Block const &block, Options const &o) {
    auto half = block.property(u8"half", u8"bottom");
    s->set(u8"upside_down_bit", Bool(half == u8"top"));
  }

  static Converter Stairs(std::optional<std::u8string> name = std::nullopt) {
    NamingFunction naming = name ? Name(*name) : Same;
    return Converter(naming, HalfToUpsideDownBit, StairsDirectionFromFacing);
  }

  static Converter Sponge(std::u8string const &type) { return Subtype(u8"sponge", u8"sponge_type", type); }

  // before 1.19.70, color was stored in states:
  // static Converter Wool(std::u8string const &color) { return Subtype(u8"wool", u8"color", color); }

  static Converter RedSandstone(std::u8string const &type) { return Subtype(u8"red_sandstone", u8"sand_stone_type", type); }

  static Converter Sandstone(std::u8string const &type) { return Subtype(u8"sandstone", u8"sand_stone_type", type); }

  static Converter Sand(std::u8string const &type) { return Subtype(u8"sand", u8"sand_type", type); }

  static Converter QuartzBlock(std::u8string const &type) { return Converter(Name(u8"quartz_block"), AxisToPillarAxis, AddStringProperty(u8"chisel_type", type)); }

  // before 1.20.x, wood type was stored in states
  // static Converter Planks(std::u8string const &type) { return Subtype(u8"planks", u8"wood_type", type); }

  static PropertyType FacingA(Block const &block) {
    auto facing = block.property(u8"facing");
    Facing4 f4 = Facing4FromJavaName(facing);
    return Int(BedrockDirectionFromFacing4(f4));
  }

  static PropertyType FacingB(Block const &block) {
    auto facing = block.property(u8"facing");
    i32 direction = 0;
    if (facing == u8"south") {
      direction = 2;
    } else if (facing == u8"east") {
      direction = 0;
    } else if (facing == u8"west") {
      direction = 1;
    } else if (facing == u8"north") {
      direction = 3;
    }
    return Int(direction);
  }

  static PropertyType FacingC(Block const &b) {
    auto facing = b.property(u8"facing");
    i32 direction = 0;
    if (facing == u8"north") {
      direction = 3;
    } else if (facing == u8"east") {
      direction = 0;
    } else if (facing == u8"south") {
      direction = 1;
    } else if (facing == u8"west") {
      direction = 2;
    }
    return Int(direction);
  }

  static PropertyType FacingD(Block const &b) {
    auto facing = b.property(u8"facing");
    i32 direction = 0;
    if (facing == u8"north") {
      direction = 2;
    } else if (facing == u8"east") {
      direction = 1;
    } else if (facing == u8"south") {
      direction = 3;
    } else if (facing == u8"west") {
      direction = 0;
    }
    return Int(direction);
  }

  static void DirectionNorth0East1South2West3FromFacing(CompoundTagPtr const &s, Block const &block, Options const &o) {
    if (o.fItem) {
      s->set(u8"direction", Int(0));
      return;
    }
    auto f4 = Facing4FromJavaName(block.property(u8"facing"));
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
    s->set(u8"direction", Int(direction));
  }

  static void DirectionNorth2East3South0West1FromFacing(CompoundTagPtr const &s, Block const &block, Options const &o) {
    auto f4 = Facing4FromJavaName(block.property(u8"facing"));
    s->set(u8"direction", Int(North2East3South0West1FromFacing4(f4)));
  }

  static void DirectionFromFacingA(CompoundTagPtr const &s, Block const &block, Options const &o) {
    s->set(u8"direction", FacingA(block));
  }

  static void CardinalDirectionFromFacing4(CompoundTagPtr const &s, Block const &block, Options const &o) {
    s->set(u8"minecraft:cardinal_direction", std::u8string(block.property(u8"facing", u8"south")));
  }

  static PropertyPickupFunction CardinalDirectionFromFacing4ByItemDefault(Facing4 f4) {
    return [=](CompoundTagPtr const &s, Block const &block, Options const &o) {
      if (o.fItem) {
        s->set(u8"minecraft:cardinal_direction", JavaNameFromFacing4(f4));
      } else {
        CardinalDirectionFromFacing4(s, block, o);
      }
    };
  }

  static void DirectionFromFacingB(CompoundTagPtr const &s, Block const &block, Options const &o) {
    s->set(u8"direction", FacingB(block));
  }

  static void DirectionFromFacingC(CompoundTagPtr const &s, Block const &block, Options const &o) {
    s->set(u8"direction", FacingC(block));
  }

  static Converter LitPumpkin() { return Converter(Name(u8"lit_pumpkin"), CardinalDirectionFromFacing4); }

  static Converter Prismarine(std::u8string const &type) { return Subtype(u8"prismarine", u8"prismarine_block_type", type); }

  static Converter CoralBlock(std::u8string const &color, bool dead) { return Converter(Name(u8"coral_block"), AddStringProperty(u8"coral_color", color), AddBoolProperty(u8"dead_bit", dead)); }

  static void StageToAgeBit(CompoundTagPtr const &s, Block const &block, Options const &o) {
    auto stage = block.property(u8"stage", u8"0");
    s->set(u8"age_bit", Bool(stage == u8"1"));
  }

  static Converter SaplingLegacy(std::u8string const &type) { return Converter(Name(u8"sapling"), AddStringProperty(u8"sapling_type", type), StageToAgeBit); }

  static Converter StoneBrick(std::u8string const &type) { return Subtype(u8"stonebrick", u8"stone_brick_type", type); }

  static void LayersToHeight(CompoundTagPtr const &s, Block const &block, Options const &o) {
    auto layers = Wrap(strings::ToI32(block.property(u8"layers", u8"1")), 1);
    s->set(u8"height", Int(layers - 1));
  }

  static CompoundTagPtr FlowerPot(Block const &b, CompoundTagConstPtr const &tile, Options const &o) {
    bool update = false;
    if (tile) {
      if (auto item = tile->int32(u8"Item"); item) {
        update = true;
      }
    }
    auto d = New(u8"flower_pot");
    auto s = States();
    AddBoolProperty(u8"update_bit", update)(s, b, o);
    return AttachStates(d, s);
  }

  static CompoundTagPtr SnowLayer(Block const &b, CompoundTagConstPtr const &, Options const &o) {
    auto d = New(u8"snow_layer");
    auto s = States();
    LayersToHeight(s, b, o);
    AddBoolProperty(u8"covered_bit", false)(s, b, o);
    return AttachStates(d, s);
  }

  static CompoundTagPtr MangrovePropagule(Block const &b, CompoundTagConstPtr const &, Options const &o) {
    auto d = New(u8"mangrove_propagule");
    auto s = States();
    int age = Wrap(strings::ToI32(b.property(u8"age", u8"0")), 0);
    bool hanging = b.property(u8"hanging") == u8"true";
    s->set(u8"hanging", Bool(hanging));
    s->set(u8"propagule_stage", Int(age));
    return AttachStates(d, s);
  }

  static CompoundTagPtr SculkCatalyst(Block const &b, CompoundTagConstPtr const &, Options const &o) {
    auto d = New(u8"sculk_catalyst");
    auto s = States();
    auto bloom = b.property(u8"bloom") == u8"true";
    s->set(u8"bloom", Bool(bloom));
    return AttachStates(d, s);
  }

  static CompoundTagPtr SculkShrieker(Block const &b, CompoundTagConstPtr const &, Options const &o) {
    auto d = New(u8"sculk_shrieker");
    auto s = States();
    auto canSummon = b.property(u8"can_summon") == u8"true";
    auto shrieking = b.property(u8"shrieking") == u8"true";
    s->set(u8"can_summon", Bool(canSummon));
    s->set(u8"active", Bool(shrieking));
    return AttachStates(d, s);
  }

  static void EndRodFacingDirectionFromFacing(CompoundTagPtr const &s, Block const &block, Options const &o) {
    if (o.fItem) {
      s->set(u8"facing_direction", Int(0));
      return;
    }
    auto facing = block.property(u8"facing", u8"up");
    i32 direction;
    if (facing == u8"east") {
      direction = 4;
    } else if (facing == u8"south") {
      direction = 2;
    } else if (facing == u8"north") {
      direction = 3;
    } else if (facing == u8"down") {
      direction = 0;
    } else if (facing == u8"west") {
      direction = 5;
    } else { // up
      direction = 1;
    }
    s->set(u8"facing_direction", Int(direction));
  }

  static Converter AnyTorch(std::u8string const &prefix) { return Converter(Name(prefix + u8"torch"), AddStringProperty(u8"torch_facing_direction", u8"top")); }

  static std::u8string GetTorchFacingDirectionFromFacing(std::u8string_view const &facing) {
    if (facing == u8"east") {
      return u8"west";
    } else if (facing == u8"west") {
      return u8"east";
    } else if (facing == u8"north") {
      return u8"south";
    } else {
      return u8"north";
    }
  }

  static void TorchFacingDirectionFromFacing(CompoundTagPtr const &s, Block const &block, Options const &o) {
    auto facing = block.property(u8"facing", u8"north");
    auto direction = GetTorchFacingDirectionFromFacing(facing);
    s->set(u8"torch_facing_direction", direction);
  }

  static Converter AnyWallTorch(std::u8string const &prefix) { return Converter(Name(prefix + u8"torch"), TorchFacingDirectionFromFacing); }

  static Converter InfestedStone(std::u8string const &type) { return Subtype(u8"monster_egg", u8"monster_egg_stone_type", type); }

  static Converter Fence(std::u8string const &type) { return Subtype(u8"fence", u8"wood_type", type); }

  static PropertyType Moisture(Block const &block) {
    auto v = Wrap(strings::ToI32(block.property(u8"moisture", u8"0")), 0);
    return Int(v);
  }

  static bool On(std::u8string_view const &actual) {
    if (actual.empty()) {
      return true;
    }
    return actual == u8"true";
  }

  static bool Off(std::u8string_view const &actual) {
    if (actual.empty()) {
      return true;
    }
    return actual == u8"false";
  }

  static PropertyPickupFunction HugeMushroomBits(bool stem) {
    return [=](CompoundTagPtr const &s, Block const &block, Options const &o) {
      auto up = block.property(u8"up");
      auto down = block.property(u8"down");
      auto north = block.property(u8"north");
      auto east = block.property(u8"east");
      auto south = block.property(u8"south");
      auto west = block.property(u8"west");

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
      s->set(u8"huge_mushroom_bits", Int(bits));
    };
  }

  static Converter AnyMushroomBlock(std::u8string const &name, bool stem) { return Converter(Name(name), HugeMushroomBits(stem)); }

  static void EyeToEndPortalEyeBit(CompoundTagPtr const &s, Block const &block, Options const &o) {
    auto eye = block.property(u8"eye", u8"false") == u8"true";
    s->set(u8"end_portal_eye_bit", Bool(eye));
  }

  static void FacingDirectionAFromFacing(CompoundTagPtr const &s, Block const &block, Options const &o) {
    i32 direction = GetFacingDirectionAFromFacing(block);
    s->set(u8"facing_direction", Int(direction));
  }

  static PropertyPickupFunction FacingDirectionAFromFacingByItemDefault(int d) {
    return [=](CompoundTagPtr const &s, Block const &block, Options const &o) {
      i32 direction = o.fItem ? d : GetFacingDirectionAFromFacing(block);
      s->set(u8"facing_direction", Int(direction));
    };
  }

  static void BlockFaceFromFacing(CompoundTagPtr const &s, Block const &block, Options const &o) {
    s->set(u8"minecraft:block_face", std::u8string(block.property(u8"facing", u8"up")));
  }

  static void PistonFacingDirectionFromFacing6(CompoundTagPtr const &s, Block const &block, Options const &o) {
    auto facing = block.property(u8"facing", u8"");
    if (o.fItem) {
      s->set(u8"facing_direction", Int(1));
    } else {
      auto f6 = Facing6FromJavaName(facing);
      i32 direction = BedrockFacingDirectionBFromFacing6(f6);
      s->set(u8"facing_direction", Int(direction));
    }
  }

  static void Door(CompoundTagPtr const &s, Block const &block, Options const &o) {
    auto upper = s->boolean(u8"upper_block_bit", false);
    if (upper) {
      s->set(u8"direction", Int(0));
      s->set(u8"open_bit", Bool(false));
    }
  }

  static std::u8string GetWallConnectionType(std::u8string_view const &type) {
    if (type == u8"low") {
      return u8"short";
    } else if (type == u8"tall") {
      return u8"tall";
    } else {
      return u8"none";
    }
  }

  static PropertyPickupFunction WallConnectionType(std::u8string const &direction) {
    return [=](CompoundTagPtr const &s, Block const &b, Options const &o) {
      std::u8string beName = u8"wall_connection_type_" + direction;
      auto v = b.property(direction, u8"none");
      if (v == u8"true" || v == u8"false") {
        if (v == u8"true") {
          s->set(beName, u8"short");
        } else {
          s->set(beName, u8"none");
        }
      } else {
        auto type = GetWallConnectionType(v);
        s->set(beName, type);
      }
    };
  }

  static void WallPostBit(CompoundTagPtr const &s, Block const &b, Options const &o) {
    auto up = b.property(u8"up", u8"false") == u8"true";
    s->set(u8"wall_post_bit", Bool(up));
  }

  static Converter Wall(std::u8string const &type) { return Converter(Name(u8"cobblestone_wall"), WallPostBit, AddStringProperty(u8"wall_block_type", type), WallConnectionType(u8"east"), WallConnectionType(u8"north"), WallConnectionType(u8"south"), WallConnectionType(u8"west")); }

  static Converter Anvil(std::u8string const &damage) { return Converter(Name(u8"anvil"), AddStringProperty(u8"damage", damage), CardinalDirectionFromFacing4); }

  static Converter CoralLegacy(std::u8string const &type, bool dead) { return Converter(Name(u8"coral"), AddStringProperty(u8"coral_color", type), AddBoolProperty(u8"dead_bit", dead)); }

  static CompoundTagPtr CoralFan(mcfile::je::Block const &block, CompoundTagConstPtr const &, Options const &o) {
    auto d = New(block.fName, true);
    auto s = States();
    s->set(u8"coral_fan_direction", Int(o.fItem ? 0 : 1));
    return AttachStates(d, s);
  }

  static Converter CoralFanLegacy(std::u8string const &type, bool dead) {
    // before bedrock 1.20.81
    return Converter(Name(dead ? u8"coral_fan_dead" : u8"coral_fan"), AddStringProperty(u8"coral_color", type), CoralFanDirection);
  }

  static void CoralFanDirection(CompoundTagPtr const &s, Block const &b, Options const &o) {
    auto waterlogged = b.property(u8"waterlogged", u8"false") == u8"true";
    s->set(u8"coral_fan_direction", Int(waterlogged ? 1 : 0));
  }

  static Converter CoralWallFan(std::u8string const &tail, bool dead, i8 type) { return Converter(Name(u8"coral_fan_hang" + tail), AddByteProperty(u8"coral_hang_type_bit", type), Name(FacingD, u8"coral_direction"), AddBoolProperty(u8"dead_bit", dead)); }

  static Converter WallSign(std::optional<std::u8string> prefix = std::nullopt) {
    std::u8string name = prefix ? *prefix + u8"_wall_sign" : u8"wall_sign";
    return Converter(Name(name), FacingDirectionAFromFacing);
  }

  static PropertyType Rotation(Block const &block) {
    auto r = Wrap(strings::ToI32(block.property(u8"rotation", u8"0")), 0);
    return Int(r);
  }

  static Converter Sign(std::optional<std::u8string> prefix = std::nullopt) {
    std::u8string name = prefix ? *prefix + u8"_standing_sign" : u8"standing_sign";
    return Converter(Name(name), Name(Rotation, u8"ground_sign_direction"));
  }

  static void PartToHeadPieceBit(CompoundTagPtr const &s, Block const &block, Options const &o) {
    auto head = block.property(u8"part", u8"foot") == u8"head";
    s->set(u8"head_piece_bit", Bool(head));
  }

  static void OccupiedToOccupiedBit(CompoundTagPtr const &s, Block const &block, Options const &o) {
    auto occupied = block.property(u8"occupied", u8"false") == u8"true";
    s->set(u8"occupied_bit", Bool(occupied));
  }

  static PropertyType Open(Block const &block) { return Bool(block.property(u8"open", u8"false") == u8"true"); }

  static void GrindstoneFaceToAttachment(CompoundTagPtr const &s, Block const &block, Options const &o) {
    auto face = block.property(u8"face", u8"wall");
    std::u8string attachment;
    if (o.fItem) {
      attachment = u8"standing";
    } else if (face == u8"wall") {
      attachment = u8"side";
    } else if (face == u8"floor") {
      attachment = u8"standing";
    } else {
      attachment = u8"hanging";
    }
    s->set(u8"attachment", attachment);
  }

  static PropertyType Hanging(Block const &block) {
    auto hanging = block.property(u8"hanging", u8"false") == u8"true";
    return Bool(hanging);
  }

  static void BellAttachmentFromAttachment(CompoundTagPtr const &s, Block const &block, Options const &o) {
    auto attachment = block.property(u8"attachment", u8"floor");
    s->set(u8"attachment", GetAttachment(attachment));
  }

  static void BellDirectionFromFacing(CompoundTagPtr const &s, Block const &block, Options const &o) {
    auto facing = block.property(u8"facing", u8"north");
    i32 direction = 0;
    if (facing == u8"north") {
      direction = 0;
    } else if (facing == u8"east") {
      direction = 1;
    } else if (facing == u8"south") {
      direction = 2;
    } else {
      direction = 3;
    }
    s->set(u8"direction", Int(direction));
  }

  static PropertyType Powered(Block const &block) {
    auto p = block.property(u8"powered", u8"false") == u8"true";
    return Bool(p);
  }

  static void RedstoneSignalFromPowered(CompoundTagPtr const &s, Block const &block, Options const &o) {
    auto powered = block.property(u8"powered", u8"false") == u8"true";
    s->set(u8"redstone_signal", Int(powered ? 1 : 0));
  }

  static PropertyType Lit(Block const &block) {
    auto l = block.property(u8"lit", u8"false") == u8"true";
    return Bool(l);
  }

  static std::function<PropertyType(Block const &)> BooleanProperty(std::u8string const &b) {
    return [=](Block const &block) {
      auto p = block.property(b, u8"false") == u8"true";
      return Bool(p);
    };
  }

  static std::function<PropertyType(Block const &)> BooleanPropertyOptional(std::u8string const &b) {
    return [=](Block const &block) -> ByteTagPtr {
      auto p = block.property(b, u8"");
      if (p == u8"true") {
        return Bool(true);
      } else if (p == u8"false") {
        return Bool(false);
      } else {
        return nullptr;
      }
    };
  }

  static std::function<PropertyType(Block const &)> StringProperty(std::u8string const &b, char8_t const *fallback = u8"") {
    return [=](Block const &block) {
      auto p = block.property(b, fallback);
      return String(p);
    };
  }

  static void LitToExtinguished(CompoundTagPtr const &s, Block const &block, Options const &o) {
    auto l = block.property(u8"lit", u8"false") == u8"true";
    s->set(u8"extinguished", Bool(!l));
  }

  static std::vector<AnyConverter> *CreateConverterTable() {
    using namespace std;
    Converter axisToPillarAxis(Same, AxisToPillarAxis);
    Converter directionFromFacing(Same, DirectionFromFacingA);
    Converter facingDirectionFromFacingA(Same, FacingDirectionAFromFacing);
    Converter blockFaceFromFacing(Same, BlockFaceFromFacing);
    Converter facingDirectionFromFacingB(Same, PistonFacingDirectionFromFacing6);
    Converter cardinalDirectionFromFacing4(Same, CardinalDirectionFromFacing4);

    auto table = new vector<AnyConverter>(mcfile::blocks::minecraft::minecraft_max_block_id);
#define E(__name, __func)                                                                               \
  assert(!(*table)[static_cast<u32>(mcfile::blocks::minecraft::__name)] && "converter is already set"); \
  if (string(#__func) != "Identity") {                                                                  \
    (*table)[static_cast<u32>(mcfile::blocks::minecraft::__name)] = __func;                             \
  }

    E(stone, Identity);
    E(granite, Identity);
    E(polished_granite, Identity);
    E(andesite, Identity);
    E(polished_andesite, Identity);
    E(diorite, Identity);
    E(polished_diorite, Identity);
    E(dirt, Dirt(u8"normal"));
    E(coarse_dirt, Dirt(u8"coarse"));
    E(grass_block, Identity);

    E(oak_log, axisToPillarAxis);
    E(spruce_log, axisToPillarAxis);
    E(birch_log, axisToPillarAxis);
    E(jungle_log, axisToPillarAxis);
    E(acacia_log, axisToPillarAxis);
    E(dark_oak_log, axisToPillarAxis);
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

    E(oak_wood, axisToPillarAxis);      // WoodLegacy(u8"oak", false));
    E(spruce_wood, axisToPillarAxis);   // WoodLegacy(u8"spruce", false));
    E(birch_wood, axisToPillarAxis);    // WoodLegacy(u8"birch", false));
    E(acacia_wood, axisToPillarAxis);   // WoodLegacy(u8"acacia", false));
    E(jungle_wood, axisToPillarAxis);   // WoodLegacy(u8"jungle", false));
    E(dark_oak_wood, axisToPillarAxis); // WoodLegacy(u8"dark_oak", false));
    Converter wood(Same, AxisToPillarAxis, AddBoolProperty(u8"stripped_bit", false));
    E(mangrove_wood, wood);
    E(stripped_oak_wood, axisToPillarAxis);      // WoodLegacy(u8"oak", true));
    E(stripped_spruce_wood, axisToPillarAxis);   // WoodLegacy(u8"spruce", true));
    E(stripped_birch_wood, axisToPillarAxis);    // WoodLegacy(u8"birch", true));
    E(stripped_acacia_wood, axisToPillarAxis);   // WoodLegacy(u8"acacia", true));
    E(stripped_jungle_wood, axisToPillarAxis);   // WoodLegacy(u8"jungle", true));
    E(stripped_dark_oak_wood, axisToPillarAxis); // WoodLegacy(u8"dark_oak", true));
    E(stripped_mangrove_wood, axisToPillarAxis);
    Converter leaves(Same, PersistentAndDistanceToPersistentBitAndUpdateBit);
    E(oak_leaves, leaves);      // LeavesLegacy(u8"oak"));
    E(spruce_leaves, leaves);   // LeavesLegacy(u8"spruce"));
    E(birch_leaves, leaves);    // LeavesLegacy(u8"birch"));
    E(jungle_leaves, leaves);   // LeavesLegacy(u8"jungle"));
    E(acacia_leaves, leaves);   // Leaves2Legacy(u8"acacia"));
    E(dark_oak_leaves, leaves); // Leaves2Legacy(u8"dark_oak"));
    E(crimson_hyphae, axisToPillarAxis);
    E(warped_hyphae, axisToPillarAxis);
    E(stripped_crimson_hyphae, axisToPillarAxis);
    E(stripped_warped_hyphae, axisToPillarAxis);
    E(stripped_crimson_stem, axisToPillarAxis);
    E(stripped_warped_stem, axisToPillarAxis);
    E(oak_slab, Slab(u8"oak_double_slab"));           // WoodenSlabLegacy(u8"oak"));
    E(birch_slab, Slab(u8"birch_double_slab"));       // WoodenSlabLegacy(u8"birch"));
    E(spruce_slab, Slab(u8"spruce_double_slab"));     // WoodenSlabLegacy(u8"spruce"));
    E(jungle_slab, Slab(u8"jungle_double_slab"));     // WoodenSlabLegacy(u8"jungle"));
    E(acacia_slab, Slab(u8"acacia_double_slab"));     // WoodenSlabLegacy(u8"acacia"));
    E(dark_oak_slab, Slab(u8"dark_oak_double_slab")); // WoodenSlabLegacy(u8"dark_oak"));
    E(petrified_oak_slab, Slab(u8"oak_double_slab")); // WoodenSlabLegacy(u8"oak"));
    E(stone_slab, StoneSlab4(u8"stone"));
    E(granite_slab, StoneSlab3(u8"granite"));
    E(andesite_slab, StoneSlab3(u8"andesite"));
    E(diorite_slab, StoneSlab3(u8"diorite"));
    E(cobblestone_slab, SlabWithStoneTypeWhenDouble(u8"cobblestone_slab", u8"double_stone_block_slab", u8"cobblestone"));
    E(stone_brick_slab, SlabWithStoneTypeWhenDouble(u8"stone_brick_slab", u8"double_stone_block_slab", u8"stone_brick"));
    E(brick_slab, SlabWithStoneTypeWhenDouble(u8"brick_slab", u8"double_stone_block_slab", u8"brick"));
    E(sandstone_slab, SlabWithStoneTypeWhenDouble(u8"sandstone_slab", u8"double_stone_block_slab", u8"sandstone"));
    E(smooth_sandstone_slab, StoneSlab2(u8"smooth_sandstone"));
    E(smooth_stone_slab, SlabWithStoneTypeWhenDouble(u8"smooth_stone_slab", u8"double_stone_block_slab", u8"smooth_stone"));
    E(nether_brick_slab, SlabWithStoneTypeWhenDouble(u8"nether_brick_slab", u8"double_stone_block_slab", u8"nether_brick"));
    E(quartz_slab, SlabWithStoneTypeWhenDouble(u8"quartz_slab", u8"double_stone_block_slab", u8"quartz"));
    E(smooth_quartz_slab, StoneSlab4(u8"smooth_quartz"));
    E(red_sandstone_slab, StoneSlab2(u8"red_sandstone"));
    E(smooth_red_sandstone_slab, StoneSlab3(u8"smooth_red_sandstone"));
    E(cut_red_sandstone_slab, StoneSlab4(u8"cut_red_sandstone"));
    E(mossy_cobblestone_slab, StoneSlab2(u8"mossy_cobblestone"));
    E(polished_diorite_slab, StoneSlab3(u8"polished_diorite"));
    E(mossy_stone_brick_slab, StoneSlab4(u8"mossy_stone_brick"));
    E(polished_granite_slab, StoneSlab3(u8"polished_granite"));
    E(dark_prismarine_slab, StoneSlab2(u8"prismarine_dark"));
    E(prismarine_brick_slab, StoneSlab2(u8"prismarine_brick"));
    E(prismarine_slab, StoneSlab2(u8"prismarine_rough"));
    E(purpur_slab, StoneSlab2(u8"purpur"));
    E(cut_sandstone_slab, StoneSlab4(u8"cut_sandstone"));
    E(polished_blackstone_brick_slab, Slab(u8"polished_blackstone_brick_double_slab"));
    E(polished_blackstone_slab, Slab(u8"polished_blackstone_double_slab"));
    E(blackstone_slab, Slab(u8"blackstone_double_slab"));
    E(polished_andesite_slab, StoneSlab3(u8"polished_andesite"));
    E(red_nether_brick_slab, StoneSlab2(u8"red_nether_brick"));
    E(end_stone_brick_slab, StoneSlab3(u8"end_stone_brick"));
    E(warped_slab, Slab(u8"warped_double_slab"));
    E(crimson_slab, Slab(u8"crimson_double_slab"));
    E(mangrove_slab, Slab(u8"mangrove_double_slab"));
    E(short_grass, Identity); // ShortGrass(u8"tall")) when < 1.21
    E(tall_grass, TallGrass);
    Converter doublePlant(Same, UpperBlockBitToHalf);
    E(large_fern, doublePlant); // DoublePlant(u8"fern")) < when < 1.21
    E(fern, Identity);          // ShortGrass(u8"fern")) when < 1.21
    E(lilac, doublePlant);      // DoublePlant(u8"syringa")) when < 1.21
    E(rose_bush, doublePlant);  // DoublePlant(u8"rose")) when < 1.21
    E(peony, doublePlant);      // DoublePlant(u8"paeonia")) when < 1.21
    E(sunflower, doublePlant);  // DoublePlant(u8"sunflower")) when < 1.21
    E(dead_bush, Rename(u8"deadbush"));
    E(sea_pickle, SeaPickle());
    E(dandelion, Rename(u8"yellow_flower"));
    E(poppy, Identity);              // RedFlowerLegacy(u8"poppy"));
    E(blue_orchid, Identity);        // RedFlowerLegacy(u8"orchid"));
    E(allium, Identity);             // RedFlowerLegacy(u8"allium"));
    E(azure_bluet, Identity);        // RedFlowerLegacy(u8"houstonia"));
    E(red_tulip, Identity);          // RedFlowerLegacy(u8"tulip_red"));
    E(orange_tulip, Identity);       // RedFlowerLegacy(u8"tulip_orange"));
    E(white_tulip, Identity);        // RedFlowerLegacy(u8"tulip_white"));
    E(pink_tulip, Identity);         // RedFlowerLegacy(u8"tulip_pink"));
    E(oxeye_daisy, Identity);        // RedFlowerLegacy(u8"oxeye"));
    E(cornflower, Identity);         // RedFlowerLegacy(u8"cornflower"));
    E(lily_of_the_valley, Identity); // RedFlowerLegacy(u8"lily_of_the_valley"));
    E(seagrass, Converter(Name(u8"seagrass"), AddStringProperty(u8"sea_grass_type", u8"default")));
    E(tall_seagrass, Converter(Name(u8"seagrass"), HalfToSeagrassType));
    E(kelp, Kelp());
    E(kelp_plant, Kelp(16));
    E(water, Liquid(u8"water"));
    E(lava, Liquid(u8"lava"));
    E(weeping_vines_plant, NetherVines(u8"weeping", 25)); // TODO(kbinani): is 25 correct?
    E(weeping_vines, NetherVines(u8"weeping"));
    E(twisting_vines_plant, NetherVines(u8"twisting", 25)); // TODO(kbinani): is 25 correct?
    E(twisting_vines, NetherVines(u8"twisting"));
    E(vine, Converter(Same, VineDirectionBits));
    E(glow_lichen, Converter(Same, MultiFaceDirectionBitsByItemDefault(63)));
    E(sculk_vein, Converter(Same, MultiFaceDirectionBitsByItemDefault(0)));
    E(cocoa, Converter(Same, Name(Age, u8"age"), DirectionFromFacingA));
    E(nether_wart, Converter(Same, Name(Age, u8"age")));
    E(cobblestone_stairs, Stairs(u8"stone_stairs"));
    E(stone_stairs, Stairs(u8"normal_stone_stairs"));
    E(end_stone_brick_stairs, Stairs(u8"end_brick_stairs"));
    E(prismarine_brick_stairs, Stairs(u8"prismarine_bricks_stairs"));
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
    E(sponge, Sponge(u8"dry"));
    E(wet_sponge, Sponge(u8"wet"));
    E(sandstone, Sandstone(u8"default"));
    E(chiseled_sandstone, Sandstone(u8"heiroglyphs"));
    E(cut_sandstone, Sandstone(u8"cut"));
    E(smooth_sandstone, Sandstone(u8"smooth"));
    E(red_sandstone, RedSandstone(u8"default"));
    E(chiseled_red_sandstone, RedSandstone(u8"heiroglyphs"));
    E(cut_red_sandstone, RedSandstone(u8"cut"));
    E(smooth_red_sandstone, RedSandstone(u8"smooth"));
    E(white_wool, Identity);
    E(orange_wool, Identity);
    E(magenta_wool, Identity);
    E(light_blue_wool, Identity);
    E(yellow_wool, Identity);
    E(lime_wool, Identity);
    E(pink_wool, Identity);
    E(gray_wool, Identity);
    E(light_gray_wool, Identity);
    E(cyan_wool, Identity);
    E(purple_wool, Identity);
    E(blue_wool, Identity);
    E(brown_wool, Identity);
    E(green_wool, Identity);
    E(red_wool, Identity);
    E(black_wool, Identity);
    E(snow_block, Rename(u8"snow"));
    E(quartz_block, QuartzBlock(u8"default"));
    E(smooth_quartz, QuartzBlock(u8"smooth"));
    E(quartz_pillar, QuartzBlock(u8"lines"));
    E(chiseled_quartz_block, QuartzBlock(u8"chiseled"));
    E(bricks, Rename(u8"brick_block"));
    E(sand, Sand(u8"normal"));
    E(red_sand, Sand(u8"red"));
    E(oak_planks, Identity);
    E(spruce_planks, Identity);
    E(birch_planks, Identity);
    E(jungle_planks, Identity);
    E(acacia_planks, Identity);
    E(dark_oak_planks, Identity);
    E(purpur_block, Converter(Same, AddStringProperty(u8"chisel_type", u8"default"), AddStringProperty(u8"pillar_axis", u8"y")));
    E(purpur_pillar, Converter(Name(u8"purpur_block"), AddStringProperty(u8"chisel_type", u8"lines"), AxisToPillarAxis));
    E(jack_o_lantern, LitPumpkin());
    E(carved_pumpkin, Converter(Same, CardinalDirectionFromFacing4ByItemDefault(Facing4::South)));
    E(white_stained_glass, Identity);
    E(orange_stained_glass, Identity);
    E(magenta_stained_glass, Identity);
    E(light_blue_stained_glass, Identity);
    E(yellow_stained_glass, Identity);
    E(lime_stained_glass, Identity);
    E(pink_stained_glass, Identity);
    E(gray_stained_glass, Identity);
    E(light_gray_stained_glass, Identity);
    E(cyan_stained_glass, Identity);
    E(purple_stained_glass, Identity);
    E(blue_stained_glass, Identity);
    E(brown_stained_glass, Identity);
    E(green_stained_glass, Identity);
    E(red_stained_glass, Identity);
    E(black_stained_glass, Identity);
    E(white_concrete_powder, Identity);
    E(orange_concrete_powder, Identity);
    E(magenta_concrete_powder, Identity);
    E(light_blue_concrete_powder, Identity);
    E(yellow_concrete_powder, Identity);
    E(lime_concrete_powder, Identity);
    E(pink_concrete_powder, Identity);
    E(gray_concrete_powder, Identity);
    E(light_gray_concrete_powder, Identity);
    E(cyan_concrete_powder, Identity);
    E(purple_concrete_powder, Identity);
    E(blue_concrete_powder, Identity);
    E(brown_concrete_powder, Identity);
    E(green_concrete_powder, Identity);
    E(red_concrete_powder, Identity);
    E(black_concrete_powder, Identity);
    E(white_concrete, Identity);
    E(orange_concrete, Identity);
    E(magenta_concrete, Identity);
    E(light_blue_concrete, Identity);
    E(yellow_concrete, Identity);
    E(lime_concrete, Identity);
    E(pink_concrete, Identity);
    E(gray_concrete, Identity);
    E(light_gray_concrete, Identity);
    E(cyan_concrete, Identity);
    E(purple_concrete, Identity);
    E(blue_concrete, Identity);
    E(brown_concrete, Identity);
    E(green_concrete, Identity);
    E(red_concrete, Identity);
    E(black_concrete, Identity);
    E(white_terracotta, Identity);
    E(orange_terracotta, Identity);
    E(magenta_terracotta, Identity);
    E(light_blue_terracotta, Identity);
    E(yellow_terracotta, Identity);
    E(lime_terracotta, Identity);
    E(pink_terracotta, Identity);
    E(gray_terracotta, Identity);
    E(light_gray_terracotta, Identity);
    E(cyan_terracotta, Identity);
    E(purple_terracotta, Identity);
    E(blue_terracotta, Identity);
    E(brown_terracotta, Identity);
    E(green_terracotta, Identity);
    E(red_terracotta, Identity);
    E(black_terracotta, Identity);
    E(nether_quartz_ore, Rename(u8"quartz_ore"));
    E(red_nether_bricks, Rename(u8"red_nether_brick"));
    E(magma_block, Rename(u8"magma"));
    E(sea_lantern, Identity);
    E(prismarine_bricks, Prismarine(u8"bricks"));
    E(dark_prismarine, Prismarine(u8"dark"));
    E(prismarine, Prismarine(u8"default"));
    E(terracotta, Rename(u8"hardened_clay"));
    E(end_stone_bricks, Rename(u8"end_bricks"));
    E(melon, Rename(u8"melon_block"));
    E(chiseled_stone_bricks, StoneBrick(u8"chiseled"));
    E(cracked_stone_bricks, StoneBrick(u8"cracked"));
    E(mossy_stone_bricks, StoneBrick(u8"mossy"));
    E(stone_bricks, StoneBrick(u8"default"));
    Converter sapling(Same, StageToAgeBit);
    E(oak_sapling, sapling);              // SaplingLegacy(u8"oak"));
    E(birch_sapling, sapling);            // SaplingLegacy(u8"birch"));
    E(jungle_sapling, sapling);           // SaplingLegacy(u8"jungle"));
    E(acacia_sapling, sapling);           // SaplingLegacy(u8"acacia"));
    E(spruce_sapling, sapling);           // SaplingLegacy(u8"spruce"));
    E(dark_oak_sapling, sapling);         // SaplingLegacy(u8"dark_oak"));
    E(tube_coral_block, Identity);        // CoralBlock(u8"blue", false)) when < 1.21
    E(brain_coral_block, Identity);       // CoralBlock(u8"pink", false)) when < 1.21
    E(bubble_coral_block, Identity);      // CoralBlock(u8"purple", false)) when < 1.21
    E(fire_coral_block, Identity);        // CoralBlock(u8"red", false)) when < 1.21
    E(horn_coral_block, Identity);        // CoralBlock(u8"yellow", false)) when < 1.21
    E(dead_tube_coral_block, Identity);   // CoralBlock(u8"blue", true)) when < 1.21
    E(dead_brain_coral_block, Identity);  // CoralBlock(u8"pink", true)) when < 1.21
    E(dead_bubble_coral_block, Identity); // CoralBlock(u8"purple", true)) when < 1.21
    E(dead_fire_coral_block, Identity);   // CoralBlock(u8"red", true)) when < 1.21
    E(dead_horn_coral_block, Identity);   // CoralBlock(u8"yellow", true)) when < 1.21
    E(snow, SnowLayer);
    E(sugar_cane, Converter(Name(u8"reeds"), Name(Age, u8"age")));
    E(end_rod, Converter(Same, EndRodFacingDirectionFromFacing));
    E(oak_fence, Identity);
    E(spruce_fence, Identity);
    E(birch_fence, Identity);
    E(jungle_fence, Identity);
    E(acacia_fence, Identity);
    E(dark_oak_fence, Identity);
    E(bamboo_fence, Identity);
    E(ladder, facingDirectionFromFacingA);
    E(chest, Converter(Same, CardinalDirectionFromFacing4ByItemDefault(Facing4::North)));
    E(furnace, Converter(PrefixLit, CardinalDirectionFromFacing4));
    E(nether_bricks, Rename(u8"nether_brick"));
    E(infested_stone, InfestedStone(u8"stone"));
    E(infested_cobblestone, InfestedStone(u8"cobblestone"));
    E(infested_stone_bricks, InfestedStone(u8"stone_brick"));
    E(infested_mossy_stone_bricks, InfestedStone(u8"mossy_stone_brick"));
    E(infested_cracked_stone_bricks, InfestedStone(u8"cracked_stone_brick"));
    E(infested_chiseled_stone_bricks, InfestedStone(u8"chiseled_stone_brick"));
    E(torch, AnyTorch(u8""));
    E(wall_torch, AnyWallTorch(u8""));
    E(soul_torch, AnyTorch(u8"soul_"));
    E(soul_wall_torch, AnyWallTorch(u8"soul_"));
    E(redstone_torch, Converter(PrefixUnlit(u8"redstone_torch"), AddStringProperty(u8"torch_facing_direction", u8"top")));
    E(redstone_wall_torch, Converter(PrefixUnlit(u8"redstone_torch"), TorchFacingDirectionFromFacing));
    E(farmland, Converter(Same, Name(Moisture, u8"moisturized_amount")));
    E(red_mushroom_block, AnyMushroomBlock(u8"red_mushroom_block", false));
    E(brown_mushroom_block, AnyMushroomBlock(u8"brown_mushroom_block", false));
    E(mushroom_stem, AnyMushroomBlock(u8"brown_mushroom_block", true));
    E(end_portal_frame, Converter(Same, CardinalDirectionFromFacing4, EyeToEndPortalEyeBit));
    E(white_shulker_box, Identity);
    E(orange_shulker_box, Identity);
    E(magenta_shulker_box, Identity);
    E(light_blue_shulker_box, Identity);
    E(yellow_shulker_box, Identity);
    E(lime_shulker_box, Identity);
    E(pink_shulker_box, Identity);
    E(gray_shulker_box, Identity);
    E(light_gray_shulker_box, Identity);
    E(cyan_shulker_box, Identity);
    E(purple_shulker_box, Identity);
    E(blue_shulker_box, Identity);
    E(brown_shulker_box, Identity);
    E(green_shulker_box, Identity);
    E(red_shulker_box, Identity);
    E(black_shulker_box, Identity);
    E(shulker_box, Rename(u8"undyed_shulker_box"));
    E(cobblestone_wall, Wall(u8"cobblestone"));
    E(mossy_cobblestone_wall, Wall(u8"mossy_cobblestone"));
    E(brick_wall, Wall(u8"brick"));
    E(prismarine_wall, Wall(u8"prismarine"));
    E(red_sandstone_wall, Wall(u8"red_sandstone"));
    E(mossy_stone_brick_wall, Wall(u8"mossy_stone_brick"));
    E(granite_wall, Wall(u8"granite"));
    E(andesite_wall, Wall(u8"andesite"));
    E(diorite_wall, Wall(u8"diorite"));
    E(stone_brick_wall, Wall(u8"stone_brick"));
    E(nether_brick_wall, Wall(u8"nether_brick"));
    E(red_nether_brick_wall, Wall(u8"red_nether_brick"));
    E(sandstone_wall, Wall(u8"sandstone"));
    E(end_stone_brick_wall, Wall(u8"end_brick"));
    Converter wall(Same, WallPostBit, WallConnectionType(u8"east"), WallConnectionType(u8"north"), WallConnectionType(u8"south"), WallConnectionType(u8"west"));
    E(blackstone_wall, wall);
    E(polished_blackstone_wall, wall);
    E(polished_blackstone_brick_wall, wall);
    E(white_carpet, Identity);
    E(orange_carpet, Identity);
    E(magenta_carpet, Identity);
    E(light_blue_carpet, Identity);
    E(yellow_carpet, Identity);
    E(lime_carpet, Identity);
    E(pink_carpet, Identity);
    E(gray_carpet, Identity);
    E(light_gray_carpet, Identity);
    E(cyan_carpet, Identity);
    E(purple_carpet, Identity);
    E(blue_carpet, Identity);
    E(brown_carpet, Identity);
    E(green_carpet, Identity);
    E(red_carpet, Identity);
    E(black_carpet, Identity);
    E(white_stained_glass_pane, Identity);
    E(orange_stained_glass_pane, Identity);
    E(magenta_stained_glass_pane, Identity);
    E(light_blue_stained_glass_pane, Identity);
    E(yellow_stained_glass_pane, Identity);
    E(lime_stained_glass_pane, Identity);
    E(pink_stained_glass_pane, Identity);
    E(gray_stained_glass_pane, Identity);
    E(light_gray_stained_glass_pane, Identity);
    E(cyan_stained_glass_pane, Identity);
    E(purple_stained_glass_pane, Identity);
    E(blue_stained_glass_pane, Identity);
    E(brown_stained_glass_pane, Identity);
    E(green_stained_glass_pane, Identity);
    E(red_stained_glass_pane, Identity);
    E(black_stained_glass_pane, Identity);
    E(slime_block, Rename(u8"slime"));
    E(anvil, Anvil(u8"undamaged"));
    E(chipped_anvil, Anvil(u8"slightly_damaged"));
    E(damaged_anvil, Anvil(u8"very_damaged"));
    E(white_glazed_terracotta, facingDirectionFromFacingA);
    E(orange_glazed_terracotta, facingDirectionFromFacingA);
    E(magenta_glazed_terracotta, facingDirectionFromFacingA);
    E(light_blue_glazed_terracotta, facingDirectionFromFacingA);
    E(yellow_glazed_terracotta, facingDirectionFromFacingA);
    E(lime_glazed_terracotta, facingDirectionFromFacingA);
    E(pink_glazed_terracotta, facingDirectionFromFacingA);
    E(gray_glazed_terracotta, facingDirectionFromFacingA);
    E(light_gray_glazed_terracotta, Converter(Name(u8"silver_glazed_terracotta"), FacingDirectionAFromFacing));
    E(cyan_glazed_terracotta, facingDirectionFromFacingA);
    E(purple_glazed_terracotta, facingDirectionFromFacingA);
    E(blue_glazed_terracotta, facingDirectionFromFacingA);
    E(brown_glazed_terracotta, facingDirectionFromFacingA);
    E(green_glazed_terracotta, facingDirectionFromFacingA);
    E(red_glazed_terracotta, facingDirectionFromFacingA);
    E(black_glazed_terracotta, facingDirectionFromFacingA);

    E(tube_coral, Identity);
    E(brain_coral, Identity);
    E(bubble_coral, Identity);
    E(fire_coral, Identity);
    E(horn_coral, Identity);

    E(dead_tube_coral, Identity);
    E(dead_brain_coral, Identity);
    E(dead_bubble_coral, Identity);
    E(dead_fire_coral, Identity);
    E(dead_horn_coral, Identity);

    E(tube_coral_fan, CoralFan);   // legacy: CoralFanLegacy(u8"blue", false));
    E(brain_coral_fan, CoralFan);  // legacy: CoralFanLegacy(u8"pink", false));
    E(bubble_coral_fan, CoralFan); // legacy: CoralFanLegacy(u8"purple", false));
    E(fire_coral_fan, CoralFan);   // legacy: CoralFanLegacy(u8"red", false));
    E(horn_coral_fan, CoralFan);   // legacy: CoralFanLegacy(u8"yellow", false));

    E(dead_tube_coral_fan, CoralFan);   // legacy: CoralFanLegacy(u8"blue", true));
    E(dead_brain_coral_fan, CoralFan);  // legacy: CoralFanLegacy(u8"pink", true));
    E(dead_bubble_coral_fan, CoralFan); // legacy: CoralFanLegacy(u8"purple", true));
    E(dead_fire_coral_fan, CoralFan);   // legacy: CoralFanLegacy(u8"red", true));
    E(dead_horn_coral_fan, CoralFan);   // legacy: CoralFanLegacy(u8"yellow", true));

    E(tube_coral_wall_fan, CoralWallFan(u8"", false, 0));
    E(brain_coral_wall_fan, CoralWallFan(u8"", false, 1));
    E(bubble_coral_wall_fan, CoralWallFan(u8"2", false, 0));
    E(fire_coral_wall_fan, CoralWallFan(u8"2", false, 1));
    E(horn_coral_wall_fan, CoralWallFan(u8"3", false, 0));

    E(dead_tube_coral_wall_fan, CoralWallFan(u8"", true, 0));
    E(dead_brain_coral_wall_fan, CoralWallFan(u8"", true, 1));
    E(dead_bubble_coral_wall_fan, CoralWallFan(u8"2", true, 0));
    E(dead_fire_coral_wall_fan, CoralWallFan(u8"2", true, 1));
    E(dead_horn_coral_wall_fan, CoralWallFan(u8"3", true, 0));

    E(oak_sign, Sign());
    E(spruce_sign, Sign(u8"spruce"));
    E(birch_sign, Sign(u8"birch"));
    E(jungle_sign, Sign(u8"jungle"));
    E(acacia_sign, Sign(u8"acacia"));
    E(dark_oak_sign, Sign(u8"darkoak"));
    E(crimson_sign, Sign(u8"crimson"));
    E(warped_sign, Sign(u8"warped"));
    E(mangrove_sign, Sign(u8"mangrove"));
    E(bamboo_sign, Sign(u8"bamboo"));

    E(oak_wall_sign, WallSign());
    E(spruce_wall_sign, WallSign(u8"spruce"));
    E(birch_wall_sign, WallSign(u8"birch"));
    E(jungle_wall_sign, WallSign(u8"jungle"));
    E(acacia_wall_sign, WallSign(u8"acacia"));
    E(dark_oak_wall_sign, WallSign(u8"darkoak"));
    E(crimson_wall_sign, WallSign(u8"crimson"));
    E(warped_wall_sign, WallSign(u8"warped"));
    E(mangrove_wall_sign, WallSign(u8"mangrove"));
    E(bamboo_wall_sign, WallSign(u8"bamboo"));

    Converter bed(Name(u8"bed"), DirectionFromFacingA, PartToHeadPieceBit, OccupiedToOccupiedBit);
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

    Converter pottedFlowerPot(Name(u8"flower_pot"), AddBoolProperty(u8"update_bit", true));
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
    E(potted_cherry_sapling, pottedFlowerPot);
    E(potted_torchflower, pottedFlowerPot);

    Converter skull(Name(u8"skull"), AddIntProperty(u8"facing_direction", 1));
    E(skeleton_skull, skull);
    E(wither_skeleton_skull, skull);
    E(player_head, skull);
    E(zombie_head, skull);
    E(creeper_head, skull);
    E(dragon_head, skull);
    E(piglin_head, skull);

    Converter wallSkull(Name(u8"skull"), WallSkullFacingDirection);
    E(skeleton_wall_skull, wallSkull);
    E(wither_skeleton_wall_skull, wallSkull);
    E(player_wall_head, wallSkull);
    E(zombie_wall_head, wallSkull);
    E(creeper_wall_head, wallSkull);
    E(dragon_wall_head, wallSkull);
    E(piglin_wall_head, wallSkull);

    Converter banner(Name(u8"standing_banner"), Name(Rotation, u8"ground_sign_direction"));
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

    Converter wallBanner(Name(u8"wall_banner"), FacingDirectionAFromFacing);
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

    E(stonecutter, Converter(Name(u8"stonecutter_block"), CardinalDirectionFromFacing4ByItemDefault(Facing4::North)));
    E(loom, directionFromFacing);
    E(grindstone, Converter(Name(u8"grindstone"), DirectionFromFacingA, GrindstoneFaceToAttachment));
    E(smoker, Converter(PrefixLit, CardinalDirectionFromFacing4ByItemDefault(Facing4::South)));
    E(blast_furnace, Converter(PrefixLit, CardinalDirectionFromFacing4));
    E(barrel, Converter(Name(u8"barrel"), FacingDirectionAFromFacing, Name(Open, u8"open_bit")));
    Converter lantern(Same, Name(Hanging, u8"hanging"));
    E(lantern, lantern);
    E(soul_lantern, lantern);
    E(bell, Converter(Name(u8"bell"), BellDirectionFromFacing, BellAttachmentFromAttachment, Name(Powered, u8"toggle_bit")));
    Converter campfire(Same, CardinalDirectionFromFacing4, LitToExtinguished);
    E(campfire, campfire);
    E(soul_campfire, campfire);
    E(piston, facingDirectionFromFacingB);
    E(sticky_piston, facingDirectionFromFacingB);
    E(piston_head, PistonHead);
    E(moving_piston, MovingPiston);
    E(note_block, Rename(u8"noteblock"));
    E(dispenser, Converter(Same, FacingDirectionAFromFacingByItemDefault(3), Name(Triggered, u8"triggered_bit")));
    E(lever, Converter(Same, LeverDirection, Name(Powered, u8"open_bit")));

    Converter fenceGate(Same, DirectionFromFacingA, Name(InWall, u8"in_wall_bit"), Name(Open, u8"open_bit"));
    E(oak_fence_gate, Converter(Name(u8"fence_gate"), DirectionFromFacingA, Name(InWall, u8"in_wall_bit"), Name(Open, u8"open_bit")));
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
    E(oak_pressure_plate, Converter(Name(u8"wooden_pressure_plate"), RedstoneSignalFromPowered));
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

    Converter trapdoor(Same, DirectionFromFacingB, Name(Open, u8"open_bit"), HalfToUpsideDownBit);
    E(oak_trapdoor, Converter(Name(u8"trapdoor"), DirectionFromFacingB, Name(Open, u8"open_bit"), HalfToUpsideDownBit));
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

    E(lily_pad, Rename(u8"waterlily"));

    Converter button(Same, ButtonFacingDirection, Name(Powered, u8"button_pressed_bit"));
    E(oak_button, Converter(Name(u8"wooden_button"), ButtonFacingDirection, Name(Powered, u8"button_pressed_bit")));
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

    E(tripwire_hook, Converter(Same, DirectionFromFacingA, Name(Attached, u8"attached_bit"), Name(Powered, u8"powered_bit")));
    E(trapped_chest, Converter(Same, CardinalDirectionFromFacing4ByItemDefault(Facing4::North)));
    E(daylight_detector, Converter(DaylightDetectorName, Name(Power, u8"redstone_signal")));
    E(hopper, Converter(Same, FacingDirectionAFromFacing, ToggleBitFromEnabled));
    E(dropper, Converter(Same, FacingDirectionAFromFacingByItemDefault(3), Name(Triggered, u8"triggered_bit")));
    E(observer, Converter(Same, Name(Facing, u8"minecraft:facing_direction"), Name(Powered, u8"powered_bit")));

    Converter door(Same, DirectionFromFacingC, Name(Open, u8"open_bit"), UpperBlockBitToHalf, DoorHingeBitFromHinge, Door);
    E(oak_door, Converter(Name(u8"wooden_door"), DirectionFromFacingC, Name(Open, u8"open_bit"), UpperBlockBitToHalf, DoorHingeBitFromHinge, Door));
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

    E(repeater, Converter(RepeaterName, Name(Delay, u8"repeater_delay"), CardinalDirectionFromFacing4));
    E(comparator, Converter(ComparatorName, CardinalDirectionFromFacing4, OutputSubtractBitFromMode, Name(Powered, u8"output_lit_bit")));
    E(powered_rail, Converter(Name(u8"golden_rail"), RailDirectionFromShape, Name(Powered, u8"rail_data_bit")));
    E(detector_rail, Converter(Same, RailDirectionFromShape, Name(Powered, u8"rail_data_bit")));
    E(activator_rail, Converter(Same, RailDirectionFromShape, Name(Powered, u8"rail_data_bit")));
    E(rail, Converter(Same, RailDirectionFromShape));
    E(nether_portal, Converter(Name(u8"portal"), Name(Axis, u8"portal_axis")));

    E(bamboo, Bamboo);
    E(sweet_berry_bush, Converter(Same, Name(Age, u8"growth")));
    E(bubble_column, Converter(Same, DragDownFromDrag));
    E(cake, Converter(Same, BiteCounterFromBites));
    E(beetroots, Converter(Name(u8"beetroot"), GrowthFromAge));
    E(potatoes, Converter(Same, Name(Age, u8"growth")));
    E(carrots, Converter(Same, Name(Age, u8"growth")));
    E(pumpkin_stem, Converter(Same, Name(Age, u8"growth"), AddIntProperty(u8"facing_direction", 0)));
    E(melon_stem, Converter(Same, Name(Age, u8"growth"), AddIntProperty(u8"facing_direction", 0)));
    E(attached_pumpkin_stem, Converter(Name(u8"pumpkin_stem"), AddIntProperty(u8"growth", 7), FacingDirectionAFromFacing));
    E(attached_melon_stem, Converter(Name(u8"melon_stem"), AddIntProperty(u8"growth", 7), FacingDirectionAFromFacing));
    E(wheat, Converter(Same, Name(Age, u8"growth")));

    E(cobweb, Converter(Name(u8"web")));
    E(lectern, Converter(Same, CardinalDirectionFromFacing4, Name(Powered, u8"powered_bit")));
    E(ender_chest, Converter(Same, CardinalDirectionFromFacing4ByItemDefault(Facing4::North)));
    E(bone_block, Converter(Same, AxisToPillarAxis, AddIntProperty(u8"deprecated", 0)));
    E(cauldron, Converter(Name(u8"cauldron"), AddIntProperty(u8"fill_level", 0), AddStringProperty(u8"cauldron_liquid", u8"water")));
    E(water_cauldron, Converter(Name(u8"cauldron"), CauldronFillLevelFromLevel, AddStringProperty(u8"cauldron_liquid", u8"water")));
    E(lava_cauldron, Converter(Name(u8"cauldron"), AddIntProperty(u8"fill_level", 6), AddStringProperty(u8"cauldron_liquid", u8"lava")));
    E(powder_snow_cauldron, Converter(Name(u8"cauldron"), CauldronFillLevelFromLevel, AddStringProperty(u8"cauldron_liquid", u8"powder_snow")));
    E(hay_block, Converter(Same, AxisToPillarAxis, AddIntProperty(u8"deprecated", 0)));
    E(composter, Converter(Same, Name(Level, u8"composter_fill_level")));
    E(cave_air, Rename(u8"air"));
    E(void_air, Rename(u8"air"));
    E(turtle_egg, Converter(Same, TurtleEggCount, TurtleEggCrackedState));

    Converter beehive(Same, Name(FacingA, u8"direction"), HoneyLevel);
    E(bee_nest, beehive);
    E(beehive, beehive);

    E(chorus_flower, Converter(Same, Name(Age, u8"age")));

    Converter commandBlock(Same, Conditional, FacingDirectionAFromFacing);
    E(command_block, commandBlock);
    E(chain_command_block, commandBlock);
    E(repeating_command_block, commandBlock);

    E(dirt_path, Rename(u8"grass_path"));
    E(waxed_copper_block, Rename(u8"waxed_copper"));
    E(rooted_dirt, Rename(u8"dirt_with_roots"));

    E(azalea_leaves, leaves);
    E(flowering_azalea_leaves, Converter(Name(u8"azalea_leaves_flowered"), PersistentAndDistanceToPersistentBitAndUpdateBit));

    E(big_dripleaf, BigDripleaf);
    E(big_dripleaf_stem, Converter(Name(u8"big_dripleaf"), AddByteProperty(u8"big_dripleaf_head", false), CardinalDirectionFromFacing4, AddStringProperty(u8"big_dripleaf_tilt", u8"none")));
    E(small_dripleaf, Converter(Name(u8"small_dripleaf_block"), CardinalDirectionFromFacing4ByItemDefault(Facing4::East), UpperBlockBitToHalfByItemDefault(true)));

    Converter candle(Same, Name(Lit, u8"lit"), Name(Candles, u8"candles"));
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

    Converter candleCake(Same, Name(Lit, u8"lit"));
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

    E(cut_copper_slab, Slab(u8"double_cut_copper_slab"));
    E(exposed_cut_copper_slab, Slab(u8"exposed_double_cut_copper_slab"));
    E(weathered_cut_copper_slab, Slab(u8"weathered_double_cut_copper_slab"));
    E(oxidized_cut_copper_slab, Slab(u8"oxidized_double_cut_copper_slab"));

    E(waxed_cut_copper_slab, Slab(u8"waxed_double_cut_copper_slab"));
    E(waxed_exposed_cut_copper_slab, Slab(u8"waxed_exposed_double_cut_copper_slab"));
    E(waxed_weathered_cut_copper_slab, Slab(u8"waxed_weathered_double_cut_copper_slab"));
    E(waxed_oxidized_cut_copper_slab, Slab(u8"waxed_oxidized_double_cut_copper_slab"));

    E(cobbled_deepslate_slab, Slab(u8"cobbled_deepslate_double_slab"));
    E(polished_deepslate_slab, Slab(u8"polished_deepslate_double_slab"));
    E(deepslate_brick_slab, Slab(u8"deepslate_brick_double_slab"));
    E(deepslate_tile_slab, Slab(u8"deepslate_tile_double_slab"));

    E(cobbled_deepslate_wall, wall);
    E(polished_deepslate_wall, wall);
    E(deepslate_brick_wall, wall);
    E(deepslate_tile_wall, wall);

    E(lightning_rod, facingDirectionFromFacingA);

    E(small_amethyst_bud, blockFaceFromFacing);
    E(medium_amethyst_bud, blockFaceFromFacing);
    E(large_amethyst_bud, blockFaceFromFacing);
    E(amethyst_cluster, blockFaceFromFacing);

    E(pointed_dripstone, PointedDripstone);
    E(light, Light);
    E(cave_vines, CaveVines);
    E(cave_vines_plant, CaveVinesPlant);
    E(chain, axisToPillarAxis);

    E(bamboo_sapling, sapling);
    E(brewing_stand, BrewingStand);
    E(cactus, Converter(Same, Name(Age, u8"age")));
    E(fire, Converter(Same, Name(Age, u8"age")));
    E(frosted_ice, Converter(Same, Name(Age, u8"age")));
    E(pumpkin, Converter(Same, AddStringProperty(u8"minecraft:cardinal_direction", u8"south")));
    E(redstone_wire, Converter(Same, Name(Power, u8"redstone_signal")));
    E(scaffolding, Scaffolding);
    E(structure_block, StructureBlock);
    E(structure_void, Converter(Same, AddStringProperty(u8"structure_void_type", u8"void")));
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
    E(frogspawn, Rename(u8"frog_spawn"));
    E(mangrove_leaves, leaves);
    E(sculk_shrieker, SculkShrieker);
    E(mud_brick_wall, wall);
    E(sculk_catalyst, SculkCatalyst);
    E(mud_brick_slab, Slab(u8"mud_brick_double_slab"));
    E(muddy_mangrove_roots, axisToPillarAxis);

    E(spawner, Rename(u8"mob_spawner"));

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

    E(bamboo_slab, Slab(u8"bamboo_double_slab"));
    E(bamboo_mosaic_slab, Slab(u8"bamboo_mosaic_double_slab"));
    E(decorated_pot, Converter(Same, DirectionNorth0East1South2West3FromFacing));
    E(torchflower_crop, Converter(Same, GrowthFromAge));
    E(jigsaw, Jigsaw);
    E(cherry_slab, Slab(u8"cherry_double_slab"));
    E(pink_petals, PinkPetals);
    E(cherry_wood, wood);
    E(stripped_cherry_wood, axisToPillarAxis);
    E(cherry_log, axisToPillarAxis);
    E(stripped_cherry_log, axisToPillarAxis);
    E(cherry_stairs, Stairs());
    E(cherry_button, button);
    E(cherry_hanging_sign, HangingSign);
    E(cherry_wall_hanging_sign, WallHangingSign);
    E(cherry_sign, Sign(u8"cherry"));
    E(cherry_wall_sign, WallSign(u8"cherry"));
    E(cherry_door, door);
    E(cherry_fence_gate, fenceGate);
    E(cherry_fence, Identity);
    E(cherry_trapdoor, trapdoor);
    E(cherry_sapling, sapling);
    E(cherry_pressure_plate, pressurePlate);
    E(cherry_leaves, leaves);

    E(pitcher_crop, Converter(Same, Name(Age, u8"growth"), UpperBlockBitToHalf));
    E(pitcher_plant, doublePlant);
    E(sniffer_egg, SnifferEgg);
    E(sculk_sensor, Converter(Same, SculkSensorPhase));
    E(calibrated_sculk_sensor, Converter(Same, SculkSensorPhase, CardinalDirectionFromFacing4));
    Converter suspiciousBlock(Same, AddIntProperty(u8"brushed_progress", 0), AddBoolProperty(u8"hanging", true));
    E(suspicious_sand, suspiciousBlock);
    E(suspicious_gravel, suspiciousBlock);
    E(bedrock, Converter(Same, AddBoolProperty(u8"infiniburn_bit", false)));
    E(bamboo_pressure_plate, pressurePlate);
    E(mangrove_pressure_plate, pressurePlate);

    // 1.20.3
    E(tuff_stairs, Stairs());
    E(tuff_slab, Slab(u8"tuff_double_slab"));
    E(tuff_wall, wall);
    E(polished_tuff_stairs, Stairs());
    E(polished_tuff_slab, Slab(u8"polished_tuff_double_slab"));
    E(polished_tuff_wall, wall);
    E(tuff_brick_slab, Slab(u8"tuff_brick_double_slab"));
    E(tuff_brick_stairs, Stairs());
    E(tuff_brick_wall, wall);
    E(copper_door, door);
    E(exposed_copper_door, door);
    E(weathered_copper_door, door);
    E(oxidized_copper_door, door);
    E(waxed_copper_door, door);
    E(waxed_exposed_copper_door, door);
    E(waxed_weathered_copper_door, door);
    E(waxed_oxidized_copper_door, door);
    E(copper_trapdoor, trapdoor);
    E(exposed_copper_trapdoor, trapdoor);
    E(weathered_copper_trapdoor, trapdoor);
    E(oxidized_copper_trapdoor, trapdoor);
    E(waxed_copper_trapdoor, trapdoor);
    E(waxed_exposed_copper_trapdoor, trapdoor);
    E(waxed_weathered_copper_trapdoor, trapdoor);
    E(waxed_oxidized_copper_trapdoor, trapdoor);
    Converter const copperBulb(Same, Name(Lit, u8"lit"), Name(Powered, u8"powered_bit"));
    E(copper_bulb, copperBulb);
    E(exposed_copper_bulb, copperBulb);
    E(weathered_copper_bulb, copperBulb);
    E(oxidized_copper_bulb, copperBulb);
    E(waxed_copper_bulb, copperBulb);
    E(waxed_exposed_copper_bulb, copperBulb);
    E(waxed_weathered_copper_bulb, copperBulb);
    E(waxed_oxidized_copper_bulb, copperBulb);
    E(crafter, Converter(Same,
                         Name(BooleanProperty(u8"crafting"), u8"crafting"),
                         Name(BooleanProperty(u8"triggered"), u8"triggered_bit"),
                         Name(StringProperty(u8"orientation", u8"down_east"), u8"orientation")));

    E(vault, Vault);
    E(heavy_core, Identity);
    E(trial_spawner, TrialSpawner);

    // 1.21.4
    E(pale_oak_leaves, leaves);
    E(resin_clump, Converter(Same, MultiFaceDirectionBitsByItemDefault(0)));
    E(pale_oak_slab, Slab(u8"pale_oak_double_slab"));
    E(resin_brick_slab, Slab(u8"resin_brick_double_slab"));
#undef E

    return table;
  }

  static CompoundTagPtr Vault(Block const &block, CompoundTagConstPtr const &, Options const &o) {
    auto c = New(block.fName, true);
    auto s = States();
    if (o.fItem) {
      s->set(u8"ominous", Bool(false));
    } else {
      Name(BooleanPropertyOptional(u8"ominous"), u8"ominous")(s, block, o);
    }
    Name(StringProperty(u8"vault_state", u8"inactive"), u8"vault_state")(s, block, o);
    CardinalDirectionFromFacing4(s, block, o);
    return AttachStates(c, s);
  }

  static CompoundTagPtr TrialSpawner(Block const &block, CompoundTagConstPtr const &, Options const &o) {
    auto c = New(block.fName, true);
    auto s = States();
    std::u8string stateJ(block.property(u8"trial_spawner_state", u8"inactive"));
    i32 stateB = TrialSpawner::BedrockTrialSpawnerStateFromJava(stateJ);
    s->set(u8"trial_spawner_state", Int(stateB));
    auto ominous = block.property(u8"ominous", u8"");
    if (o.fItem) {
      s->set(u8"ominous", Bool(false));
    } else if (ominous == u8"true") {
      s->set(u8"ominous", Bool(true));
    } else if (ominous == u8"false") {
      s->set(u8"ominous", Bool(false));
    }
    return AttachStates(c, s);
  }

  static CompoundTagPtr SnifferEgg(Block const &block, CompoundTagConstPtr const &tile, Options const &o) {
    auto c = New(block.fName, true);
    auto s = States();
    auto hatch = Wrap(strings::ToI32(block.property(u8"hatch", u8"0")), 0);
    auto crackedState = String(u8"no_cracks");
    switch (hatch) {
    case 1:
      crackedState = String(u8"cracked");
      break;
    case 2:
      crackedState = String(u8"max_cracked");
      break;
    }
    s->set(u8"cracked_state", crackedState);
    return AttachStates(c, s);
  }

  static CompoundTagPtr PinkPetals(Block const &block, CompoundTagConstPtr const &tile, Options const &o) {
    auto c = New(block.fName, true);
    auto s = States();

    CardinalDirectionFromFacing4(s, block, o);

    i32 flowerCount = Wrap(strings::ToI32(block.property(u8"flower_amount", u8"1")), 1);
    i32 growth = ClosedRange<i32>::Clamp(flowerCount - 1, 0, 3);
    s->set(u8"growth", Int(growth));

    return AttachStates(c, s);
  }

  static CompoundTagPtr Jigsaw(Block const &block, CompoundTagConstPtr const &tile, Options const &o) {
    auto c = New(block.fName, true);
    auto s = States();
    auto orientation = block.property(u8"orientation", u8"north_up");
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
    auto tokens = mcfile::String::Split(std::u8string(orientation), u8'_');
    if (tokens.size() == 2) {
      Facing6 f6 = Facing6FromJavaName(tokens[0]);
      int facingDirection = BedrockFacingDirectionAFromFacing6(f6);
      Facing4 f4 = Facing4FromJavaName(tokens[1]);
      int rotation = North0East1South2West3FromFacing4(f4);
      if (tokens[1] == u8"up" || tokens[1] == u8"down") {
        rotation = 0;
      }
      s->set(u8"facing_direction", Int(facingDirection));
      s->set(u8"rotation", Int(rotation));
    }
    return AttachStates(c, s);
  }

  static CompoundTagPtr ChiseledBookshelf(Block const &block, CompoundTagConstPtr const &, Options const &o) {
    auto c = New(block.fName, true);
    auto s = States();
    uint8_t stored = 0;
    for (int i = 0; i < 6; i++) {
      bool occupied = block.property(u8"slot_" + mcfile::String::ToString(i) + u8"_occupied", u8"false") == u8"true";
      if (occupied) {
        stored = stored | (uint8_t(1) << i);
      }
    }
    s->set(u8"books_stored", Int(stored));
    int direction = BedrockDirectionFromFacing4(Facing4FromJavaName(block.property(u8"facing", u8"south")));
    s->set(u8"direction", Int(direction));

    return AttachStates(c, s);
  }

  static CompoundTagPtr WallHangingSign(Block const &block, CompoundTagConstPtr const &, Options const &o) {
    auto c = New(strings::Replace(block.fName, u8"_wall_", u8"_"), true);
    Facing4 f4 = Facing4FromJavaName(block.property(u8"facing", u8"north"));
    int facingDirection;
    switch (f4) {
    case Facing4::West:
      facingDirection = 4;
      break;
    case Facing4::North:
      facingDirection = 2;
      break;
    case Facing4::East:
      facingDirection = 5;
      break;
    case Facing4::South:
    default:
      facingDirection = 3;
      break;
    }
    auto s = States();
    s->set(u8"ground_sign_direction", Int(0));
    s->set(u8"facing_direction", Int(facingDirection));
    s->set(u8"hanging", Bool(false));
    s->set(u8"attached_bit", Bool(false));
    return AttachStates(c, s);
  }

  static CompoundTagPtr HangingSign(Block const &block, CompoundTagConstPtr const &, Options const &o) {
    auto c = New(block.fName, true);
    auto s = States();
    auto attached = block.property(u8"attached", u8"false") == u8"true";
    s->set(u8"attached_bit", Bool(attached));
    auto rotationJ = Wrap(strings::ToI32(block.property(u8"rotation", u8"0")), 0);
    int facingDirection = 0;
    int groundSignDirection = 0;
    if (attached) {
      groundSignDirection = rotationJ;
    } else {
      switch (rotationJ) {
      case 0:
        // south
        facingDirection = 3;
        break;
      case 4:
        // west
        facingDirection = 4;
        break;
      case 8:
        // north
        facingDirection = 2;
        break;
      case 12:
        // east
        facingDirection = 5;
        break;
      }
    }
    s->set(u8"facing_direction", Int(facingDirection));
    s->set(u8"ground_sign_direction", Int(groundSignDirection));
    s->set(u8"hanging", Bool(true));
    return AttachStates(c, s);
  }

  static CompoundTagPtr Bamboo(Block const &block, CompoundTagConstPtr const &, Options const &o) {
    auto c = New(block.fName, true);
    auto s = States();
    BambooLeafSizeFromLeaves(s, block, o);
    auto stage = Wrap(strings::ToI32(block.property(u8"stage", u8"0")), 0);
    s->set(u8"age_bit", Bool(stage > 0));
    BambooStalkThicknessFromAge(s, block, o);
    return AttachStates(c, s);
  }

  static CompoundTagPtr FlowingLiquid(Block const &block) {
    auto c = New(block.fName, true);
    auto s = States();
    s->set(u8"liquid_depth", Int(8));
    return AttachStates(c, s);
  }

  static CompoundTagPtr PistonArmCollision(Block const &block) {
    auto c = New(u8"piston_arm_collision");
    auto s = States();
    auto direction = strings::ToI32(block.property(u8"facing_direction", u8"0"));
    s->set(u8"facing_direction", Int(Wrap(direction, 0)));
    return AttachStates(c, s);
  }

  static CompoundTagPtr StickyPistonArmCollision(Block const &block) {
    auto c = New(u8"sticky_piston_arm_collision");
    auto s = States();
    auto direction = strings::ToI32(block.property(u8"facing_direction", u8"0"));
    s->set(u8"facing_direction", Int(Wrap(direction, 0)));
    return AttachStates(c, s);
  }

  static CompoundTagPtr MovingPiston(Block const &block, CompoundTagConstPtr const &, Options const &o) {
    auto c = New(u8"moving_block");
    auto s = States();
    return AttachStates(c, s);
  }

  static CompoundTagPtr PistonHead(Block const &block, CompoundTagConstPtr const &, Options const &o) {
    auto type = block.property(u8"type", u8"normal");
    auto c = New(type == u8"normal" ? u8"piston_arm_collision" : u8"sticky_piston_arm_collision");
    auto f6 = Facing6FromJavaName(block.property(u8"facing", u8""));
    auto direction = BedrockFacingDirectionBFromFacing6(f6);
    auto s = States();
    s->set(u8"facing_direction", Int(direction));
    return AttachStates(c, s);
  }

  static CompoundTagPtr BigDripleaf(Block const &block, CompoundTagConstPtr const &, Options const &o) {
    auto c = New(u8"big_dripleaf");
    auto s = States();
    s->set(u8"big_dripleaf_head", Bool(true));
    CardinalDirectionFromFacing4(s, block, o);
    auto tilt = block.property(u8"tilt", u8"none");
    if (tilt == u8"none") {
      //nop
    } else if (tilt == u8"partial") {
      tilt = u8"partial_tilt";
    } else if (tilt == u8"full") {
      tilt = u8"full_tilt";
    } else if (tilt == u8"unstable") {
      //nop
    }
    s->set(u8"big_dripleaf_tilt", std::u8string(tilt));
    return AttachStates(c, s);
  }

  static CompoundTagPtr RespawnAnchor(Block const &block, CompoundTagConstPtr const &, Options const &o) {
    auto c = New(u8"respawn_anchor");
    auto s = States();
    auto charges = strings::ToI32(block.property(u8"charges", u8"0"));
    i32 charge = 0;
    if (charges) {
      charge = *charges;
    }
    s->set(u8"respawn_anchor_charge", Int(charge));
    return AttachStates(c, s);
  }

  static CompoundTagPtr Tripwire(Block const &block, CompoundTagConstPtr const &, Options const &o) {
    auto c = New(u8"trip_wire");
    auto s = States();
    auto attached = block.property(u8"attached", u8"false") == u8"true";
    auto disarmed = block.property(u8"disarmed", u8"false") == u8"true";
    auto powered = block.property(u8"powered", u8"false") == u8"true";
    s->set(u8"attached_bit", Bool(attached));
    s->set(u8"disarmed_bit", Bool(disarmed));
    s->set(u8"powered_bit", Bool(powered));
    s->set(u8"suspended_bit", Bool(false));
    return AttachStates(c, s);
  }

  static CompoundTagPtr Tnt(Block const &block, CompoundTagConstPtr const &, Options const &o) {
    auto c = New(u8"tnt");
    auto s = States();
    auto unstable = block.property(u8"unstable", u8"false") == u8"true";
    s->set(u8"explode_bit", Bool(unstable));
    s->set(u8"allow_underwater_bit", Bool(false));
    return AttachStates(c, s);
  }

  static CompoundTagPtr StructureBlock(Block const &block, CompoundTagConstPtr const &, Options const &o) {
    auto c = New(u8"structure_block");
    auto s = States();
    auto mode = block.property(u8"mode", u8"save");
    s->set(u8"structure_block_type", std::u8string(mode));
    return AttachStates(c, s);
  }

  static CompoundTagPtr Scaffolding(Block const &block, CompoundTagConstPtr const &, Options const &o) {
    auto c = New(u8"scaffolding");
    auto s = States();
    auto distance = strings::ToI32(block.property(u8"distance", u8"0"));
    auto bottom = block.property(u8"bottom", u8"false") == u8"true";
    i32 stability = 0;
    if (distance) {
      stability = *distance;
    }
    s->set(u8"stability", Int(stability));
    s->set(u8"stability_check", Bool(bottom));
    return AttachStates(c, s);
  }

  static CompoundTagPtr BrewingStand(Block const &block, CompoundTagConstPtr const &, Options const &o) {
    auto c = New(u8"brewing_stand");
    auto has0 = block.property(u8"has_bottle_0", u8"false") == u8"true";
    auto has1 = block.property(u8"has_bottle_1", u8"false") == u8"true";
    auto has2 = block.property(u8"has_bottle_2", u8"false") == u8"true";
    auto s = States();
    s->set(u8"brewing_stand_slot_a_bit", Bool(has0));
    s->set(u8"brewing_stand_slot_b_bit", Bool(has1));
    s->set(u8"brewing_stand_slot_c_bit", Bool(has2));
    return AttachStates(c, s);
  }

  static CompoundTagPtr CaveVines(Block const &block, CompoundTagConstPtr const &, Options const &o) {
    bool berries = block.property(u8"berries", u8"false") == u8"true";
    auto age = Wrap(strings::ToI32(block.property(u8"age", u8"1")), 1);
    auto c = New(berries ? u8"cave_vines_head_with_berries" : u8"cave_vines");
    auto s = States();
    s->set(u8"growing_plant_age", Int(age));
    return AttachStates(c, s);
  }

  static CompoundTagPtr CaveVinesPlant(Block const &block, CompoundTagConstPtr const &, Options const &o) {
    bool berries = block.property(u8"berries", u8"false") == u8"true";
    auto c = New(berries ? u8"cave_vines_body_with_berries" : u8"cave_vines");
    auto s = States();
    s->set(u8"growing_plant_age", Int(24));
    return AttachStates(c, s);
  }

  static PropertyType Candles(Block const &block) {
    auto candles = block.property(u8"candles", u8"1");
    auto num = strings::ToI32(candles);
    int i = 0;
    if (num) {
      i = std::clamp(*num, 1, 4) - 1;
    }
    return Int(i);
  }

  static CompoundTagPtr Light(Block const &b, CompoundTagConstPtr const &, Options const &o) {
    auto c = New(u8"light_block");
    auto s = States();
    auto level = strings::ToI32(b.property(u8"level", u8"15"));
    s->set(u8"block_light_level", Int(level ? *level : 15));
    return AttachStates(c, s);
  }

  static void Debug(CompoundTagPtr const &s, Block const &b, Options const &o) {
    (void)s;
  }

  static CompoundTagPtr PointedDripstone(Block const &b, CompoundTagConstPtr const &, Options const &o) {
    auto c = New(u8"pointed_dripstone");
    auto s = States();
    auto thickness = b.property(u8"thickness", u8"tip");
    if (thickness == u8"tip_merge") {
      thickness = u8"merge";
    }
    auto direction = b.property(u8"vertical_direction", u8"down");
    s->set(u8"dripstone_thickness", std::u8string(thickness));
    s->set(u8"hanging", Bool(direction == u8"down"));
    return AttachStates(c, s);
  }

  static void Conditional(CompoundTagPtr const &s, Block const &b, Options const &o) {
    auto conditional = b.property(u8"conditional", u8"false");
    s->set(u8"conditional_bit", Bool(conditional == u8"true"));
  }

  static void BambooStalkThicknessFromAge(CompoundTagPtr const &s, Block const &b, Options const &o) {
    auto age = b.property(u8"age", u8"0");
    if (age == u8"0") {
      s->set(u8"bamboo_stalk_thickness", u8"thin");
    } else {
      s->set(u8"bamboo_stalk_thickness", u8"thick");
    }
  }

  static void HoneyLevel(CompoundTagPtr const &s, Block const &b, Options const &o) {
    auto v = strings::ToI32(b.property(u8"honey_level", u8"0"));
    i32 level = v ? *v : 0;
    s->set(u8"honey_level", Int(level));
  }

  static void TurtleEggCount(CompoundTagPtr const &s, Block const &b, Options const &o) {
    auto eggs = b.property(u8"eggs", u8"1");
    std::u8string eggCount = u8"one_egg";
    if (eggs == u8"1") {
      eggCount = u8"one_egg";
    } else if (eggs == u8"2") {
      eggCount = u8"two_egg";
    } else if (eggs == u8"3") {
      eggCount = u8"three_egg";
    } else if (eggs == u8"4") {
      eggCount = u8"four_egg";
    }
    s->set(u8"turtle_egg_count", eggCount);
  }

  static void TurtleEggCrackedState(CompoundTagPtr const &s, Block const &b, Options const &o) {
    auto hatch = b.property(u8"hatch", u8"0");
    std::u8string state = u8"no_cracks";
    if (hatch == u8"0") {
      state = u8"no_cracks";
    } else if (hatch == u8"1") {
      state = u8"cracked";
    } else if (hatch == u8"2") {
      state = u8"max_cracked";
    }
    s->set(u8"cracked_state", state);
  }

  static void WallSkullFacingDirection(CompoundTagPtr const &s, Block const &b, Options const &o) {
    auto facing = b.property(u8"facing", u8"");
    i32 direction = 1;
    if (facing == u8"south") {
      direction = 3;
    } else if (facing == u8"east") {
      direction = 5;
    } else if (facing == u8"north") {
      direction = 2;
    } else if (facing == u8"west") {
      direction = 4;
    }
    s->set(u8"facing_direction", Int(direction));
  }

  static void CauldronFillLevelFromLevel(CompoundTagPtr const &s, Block const &b, Options const &o) {
    auto level = Wrap(strings::ToI32(b.property(u8"level", u8"0")), 0);
    s->set(u8"fill_level", Int(level * 2));
  }

  static void BiteCounterFromBites(CompoundTagPtr const &s, Block const &b, Options const &o) {
    auto bites = Wrap(strings::ToI32(b.property(u8"bites", u8"0")), 0);
    s->set(u8"bite_counter", Int(bites));
  }

  static void DragDownFromDrag(CompoundTagPtr const &s, Block const &b, Options const &o) {
    auto drag = b.property(u8"drag", u8"true") == u8"true";
    s->set(u8"drag_down", Bool(drag));
  }

  static void GrowthFromAge(CompoundTagPtr const &s, Block const &b, Options const &o) {
    auto age = Wrap(strings::ToI32(b.property(u8"age", u8"0")), 0);
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
    s->set(u8"growth", Int(growth));
  }

  static void AgeBitFromAge(CompoundTagPtr const &s, Block const &b, Options const &o) {
    auto age = b.property(u8"age", u8"0");
    s->set(u8"age", Bool(age == u8"1"));
  }

  static void BambooLeafSizeFromLeaves(CompoundTagPtr const &s, Block const &b, Options const &o) {
    auto leaves = b.property(u8"leaves", u8"none");
    std::u8string size = u8"no_leaves";
    if (leaves == u8"none") {
      size = u8"no_leaves";
    } else if (leaves == u8"large") {
      size = u8"large_leaves";
    } else if (leaves == u8"small") {
      size = u8"small_leaves";
    }
    s->set(u8"bamboo_leaf_size", size);
  }

  static PropertyType Axis(Block const &b) {
    auto axis = b.property(u8"axis", u8"x");
    return String(axis);
  }

  static void RailDirectionFromShape(CompoundTagPtr const &s, Block const &b, Options const &o) {
    auto shape = b.property(u8"shape", u8"north_south");
    i32 direction = 0;
    if (shape == u8"north_south") {
      direction = 0;
    } else if (shape == u8"east_west") {
      direction = 1;
    } else if (shape == u8"ascending_east") {
      direction = 2;
    } else if (shape == u8"ascending_south") {
      direction = 5;
    } else if (shape == u8"ascending_west") {
      direction = 3;
    } else if (shape == u8"ascending_north") {
      direction = 4;
    } else if (shape == u8"north_east") {
      direction = 9;
    } else if (shape == u8"north_west") {
      direction = 8;
    } else if (shape == u8"south_east") {
      direction = 6;
    } else if (shape == u8"south_west") {
      direction = 7;
    }
    s->set(u8"rail_direction", Int(direction));
  }

  static void OutputSubtractBitFromMode(CompoundTagPtr const &s, Block const &b, Options const &o) {
    auto mode = b.property(u8"mode", u8"compare");
    s->set(u8"output_subtract_bit", Bool(mode == u8"subtract"));
  }

  static PropertyType Delay(Block const &b) {
    auto delay = Wrap(strings::ToI32(b.property(u8"delay", u8"1")), 1);
    return Int(delay - 1);
  }

  static PropertyType Facing(Block const &b) {
    auto facing = b.property(u8"facing", u8"down");
    return String(facing);
  }

  static std::u8string ComparatorName(Block const &b, Options const &o) {
    auto powered = b.property(u8"powered", u8"false") == u8"true";
    if (powered) {
      return u8"minecraft:powered_comparator";
    } else {
      return u8"minecraft:unpowered_comparator";
    }
  }

  static std::u8string RepeaterName(Block const &b, Options const &o) {
    auto powered = b.property(u8"powered", u8"false") == u8"true";
    if (powered) {
      return u8"minecraft:powered_repeater";
    } else {
      return u8"minecraft:unpowered_repeater";
    }
  }

  static std::u8string PrefixLit(Block const &b, Options const &o) {
    auto lit = b.property(u8"lit", u8"false") == u8"true";
    if (lit && !o.fItem) {
      auto name = Namespace::Remove(b.fName);
      return u8"minecraft:lit_" + std::u8string(name);
    } else {
      return b.name();
    }
  }

  static NamingFunction PrefixUnlit(std::u8string name) {
    return [name](Block const &b, Options const &o) {
      auto lit = b.property(u8"lit", u8"false") == u8"true";
      if (lit || o.fItem) {
        return u8"minecraft:" + name;
      } else {
        return u8"minecraft:unlit_" + name;
      }
    };
  }

  static void DoorHingeBitFromHinge(CompoundTagPtr const &s, Block const &b, Options const &o) {
    auto hinge = b.property(u8"hinge", u8"left");
    s->set(u8"door_hinge_bit", Bool(hinge == u8"right"));
  }

  static void ToggleBitFromEnabled(CompoundTagPtr const &s, Block const &b, Options const &o) {
    auto enabled = b.property(u8"enabled", u8"true") == u8"true";
    s->set(u8"toggle_bit", Bool(!enabled));
  }

  static std::u8string DaylightDetectorName(Block const &b, Options const &o) {
    auto inverted = b.property(u8"inverted", u8"false") == u8"true";
    if (inverted) {
      return u8"minecraft:daylight_detector_inverted";
    } else {
      return u8"minecraft:daylight_detector";
    }
  }

  static PropertyType Attached(Block const &b) { return Bool(b.property(u8"attached", u8"false") == u8"true"); }

  static void ButtonFacingDirection(CompoundTagPtr const &s, Block const &b, Options const &o) {
    if (o.fItem) {
      s->set(u8"facing_direction", Int(0));
    } else {
      auto face = b.property(u8"face", u8"wall");
      auto facing = b.property(u8"facing", u8"north");
      i32 direction = 0;
      if (face == u8"floor") {
        direction = 1;
      } else if (face == u8"ceiling") {
        direction = 0;
      } else {
        if (facing == u8"south") {
          direction = 3;
        } else if (facing == u8"north") {
          direction = 2;
        } else if (facing == u8"east") {
          direction = 5;
        } else if (facing == u8"west") {
          direction = 4;
        }
      }
      s->set(u8"facing_direction", Int(direction));
    }
  }

  static PropertyType Power(Block const &b) { return Int(Wrap(strings::ToI32(b.property(u8"power", u8"0")), 0)); }

  static PropertyType InWall(Block const &b) { return Bool(b.property(u8"in_wall", u8"false") == u8"true"); }

  static void LeverDirection(CompoundTagPtr const &s, Block const &b, Options const &o) {
    std::u8string result;
    if (o.fItem) {
      result = u8"down_east_west";
    } else {
      auto face = b.property(u8"face", u8"wall");
      auto facing = b.property(u8"facing", u8"north");
      if (face == u8"floor") {
        if (facing == u8"west" || facing == u8"east") {
          result = u8"up_east_west";
        } else {
          result = u8"up_north_south";
        }
      } else if (face == u8"ceiling") {
        if (facing == u8"west" || facing == u8"east") {
          result = u8"down_east_west";
        } else {
          result = u8"down_north_south";
        }
      } else {
        result = facing;
      }
    }
    s->set(u8"lever_direction", result);
  }

  static PropertyType Triggered(Block const &b) {
    bool v = b.property(u8"triggered", u8"false") == u8"true";
    return Bool(v);
  }

  static std::u8string GetAttachment(std::u8string_view const &attachment) {
    if (attachment == u8"floor") {
      return u8"standing";
    } else if (attachment == u8"ceiling") {
      return u8"hanging";
    } else if (attachment == u8"double_wall") {
      return u8"multiple";
    } else if (attachment == u8"single_wall") {
      return u8"side";
    }
    return std::u8string(attachment);
  }

  static CompoundTagPtr New(std::u8string_view const &name, bool nameIsFull = false) {
    using namespace std;
    auto tag = Compound();
    u8string fullName = nameIsFull ? u8string(name) : u8"minecraft:" + u8string(name);
    tag->set(u8"name", fullName);
    tag->set(u8"version", Int(kBlockDataVersion));
    return tag;
  }

  static CompoundTagPtr States() { return Compound(); }

  static CompoundTagPtr AttachStates(CompoundTagPtr const &data, CompoundTagPtr const &s) {
    data->set(u8"states", s);
    return data;
  }
};

CompoundTagPtr BlockData::From(std::shared_ptr<mcfile::je::Block const> const &input, CompoundTagConstPtr const &tile, DataVersion const &dataVersion, BlockData::Options const &options) {
  using namespace std;
  if (!input) {
    return Air();
  }
  // "j2b:sticky_piston_arm_collision" is created at Converter::PreprocessChunk
  if (input->fName == u8"j2b:sticky_piston_arm_collision") {
    return Impl::StickyPistonArmCollision(*input);
  } else if (input->fName == u8"j2b:piston_arm_collision") {
    return Impl::PistonArmCollision(*input);
  }
  static unique_ptr<vector<Impl::AnyConverter> const> const table(Impl::CreateConverterTable());
  auto block = make_shared<mcfile::je::Block>(input->fId, mcfile::blocks::Name(input->fId, dataVersion.fTarget), u8string(input->fData), dataVersion.fTarget);
  u32 index = static_cast<u32>(block->fId);
  if (index < table->size()) {
    Impl::AnyConverter func = (*table)[index];
    if (func) {
      return func(*block, tile, options);
    }
  }
  if (block->fName == u8"minecraft:flowing_water" || block->fName == u8"minecraft:flowing_lava") {
    return Impl::FlowingLiquid(*block);
  } else {
    return Impl::Identity(*block, nullptr, options);
  }
}

CompoundTagPtr BlockData::Air() {
  static CompoundTagPtr const air = Make(u8"air");
  return air;
}

CompoundTagPtr BlockData::Make(std::u8string const &name) {
  auto tag = Impl::New(name);
  auto states = Impl::States();
  tag->set(u8"states", states);
  return tag;
}

i32 BlockData::GetFacingDirectionAFromFacing(mcfile::je::Block const &block) {
  auto facing = block.property(u8"facing", u8"down");
  auto f6 = Facing6FromJavaName(facing);
  return BedrockFacingDirectionAFromFacing6(f6);
}

} // namespace je2be::java
