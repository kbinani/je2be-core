#include "terraform/_fence-connectable.hpp"

#include "enums/_facing4.hpp"
#include "terraform/_block-property-accessor.hpp"
#include "terraform/_shape-of-stairs.hpp"

namespace je2be::terraform {

class FenceConnectable::Impl {
  Impl() = delete;

public:
  static void Do(mcfile::je::Chunk &out, BlockAccessor<mcfile::je::Block> &cache, BlockPropertyAccessor const &accessor) {
    DoFence(out, cache, accessor);
    DoGlassPaneOrIronBars(out, cache, accessor);
  }

  static void DoFence(mcfile::je::Chunk &out, BlockAccessor<mcfile::je::Block> &cache, BlockPropertyAccessor const &accessor) {
    using namespace std;
    if (!accessor.fHasFence) {
      return;
    }
    int cx = out.fChunkX;
    int cz = out.fChunkZ;

    static vector<pair<u8string, Pos2i>> const nesw({{u8"north", Pos2i(0, -1)}, {u8"east", Pos2i(1, 0)}, {u8"south", Pos2i(0, 1)}, {u8"west", Pos2i(-1, 0)}});

    for (int y = accessor.minBlockY(); y <= accessor.maxBlockY(); y++) {
      for (int z = cz * 16; z < cz * 16 + 16; z++) {
        for (int x = cx * 16; x < cx * 16 + 16; x++) {
          auto p = accessor.property(x, y, z);
          if (p != BlockPropertyAccessor::FENCE) {
            continue;
          }
          auto blockJ = out.blockAt(x, y, z);
          if (!blockJ) {
            continue;
          }
          map<u8string, optional<u8string>> props;
          Pos2i const pos(x, z);
          for (auto const &it : nesw) {
            Pos2i direction = it.second;
            Pos2i targetPos = pos + direction;
            auto target = cache.blockAt(targetPos.fX, y, targetPos.fZ);
            if (target) {
              props[it.first] = ToString(IsFenceConnectable(*blockJ, *target, direction));
            } else {
              props[it.first] = ToString(false);
            }
          }
          auto replace = blockJ->applying(props);
          out.setBlockAt(x, y, z, replace);
        }
      }
    }
  }

  static std::u8string ToString(bool b) {
    return b ? u8"true" : u8"false";
  }

  static bool IsFenceConnectable(mcfile::je::Block const &center, mcfile::je::Block const &targetJ, Pos2i const &targetDirection) {
    if (center.fId == mcfile::blocks::minecraft::nether_brick_fence) {
      if (targetJ.fName.ends_with(u8"fence")) {
        return targetJ.fId == mcfile::blocks::minecraft::nether_brick_fence;
      }
    }
    if (IsFenceAlwaysConnectable(targetJ.fId)) {
      return true;
    }
    return IsConnectableByBlockNameAndStates(targetJ, targetDirection);
  }

  static bool IsConnectableByBlockNameAndStates(mcfile::je::Block const &target, Pos2i const &targetDirection) {
    if (target.fName.ends_with(u8"stairs")) {
      auto data = mcfile::blocks::BlockData::Make(target);
      if (auto stairs = std::dynamic_pointer_cast<mcfile::blocks::data::type::Stairs>(data); stairs) {
        auto weridoDirection = stairs->facing();
        Pos2i vec = ShapeOfStairs::VecFromWeirdoDirection(weridoDirection);
        if (vec.fX == -targetDirection.fX && vec.fZ == -targetDirection.fZ) {
          return true;
        }
      }
    } else if (target.fName.ends_with(u8"slab") && target.property(u8"type") == u8"double") {
      return true;
    } else if (target.fName.ends_with(u8"fence_gate")) {
      auto facing = target.property(u8"facing");
      Facing4 f4 = Facing4FromJavaName(facing);
      Pos2i gateDirection = Pos2iFromFacing4(f4);
      return IsOrthogonal(gateDirection, targetDirection);
    }
    return false;
  }

  static void DoGlassPaneOrIronBars(mcfile::je::Chunk &out, BlockAccessor<mcfile::je::Block> &cache, BlockPropertyAccessor const &accessor) {
    using namespace std;
    if (!accessor.fHasGlassPaneOrIronBars) {
      return;
    }
    int cx = out.fChunkX;
    int cz = out.fChunkZ;

    static vector<pair<u8string, Pos2i>> const nesw({{u8"north", Pos2i(0, -1)}, {u8"east", Pos2i(1, 0)}, {u8"south", Pos2i(0, 1)}, {u8"west", Pos2i(-1, 0)}});

    for (int y = accessor.minBlockY(); y <= accessor.maxBlockY(); y++) {
      for (int z = cz * 16; z < cz * 16 + 16; z++) {
        for (int x = cx * 16; x < cx * 16 + 16; x++) {
          auto p = accessor.property(x, y, z);
          if (p != BlockPropertyAccessor::GLASS_PANE_OR_IRON_BARS) {
            continue;
          }
          auto blockJ = out.blockAt(x, y, z);
          if (!blockJ) {
            continue;
          }
          map<u8string, optional<u8string>> props;
          Pos2i const pos(x, z);
          for (auto const &it : nesw) {
            Pos2i direction = it.second;
            Pos2i targetPos = pos + direction;
            auto target = cache.blockAt(targetPos.fX, y, targetPos.fZ);
            if (target) {
              props[it.first] = ToString(IsGlassPaneOrIronBarsConnectable(*target, direction));
            } else {
              props[it.first] = ToString(false);
            }
          }
          auto replace = blockJ->applying(props);
          out.setBlockAt(x, y, z, replace);
        }
      }
    }
  }

  static bool IsGlassPaneOrIronBarsConnectable(mcfile::je::Block const &target, Pos2i const &targetDirection) {
    if (IsGlassPaneOrIronBarsAlwaysConnectable(target.fId)) {
      return true;
    }
    return IsConnectableByBlockNameAndStates(target, targetDirection);
  }

  static bool IsFenceAlwaysConnectable(mcfile::blocks::BlockId id) {
    using namespace mcfile::blocks::minecraft;
    switch (id) {
    case acacia_fence:
    case acacia_log:
    case acacia_planks:
    case acacia_wood:
    case amethyst_block:
    case ancient_debris:
    case andesite:
    case bamboo_block:
    case bamboo_fence:
    case bamboo_mosaic:
    case bamboo_planks:
    case barrel:
    case basalt:
    case beacon:
    case bedrock:
    case bee_nest:
    case beehive:
    case birch_fence:
    case birch_log:
    case birch_planks:
    case birch_wood:
    case black_concrete:
    case black_concrete_powder:
    case black_glazed_terracotta:
    case black_stained_glass:
    case black_terracotta:
    case black_wool:
    case blackstone:
    case blast_furnace:
    case blue_concrete:
    case blue_concrete_powder:
    case blue_glazed_terracotta:
    case blue_ice:
    case blue_stained_glass:
    case blue_terracotta:
    case blue_wool:
    case bone_block:
    case bookshelf:
    case brain_coral_block:
    case bricks:
    case brown_concrete:
    case brown_concrete_powder:
    case brown_glazed_terracotta:
    case brown_mushroom_block:
    case brown_stained_glass:
    case brown_terracotta:
    case brown_wool:
    case bubble_coral_block:
    case budding_amethyst:
    case calcite:
    case cartography_table:
    case chain_command_block:
    case cherry_fence:
    case cherry_log:
    case cherry_planks:
    case cherry_wood:
    case chiseled_bookshelf:
    case chiseled_copper:
    case chiseled_deepslate:
    case chiseled_nether_bricks:
    case chiseled_polished_blackstone:
    case chiseled_quartz_block:
    case chiseled_red_sandstone:
    case chiseled_resin_bricks:
    case chiseled_sandstone:
    case chiseled_stone_bricks:
    case chorus_flower:
    case chiseled_tuff:
    case chiseled_tuff_bricks:
    case clay:
    case coal_block:
    case coal_ore:
    case coarse_dirt:
    case cobbled_deepslate:
    case cobblestone:
    case command_block:
    case composter:
    case copper_block:
    case copper_bulb:
    case copper_grate:
    case copper_ore:
    case cracked_deepslate_bricks:
    case cracked_deepslate_tiles:
    case cracked_nether_bricks:
    case cracked_polished_blackstone_bricks:
    case cracked_stone_bricks:
    case crafter:
    case crafting_table:
    case creaking_heart:
    case crimson_fence:
    case crimson_hyphae:
    case crimson_nylium:
    case crimson_planks:
    case crimson_stem:
    case crying_obsidian:
    case cut_copper:
    case cut_red_sandstone:
    case cut_sandstone:
    case cyan_concrete:
    case cyan_concrete_powder:
    case cyan_glazed_terracotta:
    case cyan_stained_glass:
    case cyan_terracotta:
    case cyan_wool:
    case dark_oak_fence:
    case dark_oak_log:
    case dark_oak_planks:
    case dark_oak_wood:
    case dark_prismarine:
    case dead_brain_coral_block:
    case dead_bubble_coral_block:
    case dead_fire_coral_block:
    case dead_horn_coral_block:
    case dead_tube_coral_block:
    case deepslate:
    case deepslate_bricks:
    case deepslate_coal_ore:
    case deepslate_copper_ore:
    case deepslate_diamond_ore:
    case deepslate_emerald_ore:
    case deepslate_gold_ore:
    case deepslate_iron_ore:
    case deepslate_lapis_ore:
    case deepslate_redstone_ore:
    case deepslate_tiles:
    case diamond_block:
    case diamond_ore:
    case diorite:
    case dirt:
    case dispenser:
    case dried_kelp_block:
    case dripstone_block:
    case dropper:
    case emerald_block:
    case emerald_ore:
    case end_stone:
    case end_stone_bricks:
    case exposed_chiseled_copper:
    case exposed_copper:
    case exposed_copper_bulb:
    case exposed_copper_grate:
    case exposed_cut_copper:
    case fire_coral_block:
    case fletching_table:
    case frosted_ice:
    case furnace:
    case gilded_blackstone:
    case glass:
    case glowstone:
    case gold_block:
    case gold_ore:
    case granite:
    case grass_block:
    case gravel:
    case gray_concrete:
    case gray_concrete_powder:
    case gray_glazed_terracotta:
    case gray_stained_glass:
    case gray_terracotta:
    case gray_wool:
    case green_concrete:
    case green_concrete_powder:
    case green_glazed_terracotta:
    case green_stained_glass:
    case green_terracotta:
    case green_wool:
    case hay_block:
    case honeycomb_block:
    case horn_coral_block:
    case ice:
    case infested_chiseled_stone_bricks:
    case infested_cobblestone:
    case infested_cracked_stone_bricks:
    case infested_deepslate:
    case infested_mossy_stone_bricks:
    case infested_stone:
    case infested_stone_bricks:
    case iron_block:
    case iron_ore:
    case jigsaw:
    case jukebox:
    case jungle_fence:
    case jungle_log:
    case jungle_planks:
    case jungle_wood:
    case lapis_block:
    case lapis_ore:
    case light_blue_concrete:
    case light_blue_concrete_powder:
    case light_blue_glazed_terracotta:
    case light_blue_stained_glass:
    case light_blue_terracotta:
    case light_blue_wool:
    case light_gray_concrete:
    case light_gray_concrete_powder:
    case light_gray_glazed_terracotta:
    case light_gray_stained_glass:
    case light_gray_terracotta:
    case light_gray_wool:
    case lime_concrete:
    case lime_concrete_powder:
    case lime_glazed_terracotta:
    case lime_stained_glass:
    case lime_terracotta:
    case lime_wool:
    case lodestone:
    case loom:
    case magenta_concrete:
    case magenta_concrete_powder:
    case magenta_glazed_terracotta:
    case magenta_stained_glass:
    case magenta_terracotta:
    case magenta_wool:
    case magma_block:
    case mangrove_fence:
    case mangrove_log:
    case mangrove_planks:
    case mangrove_roots:
    case mangrove_wood:
    case moss_block:
    case mossy_cobblestone:
    case mossy_stone_bricks:
    case mud:
    case mud_bricks:
    case muddy_mangrove_roots:
    case mushroom_stem:
    case mycelium:
    case nether_bricks:
    case nether_gold_ore:
    case nether_quartz_ore:
    case nether_wart_block:
    case netherite_block:
    case netherrack:
    case note_block:
    case oak_fence:
    case oak_log:
    case oak_planks:
    case oak_wood:
    case observer:
    case obsidian:
    case ochre_froglight:
    case orange_concrete:
    case orange_concrete_powder:
    case orange_glazed_terracotta:
    case orange_stained_glass:
    case orange_terracotta:
    case orange_wool:
    case oxidized_chiseled_copper:
    case oxidized_copper:
    case oxidized_copper_bulb:
    case oxidized_copper_grate:
    case oxidized_cut_copper:
    case packed_ice:
    case packed_mud:
    case pale_moss_block:
    case pale_oak_fence:
    case pale_oak_log:
    case pale_oak_planks:
    case pale_oak_wood:
    case pearlescent_froglight:
    case pink_concrete:
    case pink_concrete_powder:
    case pink_glazed_terracotta:
    case pink_stained_glass:
    case pink_terracotta:
    case pink_wool:
    case piston_head:
    case podzol:
    case polished_andesite:
    case polished_basalt:
    case polished_blackstone:
    case polished_blackstone_bricks:
    case polished_deepslate:
    case polished_diorite:
    case polished_granite:
    case polished_tuff:
    case prismarine:
    case prismarine_bricks:
    case purple_concrete:
    case purple_concrete_powder:
    case purple_glazed_terracotta:
    case purple_stained_glass:
    case purple_terracotta:
    case purple_wool:
    case purpur_block:
    case purpur_pillar:
    case quartz_block:
    case quartz_bricks:
    case quartz_pillar:
    case raw_copper_block:
    case raw_gold_block:
    case raw_iron_block:
    case red_concrete:
    case red_concrete_powder:
    case red_glazed_terracotta:
    case red_mushroom_block:
    case red_nether_bricks:
    case red_sand:
    case red_sandstone:
    case red_stained_glass:
    case red_terracotta:
    case red_wool:
    case redstone_block:
    case redstone_lamp:
    case redstone_ore:
    case reinforced_deepslate:
    case repeating_command_block:
    case resin_block:
    case resin_bricks:
    case respawn_anchor:
    case rooted_dirt:
    case sand:
    case sandstone:
    case sculk:
    case sculk_catalyst:
    case sea_lantern:
    case shroomlight:
    case slime_block:
    case smithing_table:
    case smoker:
    case smooth_basalt:
    case smooth_quartz:
    case smooth_red_sandstone:
    case smooth_sandstone:
    case smooth_stone:
    case snow_block:
    case soul_sand:
    case soul_soil:
    case spawner:
    case sponge:
    case spruce_fence:
    case spruce_log:
    case spruce_planks:
    case spruce_wood:
    case stone:
    case stone_bricks:
    case stripped_acacia_log:
    case stripped_acacia_wood:
    case stripped_bamboo_block:
    case stripped_birch_log:
    case stripped_birch_wood:
    case stripped_cherry_log:
    case stripped_cherry_wood:
    case stripped_crimson_hyphae:
    case stripped_crimson_stem:
    case stripped_dark_oak_log:
    case stripped_dark_oak_wood:
    case stripped_jungle_log:
    case stripped_jungle_wood:
    case stripped_mangrove_log:
    case stripped_mangrove_wood:
    case stripped_oak_log:
    case stripped_oak_wood:
    case stripped_pale_oak_log:
    case stripped_pale_oak_wood:
    case stripped_spruce_log:
    case stripped_spruce_wood:
    case stripped_warped_hyphae:
    case stripped_warped_stem:
    case structure_block:
    case suspicious_gravel:
    case suspicious_sand:
    case target:
    case terracotta:
    case test_block:
    case test_instance_block:
    case tinted_glass:
    case tnt:
    case trial_spawner:
    case tube_coral_block:
    case tuff:
    case tuff_bricks:
    case vault:
    case verdant_froglight:
    case warped_fence:
    case warped_hyphae:
    case warped_nylium:
    case warped_planks:
    case warped_stem:
    case warped_wart_block:
    case waxed_chiseled_copper:
    case waxed_copper_block:
    case waxed_copper_bulb:
    case waxed_copper_grate:
    case waxed_cut_copper:
    case waxed_exposed_chiseled_copper:
    case waxed_exposed_copper:
    case waxed_exposed_copper_bulb:
    case waxed_exposed_copper_grate:
    case waxed_exposed_cut_copper:
    case waxed_oxidized_chiseled_copper:
    case waxed_oxidized_copper:
    case waxed_oxidized_copper_bulb:
    case waxed_oxidized_copper_grate:
    case waxed_oxidized_cut_copper:
    case waxed_weathered_chiseled_copper:
    case waxed_weathered_copper:
    case waxed_weathered_copper_bulb:
    case waxed_weathered_copper_grate:
    case waxed_weathered_cut_copper:
    case weathered_chiseled_copper:
    case weathered_copper:
    case weathered_copper_bulb:
    case weathered_copper_grate:
    case weathered_cut_copper:
    case wet_sponge:
    case white_concrete:
    case white_concrete_powder:
    case white_glazed_terracotta:
    case white_stained_glass:
    case white_terracotta:
    case white_wool:
    case yellow_concrete:
    case yellow_concrete_powder:
    case yellow_glazed_terracotta:
    case yellow_stained_glass:
    case yellow_terracotta:
    case yellow_wool:
      return true;
    default:
      break;
    }
    // TODO:
    return false;
  }

  static bool IsGlassPaneOrIronBarsAlwaysConnectable(mcfile::blocks::BlockId id) {
    using namespace mcfile::blocks::minecraft;
    switch (id) {
    case acacia_log:
    case acacia_planks:
    case acacia_wood:
    case amethyst_block:
    case ancient_debris:
    case andesite:
    case andesite_wall:
    case bamboo_block:
    case bamboo_mosaic:
    case bamboo_planks:
    case barrel:
    case basalt:
    case beacon:
    case bedrock:
    case bee_nest:
    case beehive:
    case birch_log:
    case birch_planks:
    case birch_wood:
    case black_concrete:
    case black_concrete_powder:
    case black_glazed_terracotta:
    case black_stained_glass:
    case black_stained_glass_pane:
    case black_terracotta:
    case black_wool:
    case blackstone:
    case blackstone_wall:
    case blast_furnace:
    case blue_concrete:
    case blue_concrete_powder:
    case blue_glazed_terracotta:
    case blue_ice:
    case blue_stained_glass:
    case blue_stained_glass_pane:
    case blue_terracotta:
    case blue_wool:
    case bone_block:
    case bookshelf:
    case brain_coral_block:
    case brick_wall:
    case bricks:
    case brown_concrete:
    case brown_concrete_powder:
    case brown_glazed_terracotta:
    case brown_mushroom_block:
    case brown_stained_glass:
    case brown_stained_glass_pane:
    case brown_terracotta:
    case brown_wool:
    case bubble_coral_block:
    case budding_amethyst:
    case calcite:
    case cartography_table:
    case chain_command_block:
    case cherry_log:
    case cherry_planks:
    case cherry_wood:
    case chiseled_bookshelf:
    case chiseled_copper:
    case chiseled_deepslate:
    case chiseled_nether_bricks:
    case chiseled_polished_blackstone:
    case chiseled_quartz_block:
    case chiseled_red_sandstone:
    case chiseled_resin_bricks:
    case chiseled_sandstone:
    case chiseled_stone_bricks:
    case chorus_flower:
    case chiseled_tuff:
    case chiseled_tuff_bricks:
    case clay:
    case coal_block:
    case coal_ore:
    case coarse_dirt:
    case cobbled_deepslate:
    case cobbled_deepslate_wall:
    case cobblestone:
    case cobblestone_wall:
    case command_block:
    case composter:
    case copper_bars:
    case copper_block:
    case copper_bulb:
    case copper_grate:
    case copper_ore:
    case cracked_deepslate_bricks:
    case cracked_deepslate_tiles:
    case cracked_nether_bricks:
    case cracked_polished_blackstone_bricks:
    case cracked_stone_bricks:
    case crafter:
    case crafting_table:
    case creaking_heart:
    case crimson_hyphae:
    case crimson_nylium:
    case crimson_planks:
    case crimson_stem:
    case crying_obsidian:
    case cut_copper:
    case cut_red_sandstone:
    case cut_sandstone:
    case cyan_concrete:
    case cyan_concrete_powder:
    case cyan_glazed_terracotta:
    case cyan_stained_glass:
    case cyan_stained_glass_pane:
    case cyan_terracotta:
    case cyan_wool:
    case dark_oak_log:
    case dark_oak_planks:
    case dark_oak_wood:
    case dark_prismarine:
    case dead_brain_coral_block:
    case dead_bubble_coral_block:
    case dead_fire_coral_block:
    case dead_horn_coral_block:
    case dead_tube_coral_block:
    case deepslate:
    case deepslate_brick_wall:
    case deepslate_bricks:
    case deepslate_coal_ore:
    case deepslate_copper_ore:
    case deepslate_diamond_ore:
    case deepslate_emerald_ore:
    case deepslate_gold_ore:
    case deepslate_iron_ore:
    case deepslate_lapis_ore:
    case deepslate_redstone_ore:
    case deepslate_tile_wall:
    case deepslate_tiles:
    case diamond_block:
    case diamond_ore:
    case diorite:
    case diorite_wall:
    case dirt:
    case dispenser:
    case dried_kelp_block:
    case dripstone_block:
    case dropper:
    case emerald_block:
    case emerald_ore:
    case end_stone:
    case end_stone_brick_wall:
    case end_stone_bricks:
    case exposed_chiseled_copper:
    case exposed_copper:
    case exposed_copper_bars:
    case exposed_copper_bulb:
    case exposed_copper_grate:
    case exposed_cut_copper:
    case fire_coral_block:
    case fletching_table:
    case frosted_ice:
    case furnace:
    case gilded_blackstone:
    case glass:
    case glass_pane:
    case glowstone:
    case gold_block:
    case gold_ore:
    case granite:
    case granite_wall:
    case grass_block:
    case gravel:
    case gray_concrete:
    case gray_concrete_powder:
    case gray_glazed_terracotta:
    case gray_stained_glass:
    case gray_stained_glass_pane:
    case gray_terracotta:
    case gray_wool:
    case green_concrete:
    case green_concrete_powder:
    case green_glazed_terracotta:
    case green_stained_glass:
    case green_stained_glass_pane:
    case green_terracotta:
    case green_wool:
    case hay_block:
    case honeycomb_block:
    case horn_coral_block:
    case ice:
    case infested_chiseled_stone_bricks:
    case infested_cobblestone:
    case infested_cracked_stone_bricks:
    case infested_deepslate:
    case infested_mossy_stone_bricks:
    case infested_stone:
    case infested_stone_bricks:
    case iron_bars:
    case iron_block:
    case iron_ore:
    case jigsaw:
    case jukebox:
    case jungle_log:
    case jungle_planks:
    case jungle_wood:
    case lapis_block:
    case lapis_ore:
    case light_blue_concrete:
    case light_blue_concrete_powder:
    case light_blue_glazed_terracotta:
    case light_blue_stained_glass:
    case light_blue_stained_glass_pane:
    case light_blue_terracotta:
    case light_blue_wool:
    case light_gray_concrete:
    case light_gray_concrete_powder:
    case light_gray_glazed_terracotta:
    case light_gray_stained_glass:
    case light_gray_stained_glass_pane:
    case light_gray_terracotta:
    case light_gray_wool:
    case lime_concrete:
    case lime_concrete_powder:
    case lime_glazed_terracotta:
    case lime_stained_glass:
    case lime_stained_glass_pane:
    case lime_terracotta:
    case lime_wool:
    case lodestone:
    case loom:
    case magenta_concrete:
    case magenta_concrete_powder:
    case magenta_glazed_terracotta:
    case magenta_stained_glass:
    case magenta_stained_glass_pane:
    case magenta_terracotta:
    case magenta_wool:
    case magma_block:
    case mangrove_log:
    case mangrove_planks:
    case mangrove_roots:
    case mangrove_wood:
    case moss_block:
    case mossy_cobblestone:
    case mossy_cobblestone_wall:
    case mossy_stone_brick_wall:
    case mossy_stone_bricks:
    case mud:
    case mud_brick_wall:
    case mud_bricks:
    case muddy_mangrove_roots:
    case mushroom_stem:
    case mycelium:
    case nether_brick_wall:
    case nether_bricks:
    case nether_gold_ore:
    case nether_quartz_ore:
    case nether_wart_block:
    case netherite_block:
    case netherrack:
    case note_block:
    case oak_log:
    case oak_planks:
    case oak_wood:
    case observer:
    case obsidian:
    case ochre_froglight:
    case orange_concrete:
    case orange_concrete_powder:
    case orange_glazed_terracotta:
    case orange_stained_glass:
    case orange_stained_glass_pane:
    case orange_terracotta:
    case orange_wool:
    case oxidized_chiseled_copper:
    case oxidized_copper:
    case oxidized_copper_bars:
    case oxidized_copper_bulb:
    case oxidized_copper_grate:
    case oxidized_cut_copper:
    case packed_ice:
    case packed_mud:
    case pale_moss_block:
    case pale_oak_log:
    case pale_oak_planks:
    case pale_oak_wood:
    case pearlescent_froglight:
    case pink_concrete:
    case pink_concrete_powder:
    case pink_glazed_terracotta:
    case pink_stained_glass:
    case pink_stained_glass_pane:
    case pink_terracotta:
    case pink_wool:
    case podzol:
    case polished_andesite:
    case polished_basalt:
    case polished_blackstone:
    case polished_blackstone_brick_wall:
    case polished_blackstone_bricks:
    case polished_blackstone_wall:
    case polished_deepslate:
    case polished_deepslate_wall:
    case polished_diorite:
    case polished_granite:
    case polished_tuff:
    case polished_tuff_wall:
    case prismarine:
    case prismarine_bricks:
    case prismarine_wall:
    case purple_concrete:
    case purple_concrete_powder:
    case purple_glazed_terracotta:
    case purple_stained_glass:
    case purple_stained_glass_pane:
    case purple_terracotta:
    case purple_wool:
    case purpur_block:
    case purpur_pillar:
    case quartz_block:
    case quartz_bricks:
    case quartz_pillar:
    case raw_copper_block:
    case raw_gold_block:
    case raw_iron_block:
    case red_concrete:
    case red_concrete_powder:
    case red_glazed_terracotta:
    case red_mushroom_block:
    case red_nether_brick_wall:
    case red_nether_bricks:
    case red_sand:
    case red_sandstone:
    case red_sandstone_wall:
    case red_stained_glass:
    case red_stained_glass_pane:
    case red_terracotta:
    case red_wool:
    case redstone_block:
    case redstone_lamp:
    case redstone_ore:
    case reinforced_deepslate:
    case repeating_command_block:
    case resin_block:
    case resin_brick_wall:
    case resin_bricks:
    case respawn_anchor:
    case rooted_dirt:
    case sand:
    case sandstone:
    case sandstone_wall:
    case sculk:
    case sculk_catalyst:
    case sea_lantern:
    case shroomlight:
    case slime_block:
    case smithing_table:
    case smoker:
    case smooth_basalt:
    case smooth_quartz:
    case smooth_red_sandstone:
    case smooth_sandstone:
    case smooth_stone:
    case snow_block:
    case soul_sand:
    case soul_soil:
    case spawner:
    case sponge:
    case spruce_log:
    case spruce_planks:
    case spruce_wood:
    case stone:
    case stone_brick_wall:
    case stone_bricks:
    case stripped_acacia_log:
    case stripped_acacia_wood:
    case stripped_bamboo_block:
    case stripped_birch_log:
    case stripped_birch_wood:
    case stripped_cherry_log:
    case stripped_cherry_wood:
    case stripped_crimson_hyphae:
    case stripped_crimson_stem:
    case stripped_dark_oak_log:
    case stripped_dark_oak_wood:
    case stripped_jungle_log:
    case stripped_jungle_wood:
    case stripped_mangrove_log:
    case stripped_mangrove_wood:
    case stripped_oak_log:
    case stripped_oak_wood:
    case stripped_pale_oak_log:
    case stripped_pale_oak_wood:
    case stripped_spruce_log:
    case stripped_spruce_wood:
    case stripped_warped_hyphae:
    case stripped_warped_stem:
    case structure_block:
    case suspicious_gravel:
    case suspicious_sand:
    case target:
    case terracotta:
    case test_block:
    case test_instance_block:
    case tinted_glass:
    case tnt:
    case trial_spawner:
    case tube_coral_block:
    case tuff:
    case tuff_brick_wall:
    case tuff_bricks:
    case tuff_wall:
    case vault:
    case verdant_froglight:
    case warped_hyphae:
    case warped_nylium:
    case warped_planks:
    case warped_stem:
    case warped_wart_block:
    case waxed_chiseled_copper:
    case waxed_copper_bars:
    case waxed_copper_block:
    case waxed_copper_bulb:
    case waxed_copper_grate:
    case waxed_cut_copper:
    case waxed_exposed_chiseled_copper:
    case waxed_exposed_copper:
    case waxed_exposed_copper_bars:
    case waxed_exposed_copper_bulb:
    case waxed_exposed_copper_grate:
    case waxed_exposed_cut_copper:
    case waxed_oxidized_chiseled_copper:
    case waxed_oxidized_copper:
    case waxed_oxidized_copper_bars:
    case waxed_oxidized_copper_bulb:
    case waxed_oxidized_copper_grate:
    case waxed_oxidized_cut_copper:
    case waxed_weathered_chiseled_copper:
    case waxed_weathered_copper:
    case waxed_weathered_copper_bars:
    case waxed_weathered_copper_bulb:
    case waxed_weathered_copper_grate:
    case waxed_weathered_cut_copper:
    case weathered_chiseled_copper:
    case weathered_copper:
    case weathered_copper_bars:
    case weathered_copper_bulb:
    case weathered_copper_grate:
    case weathered_cut_copper:
    case wet_sponge:
    case white_concrete:
    case white_concrete_powder:
    case white_glazed_terracotta:
    case white_stained_glass:
    case white_stained_glass_pane:
    case white_terracotta:
    case white_wool:
    case yellow_concrete:
    case yellow_concrete_powder:
    case yellow_glazed_terracotta:
    case yellow_stained_glass:
    case yellow_stained_glass_pane:
    case yellow_terracotta:
    case yellow_wool:
      return true;
    default:
      break;
    }
    // TODO:
    return false;
  }
};

void FenceConnectable::Do(mcfile::je::Chunk &out, BlockAccessor<mcfile::je::Block> &cache, BlockPropertyAccessor const &accessor) {
  Impl::Do(out, cache, accessor);
}

} // namespace je2be::terraform
