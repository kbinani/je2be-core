#pragma once

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

  struct PropertySelector {
    PropertySelector(String const &propertyName,
                     std::optional<int16_t> mask,
                     std::initializer_list<String> table,
                     std::optional<String> blockName = std::nullopt)
        : fPropertyName(propertyName),
          fMask(mask),
          fTable(table),
          fBlockName(blockName) {}

    String operator()(String const &n, int16_t v, CompoundTag &s) const {
      int16_t index;
      if (fMask) {
        index = v & (*fMask);
      } else {
        index = v;
      }
      if (0 <= index && index < fTable.size()) {
        s[fPropertyName] = std::make_shared<StringTag>(fTable[index]);
      }
      if (fBlockName) {
        return *fBlockName;
      } else {
        return n;
      }
    }

    String fPropertyName;
    std::optional<int16_t> fMask;
    std::vector<String> fTable;
    std::optional<String> fBlockName;
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
      static std::unique_ptr<std::unordered_map<std::u8string_view, Migrator> const> const sTable(CreateMigratorTable());
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

  static String Planks(String const &n, int16_t v, CompoundTag &s) {
    switch (v) {
    case 1:
      s[u8"wood_type"] = Str(u8"spruce");
      break;
    case 2:
      s[u8"wood_type"] = Str(u8"birch");
      break;
    case 3:
      s[u8"wood_type"] = Str(u8"jungle");
      break;
    case 4:
      s[u8"wood_type"] = Str(u8"acacia");
      break;
    case 5:
      s[u8"wood_type"] = Str(u8"dark_oak");
      break;
    case 0:
    default:
      s[u8"wood_type"] = Str(u8"oak");
      break;
    }
    return n;
  }

  static String Fence(String const &n, int16_t v, CompoundTag &s) {
    switch (v) {
    case 1:
      return u8"spruce_fence";
    case 2:
      return u8"birch_fence";
    case 3:
      return u8"jungle_fence";
    case 4:
      return u8"acacia_fence";
    case 5:
      return u8"dark_oak_fence";
    case 0:
    default:
      return u8"oak_fence";
    }
  }

  static String BlockWithDirection(String const &n, int16_t v, CompoundTag &s) {
    s[u8"direction"] = Int(v);
    return n;
  }

  static String Stairs(String const &n, int16_t v, CompoundTag &s) {
    s[u8"weirdo_direction"] = Int(v);
    return n;
  }

  static String Door(String const &n, int16_t v, CompoundTag &s) {
    s[u8"direction"] = Int(v & 0x7);
    s[u8"upper_block_bit"] = Bool((v & 0x8) == 0x8);
    return n;
  }

  static String Trapdoor(String const &n, int16_t v, CompoundTag &s) {
    s[u8"direction"] = Int(v & 0x7);
    s[u8"upside_down_bit"] = Bool((v & 0x8) == 0x8);
    return n;
  }

  static String Stone(String const &n, int16_t v, CompoundTag &s) {
    switch (v) {
    case 0:
    default:
      s[u8"stone_type"] = Str(u8"stone");
      break;
    }
    return n;
  }

  static String BlockWithFacingDirection(String const &n, int16_t v, CompoundTag &s) {
    s[u8"facing_direction"] = Int(v);
    return n;
  }

  static String TopSlotBit(String const &n, int16_t v, CompoundTag &s) {
    s[u8"top_slot_bit"] = Bool((v & 0x8) == 0x8);
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

  static std::unordered_map<std::u8string_view, Migrator> *CreateMigratorTable() {
    auto table = new std::unordered_map<std::u8string_view, Migrator>();

#define E(__name, __migrator)                        \
  assert(table->find(u8"" #__name) == table->end()); \
  table->insert(std::make_pair(u8"" #__name, __migrator))

    static PropertySelector const sColoredBlock(u8"color", std::nullopt, {
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
                                                                         });
    E(dirt, Dirt);
    E(planks, Planks);
    static Compose const sCobblestoneWall(PropertySelector(u8"wall_block_type", std::nullopt, {
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
                                                                                              }));
    E(cobblestone_wall, sCobblestoneWall);
    E(fence, Fence);
    E(fence_gate, BlockWithDirection);
    E(spruce_fence_gate, BlockWithDirection);
    E(birch_fence_gate, BlockWithDirection);
    E(jungle_fence_gate, BlockWithDirection);
    E(acacia_fence_gate, BlockWithDirection);
    E(dark_oak_fence_gate, BlockWithDirection);
    E(normal_stone_stairs, Stairs);
    E(stone_stairs, Stairs);
    E(mossy_cobblestone_stairs, Stairs);
    E(oak_stairs, Stairs);
    E(spruce_stairs, Stairs);
    E(birch_stairs, Stairs);
    E(jungle_stairs, Stairs);
    E(acacia_stairs, Stairs);
    E(dark_oak_stairs, Stairs);
    E(stone_brick_stairs, Stairs);
    E(mossy_stone_brick_stairs, Stairs);
    E(sandstone_stairs, Stairs);
    E(smooth_sandstone_stairs, Stairs);
    E(red_sandstone_stairs, Stairs);
    E(smooth_red_sandstone_stairs, Stairs);
    E(granite_stairs, Stairs);
    E(polished_granite_stairs, Stairs);
    E(diorite_stairs, Stairs);
    E(polished_diorite_stairs, Stairs);
    E(andesite_stairs, Stairs);
    E(polished_andesite_stairs, Stairs);
    E(brick_stairs, Stairs);
    E(nether_brick_stairs, Stairs);
    E(red_nether_brick_stairs, Stairs);
    E(end_brick_stairs, Stairs);
    E(quartz_stairs, Stairs);
    E(smooth_quartz_stairs, Stairs);
    E(purpur_stairs, Stairs);
    E(prismarine_stairs, Stairs);
    E(dark_prismarine_stairs, Stairs);
    E(prismarine_bricks_stairs, Stairs);
    E(wooden_door, Door);
    E(spruce_door, Door);
    E(birch_door, Door);
    E(jungle_door, Door);
    E(acacia_door, Door);
    E(dark_oak_door, Door);
    E(trapdoor, Trapdoor);
    E(spruce_trapdoor, Trapdoor);
    E(birch_trapdoor, Trapdoor);
    E(jungle_trapdoor, Trapdoor);
    E(acacia_trapdoor, Trapdoor);
    E(dark_oak_trapdoor, Trapdoor);
    E(iron_trapdoor, Trapdoor);
    E(stained_glass, sColoredBlock);
    E(stained_glass_pane, sColoredBlock);
    E(ladder, BlockWithFacingDirection);
    E(stone, Stone);
    static PropertySelector const sStoneSlabType(u8"stone_slab_type", 0x7, {
                                                                               u8"smooth_stone", // 0
                                                                               u8"sandstone",    // 1
                                                                               u8"",             // 2
                                                                               u8"cobblestone",  // 3
                                                                               u8"brick",        // 4
                                                                               u8"stone_brick",  // 5
                                                                               u8"quartz",       // 6
                                                                               u8"nether_brick", // 7
                                                                           });
    E(stone_slab, Compose(sStoneSlabType, Rename(u8"stone_block_slab"), TopSlotBit));
    E(double_stone_slab, Compose(sStoneSlabType, Rename(u8"double_stone_block_slab"), TopSlotBit));
    static PropertySelector const sStoneSlabType2(u8"stone_slab_type_2", 0x7, {
                                                                                  u8"red_sandstone",     // 0
                                                                                  u8"purpur",            // 1
                                                                                  u8"prismarine_rough",  // 2
                                                                                  u8"prismarine_dark",   // 3
                                                                                  u8"prismarine_brick",  // 4
                                                                                  u8"mossy_cobblestone", // 5
                                                                                  u8"smooth_sandstone",  // 6
                                                                                  u8"red_nether_brick",  // 7
                                                                              });
    E(stone_slab2, Compose(sStoneSlabType2, Rename(u8"stone_block_slab2"), TopSlotBit));
    E(double_stone_slab2, Compose(sStoneSlabType2, Rename(u8"double_stone_block_slab2"), TopSlotBit));
    static PropertySelector const sStoneSlabType3(u8"stone_slab_type_3", 0x7, {
                                                                                  u8"end_stone_brick",      // 0
                                                                                  u8"smooth_red_sandstone", // 1
                                                                                  u8"polished_andesite",    // 2
                                                                                  u8"andesite",             // 3
                                                                                  u8"diorite",              // 4
                                                                                  u8"polished_diorite",     // 5
                                                                                  u8"granite",              // 6
                                                                                  u8"polished_granite",     // 7
                                                                              });
    E(stone_slab3, Compose(sStoneSlabType3, Rename(u8"stone_block_slab3"), TopSlotBit));
    E(double_stone_slab3, Compose(sStoneSlabType3, Rename(u8"double_stone_block_slab3"), TopSlotBit));
    static PropertySelector const sStoneSlabType4(u8"stone_slab_type_4", 0x7, {
                                                                                  u8"mossy_stone_brick", // 0
                                                                                  u8"",                  // 1
                                                                                  u8"stone",             // 2
                                                                                  u8"cut_sandstone",     // 3
                                                                                  u8"cut_red_sandstone", // 4
                                                                              });
    E(stone_slab4, Compose(sStoneSlabType4, Rename(u8"stone_block_slab4"), TopSlotBit));
    E(double_stone_slab4, Compose(sStoneSlabType4, Rename(u8"double_stone_block_slab4"), TopSlotBit));
    E(scaffolding, Scaffolding);
    static Compose const sWoodenSlab(PropertySelector(u8"wood_type", 0x7, {
                                                                              u8"oak",      // 0
                                                                              u8"spruce",   // 1
                                                                              u8"birch",    // 2
                                                                              u8"jungle",   // 3
                                                                              u8"acacia",   // 4
                                                                              u8"dark_oak", // 5
                                                                          }),
                                     TopSlotBit);
    E(wooden_slab, sWoodenSlab);
    E(double_wooden_slab, sWoodenSlab);
    static Compose const sStonebrick(PropertySelector(u8"stone_brick_type", std::nullopt, {
                                                                                              u8"default", // 0
                                                                                              u8"mossy",   // 1
                                                                                              u8"cracked", // 2
                                                                                          }));
    E(stonebrick, sStonebrick);

#undef E
#undef A
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
