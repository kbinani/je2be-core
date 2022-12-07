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

  static std::string NoteBlockInstrument(mcfile::blocks::BlockId id) {
    using namespace mcfile::blocks::minecraft;
    switch (id) {
    case mcfile::blocks::minecraft::hay_block:
      return "banjo";
    case mcfile::blocks::minecraft::andesite:
    case mcfile::blocks::minecraft::andesite_slab:
    case mcfile::blocks::minecraft::andesite_stairs:
    case mcfile::blocks::minecraft::andesite_wall:
    case mcfile::blocks::minecraft::basalt:
    case mcfile::blocks::minecraft::bedrock:
    case mcfile::blocks::minecraft::black_concrete:
    case mcfile::blocks::minecraft::black_glazed_terracotta:
    case mcfile::blocks::minecraft::black_terracotta:
    case mcfile::blocks::minecraft::blackstone:
    case mcfile::blocks::minecraft::blackstone_slab:
    case mcfile::blocks::minecraft::blackstone_stairs:
    case mcfile::blocks::minecraft::blackstone_wall:
    case mcfile::blocks::minecraft::blast_furnace:
    case mcfile::blocks::minecraft::blue_concrete:
    case mcfile::blocks::minecraft::blue_glazed_terracotta:
    case mcfile::blocks::minecraft::blue_terracotta:
    case mcfile::blocks::minecraft::brain_coral_block:
    case mcfile::blocks::minecraft::brick_slab:
    case mcfile::blocks::minecraft::brick_stairs:
    case mcfile::blocks::minecraft::brick_wall:
    case mcfile::blocks::minecraft::bricks:
    case mcfile::blocks::minecraft::brown_concrete:
    case mcfile::blocks::minecraft::brown_glazed_terracotta:
    case mcfile::blocks::minecraft::brown_terracotta:
    case mcfile::blocks::minecraft::bubble_coral_block:
    case mcfile::blocks::minecraft::calcite:
    case mcfile::blocks::minecraft::chiseled_deepslate:
    case mcfile::blocks::minecraft::chiseled_nether_bricks:
    case mcfile::blocks::minecraft::chiseled_polished_blackstone:
    case mcfile::blocks::minecraft::chiseled_quartz_block:
    case mcfile::blocks::minecraft::chiseled_red_sandstone:
    case mcfile::blocks::minecraft::chiseled_sandstone:
    case mcfile::blocks::minecraft::chiseled_stone_bricks:
    case mcfile::blocks::minecraft::coal_block:
    case mcfile::blocks::minecraft::coal_ore:
    case mcfile::blocks::minecraft::cobbled_deepslate:
    case mcfile::blocks::minecraft::cobbled_deepslate_slab:
    case mcfile::blocks::minecraft::cobbled_deepslate_stairs:
    case mcfile::blocks::minecraft::cobbled_deepslate_wall:
    case mcfile::blocks::minecraft::cobblestone:
    case mcfile::blocks::minecraft::cobblestone_slab:
    case mcfile::blocks::minecraft::cobblestone_stairs:
    case mcfile::blocks::minecraft::cobblestone_wall:
    case mcfile::blocks::minecraft::copper_ore:
    case mcfile::blocks::minecraft::cracked_deepslate_bricks:
    case mcfile::blocks::minecraft::cracked_deepslate_tiles:
    case mcfile::blocks::minecraft::cracked_nether_bricks:
    case mcfile::blocks::minecraft::cracked_polished_blackstone_bricks:
    case mcfile::blocks::minecraft::cracked_stone_bricks:
    case mcfile::blocks::minecraft::crimson_nylium:
    case mcfile::blocks::minecraft::crying_obsidian:
    case mcfile::blocks::minecraft::cut_red_sandstone:
    case mcfile::blocks::minecraft::cut_red_sandstone_slab:
    case mcfile::blocks::minecraft::cut_sandstone:
    case mcfile::blocks::minecraft::cut_sandstone_slab:
    case mcfile::blocks::minecraft::cyan_concrete:
    case mcfile::blocks::minecraft::cyan_glazed_terracotta:
    case mcfile::blocks::minecraft::cyan_terracotta:
    case mcfile::blocks::minecraft::dark_prismarine:
    case mcfile::blocks::minecraft::dark_prismarine_slab:
    case mcfile::blocks::minecraft::dark_prismarine_stairs:
    case mcfile::blocks::minecraft::dead_brain_coral:
    case mcfile::blocks::minecraft::dead_brain_coral_block:
    case mcfile::blocks::minecraft::dead_brain_coral_fan:
    case mcfile::blocks::minecraft::dead_brain_coral_wall_fan:
    case mcfile::blocks::minecraft::dead_bubble_coral:
    case mcfile::blocks::minecraft::dead_bubble_coral_block:
    case mcfile::blocks::minecraft::dead_bubble_coral_fan:
    case mcfile::blocks::minecraft::dead_bubble_coral_wall_fan:
    case mcfile::blocks::minecraft::dead_fire_coral:
    case mcfile::blocks::minecraft::dead_fire_coral_block:
    case mcfile::blocks::minecraft::dead_fire_coral_fan:
    case mcfile::blocks::minecraft::dead_fire_coral_wall_fan:
    case mcfile::blocks::minecraft::dead_horn_coral:
    case mcfile::blocks::minecraft::dead_horn_coral_block:
    case mcfile::blocks::minecraft::dead_horn_coral_fan:
    case mcfile::blocks::minecraft::dead_horn_coral_wall_fan:
    case mcfile::blocks::minecraft::dead_tube_coral:
    case mcfile::blocks::minecraft::dead_tube_coral_block:
    case mcfile::blocks::minecraft::dead_tube_coral_fan:
    case mcfile::blocks::minecraft::dead_tube_coral_wall_fan:
    case mcfile::blocks::minecraft::deepslate:
    case mcfile::blocks::minecraft::deepslate_brick_slab:
    case mcfile::blocks::minecraft::deepslate_brick_stairs:
    case mcfile::blocks::minecraft::deepslate_brick_wall:
    case mcfile::blocks::minecraft::deepslate_bricks:
    case mcfile::blocks::minecraft::deepslate_coal_ore:
    case mcfile::blocks::minecraft::deepslate_copper_ore:
    case mcfile::blocks::minecraft::deepslate_diamond_ore:
    case mcfile::blocks::minecraft::deepslate_emerald_ore:
    case mcfile::blocks::minecraft::deepslate_gold_ore:
    case mcfile::blocks::minecraft::deepslate_iron_ore:
    case mcfile::blocks::minecraft::deepslate_lapis_ore:
    case mcfile::blocks::minecraft::deepslate_redstone_ore:
    case mcfile::blocks::minecraft::deepslate_tile_slab:
    case mcfile::blocks::minecraft::deepslate_tile_stairs:
    case mcfile::blocks::minecraft::deepslate_tile_wall:
    case mcfile::blocks::minecraft::deepslate_tiles:
    case mcfile::blocks::minecraft::diamond_ore:
    case mcfile::blocks::minecraft::diorite:
    case mcfile::blocks::minecraft::diorite_slab:
    case mcfile::blocks::minecraft::diorite_stairs:
    case mcfile::blocks::minecraft::diorite_wall:
    case mcfile::blocks::minecraft::dispenser:
    case mcfile::blocks::minecraft::dripstone_block:
    case mcfile::blocks::minecraft::dropper:
    case mcfile::blocks::minecraft::emerald_ore:
    case mcfile::blocks::minecraft::enchanting_table:
    case mcfile::blocks::minecraft::end_portal_frame:
    case mcfile::blocks::minecraft::end_stone:
    case mcfile::blocks::minecraft::end_stone_brick_slab:
    case mcfile::blocks::minecraft::end_stone_brick_stairs:
    case mcfile::blocks::minecraft::end_stone_brick_wall:
    case mcfile::blocks::minecraft::end_stone_bricks:
    case mcfile::blocks::minecraft::ender_chest:
    case mcfile::blocks::minecraft::fire_coral_block:
    case mcfile::blocks::minecraft::furnace:
    case mcfile::blocks::minecraft::gilded_blackstone:
    case mcfile::blocks::minecraft::gold_ore:
    case mcfile::blocks::minecraft::granite:
    case mcfile::blocks::minecraft::granite_slab:
    case mcfile::blocks::minecraft::granite_stairs:
    case mcfile::blocks::minecraft::granite_wall:
    case mcfile::blocks::minecraft::gray_concrete:
    case mcfile::blocks::minecraft::gray_glazed_terracotta:
    case mcfile::blocks::minecraft::gray_terracotta:
    case mcfile::blocks::minecraft::green_concrete:
    case mcfile::blocks::minecraft::green_glazed_terracotta:
    case mcfile::blocks::minecraft::green_terracotta:
    case mcfile::blocks::minecraft::horn_coral_block:
    case mcfile::blocks::minecraft::iron_ore:
    case mcfile::blocks::minecraft::lapis_ore:
    case mcfile::blocks::minecraft::light_blue_concrete:
    case mcfile::blocks::minecraft::light_blue_glazed_terracotta:
    case mcfile::blocks::minecraft::light_blue_terracotta:
    case mcfile::blocks::minecraft::light_gray_concrete:
    case mcfile::blocks::minecraft::light_gray_glazed_terracotta:
    case mcfile::blocks::minecraft::light_gray_terracotta:
    case mcfile::blocks::minecraft::lime_concrete:
    case mcfile::blocks::minecraft::lime_glazed_terracotta:
    case mcfile::blocks::minecraft::lime_terracotta:
    case mcfile::blocks::minecraft::magenta_concrete:
    case mcfile::blocks::minecraft::magenta_glazed_terracotta:
    case mcfile::blocks::minecraft::magenta_terracotta:
    case mcfile::blocks::minecraft::magma_block:
    case mcfile::blocks::minecraft::mossy_cobblestone:
    case mcfile::blocks::minecraft::mossy_cobblestone_slab:
    case mcfile::blocks::minecraft::mossy_cobblestone_stairs:
    case mcfile::blocks::minecraft::mossy_cobblestone_wall:
    case mcfile::blocks::minecraft::mossy_stone_brick_slab:
    case mcfile::blocks::minecraft::mossy_stone_brick_stairs:
    case mcfile::blocks::minecraft::mossy_stone_brick_wall:
    case mcfile::blocks::minecraft::mossy_stone_bricks:
    case mcfile::blocks::minecraft::nether_brick_fence:
    case mcfile::blocks::minecraft::nether_brick_slab:
    case mcfile::blocks::minecraft::nether_brick_stairs:
    case mcfile::blocks::minecraft::nether_brick_wall:
    case mcfile::blocks::minecraft::nether_bricks:
    case mcfile::blocks::minecraft::nether_gold_ore:
    case mcfile::blocks::minecraft::nether_quartz_ore:
    case mcfile::blocks::minecraft::netherrack:
    case mcfile::blocks::minecraft::observer:
    case mcfile::blocks::minecraft::obsidian:
    case mcfile::blocks::minecraft::orange_concrete:
    case mcfile::blocks::minecraft::orange_glazed_terracotta:
    case mcfile::blocks::minecraft::orange_terracotta:
    case mcfile::blocks::minecraft::petrified_oak_slab:
    case mcfile::blocks::minecraft::pink_concrete:
    case mcfile::blocks::minecraft::pink_glazed_terracotta:
    case mcfile::blocks::minecraft::pink_terracotta:
    case mcfile::blocks::minecraft::pointed_dripstone:
    case mcfile::blocks::minecraft::polished_andesite:
    case mcfile::blocks::minecraft::polished_andesite_slab:
    case mcfile::blocks::minecraft::polished_andesite_stairs:
    case mcfile::blocks::minecraft::polished_basalt:
    case mcfile::blocks::minecraft::polished_blackstone:
    case mcfile::blocks::minecraft::polished_blackstone_brick_slab:
    case mcfile::blocks::minecraft::polished_blackstone_brick_stairs:
    case mcfile::blocks::minecraft::polished_blackstone_brick_wall:
    case mcfile::blocks::minecraft::polished_blackstone_bricks:
    case mcfile::blocks::minecraft::polished_blackstone_pressure_plate:
    case mcfile::blocks::minecraft::polished_blackstone_slab:
    case mcfile::blocks::minecraft::polished_blackstone_stairs:
    case mcfile::blocks::minecraft::polished_blackstone_wall:
    case mcfile::blocks::minecraft::polished_deepslate:
    case mcfile::blocks::minecraft::polished_deepslate_slab:
    case mcfile::blocks::minecraft::polished_deepslate_stairs:
    case mcfile::blocks::minecraft::polished_deepslate_wall:
    case mcfile::blocks::minecraft::polished_diorite:
    case mcfile::blocks::minecraft::polished_diorite_slab:
    case mcfile::blocks::minecraft::polished_diorite_stairs:
    case mcfile::blocks::minecraft::polished_granite:
    case mcfile::blocks::minecraft::polished_granite_slab:
    case mcfile::blocks::minecraft::polished_granite_stairs:
    case mcfile::blocks::minecraft::prismarine:
    case mcfile::blocks::minecraft::prismarine_brick_slab:
    case mcfile::blocks::minecraft::prismarine_brick_stairs:
    case mcfile::blocks::minecraft::prismarine_bricks:
    case mcfile::blocks::minecraft::prismarine_slab:
    case mcfile::blocks::minecraft::prismarine_stairs:
    case mcfile::blocks::minecraft::prismarine_wall:
    case mcfile::blocks::minecraft::purple_concrete:
    case mcfile::blocks::minecraft::purple_glazed_terracotta:
    case mcfile::blocks::minecraft::purple_terracotta:
    case mcfile::blocks::minecraft::purpur_block:
    case mcfile::blocks::minecraft::purpur_pillar:
    case mcfile::blocks::minecraft::purpur_slab:
    case mcfile::blocks::minecraft::purpur_stairs:
    case mcfile::blocks::minecraft::quartz_block:
    case mcfile::blocks::minecraft::quartz_bricks:
    case mcfile::blocks::minecraft::quartz_pillar:
    case mcfile::blocks::minecraft::quartz_slab:
    case mcfile::blocks::minecraft::quartz_stairs:
    case mcfile::blocks::minecraft::raw_copper_block:
    case mcfile::blocks::minecraft::raw_gold_block:
    case mcfile::blocks::minecraft::raw_iron_block:
    case mcfile::blocks::minecraft::red_concrete:
    case mcfile::blocks::minecraft::red_glazed_terracotta:
    case mcfile::blocks::minecraft::red_nether_brick_slab:
    case mcfile::blocks::minecraft::red_nether_brick_stairs:
    case mcfile::blocks::minecraft::red_nether_brick_wall:
    case mcfile::blocks::minecraft::red_nether_bricks:
    case mcfile::blocks::minecraft::red_sandstone:
    case mcfile::blocks::minecraft::red_sandstone_slab:
    case mcfile::blocks::minecraft::red_sandstone_stairs:
    case mcfile::blocks::minecraft::red_sandstone_wall:
    case mcfile::blocks::minecraft::red_terracotta:
    case mcfile::blocks::minecraft::redstone_ore:
    case mcfile::blocks::minecraft::respawn_anchor:
    case mcfile::blocks::minecraft::sandstone:
    case mcfile::blocks::minecraft::sandstone_slab:
    case mcfile::blocks::minecraft::sandstone_stairs:
    case mcfile::blocks::minecraft::sandstone_wall:
    case mcfile::blocks::minecraft::smoker:
    case mcfile::blocks::minecraft::smooth_basalt:
    case mcfile::blocks::minecraft::smooth_quartz:
    case mcfile::blocks::minecraft::smooth_quartz_slab:
    case mcfile::blocks::minecraft::smooth_quartz_stairs:
    case mcfile::blocks::minecraft::smooth_red_sandstone:
    case mcfile::blocks::minecraft::smooth_red_sandstone_slab:
    case mcfile::blocks::minecraft::smooth_red_sandstone_stairs:
    case mcfile::blocks::minecraft::smooth_sandstone:
    case mcfile::blocks::minecraft::smooth_sandstone_slab:
    case mcfile::blocks::minecraft::smooth_sandstone_stairs:
    case mcfile::blocks::minecraft::smooth_stone:
    case mcfile::blocks::minecraft::smooth_stone_slab:
    case mcfile::blocks::minecraft::spawner:
    case mcfile::blocks::minecraft::stone:
    case mcfile::blocks::minecraft::stone_brick_slab:
    case mcfile::blocks::minecraft::stone_brick_stairs:
    case mcfile::blocks::minecraft::stone_brick_wall:
    case mcfile::blocks::minecraft::stone_bricks:
    case mcfile::blocks::minecraft::stone_pressure_plate:
    case mcfile::blocks::minecraft::stone_slab:
    case mcfile::blocks::minecraft::stone_stairs:
    case mcfile::blocks::minecraft::stonecutter:
    case mcfile::blocks::minecraft::terracotta:
    case mcfile::blocks::minecraft::tuff:
    case mcfile::blocks::minecraft::warped_nylium:
    case mcfile::blocks::minecraft::white_concrete:
    case mcfile::blocks::minecraft::white_glazed_terracotta:
    case mcfile::blocks::minecraft::white_terracotta:
    case mcfile::blocks::minecraft::yellow_concrete:
    case mcfile::blocks::minecraft::yellow_glazed_terracotta:
    case mcfile::blocks::minecraft::yellow_terracotta:
    case mud_brick_slab:
    case mud_bricks:
    case mud_brick_wall:
    case mud_brick_stairs:
    case reinforced_deepslate:
      return "basedrum";
    case mcfile::blocks::minecraft::acacia_fence:
    case mcfile::blocks::minecraft::acacia_fence_gate:
    case mcfile::blocks::minecraft::acacia_log:
    case mcfile::blocks::minecraft::acacia_planks:
    case mcfile::blocks::minecraft::acacia_pressure_plate:
    case mcfile::blocks::minecraft::acacia_sign:
    case mcfile::blocks::minecraft::acacia_slab:
    case mcfile::blocks::minecraft::acacia_stairs:
    case mcfile::blocks::minecraft::acacia_trapdoor:
    case mcfile::blocks::minecraft::acacia_wall_sign:
    case mcfile::blocks::minecraft::acacia_wood:
    case mcfile::blocks::minecraft::barrel:
    case mcfile::blocks::minecraft::bee_nest:
    case mcfile::blocks::minecraft::beehive:
    case mcfile::blocks::minecraft::birch_fence:
    case mcfile::blocks::minecraft::birch_fence_gate:
    case mcfile::blocks::minecraft::birch_log:
    case mcfile::blocks::minecraft::birch_planks:
    case mcfile::blocks::minecraft::birch_pressure_plate:
    case mcfile::blocks::minecraft::birch_sign:
    case mcfile::blocks::minecraft::birch_slab:
    case mcfile::blocks::minecraft::birch_stairs:
    case mcfile::blocks::minecraft::birch_trapdoor:
    case mcfile::blocks::minecraft::birch_wall_sign:
    case mcfile::blocks::minecraft::birch_wood:
    case mcfile::blocks::minecraft::black_banner:
    case mcfile::blocks::minecraft::black_wall_banner:
    case mcfile::blocks::minecraft::blue_banner:
    case mcfile::blocks::minecraft::blue_wall_banner:
    case mcfile::blocks::minecraft::bookshelf:
    case mcfile::blocks::minecraft::brown_banner:
    case mcfile::blocks::minecraft::brown_mushroom_block:
    case mcfile::blocks::minecraft::brown_wall_banner:
    case mcfile::blocks::minecraft::campfire:
    case mcfile::blocks::minecraft::cartography_table:
    case mcfile::blocks::minecraft::chest:
    case mcfile::blocks::minecraft::composter:
    case mcfile::blocks::minecraft::crafting_table:
    case mcfile::blocks::minecraft::crimson_fence:
    case mcfile::blocks::minecraft::crimson_fence_gate:
    case mcfile::blocks::minecraft::crimson_hyphae:
    case mcfile::blocks::minecraft::crimson_planks:
    case mcfile::blocks::minecraft::crimson_pressure_plate:
    case mcfile::blocks::minecraft::crimson_sign:
    case mcfile::blocks::minecraft::crimson_slab:
    case mcfile::blocks::minecraft::crimson_stairs:
    case mcfile::blocks::minecraft::crimson_stem:
    case mcfile::blocks::minecraft::crimson_trapdoor:
    case mcfile::blocks::minecraft::crimson_wall_sign:
    case mcfile::blocks::minecraft::cyan_banner:
    case mcfile::blocks::minecraft::cyan_wall_banner:
    case mcfile::blocks::minecraft::dark_oak_fence:
    case mcfile::blocks::minecraft::dark_oak_fence_gate:
    case mcfile::blocks::minecraft::dark_oak_log:
    case mcfile::blocks::minecraft::dark_oak_planks:
    case mcfile::blocks::minecraft::dark_oak_pressure_plate:
    case mcfile::blocks::minecraft::dark_oak_sign:
    case mcfile::blocks::minecraft::dark_oak_slab:
    case mcfile::blocks::minecraft::dark_oak_stairs:
    case mcfile::blocks::minecraft::dark_oak_trapdoor:
    case mcfile::blocks::minecraft::dark_oak_wall_sign:
    case mcfile::blocks::minecraft::dark_oak_wood:
    case mcfile::blocks::minecraft::daylight_detector:
    case mcfile::blocks::minecraft::fletching_table:
    case mcfile::blocks::minecraft::gray_banner:
    case mcfile::blocks::minecraft::gray_wall_banner:
    case mcfile::blocks::minecraft::green_banner:
    case mcfile::blocks::minecraft::green_wall_banner:
    case mcfile::blocks::minecraft::jukebox:
    case mcfile::blocks::minecraft::jungle_fence:
    case mcfile::blocks::minecraft::jungle_fence_gate:
    case mcfile::blocks::minecraft::jungle_log:
    case mcfile::blocks::minecraft::jungle_planks:
    case mcfile::blocks::minecraft::jungle_pressure_plate:
    case mcfile::blocks::minecraft::jungle_sign:
    case mcfile::blocks::minecraft::jungle_slab:
    case mcfile::blocks::minecraft::jungle_stairs:
    case mcfile::blocks::minecraft::jungle_trapdoor:
    case mcfile::blocks::minecraft::jungle_wall_sign:
    case mcfile::blocks::minecraft::jungle_wood:
    case mcfile::blocks::minecraft::lectern:
    case mcfile::blocks::minecraft::light_blue_banner:
    case mcfile::blocks::minecraft::light_blue_wall_banner:
    case mcfile::blocks::minecraft::light_gray_banner:
    case mcfile::blocks::minecraft::light_gray_wall_banner:
    case mcfile::blocks::minecraft::lime_banner:
    case mcfile::blocks::minecraft::lime_wall_banner:
    case mcfile::blocks::minecraft::loom:
    case mcfile::blocks::minecraft::magenta_banner:
    case mcfile::blocks::minecraft::magenta_wall_banner:
    case mcfile::blocks::minecraft::mushroom_stem:
    case mcfile::blocks::minecraft::note_block:
    case mcfile::blocks::minecraft::oak_fence:
    case mcfile::blocks::minecraft::oak_fence_gate:
    case mcfile::blocks::minecraft::oak_log:
    case mcfile::blocks::minecraft::oak_planks:
    case mcfile::blocks::minecraft::oak_pressure_plate:
    case mcfile::blocks::minecraft::oak_sign:
    case mcfile::blocks::minecraft::oak_slab:
    case mcfile::blocks::minecraft::oak_stairs:
    case mcfile::blocks::minecraft::oak_trapdoor:
    case mcfile::blocks::minecraft::oak_wall_sign:
    case mcfile::blocks::minecraft::oak_wood:
    case mcfile::blocks::minecraft::orange_banner:
    case mcfile::blocks::minecraft::orange_wall_banner:
    case mcfile::blocks::minecraft::pink_banner:
    case mcfile::blocks::minecraft::pink_wall_banner:
    case mcfile::blocks::minecraft::purple_banner:
    case mcfile::blocks::minecraft::purple_wall_banner:
    case mcfile::blocks::minecraft::red_banner:
    case mcfile::blocks::minecraft::red_mushroom_block:
    case mcfile::blocks::minecraft::red_wall_banner:
    case mcfile::blocks::minecraft::smithing_table:
    case mcfile::blocks::minecraft::soul_campfire:
    case mcfile::blocks::minecraft::spruce_fence:
    case mcfile::blocks::minecraft::spruce_fence_gate:
    case mcfile::blocks::minecraft::spruce_log:
    case mcfile::blocks::minecraft::spruce_planks:
    case mcfile::blocks::minecraft::spruce_pressure_plate:
    case mcfile::blocks::minecraft::spruce_sign:
    case mcfile::blocks::minecraft::spruce_slab:
    case mcfile::blocks::minecraft::spruce_stairs:
    case mcfile::blocks::minecraft::spruce_trapdoor:
    case mcfile::blocks::minecraft::spruce_wall_sign:
    case mcfile::blocks::minecraft::spruce_wood:
    case mcfile::blocks::minecraft::stripped_acacia_log:
    case mcfile::blocks::minecraft::stripped_acacia_wood:
    case mcfile::blocks::minecraft::stripped_birch_log:
    case mcfile::blocks::minecraft::stripped_birch_wood:
    case mcfile::blocks::minecraft::stripped_crimson_hyphae:
    case mcfile::blocks::minecraft::stripped_crimson_stem:
    case mcfile::blocks::minecraft::stripped_dark_oak_log:
    case mcfile::blocks::minecraft::stripped_dark_oak_wood:
    case mcfile::blocks::minecraft::stripped_jungle_log:
    case mcfile::blocks::minecraft::stripped_jungle_wood:
    case mcfile::blocks::minecraft::stripped_oak_log:
    case mcfile::blocks::minecraft::stripped_oak_wood:
    case mcfile::blocks::minecraft::stripped_spruce_log:
    case mcfile::blocks::minecraft::stripped_spruce_wood:
    case mcfile::blocks::minecraft::stripped_warped_hyphae:
    case mcfile::blocks::minecraft::stripped_warped_stem:
    case mcfile::blocks::minecraft::trapped_chest:
    case mcfile::blocks::minecraft::warped_fence:
    case mcfile::blocks::minecraft::warped_fence_gate:
    case mcfile::blocks::minecraft::warped_hyphae:
    case mcfile::blocks::minecraft::warped_planks:
    case mcfile::blocks::minecraft::warped_pressure_plate:
    case mcfile::blocks::minecraft::warped_sign:
    case mcfile::blocks::minecraft::warped_slab:
    case mcfile::blocks::minecraft::warped_stairs:
    case mcfile::blocks::minecraft::warped_stem:
    case mcfile::blocks::minecraft::warped_trapdoor:
    case mcfile::blocks::minecraft::warped_wall_sign:
    case mcfile::blocks::minecraft::white_banner:
    case mcfile::blocks::minecraft::white_wall_banner:
    case mcfile::blocks::minecraft::yellow_banner:
    case mcfile::blocks::minecraft::yellow_wall_banner:
    case acacia_door:
    case birch_door:
    case crimson_door:
    case dark_oak_door:
    case jungle_door:
    case oak_door:
    case spruce_door:
    case warped_door:
    case mangrove_planks:
    case mangrove_roots:
    case mangrove_log:
    case stripped_mangrove_log:
    case stripped_mangrove_wood:
    case mangrove_wood:
    case mangrove_slab:
    case mangrove_fence:
    case mangrove_stairs:
    case mangrove_fence_gate:
    case mangrove_sign:
    case mangrove_wall_sign:
    case mangrove_pressure_plate:
    case mangrove_door:
    case mangrove_trapdoor:
      return "bass";
    case mcfile::blocks::minecraft::gold_block:
      return "bell";
    case mcfile::blocks::minecraft::emerald_block:
      return "bit";
    case mcfile::blocks::minecraft::packed_ice:
      return "chime";
    case mcfile::blocks::minecraft::soul_sand:
      return "cow_bell";
    case mcfile::blocks::minecraft::pumpkin:
      return "didgeridoo";
    case mcfile::blocks::minecraft::clay:
      return "flute";
    case mcfile::blocks::minecraft::black_wool:
    case mcfile::blocks::minecraft::blue_wool:
    case mcfile::blocks::minecraft::brown_wool:
    case mcfile::blocks::minecraft::cyan_wool:
    case mcfile::blocks::minecraft::gray_wool:
    case mcfile::blocks::minecraft::green_wool:
    case mcfile::blocks::minecraft::light_blue_wool:
    case mcfile::blocks::minecraft::light_gray_wool:
    case mcfile::blocks::minecraft::lime_wool:
    case mcfile::blocks::minecraft::magenta_wool:
    case mcfile::blocks::minecraft::orange_wool:
    case mcfile::blocks::minecraft::pink_wool:
    case mcfile::blocks::minecraft::purple_wool:
    case mcfile::blocks::minecraft::red_wool:
    case mcfile::blocks::minecraft::white_wool:
    case mcfile::blocks::minecraft::yellow_wool:
      return "guitar";
    case mcfile::blocks::minecraft::beacon:
    case mcfile::blocks::minecraft::black_stained_glass:
    case mcfile::blocks::minecraft::black_stained_glass_pane:
    case mcfile::blocks::minecraft::blue_stained_glass:
    case mcfile::blocks::minecraft::blue_stained_glass_pane:
    case mcfile::blocks::minecraft::brown_stained_glass:
    case mcfile::blocks::minecraft::brown_stained_glass_pane:
    case mcfile::blocks::minecraft::conduit:
    case mcfile::blocks::minecraft::cyan_stained_glass:
    case mcfile::blocks::minecraft::cyan_stained_glass_pane:
    case mcfile::blocks::minecraft::glass:
    case mcfile::blocks::minecraft::glass_pane:
    case mcfile::blocks::minecraft::gray_stained_glass:
    case mcfile::blocks::minecraft::gray_stained_glass_pane:
    case mcfile::blocks::minecraft::green_stained_glass:
    case mcfile::blocks::minecraft::green_stained_glass_pane:
    case mcfile::blocks::minecraft::light_blue_stained_glass:
    case mcfile::blocks::minecraft::light_blue_stained_glass_pane:
    case mcfile::blocks::minecraft::light_gray_stained_glass:
    case mcfile::blocks::minecraft::light_gray_stained_glass_pane:
    case mcfile::blocks::minecraft::lime_stained_glass:
    case mcfile::blocks::minecraft::lime_stained_glass_pane:
    case mcfile::blocks::minecraft::magenta_stained_glass:
    case mcfile::blocks::minecraft::magenta_stained_glass_pane:
    case mcfile::blocks::minecraft::orange_stained_glass:
    case mcfile::blocks::minecraft::orange_stained_glass_pane:
    case mcfile::blocks::minecraft::pink_stained_glass:
    case mcfile::blocks::minecraft::pink_stained_glass_pane:
    case mcfile::blocks::minecraft::purple_stained_glass:
    case mcfile::blocks::minecraft::purple_stained_glass_pane:
    case mcfile::blocks::minecraft::red_stained_glass:
    case mcfile::blocks::minecraft::red_stained_glass_pane:
    case mcfile::blocks::minecraft::sea_lantern:
    case mcfile::blocks::minecraft::tinted_glass:
    case mcfile::blocks::minecraft::white_stained_glass:
    case mcfile::blocks::minecraft::white_stained_glass_pane:
    case mcfile::blocks::minecraft::yellow_stained_glass:
    case mcfile::blocks::minecraft::yellow_stained_glass_pane:
      return "hat";
    case mcfile::blocks::minecraft::iron_block:
      return "iron_xylophone";
    case mcfile::blocks::minecraft::glowstone:
      return "pling";
    case mcfile::blocks::minecraft::black_concrete_powder:
    case mcfile::blocks::minecraft::blue_concrete_powder:
    case mcfile::blocks::minecraft::brown_concrete_powder:
    case mcfile::blocks::minecraft::cyan_concrete_powder:
    case mcfile::blocks::minecraft::gravel:
    case mcfile::blocks::minecraft::gray_concrete_powder:
    case mcfile::blocks::minecraft::green_concrete_powder:
    case mcfile::blocks::minecraft::light_blue_concrete_powder:
    case mcfile::blocks::minecraft::light_gray_concrete_powder:
    case mcfile::blocks::minecraft::lime_concrete_powder:
    case mcfile::blocks::minecraft::magenta_concrete_powder:
    case mcfile::blocks::minecraft::orange_concrete_powder:
    case mcfile::blocks::minecraft::pink_concrete_powder:
    case mcfile::blocks::minecraft::purple_concrete_powder:
    case mcfile::blocks::minecraft::red_concrete_powder:
    case mcfile::blocks::minecraft::red_sand:
    case mcfile::blocks::minecraft::sand:
    case mcfile::blocks::minecraft::white_concrete_powder:
    case mcfile::blocks::minecraft::yellow_concrete_powder:
      return "snare";
    case mcfile::blocks::minecraft::bone_block:
      return "xylophone";
    }
    return "harp";
  }
};

} // namespace je2be::terraform
