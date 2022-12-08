#pragma once

namespace je2be::terraform {

class NoteBlock {
  NoteBlock() = delete;

public:
  static void Do(mcfile::je::Chunk &out, BlockAccessor &cache, terraform::BlockPropertyAccessor const &accessor) {
    using namespace std;
    using namespace mcfile::blocks;

    if (!accessor.fHasNoteBlock) {
      return;
    }

    int cx = out.fChunkX;
    int cz = out.fChunkZ;

    for (int y = accessor.minBlockY() + 1; y <= accessor.maxBlockY(); y++) {
      for (int z = cz * 16; z < cz * 16 + 16; z++) {
        for (int x = cx * 16; x < cx * 16 + 16; x++) {
          auto p = accessor.property(x, y, z);
          if (!terraform::BlockPropertyAccessor::IsNoteBlock(p)) {
            continue;
          }
          auto blockJ = out.blockAt(x, y, z);
          if (!blockJ) {
            continue;
          }
          map<string, string> props(blockJ->fProperties);
          string instrument = "harp";
          auto lowerJ = out.blockAt(x, y - 1, z);
          if (lowerJ) {
            instrument = NoteBlockInstrument(lowerJ->fId);
          }
          auto upperJ = out.blockAt(x, y + 1, z);
          if (upperJ) {
            switch (upperJ->fId) {
            case minecraft::skeleton_skull:
              instrument = "skeleton";
              break;
            case minecraft::wither_skeleton_skull:
              instrument = "wither_skeleton";
              break;
            case minecraft::player_head:
              instrument = "custom_head";
              break;
            case minecraft::zombie_head:
              instrument = "zombie";
              break;
            case minecraft::creeper_head:
              instrument = "creeper";
              break;
            case minecraft::piglin_head:
              instrument = "piglin";
              break;
            case minecraft::dragon_head:
              instrument = "dragon";
              break;
            }
          }
          props["instrument"] = instrument;
          auto replace = make_shared<mcfile::je::Block const>(blockJ->fName, props);
          out.setBlockAt(x, y, z, replace);
        }
      }
    }
  }

  static std::string NoteBlockInstrumentAutogenCode(mcfile::blocks::BlockId id) {
    using namespace mcfile::blocks::minecraft;
    auto static const bedrock = mcfile::blocks::minecraft::bedrock;
    switch (id) {
    case hay_block:
      return "banjo";
    case andesite:
    case andesite_slab:
    case andesite_stairs:
    case andesite_wall:
    case basalt:
    case bedrock:
    case black_concrete:
    case black_glazed_terracotta:
    case black_terracotta:
    case blackstone:
    case blackstone_slab:
    case blackstone_stairs:
    case blackstone_wall:
    case blast_furnace:
    case blue_concrete:
    case blue_glazed_terracotta:
    case blue_terracotta:
    case brain_coral_block:
    case brick_slab:
    case brick_stairs:
    case brick_wall:
    case bricks:
    case brown_concrete:
    case brown_glazed_terracotta:
    case brown_terracotta:
    case bubble_coral_block:
    case calcite:
    case chiseled_deepslate:
    case chiseled_nether_bricks:
    case chiseled_polished_blackstone:
    case chiseled_quartz_block:
    case chiseled_red_sandstone:
    case chiseled_sandstone:
    case chiseled_stone_bricks:
    case coal_block:
    case coal_ore:
    case cobbled_deepslate:
    case cobbled_deepslate_slab:
    case cobbled_deepslate_stairs:
    case cobbled_deepslate_wall:
    case cobblestone:
    case cobblestone_slab:
    case cobblestone_stairs:
    case cobblestone_wall:
    case copper_ore:
    case cracked_deepslate_bricks:
    case cracked_deepslate_tiles:
    case cracked_nether_bricks:
    case cracked_polished_blackstone_bricks:
    case cracked_stone_bricks:
    case crimson_nylium:
    case crying_obsidian:
    case cut_red_sandstone:
    case cut_red_sandstone_slab:
    case cut_sandstone:
    case cut_sandstone_slab:
    case cyan_concrete:
    case cyan_glazed_terracotta:
    case cyan_terracotta:
    case dark_prismarine:
    case dark_prismarine_slab:
    case dark_prismarine_stairs:
    case dead_brain_coral:
    case dead_brain_coral_block:
    case dead_brain_coral_fan:
    case dead_brain_coral_wall_fan:
    case dead_bubble_coral:
    case dead_bubble_coral_block:
    case dead_bubble_coral_fan:
    case dead_bubble_coral_wall_fan:
    case dead_fire_coral:
    case dead_fire_coral_block:
    case dead_fire_coral_fan:
    case dead_fire_coral_wall_fan:
    case dead_horn_coral:
    case dead_horn_coral_block:
    case dead_horn_coral_fan:
    case dead_horn_coral_wall_fan:
    case dead_tube_coral:
    case dead_tube_coral_block:
    case dead_tube_coral_fan:
    case dead_tube_coral_wall_fan:
    case deepslate:
    case deepslate_brick_slab:
    case deepslate_brick_stairs:
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
    case deepslate_tile_slab:
    case deepslate_tile_stairs:
    case deepslate_tile_wall:
    case deepslate_tiles:
    case diamond_ore:
    case diorite:
    case diorite_slab:
    case diorite_stairs:
    case diorite_wall:
    case dispenser:
    case dripstone_block:
    case dropper:
    case emerald_ore:
    case enchanting_table:
    case end_portal_frame:
    case end_stone:
    case end_stone_brick_slab:
    case end_stone_brick_stairs:
    case end_stone_brick_wall:
    case end_stone_bricks:
    case ender_chest:
    case fire_coral_block:
    case furnace:
    case gilded_blackstone:
    case gold_ore:
    case granite:
    case granite_slab:
    case granite_stairs:
    case granite_wall:
    case gray_concrete:
    case gray_glazed_terracotta:
    case gray_terracotta:
    case green_concrete:
    case green_glazed_terracotta:
    case green_terracotta:
    case horn_coral_block:
    case iron_ore:
    case lapis_ore:
    case light_blue_concrete:
    case light_blue_glazed_terracotta:
    case light_blue_terracotta:
    case light_gray_concrete:
    case light_gray_glazed_terracotta:
    case light_gray_terracotta:
    case lime_concrete:
    case lime_glazed_terracotta:
    case lime_terracotta:
    case magenta_concrete:
    case magenta_glazed_terracotta:
    case magenta_terracotta:
    case magma_block:
    case mossy_cobblestone:
    case mossy_cobblestone_slab:
    case mossy_cobblestone_stairs:
    case mossy_cobblestone_wall:
    case mossy_stone_brick_slab:
    case mossy_stone_brick_stairs:
    case mossy_stone_brick_wall:
    case mossy_stone_bricks:
    case mud_brick_slab:
    case mud_brick_stairs:
    case mud_brick_wall:
    case mud_bricks:
    case nether_brick_fence:
    case nether_brick_slab:
    case nether_brick_stairs:
    case nether_brick_wall:
    case nether_bricks:
    case nether_gold_ore:
    case nether_quartz_ore:
    case netherrack:
    case observer:
    case obsidian:
    case orange_concrete:
    case orange_glazed_terracotta:
    case orange_terracotta:
    case petrified_oak_slab:
    case pink_concrete:
    case pink_glazed_terracotta:
    case pink_terracotta:
    case pointed_dripstone:
    case polished_andesite:
    case polished_andesite_slab:
    case polished_andesite_stairs:
    case polished_basalt:
    case polished_blackstone:
    case polished_blackstone_brick_slab:
    case polished_blackstone_brick_stairs:
    case polished_blackstone_brick_wall:
    case polished_blackstone_bricks:
    case polished_blackstone_pressure_plate:
    case polished_blackstone_slab:
    case polished_blackstone_stairs:
    case polished_blackstone_wall:
    case polished_deepslate:
    case polished_deepslate_slab:
    case polished_deepslate_stairs:
    case polished_deepslate_wall:
    case polished_diorite:
    case polished_diorite_slab:
    case polished_diorite_stairs:
    case polished_granite:
    case polished_granite_slab:
    case polished_granite_stairs:
    case prismarine:
    case prismarine_brick_slab:
    case prismarine_brick_stairs:
    case prismarine_bricks:
    case prismarine_slab:
    case prismarine_stairs:
    case prismarine_wall:
    case purple_concrete:
    case purple_glazed_terracotta:
    case purple_terracotta:
    case purpur_block:
    case purpur_pillar:
    case purpur_slab:
    case purpur_stairs:
    case quartz_block:
    case quartz_bricks:
    case quartz_pillar:
    case quartz_slab:
    case quartz_stairs:
    case raw_copper_block:
    case raw_gold_block:
    case raw_iron_block:
    case red_concrete:
    case red_glazed_terracotta:
    case red_nether_brick_slab:
    case red_nether_brick_stairs:
    case red_nether_brick_wall:
    case red_nether_bricks:
    case red_sandstone:
    case red_sandstone_slab:
    case red_sandstone_stairs:
    case red_sandstone_wall:
    case red_terracotta:
    case redstone_ore:
    case reinforced_deepslate:
    case respawn_anchor:
    case sandstone:
    case sandstone_slab:
    case sandstone_stairs:
    case sandstone_wall:
    case smoker:
    case smooth_basalt:
    case smooth_quartz:
    case smooth_quartz_slab:
    case smooth_quartz_stairs:
    case smooth_red_sandstone:
    case smooth_red_sandstone_slab:
    case smooth_red_sandstone_stairs:
    case smooth_sandstone:
    case smooth_sandstone_slab:
    case smooth_sandstone_stairs:
    case smooth_stone:
    case smooth_stone_slab:
    case spawner:
    case stone:
    case stone_brick_slab:
    case stone_brick_stairs:
    case stone_brick_wall:
    case stone_bricks:
    case stone_pressure_plate:
    case stone_slab:
    case stone_stairs:
    case stonecutter:
    case terracotta:
    case tuff:
    case warped_nylium:
    case white_concrete:
    case white_glazed_terracotta:
    case white_terracotta:
    case yellow_concrete:
    case yellow_glazed_terracotta:
    case yellow_terracotta:
      return "basedrum";
    case acacia_door:
    case acacia_fence:
    case acacia_fence_gate:
    case acacia_hanging_sign:
    case acacia_log:
    case acacia_planks:
    case acacia_pressure_plate:
    case acacia_sign:
    case acacia_slab:
    case acacia_stairs:
    case acacia_trapdoor:
    case acacia_wall_hanging_sign:
    case acacia_wall_sign:
    case acacia_wood:
    case bamboo_block:
    case bamboo_door:
    case bamboo_fence:
    case bamboo_fence_gate:
    case bamboo_hanging_sign:
    case bamboo_mosaic:
    case bamboo_mosaic_slab:
    case bamboo_mosaic_stairs:
    case bamboo_planks:
    case bamboo_pressure_plate:
    case bamboo_sign:
    case bamboo_slab:
    case bamboo_stairs:
    case bamboo_trapdoor:
    case bamboo_wall_hanging_sign:
    case bamboo_wall_sign:
    case barrel:
    case bee_nest:
    case beehive:
    case birch_door:
    case birch_fence:
    case birch_fence_gate:
    case birch_hanging_sign:
    case birch_log:
    case birch_planks:
    case birch_pressure_plate:
    case birch_sign:
    case birch_slab:
    case birch_stairs:
    case birch_trapdoor:
    case birch_wall_sign:
    case birch_wood:
    case black_banner:
    case black_wall_banner:
    case blue_banner:
    case blue_wall_banner:
    case bookshelf:
    case brown_banner:
    case brown_mushroom_block:
    case brown_wall_banner:
    case campfire:
    case cartography_table:
    case chest:
    case chiseled_bookshelf:
    case composter:
    case crafting_table:
    case crimson_door:
    case crimson_fence:
    case crimson_fence_gate:
    case crimson_hanging_sign:
    case crimson_hyphae:
    case crimson_planks:
    case crimson_pressure_plate:
    case crimson_sign:
    case crimson_slab:
    case crimson_stairs:
    case crimson_stem:
    case crimson_trapdoor:
    case crimson_wall_hanging_sign:
    case crimson_wall_sign:
    case cyan_banner:
    case cyan_wall_banner:
    case dark_oak_door:
    case dark_oak_fence:
    case dark_oak_fence_gate:
    case dark_oak_hanging_sign:
    case dark_oak_log:
    case dark_oak_planks:
    case dark_oak_pressure_plate:
    case dark_oak_sign:
    case dark_oak_slab:
    case dark_oak_stairs:
    case dark_oak_trapdoor:
    case dark_oak_wall_hanging_sign:
    case dark_oak_wall_sign:
    case dark_oak_wood:
    case daylight_detector:
    case fletching_table:
    case gray_banner:
    case gray_wall_banner:
    case green_banner:
    case green_wall_banner:
    case jukebox:
    case jungle_door:
    case jungle_fence:
    case jungle_fence_gate:
    case jungle_hanging_sign:
    case jungle_log:
    case jungle_planks:
    case jungle_pressure_plate:
    case jungle_sign:
    case jungle_slab:
    case jungle_stairs:
    case jungle_trapdoor:
    case jungle_wall_hanging_sign:
    case jungle_wall_sign:
    case jungle_wood:
    case lectern:
    case light_blue_banner:
    case light_blue_wall_banner:
    case light_gray_banner:
    case light_gray_wall_banner:
    case lime_banner:
    case lime_wall_banner:
    case loom:
    case magenta_banner:
    case magenta_wall_banner:
    case mangrove_door:
    case mangrove_fence:
    case mangrove_fence_gate:
    case mangrove_hanging_sign:
    case mangrove_log:
    case mangrove_planks:
    case mangrove_pressure_plate:
    case mangrove_roots:
    case mangrove_sign:
    case mangrove_slab:
    case mangrove_stairs:
    case mangrove_trapdoor:
    case mangrove_wall_hanging_sign:
    case mangrove_wall_sign:
    case mangrove_wood:
    case mushroom_stem:
    case note_block:
    case oak_door:
    case oak_fence:
    case oak_fence_gate:
    case oak_hanging_sign:
    case oak_log:
    case oak_planks:
    case oak_pressure_plate:
    case oak_sign:
    case oak_slab:
    case oak_stairs:
    case oak_trapdoor:
    case oak_wall_hanging_sign:
    case oak_wall_sign:
    case oak_wood:
    case orange_banner:
    case orange_wall_banner:
    case pink_banner:
    case pink_wall_banner:
    case purple_banner:
    case purple_wall_banner:
    case red_banner:
    case red_mushroom_block:
    case red_wall_banner:
    case smithing_table:
    case soul_campfire:
    case spruce_door:
    case spruce_fence:
    case spruce_fence_gate:
    case spruce_hanging_sign:
    case spruce_log:
    case spruce_planks:
    case spruce_pressure_plate:
    case spruce_sign:
    case spruce_slab:
    case spruce_stairs:
    case spruce_trapdoor:
    case spruce_wall_hanging_sign:
    case spruce_wall_sign:
    case spruce_wood:
    case stripped_acacia_log:
    case stripped_acacia_wood:
    case stripped_bamboo_block:
    case stripped_birch_log:
    case stripped_birch_wood:
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
    case stripped_spruce_log:
    case stripped_spruce_wood:
    case stripped_warped_hyphae:
    case stripped_warped_stem:
    case trapped_chest:
    case warped_door:
    case warped_fence:
    case warped_fence_gate:
    case warped_hanging_sign:
    case warped_hyphae:
    case warped_planks:
    case warped_pressure_plate:
    case warped_sign:
    case warped_slab:
    case warped_stairs:
    case warped_stem:
    case warped_trapdoor:
    case warped_wall_hanging_sign:
    case warped_wall_sign:
    case white_banner:
    case white_wall_banner:
    case yellow_banner:
    case yellow_wall_banner:
      return "bass";
    case gold_block:
      return "bell";
    case emerald_block:
      return "bit";
    case packed_ice:
      return "chime";
    case soul_sand:
      return "cow_bell";
    case pumpkin:
      return "didgeridoo";
    case clay:
      return "flute";
    case black_wool:
    case blue_wool:
    case brown_wool:
    case cyan_wool:
    case gray_wool:
    case green_wool:
    case light_blue_wool:
    case light_gray_wool:
    case lime_wool:
    case magenta_wool:
    case orange_wool:
    case pink_wool:
    case purple_wool:
    case red_wool:
    case white_wool:
    case yellow_wool:
      return "guitar";
    case beacon:
    case black_stained_glass:
    case black_stained_glass_pane:
    case blue_stained_glass:
    case blue_stained_glass_pane:
    case brown_stained_glass:
    case brown_stained_glass_pane:
    case conduit:
    case cyan_stained_glass:
    case cyan_stained_glass_pane:
    case glass:
    case glass_pane:
    case gray_stained_glass:
    case gray_stained_glass_pane:
    case green_stained_glass:
    case green_stained_glass_pane:
    case light_blue_stained_glass:
    case light_blue_stained_glass_pane:
    case light_gray_stained_glass:
    case light_gray_stained_glass_pane:
    case lime_stained_glass:
    case lime_stained_glass_pane:
    case magenta_stained_glass:
    case magenta_stained_glass_pane:
    case orange_stained_glass:
    case orange_stained_glass_pane:
    case pink_stained_glass:
    case pink_stained_glass_pane:
    case purple_stained_glass:
    case purple_stained_glass_pane:
    case red_stained_glass:
    case red_stained_glass_pane:
    case sea_lantern:
    case tinted_glass:
    case white_stained_glass:
    case white_stained_glass_pane:
    case yellow_stained_glass:
    case yellow_stained_glass_pane:
      return "hat";
    case iron_block:
      return "iron_xylophone";
    case glowstone:
      return "pling";
    case birch_wall_hanging_sign:
    case black_concrete_powder:
    case blue_concrete_powder:
    case brown_concrete_powder:
    case cyan_concrete_powder:
    case gravel:
    case gray_concrete_powder:
    case green_concrete_powder:
    case light_blue_concrete_powder:
    case light_gray_concrete_powder:
    case lime_concrete_powder:
    case magenta_concrete_powder:
    case orange_concrete_powder:
    case pink_concrete_powder:
    case purple_concrete_powder:
    case red_concrete_powder:
    case red_sand:
    case sand:
    case white_concrete_powder:
    case yellow_concrete_powder:
      return "snare";
    case bone_block:
      return "xylophone";
    default:
      return "harp";
    }
  }

  static std::string NoteBlockInstrument(mcfile::blocks::BlockId id) {
    using namespace mcfile::blocks::minecraft;
    switch (id) {
    case cave_vines_plant:
    case chorus_flower:
    case chorus_plant:
    case soul_fire:
    case sugar_cane:
      return "harp";
      // case kelp_plant:
      // case twisting_vines_plant:
      // case weeping_vines_plant:
    }
    return NoteBlockInstrumentAutogenCode(id);
  }
};

} // namespace je2be::terraform
