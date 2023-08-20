#pragma once

#include <array>

namespace je2be::toje {

class LegacyBlock {
  LegacyBlock() = delete;

  using String = std::u8string;
  using Migrator = std::function<String(String const &name, int16_t val, CompoundTag &)>;

  struct Compose {
    template <class... Args>
    Compose(Args... migrators) : fMigrators(std::initializer_list<Migrator>{migrators...}) {
    }

    String operator()(String const &n, int16_t v, CompoundTag &s) {
      String name = n;
      for (auto const &m : fMigrators) {
        name = m(name, v, s);
      }
      return name;
    }

    std::vector<Migrator> fMigrators;
  };

  template <size_t RightShift, size_t Bits>
  struct PropertySelector {
    static_assert(RightShift < 16);
    static_assert(Bits > 0);

    PropertySelector(String const &propertyName,
                     std::array<std::optional<String>, 1 << Bits> const &table,
                     std::optional<String> blockName = std::nullopt)
        : fPropertyName(propertyName),
          fBlockName(blockName) {
      fTable.resize(1 << Bits);
      for (size_t i = 0; i < table.size(); i++) {
        fTable[i] = table[i];
      }
    }

    String operator()(String const &n, int16_t v, CompoundTag &s) const {
      constexpr uint16_t cMask = ~((~uint16_t(0)) << Bits);
      int16_t index = (v >> RightShift) & cMask;
      if (0 <= index && index < fTable.size()) {
        std::optional<String> value = fTable[index];
        if (value) {
          s[fPropertyName] = std::make_shared<StringTag>(*value);
        }
      }
      if (fBlockName) {
        return *fBlockName;
      } else {
        return n;
      }
    }

    String fPropertyName;
    std::vector<std::optional<String>> fTable;
    std::optional<String> fBlockName;
  };

  template <size_t RightShift, size_t Bits>
  struct NamePrefixer {
    NamePrefixer(String const &suffix, std::array<std::optional<String>, 1 << Bits> const &table) : fSuffix(suffix) {
      fTable.resize(1 << Bits);
      for (size_t i = 0; i < (1 << Bits); i++) {
        fTable[i] = table[i];
      }
    }

    String operator()(String const &n, int16_t v, CompoundTag &s) const {
      constexpr uint16_t cMask = ~((~uint16_t(0)) << Bits);
      int16_t index = (v >> RightShift) & cMask;
      if (0 <= index && index < fTable.size()) {
        auto prefix = fTable[index];
        if (prefix) {
          return *prefix + fSuffix;
        } else {
          return n;
        }
      } else {
        return n;
      }
    }

    std::vector<std::optional<String>> fTable;
    String fSuffix;
  };

public:
  static void Migrate(mcfile::be::Block &b) {
    if (b.fVal) {
      int16_t val = *b.fVal;

      // 1.10.1.1: val
      // 1.11.0.23: states: version = 17432626
      // 1.11.4.2: states: version = 17432626
      // 1.12.1.1: states: version = 17563649
      // 1.14.60.5: states: version = 17760256

      b.fVersion = 18090528;
      b.fVal = std::nullopt;
      if (!b.fStates) {
        b.fStates = std::make_shared<mcfile::nbt::CompoundTag>();
      }
      static std::unique_ptr<std::unordered_map<std::u8string, Migrator> const> const sTable(CreateMigratorTable());
      std::u8string name = Namespace::Remove(b.fName);
      if (auto found = sTable->find(name); found != sTable->end()) {
        b.fName = Namespace::Add(found->second(name, val, *b.fStates));
      }
    }
  }

private:
  static String Dirt(String const &n, int16_t val, CompoundTag &s) {
    switch (val) {
    case 0:
    default:
      s[u8"dirt_type"] = Str(u8"normal");
      break;
    }
    return n;
  }

  static String BlockWithDirection(String const &n, int16_t v, CompoundTag &s) {
    s[u8"direction"] = Int(v);
    return n;
  }

  template <size_t RightShift, size_t Bits>
  static Migrator IntProperty(String name) {
    static_assert(0 < Bits && Bits <= 16);
    return [=](String const &n, int16_t v, CompoundTag &s) {
      constexpr uint16_t cMask = ~((~uint16_t(0)) << Bits);
      s[name] = Int((v >> RightShift) & cMask);
      return n;
    };
  }

  template <size_t RightShift, size_t Bits>
  static Migrator ByteProperty(String name) {
    static_assert(0 < Bits && Bits <= 8);
    return [=](String const &n, int16_t v, CompoundTag &s) {
      constexpr uint16_t cMask = ~((~uint16_t(0)) << Bits);
      s[name] = std::make_shared<ByteTag>((v >> RightShift) & cMask);
      return n;
    };
  }

  template <size_t RightShift>
  static Migrator BoolProperty(String name) {
    return [=](String const &n, int16_t v, CompoundTag &s) {
      s[name] = Bool(((v >> RightShift) & 0x1) == 0x1);
      return n;
    };
  }

  static String TopSlotBit(String const &n, int16_t v, CompoundTag &s) {
    s[u8"top_slot_bit"] = Bool((v & 0x8) == 0x8);
    return n;
  }

  static String StrippedBit(String const &n, int16_t v, CompoundTag &s) {
    s[u8"stripped_bit"] = Bool((v & 0x8) == 0x8);
    return n;
  }

  static String Scaffolding(String const &n, int16_t v, CompoundTag &s) {
    s[u8"stability_check"] = Bool((v & 0x8) == 0x8);
    s[u8"stability"] = Int(v & 0x7);
    return n;
  }

  static Migrator Rename(String n) {
    return [n](String const &, int16_t v, CompoundTag &s) {
      return n;
    };
  }

  static Migrator Const(String propertyName, TagPtr propertyValue) {
    return [=](String const &n, int16_t v, CompoundTag &s) {
      s[propertyName] = propertyValue;
      return n;
    };
  }

  static String Growth(String const &n, int16_t v, CompoundTag &s) {
    s[u8"growth"] = Int((v & 0x7));
    return n;
  }

  static std::unordered_map<std::u8string, Migrator> *CreateMigratorTable() {
    using namespace std;
    auto table = new unordered_map<u8string, Migrator>();

    auto E = [&](u8string const &name, Migrator m) {
      assert(table->find(name) == table->end());
      table->insert(make_pair(name, m));
    };

    static array<optional<String>, 16> const sLegacyColors = {
        u8"white",      // 0
        u8"orange",     // 1
        u8"magenta",    // 2
        u8"light_blue", // 3
        u8"yellow",     // 4
        u8"lime",       // 5
        u8"pink",       // 6
        u8"gray",       // 7
        u8"silver",     // 8
        u8"cyan",       // 9
        u8"purple",     // 10
        u8"blue",       // 11
        u8"brown",      // 12
        u8"green",      // 13
        u8"red",        // 14
        u8"black",      // 15
    };
    static array<optional<String>, 16> const sColors = {
        u8"white",      // 0
        u8"orange",     // 1
        u8"magenta",    // 2
        u8"light_blue", // 3
        u8"yellow",     // 4
        u8"lime",       // 5
        u8"pink",       // 6
        u8"gray",       // 7
        u8"light_gray", // 8
        u8"cyan",       // 9
        u8"purple",     // 10
        u8"blue",       // 11
        u8"brown",      // 12
        u8"green",      // 13
        u8"red",        // 14
        u8"black",      // 15
    };
    static PropertySelector<0, 4> const sColoredBlock(u8"color", sLegacyColors);
    static array<optional<String>, 8> const sWoodTypes = {
        u8"oak",      // 0
        u8"spruce",   // 1
        u8"birch",    // 2
        u8"jungle",   // 3
        u8"acacia",   // 4
        u8"dark_oak", // 5
        nullopt,
        nullopt,
    };

    static PropertySelector<0, 1> const sDirtType(u8"dirt_type", {
                                                                     u8"normal", // 0
                                                                     u8"coarse", // 1
                                                                 });
    E(u8"dirt", sDirtType);
    static PropertySelector<0, 3> const sWoodType(u8"wood_type", sWoodTypes);
    E(u8"planks", sWoodType);
    static Compose const sCobblestoneWall(PropertySelector<0, 4>(u8"wall_block_type", {
                                                                                          u8"cobblestone",       // 0
                                                                                          u8"mossy_cobblestone", // 1
                                                                                          u8"granite",           // 2
                                                                                          u8"diorite",           // 3
                                                                                          u8"andesite",          // 4
                                                                                          u8"sandstone",         // 5
                                                                                          u8"brick",             // 6
                                                                                          u8"stone_brick",       // 7
                                                                                          u8"mossy_stone_brick", // 8
                                                                                          u8"nether_brick",      // 9
                                                                                          u8"end_brick",         // 10
                                                                                          u8"prismarine",        // 11
                                                                                          u8"red_sandstone",     // 12
                                                                                          u8"red_nether_brick",  // 13
                                                                                          nullopt,
                                                                                          nullopt,
                                                                                      }));
    E(u8"cobblestone_wall", sCobblestoneWall);
    E(u8"fence", NamePrefixer<0, 3>(u8"_fence", sWoodTypes));
    static Compose const sFenceGate(IntProperty<0, 2>(u8"direction"), BoolProperty<2>(u8"open_bit"), BoolProperty<3>(u8"in_wall_bit"));
    E(u8"fence_gate", sFenceGate);
    E(u8"spruce_fence_gate", sFenceGate);
    E(u8"birch_fence_gate", sFenceGate);
    E(u8"jungle_fence_gate", sFenceGate);
    E(u8"acacia_fence_gate", sFenceGate);
    E(u8"dark_oak_fence_gate", sFenceGate);
    static Compose const sStairs(IntProperty<0, 2>(u8"weirdo_direction"), BoolProperty<2>(u8"upside_down_bit"));
    E(u8"normal_stone_stairs", sStairs);
    E(u8"stone_stairs", sStairs);
    E(u8"mossy_cobblestone_stairs", sStairs);
    E(u8"oak_stairs", sStairs);
    E(u8"spruce_stairs", sStairs);
    E(u8"birch_stairs", sStairs);
    E(u8"jungle_stairs", sStairs);
    E(u8"acacia_stairs", sStairs);
    E(u8"dark_oak_stairs", sStairs);
    E(u8"stone_brick_stairs", sStairs);
    E(u8"mossy_stone_brick_stairs", sStairs);
    E(u8"sandstone_stairs", sStairs);
    E(u8"smooth_sandstone_stairs", sStairs);
    E(u8"red_sandstone_stairs", sStairs);
    E(u8"smooth_red_sandstone_stairs", sStairs);
    E(u8"granite_stairs", sStairs);
    E(u8"polished_granite_stairs", sStairs);
    E(u8"diorite_stairs", sStairs);
    E(u8"polished_diorite_stairs", sStairs);
    E(u8"andesite_stairs", sStairs);
    E(u8"polished_andesite_stairs", sStairs);
    E(u8"brick_stairs", sStairs);
    E(u8"nether_brick_stairs", sStairs);
    E(u8"red_nether_brick_stairs", sStairs);
    E(u8"end_brick_stairs", sStairs);
    E(u8"quartz_stairs", sStairs);
    E(u8"smooth_quartz_stairs", sStairs);
    E(u8"purpur_stairs", sStairs);
    E(u8"prismarine_stairs", sStairs);
    E(u8"dark_prismarine_stairs", sStairs);
    E(u8"prismarine_bricks_stairs", sStairs);
    static Compose const sDoor(IntProperty<0, 2>(u8"direction"), BoolProperty<2>(u8"open_bit"), BoolProperty<3>(u8"upper_block_bit"));
    E(u8"wooden_door", sDoor);
    E(u8"spruce_door", sDoor);
    E(u8"birch_door", sDoor);
    E(u8"jungle_door", sDoor);
    E(u8"acacia_door", sDoor);
    E(u8"dark_oak_door", sDoor);
    static Compose const sTrapdoor(IntProperty<0, 2>(u8"direction"), BoolProperty<3>(u8"open_bit"), BoolProperty<2>(u8"upside_down_bit"));
    E(u8"trapdoor", sTrapdoor);
    E(u8"spruce_trapdoor", sTrapdoor);
    E(u8"birch_trapdoor", sTrapdoor);
    E(u8"jungle_trapdoor", sTrapdoor);
    E(u8"acacia_trapdoor", sTrapdoor);
    E(u8"dark_oak_trapdoor", sTrapdoor);
    E(u8"iron_trapdoor", sTrapdoor);
    E(u8"stained_glass", sColoredBlock);
    E(u8"stained_glass_pane", sColoredBlock);
    static Migrator const sBlockWithFacingDirection = IntProperty<0, 16>(u8"facing_direction");
    E(u8"ladder", sBlockWithFacingDirection);
    static PropertySelector<0, 3> const sStoneType(u8"stone_type", {
                                                                       u8"stone",           // 0
                                                                       u8"granite",         // 1
                                                                       u8"granite_smooth",  // 2
                                                                       u8"diorite",         // 3
                                                                       u8"diorite_smooth",  // 4
                                                                       u8"andesite",        // 5
                                                                       u8"andesite_smooth", // 6
                                                                       nullopt,
                                                                   });
    E(u8"stone", sStoneType);
    static PropertySelector<0, 3> const sStoneSlabType(u8"stone_slab_type", {
                                                                                u8"smooth_stone", // 0
                                                                                u8"sandstone",    // 1
                                                                                nullopt,          // 2
                                                                                u8"cobblestone",  // 3
                                                                                u8"brick",        // 4
                                                                                u8"stone_brick",  // 5
                                                                                u8"quartz",       // 6
                                                                                u8"nether_brick", // 7
                                                                            });
    E(u8"stone_slab", Compose(sStoneSlabType, Rename(u8"stone_block_slab"), TopSlotBit));
    E(u8"double_stone_slab", Compose(sStoneSlabType, Rename(u8"double_stone_block_slab"), TopSlotBit));
    static PropertySelector<0, 3> const sStoneSlabType2(u8"stone_slab_type_2", {
                                                                                   u8"red_sandstone",     // 0
                                                                                   u8"purpur",            // 1
                                                                                   u8"prismarine_rough",  // 2
                                                                                   u8"prismarine_dark",   // 3
                                                                                   u8"prismarine_brick",  // 4
                                                                                   u8"mossy_cobblestone", // 5
                                                                                   u8"smooth_sandstone",  // 6
                                                                                   u8"red_nether_brick",  // 7
                                                                               });
    E(u8"stone_slab2", Compose(sStoneSlabType2, Rename(u8"stone_block_slab2"), TopSlotBit));
    E(u8"double_stone_slab2", Compose(sStoneSlabType2, Rename(u8"double_stone_block_slab2"), TopSlotBit));
    static PropertySelector<0, 3> const sStoneSlabType3(u8"stone_slab_type_3", {
                                                                                   u8"end_stone_brick",      // 0
                                                                                   u8"smooth_red_sandstone", // 1
                                                                                   u8"polished_andesite",    // 2
                                                                                   u8"andesite",             // 3
                                                                                   u8"diorite",              // 4
                                                                                   u8"polished_diorite",     // 5
                                                                                   u8"granite",              // 6
                                                                                   u8"polished_granite",     // 7
                                                                               });
    E(u8"stone_slab3", Compose(sStoneSlabType3, Rename(u8"stone_block_slab3"), TopSlotBit));
    E(u8"double_stone_slab3", Compose(sStoneSlabType3, Rename(u8"double_stone_block_slab3"), TopSlotBit));
    static PropertySelector<0, 3> const sStoneSlabType4(u8"stone_slab_type_4", {
                                                                                   u8"mossy_stone_brick", // 0
                                                                                   nullopt,               // 1
                                                                                   u8"stone",             // 2
                                                                                   u8"cut_sandstone",     // 3
                                                                                   u8"cut_red_sandstone", // 4
                                                                               });
    E(u8"stone_slab4", Compose(sStoneSlabType4, Rename(u8"stone_block_slab4"), TopSlotBit));
    E(u8"double_stone_slab4", Compose(sStoneSlabType4, Rename(u8"double_stone_block_slab4"), TopSlotBit));
    E(u8"scaffolding", Scaffolding);
    static Compose const sWoodenSlab(PropertySelector<0, 3>(u8"wood_type", sWoodTypes),
                                     TopSlotBit);
    E(u8"wooden_slab", sWoodenSlab);
    E(u8"double_wooden_slab", sWoodenSlab);
    static PropertySelector<0, 2> const sStoneBrickType(u8"stone_brick_type", {
                                                                                  u8"default",  // 0
                                                                                  u8"mossy",    // 1
                                                                                  u8"cracked",  // 2
                                                                                  u8"chiseled", // 3
                                                                              });
    E(u8"stonebrick", sStoneBrickType);
    static PropertySelector<0, 2> const sPrismarineBlockType(u8"prismarine_block_type", {
                                                                                            u8"default", // 0
                                                                                            u8"dark",    // 1
                                                                                            u8"bricks",  // 2
                                                                                        });
    E(u8"prismarine", sPrismarineBlockType);
    static PropertySelector<0, 2> const sSandstoneType(u8"sand_stone_type", {
                                                                                u8"default",     // 0
                                                                                u8"heiroglyphs", // 1
                                                                                u8"cut",         // 2
                                                                                u8"smooth",      // 3
                                                                            });
    E(u8"sandstone", sSandstoneType);
    E(u8"red_sandstone", sSandstoneType);
    static PropertySelector<0, 2> const sChiselType(u8"chisel_type", {
                                                                         u8"default",  // 0
                                                                         u8"chiseled", // 1
                                                                         u8"lines",    // 2
                                                                         u8"smooth",   // 3
                                                                     });
    static array<optional<String>, 4> const sPillarAxes = {
        u8"y", // 0
        u8"x", // 1
        u8"z", // 2
    };
    static PropertySelector<0, 2> const sPillarAxis0(u8"pillar_axis", sPillarAxes);
    static PropertySelector<2, 2> const sPillarAxis2(u8"pillar_axis", sPillarAxes);
    E(u8"quartz_block", Compose(sChiselType, sPillarAxis2));
    E(u8"hay_block", Compose(sPillarAxis2, Const(u8"deprecated", Int(0))));
    E(u8"bone_block", Compose(sPillarAxis2, Const(u8"deprecated", Int(0))));
    E(u8"wool", NamePrefixer<0, 4>(u8"_wool", sColors));
    E(u8"carpet", NamePrefixer<0, 4>(u8"_carpet", sColors));
    E(u8"concretePowder", Compose(Rename(u8"concrete_powder"), sColoredBlock));
    E(u8"concrete", NamePrefixer<0, 4>(u8"_concrete", sColors));
    E(u8"stained_hardened_clay", sColoredBlock);
    E(u8"white_glazed_terracotta", sBlockWithFacingDirection);
    E(u8"orange_glazed_terracotta", sBlockWithFacingDirection);
    E(u8"magenta_glazed_terracotta", sBlockWithFacingDirection);
    E(u8"light_blue_glazed_terracotta", sBlockWithFacingDirection);
    E(u8"yellow_glazed_terracotta", sBlockWithFacingDirection);
    E(u8"lime_glazed_terracotta", sBlockWithFacingDirection);
    E(u8"pink_glazed_terracotta", sBlockWithFacingDirection);
    E(u8"gray_glazed_terracotta", sBlockWithFacingDirection);
    E(u8"silver_glazed_terracotta", sBlockWithFacingDirection);
    E(u8"cyan_glazed_terracotta", sBlockWithFacingDirection);
    E(u8"purple_glazed_terracotta", sBlockWithFacingDirection);
    E(u8"blue_glazed_terracotta", sBlockWithFacingDirection);
    E(u8"brown_glazed_terracotta", sBlockWithFacingDirection);
    E(u8"green_glazed_terracotta", sBlockWithFacingDirection);
    E(u8"red_glazed_terracotta", sBlockWithFacingDirection);
    E(u8"black_glazed_terracotta", sBlockWithFacingDirection);
    E(u8"purpur_block", Compose(sChiselType, sPillarAxis2));
    static PropertySelector<0, 1> const sSandType(u8"sand_type", {
                                                                     u8"normal", // 0
                                                                     u8"red",    // 1
                                                                 });
    E(u8"sand", sSandType);
    static Migrator const sBlockWithAge = IntProperty<0, 16>(u8"age");
    E(u8"cactus", sBlockWithAge);
    E(u8"log", Compose(NamePrefixer<0, 2>(u8"_log", {
                                                        u8"oak",    // 0
                                                        u8"spruce", // 1
                                                        u8"birch",  // 2
                                                        u8"jungle", // 3
                                                    }),
                       sPillarAxis2));
    E(u8"stripped_oak_log", sPillarAxis0);
    E(u8"stripped_spruce_log", sPillarAxis0);
    E(u8"stripped_birch_log", sPillarAxis0);
    E(u8"stripped_jungle_log", sPillarAxis0);
    E(u8"stripped_acacia_log", sPillarAxis0);
    E(u8"stripped_dark_oak_log", sPillarAxis0);
    E(u8"log2", Compose(NamePrefixer<0, 2>(u8"_log", {
                                                         u8"acacia",   // 0
                                                         u8"dark_oak", // 1
                                                     }),
                        sPillarAxis2));
    E(u8"wood", Compose(sWoodType, Const(u8"pillar_axis", Str(u8"y")), StrippedBit));
    static Migrator const sLiquidDepth = IntProperty<0, 16>(u8"liquid_depth");
    E(u8"water", sLiquidDepth);
    E(u8"lava", sLiquidDepth);
    static Migrator const sPersistentBit = BoolProperty<3>(u8"persistent_bit");
    E(u8"leaves", Compose(PropertySelector<0, 3>(u8"old_leaf_type", {
                                                                        u8"oak",    // 0
                                                                        u8"spruce", // 1
                                                                        u8"birch",  // 2
                                                                        u8"jungle", // 3
                                                                    }),
                          sPersistentBit, Const(u8"update_bit", Bool(false))));
    E(u8"leaves2", Compose(PropertySelector<0, 3>(u8"new_leaf_type", {
                                                                         u8"acacia",   // 0
                                                                         u8"dark_oak", // 1
                                                                     }),
                           sPersistentBit, Const(u8"update_bit", Bool(false))));
    E(u8"sapling", Compose(PropertySelector<0, 3>(u8"sapling_type", sWoodTypes), Const(u8"age_bit", Bool(false))));
    E(u8"farmland", IntProperty<0, 16>(u8"moisturized_amount"));
    E(u8"wheat", Growth);
    static Migrator sStem = Compose(IntProperty<3, 4>(u8"facing_direction"), Growth);
    E(u8"pumpkin_stem", sStem);
    E(u8"melon_stem", sStem);
    E(u8"beetroot", Growth);
    static PropertySelector<0, 2> const sCardinalDirection(u8"minecraft:cardinal_direction", {
                                                                                                 u8"south", // 0
                                                                                                 u8"west",  // 1
                                                                                                 u8"north", // 2
                                                                                                 u8"east",  // 3
                                                                                             });
    E(u8"pumpkin", sCardinalDirection);
    E(u8"carved_pumpkin", sCardinalDirection);
    E(u8"lit_pumpkin", sCardinalDirection);
    E(u8"tallgrass", PropertySelector<0, 2>(u8"tall_grass_type", {
                                                                     nullopt,  // 0
                                                                     u8"tall", // 1
                                                                     u8"fern", // 2
                                                                 }));
    E(u8"double_plant", Compose(PropertySelector<0, 3>(u8"double_plant_type", {
                                                                                  u8"sunflower", // 0
                                                                                  u8"syringa",   // 1
                                                                                  u8"grass",     // 2
                                                                                  u8"fern",      // 3
                                                                                  u8"rose",      // 4
                                                                                  u8"paeonia",   // 5
                                                                              }),
                                BoolProperty<3>(u8"upper_block_bit")));
    static Migrator sHugeMushroomBits = IntProperty<0, 16>(u8"huge_mushroom_bits");
    E(u8"brown_mushroom_block", sHugeMushroomBits);
    E(u8"red_mushroom_block", sHugeMushroomBits);
    E(u8"coral", NamePrefixer<0, 3>(u8"_coral", {
                                                    u8"tube",   // 0
                                                    u8"brain",  // 1
                                                    u8"bubble", // 2
                                                    u8"fire",   // 3
                                                    u8"horn",   // 4
                                                }));
    static PropertySelector<0, 3> const sCoralColor(u8"coral_color", {
                                                                         u8"blue",   // 0
                                                                         u8"pink",   // 1
                                                                         u8"purple", // 2
                                                                         u8"red",    // 3
                                                                         u8"yellow", // 4
                                                                     });
    static Compose sCoral(sCoralColor, IntProperty<3, 3>(u8"coral_fan_direction"));
    E(u8"coral_fan", sCoral);
    E(u8"coral_fan_dead", sCoral);
    E(u8"coral_block", Compose(sCoralColor, BoolProperty<3>(u8"dead_bit")));
    static Compose const sCoralFanHang(IntProperty<2, 3>(u8"coral_direction"), BoolProperty<0>(u8"coral_hang_type_bit"), BoolProperty<1>(u8"dead_bit"));
    E(u8"coral_fan_hang", sCoralFanHang);
    E(u8"coral_fan_hang2", sCoralFanHang);
    E(u8"coral_fan_hang3", sCoralFanHang);
    E(u8"vine", IntProperty<0, 16>(u8"vine_direction_bits"));
    E(u8"kelp", IntProperty<0, 16>(u8"kelp_age"));
    E(u8"seagrass", PropertySelector<0, 2>(u8"sea_grass_type", {
                                                                   u8"default",    // 0
                                                                   u8"double_top", // 1
                                                                   u8"double_bot", // 2
                                                               }));
    E(u8"red_flower", PropertySelector<0, 4>(u8"flower_type", {
                                                                  u8"poppy",              // 0
                                                                  u8"orchid",             // 1
                                                                  u8"allium",             // 2
                                                                  u8"houstonia",          // 3
                                                                  u8"tulip_red",          // 4
                                                                  u8"tulip_orange",       // 5
                                                                  u8"tulip_white",        // 6
                                                                  u8"tulip_pink",         // 7
                                                                  u8"oxeye",              // 8
                                                                  u8"cornflower",         // 9
                                                                  u8"lily_of_the_valley", // 10
                                                              }));
    E(u8"bamboo_sapling", Compose(ByteProperty<0, 8>(u8"age_bit"), Const(u8"sapling_type", Str(u8"oak"))));
    E(u8"bamboo", Compose(ByteProperty<3, 8>(u8"age_bit"), PropertySelector<0, 1>(u8"bamboo_stalk_thickness", {
                                                                                                                  u8"thin",
                                                                                                                  u8"thick",
                                                                                                              }),
                          PropertySelector<1, 2>(u8"bamboo_leaf_size", {
                                                                           u8"no_leaves",    // 0
                                                                           u8"small_leaves", // 1
                                                                           u8"large_leaves", // 2
                                                                       })));
    E(u8"snow_layer", Compose(IntProperty<0, 8>(u8"height"), Const(u8"covered_bit", Bool(false))));
    E(u8"monster_egg", PropertySelector<0, 3>(u8"monster_egg_stone_type", {
                                                                              u8"stone",                // 0
                                                                              u8"cobblestone",          // 1
                                                                              u8"stone_brick",          // 2
                                                                              u8"mossy_stone_brick",    // 3
                                                                              u8"cracked_stone_brick",  // 4
                                                                              u8"chiseled_stone_brick", // 5
                                                                          }));
    E(u8"reeds", sBlockWithAge);
    E(u8"turtle_egg", Compose(PropertySelector<2, 2>(u8"cracked_state", {
                                                                            u8"no_cracks",   // 0
                                                                            u8"cracked",     // 1
                                                                            u8"max_cracked", // 2
                                                                        }),
                              PropertySelector<0, 2>(u8"turtle_egg_count", {
                                                                               u8"one_egg",
                                                                               u8"two_egg",
                                                                               u8"three_egg",
                                                                               u8"four_egg",
                                                                           })));
    E(u8"nether_wart", sBlockWithAge);
    E(u8"chorus_flower", sBlockWithAge);
    E(u8"sponge", PropertySelector<0, 1>(u8"sponge_type", {
                                                              u8"dry", // 0
                                                              u8"wet", // 1
                                                          }));
    E(u8"cake", IntProperty<0, 16>(u8"bite_counter"));
    E(u8"bed", Compose(IntProperty<0, 2>(u8"direction"), BoolProperty<3>(u8"head_piece_bit"), BoolProperty<4>(u8"occupied_bit")));
    static PropertySelector<0, 3> const sTorchFacingDirection(u8"torch_facing_direction", {
                                                                                              nullopt,   // 0
                                                                                              u8"west",  // 1
                                                                                              u8"east",  // 2
                                                                                              u8"north", // 3
                                                                                              u8"south", // 4
                                                                                              u8"top",   // 5
                                                                                          });
    E(u8"torch", sTorchFacingDirection);
    E(u8"redstone_torch", sTorchFacingDirection);
    E(u8"unlit_redstone_torch", sTorchFacingDirection);
    E(u8"sea_pickle", Compose(BoolProperty<2>(u8"dead_bit"), IntProperty<0, 2>(u8"cluster_count")));
    E(u8"lantern", BoolProperty<0>(u8"hanging"));
    E(u8"furnace", sBlockWithFacingDirection);
    E(u8"lit_furnace", sBlockWithFacingDirection);
    E(u8"anvil", Compose(PropertySelector<2, 2>(u8"damage", {
                                                                u8"undamaged",        // 0
                                                                u8"slightly_damaged", // 1
                                                                u8"very_damaged",     // 2
                                                            }),
                         IntProperty<0, 2>(u8"direction")));
    E(u8"lectern", Compose(IntProperty<0, 2>(u8"direction"), BoolProperty<2>(u8"powered_bit")));
    E(u8"cauldron", Compose(IntProperty<0, 3>(u8"fill_level"), Const(u8"cauldron_liquid", Str(u8"water"))));
    E(u8"chest", sBlockWithFacingDirection);
    E(u8"trapped_chest", sBlockWithFacingDirection);
    E(u8"ender_chest", sBlockWithFacingDirection);
    E(u8"shulker_box", NamePrefixer<0, 4>(u8"_shulker_box", sColors));
    E(u8"seaLantern", Rename(u8"sea_lantern"));
    E(u8"wall_sign", sBlockWithFacingDirection);
    E(u8"spruce_wall_sign", sBlockWithFacingDirection);
    E(u8"birch_wall_sign", sBlockWithFacingDirection);
    E(u8"jungle_wall_sign", sBlockWithFacingDirection);
    E(u8"acacia_wall_sign", sBlockWithFacingDirection);
    E(u8"dark_oak_wall_sign", sBlockWithFacingDirection);
    static Migrator const sGroundSignDirection = IntProperty<0, 16>(u8"ground_sign_direction");
    E(u8"standing_sign", sGroundSignDirection);
    E(u8"spruce_standing_sign", sGroundSignDirection);
    E(u8"birch_standing_sign", sGroundSignDirection);
    E(u8"jungle_standing_sign", sGroundSignDirection);
    E(u8"acacia_standing_sign", sGroundSignDirection);
    E(u8"dark_oak_standing_sign", sGroundSignDirection);
    E(u8"flower_pot", Const(u8"update_bit", Bool(false)));
    E(u8"skull", sBlockWithFacingDirection);
    E(u8"lever", Compose(PropertySelector<0, 3>(u8"lever_direction", {
                                                                         u8"down_east_west",   // 0
                                                                         u8"east",             // 1
                                                                         u8"west",             // 2
                                                                         u8"south",            // 3
                                                                         u8"north",            // 4
                                                                         u8"up_north_south",   // 5
                                                                         u8"up_east_west",     // 6
                                                                         u8"down_north_south", // 7
                                                                     }),
                         BoolProperty<3>(u8"open_bit")));
    static Compose const sButton(IntProperty<0, 3>(u8"facing_direction"), BoolProperty<3>(u8"button_pressed_bit"));
    E(u8"wooden_button", sButton);
    E(u8"spruce_button", sButton);
    E(u8"birch_button", sButton);
    E(u8"jungle_button", sButton);
    E(u8"acacia_button", sButton);
    E(u8"dark_oak_button", sButton);
    E(u8"stone_button", sButton);
    E(u8"end_portal_frame", Compose(IntProperty<0, 2>(u8"direction"), BoolProperty<2>(u8"end_portal_eye_bit")));
    E(u8"end_rod", sBlockWithFacingDirection);
    E(u8"tripwire_hook", Compose(IntProperty<0, 2>(u8"direction"), BoolProperty<2>(u8"attached_bit"), BoolProperty<3>(u8"powered_bit")));
    E(u8"tripWire", Compose(Rename(u8"trip_wire"), BoolProperty<2>(u8"attached_bit"), BoolProperty<1>(u8"suspended_bit"), BoolProperty<0>(u8"powered_bit"), BoolProperty<3>(u8"disarmed_bit")));
    E(u8"redstone_wire", IntProperty<0, 16>(u8"redstone_signal"));
    E(u8"brewing_stand", Compose(BoolProperty<0>(u8"brewing_stand_slot_a_bit"), BoolProperty<1>(u8"brewing_stand_slot_b_bit"), BoolProperty<2>(u8"brewing_stand_slot_c_bit")));
    static Migrator const sRailDirection = IntProperty<0, 4>(u8"rail_direction");
    E(u8"rail", sRailDirection);
    static Migrator const sRailDataBit = BoolProperty<3>(u8"rail_data_bit");
    static Compose const sActiveRail(IntProperty<0, 3>(u8"rail_direction"), sRailDataBit);
    E(u8"golden_rail", sActiveRail);
    E(u8"activator_rail", sActiveRail);
    E(u8"detector_rail", sActiveRail);
    static Migrator const sRedstoneSignal = IntProperty<0, 16>(u8"redstone_signal");
    E(u8"wooden_pressure_plate", sRedstoneSignal);
    E(u8"spruce_pressure_plate", sRedstoneSignal);
    E(u8"birch_pressure_plate", sRedstoneSignal);
    E(u8"jungle_pressure_plate", sRedstoneSignal);
    E(u8"acacia_pressure_plate", sRedstoneSignal);
    E(u8"dark_oak_pressure_plate", sRedstoneSignal);
    E(u8"stone_pressure_plate", sRedstoneSignal);
    E(u8"light_weighted_pressure_plate", sRedstoneSignal);
    E(u8"heavy_weighted_pressure_plate", sRedstoneSignal);
    static PropertySelector<0, 3> sMinecraftFacingDirection(u8"minecraft:facing_direction", {
                                                                                                u8"down",  // 0
                                                                                                u8"up",    // 1
                                                                                                u8"north", // 2
                                                                                                u8"south", // 3
                                                                                                u8"west",  // 4
                                                                                                u8"east",  // 5
                                                                                            });
    E(u8"observer", Compose(sMinecraftFacingDirection, BoolProperty<3>(u8"powered_bit")));
    E(u8"daylight_detector", sRedstoneSignal);
    E(u8"daylight_detector_inverted", sRedstoneSignal);
    static Compose const sRepeater(IntProperty<0, 2>(u8"direction"), IntProperty<2, 2>(u8"repeater_delay"));
    E(u8"powered_repeater", sRepeater);
    E(u8"unpowered_repeater", sRepeater);
    static Compose const sComparator(IntProperty<0, 2>(u8"direction"), BoolProperty<2>(u8"output_subtract_bit"), BoolProperty<3>(u8"output_lit_bit"));
    E(u8"powered_comparator", sComparator);
    E(u8"unpowered_comparator", sComparator);
    E(u8"hopper", Compose(IntProperty<0, 3>(u8"facing_direction"), BoolProperty<3>(u8"toggle_bit")));
    E(u8"dropper", Compose(IntProperty<0, 3>(u8"facing_direction"), BoolProperty<3>(u8"triggered_bit")));
    E(u8"dispenser", Compose(IntProperty<0, 3>(u8"facing_direction"), BoolProperty<3>(u8"triggered_bit")));
    E(u8"piston", Compose(IntProperty<0, 3>(u8"facing_direction")));
    E(u8"sticky_piston", Compose(IntProperty<0, 3>(u8"facing_direction")));
    // pistonArmCollision was separated to piston_arm_collision and sticky_piston_arm_collision. Here we can't identify stickyness, so leaving name as is.
    E(u8"pistonArmCollision", IntProperty<0, 3>(u8"facing_direction"));
    E(u8"loom", IntProperty<0, 16>(u8"direction"));
    E(u8"standing_banner", sGroundSignDirection);
    E(u8"tnt", Compose(BoolProperty<1>(u8"allow_underwater_bit"), BoolProperty<0>(u8"explode_bit")));

    return table;
  }

  static StringTagPtr Str(std::u8string const &v) {
    return std::make_shared<StringTag>(v);
  }

  static IntTagPtr Int(int32_t v) {
    return std::make_shared<IntTag>(v);
  }

  static ByteTagPtr Bool(bool b) {
    return std::make_shared<ByteTag>(b ? 1 : 0);
  }
};

} // namespace je2be::toje
