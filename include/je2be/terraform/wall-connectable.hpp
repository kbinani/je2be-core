#pragma once

namespace je2be::terraform {

class WallConnectable {
  WallConnectable() = delete;

public:
  static void Do(mcfile::je::Chunk &out, BlockAccessor &blockAccessor, BlockPropertyAccessor const &accessor) {
    using namespace std;

    if (!accessor.fHasWall) {
      return;
    }

    int cx = out.fChunkX;
    int cz = out.fChunkZ;

    vector<pair<string, Pos2i>> const nesw({{"north", Pos2i(0, -1)}, {"east", Pos2i(1, 0)}, {"south", Pos2i(0, 1)}, {"west", Pos2i(-1, 0)}});

    for (int y = accessor.minBlockY(); y <= accessor.maxBlockY(); y++) {
      for (int lz = 0; lz < 16; lz++) {
        int z = cz * 16 + lz;
        for (int lx = 0; lx < 16; lx++) {
          int x = cx * 16 + lx;

          auto p = accessor.property(x, y, z);
          if (!BlockPropertyAccessor::IsWall(p)) {
            continue;
          }

          auto blockJ = out.blockAt(x, y, z);
          if (!blockJ) {
            continue;
          }
          map<string, string> props(blockJ->fProperties);

          auto upper = blockAccessor.blockAt(x, y + 1, z);

          for (auto d : nesw) {
            string direction = d.first;
            Pos2i vec = d.second;
            props[direction] = "none";
            auto pos = Pos2i(x, z) + vec;
            auto target = blockAccessor.blockAt(pos.fX, y, pos.fZ);
            if (!target) {
              continue;
            }
            if (IsWallConnectable(*target, vec)) {
              props[direction] = "low";
            }
            if (upper && IsBlockMakeWallTallShape(*upper)) {
              props[direction] = "tall";
            }
          }

          auto replace = make_shared<mcfile::je::Block const>(blockJ->fName, props);
          out.setBlockAt(x, y, z, replace);
        }
      }
    }
  }

  static bool IsWallConnectable(mcfile::je::Block const &target, Pos2i targetDirection) {
    if (IsWallAlwaysConnectable(target.fId)) {
      return true;
    }
    if (target.fName.find("trapdoor") != std::string::npos) {
      auto targetBlockFace = Pos2iFromFacing4(Facing4FromJavaName(target.property("facing")));
      if (target.property("open") == "true" && targetBlockFace == targetDirection) {
        return true;
      }
    } else if (target.fName.ends_with("stairs")) {
      auto targetBlockFace = Pos2iFromFacing4(Facing4FromJavaName(target.property("facing")));
      if (IsOrthogonal(targetBlockFace, targetDirection)) {
        return true;
      }
    } else if (target.fName.ends_with("slab")) {
      if (target.property("type") == "double") {
        return true;
      }
    }
    // TODO: doors, piston_head, etc.
    return false;
  }

  static bool IsBlockMakeWallTallShape(mcfile::je::Block const &b) {
    if (IsBlockAlwaysMakeWallTallShape(b.fId)) {
      return true;
    }
    if (b.fName.ends_with("slab") && b.property("type") != "top") {
      return true;
    }
    if (b.fName.ends_with("trapdoor") && b.property("half") == "bottom" && b.property("open") == "false") {
      return true;
    }
    if (b.fName.ends_with("stairs") && b.property("half") == "bottom") {
      return true;
    }
    return false;
  }

  static bool IsWallAlwaysConnectable(mcfile::blocks::BlockId id) {
    using namespace mcfile::blocks;
    switch (id) {
    case minecraft::acacia_log:
    case minecraft::acacia_planks:
    case minecraft::acacia_wood:
    case minecraft::amethyst_block:
    case minecraft::ancient_debris:
    case minecraft::andesite:
    case minecraft::andesite_wall:
    case minecraft::barrel:
    case minecraft::basalt:
    case minecraft::beacon:
    case minecraft::bedrock:
    case minecraft::bee_nest:
    case minecraft::beehive:
    case minecraft::birch_log:
    case minecraft::birch_planks:
    case minecraft::birch_wood:
    case minecraft::black_concrete:
    case minecraft::black_concrete_powder:
    case minecraft::black_glazed_terracotta:
    case minecraft::black_stained_glass:
    case minecraft::black_stained_glass_pane:
    case minecraft::black_terracotta:
    case minecraft::black_wool:
    case minecraft::blackstone:
    case minecraft::blackstone_wall:
    case minecraft::blast_furnace:
    case minecraft::blue_concrete:
    case minecraft::blue_concrete_powder:
    case minecraft::blue_glazed_terracotta:
    case minecraft::blue_ice:
    case minecraft::blue_stained_glass:
    case minecraft::blue_stained_glass_pane:
    case minecraft::blue_terracotta:
    case minecraft::blue_wool:
    case minecraft::bone_block:
    case minecraft::bookshelf:
    case minecraft::brain_coral_block:
    case minecraft::brick_wall:
    case minecraft::bricks:
    case minecraft::brown_concrete:
    case minecraft::brown_concrete_powder:
    case minecraft::brown_glazed_terracotta:
    case minecraft::brown_mushroom_block:
    case minecraft::brown_stained_glass:
    case minecraft::brown_stained_glass_pane:
    case minecraft::brown_terracotta:
    case minecraft::brown_wool:
    case minecraft::bubble_coral_block:
    case minecraft::budding_amethyst:
    case minecraft::calcite:
    case minecraft::cartography_table:
    case minecraft::chain_command_block:
    case minecraft::chiseled_deepslate:
    case minecraft::chiseled_nether_bricks:
    case minecraft::chiseled_polished_blackstone:
    case minecraft::chiseled_quartz_block:
    case minecraft::chiseled_red_sandstone:
    case minecraft::chiseled_sandstone:
    case minecraft::chiseled_stone_bricks:
    case minecraft::clay:
    case minecraft::coal_block:
    case minecraft::coal_ore:
    case minecraft::coarse_dirt:
    case minecraft::cobbled_deepslate:
    case minecraft::cobbled_deepslate_wall:
    case minecraft::cobblestone:
    case minecraft::cobblestone_wall:
    case minecraft::command_block:
    case minecraft::composter:
    case minecraft::copper_block:
    case minecraft::copper_ore:
    case minecraft::cracked_deepslate_bricks:
    case minecraft::cracked_deepslate_tiles:
    case minecraft::cracked_nether_bricks:
    case minecraft::cracked_polished_blackstone_bricks:
    case minecraft::cracked_stone_bricks:
    case minecraft::crafting_table:
    case minecraft::crimson_hyphae:
    case minecraft::crimson_nylium:
    case minecraft::crimson_planks:
    case minecraft::crimson_stem:
    case minecraft::crying_obsidian:
    case minecraft::cut_copper:
    case minecraft::cut_red_sandstone:
    case minecraft::cut_sandstone:
    case minecraft::cyan_concrete:
    case minecraft::cyan_concrete_powder:
    case minecraft::cyan_glazed_terracotta:
    case minecraft::cyan_stained_glass:
    case minecraft::cyan_stained_glass_pane:
    case minecraft::cyan_terracotta:
    case minecraft::cyan_wool:
    case minecraft::dark_oak_log:
    case minecraft::dark_oak_planks:
    case minecraft::dark_oak_wood:
    case minecraft::dark_prismarine:
    case minecraft::dead_brain_coral_block:
    case minecraft::dead_bubble_coral_block:
    case minecraft::dead_fire_coral_block:
    case minecraft::dead_horn_coral_block:
    case minecraft::dead_tube_coral_block:
    case minecraft::deepslate:
    case minecraft::deepslate_brick_wall:
    case minecraft::deepslate_bricks:
    case minecraft::deepslate_coal_ore:
    case minecraft::deepslate_copper_ore:
    case minecraft::deepslate_diamond_ore:
    case minecraft::deepslate_emerald_ore:
    case minecraft::deepslate_gold_ore:
    case minecraft::deepslate_iron_ore:
    case minecraft::deepslate_lapis_ore:
    case minecraft::deepslate_redstone_ore:
    case minecraft::deepslate_tile_wall:
    case minecraft::deepslate_tiles:
    case minecraft::diamond_block:
    case minecraft::diamond_ore:
    case minecraft::diorite:
    case minecraft::diorite_wall:
    case minecraft::dirt:
    case minecraft::dispenser:
    case minecraft::dried_kelp_block:
    case minecraft::dripstone_block:
    case minecraft::dropper:
    case minecraft::emerald_block:
    case minecraft::emerald_ore:
    case minecraft::end_stone:
    case minecraft::end_stone_brick_wall:
    case minecraft::end_stone_bricks:
    case minecraft::exposed_copper:
    case minecraft::exposed_cut_copper:
    case minecraft::fire_coral_block:
    case minecraft::fletching_table:
    case minecraft::frosted_ice:
    case minecraft::furnace:
    case minecraft::gilded_blackstone:
    case minecraft::glass:
    case minecraft::glass_pane:
    case minecraft::glowstone:
    case minecraft::gold_block:
    case minecraft::gold_ore:
    case minecraft::granite:
    case minecraft::granite_wall:
    case minecraft::grass_block:
    case minecraft::gravel:
    case minecraft::gray_concrete:
    case minecraft::gray_concrete_powder:
    case minecraft::gray_glazed_terracotta:
    case minecraft::gray_stained_glass:
    case minecraft::gray_stained_glass_pane:
    case minecraft::gray_terracotta:
    case minecraft::gray_wool:
    case minecraft::green_concrete:
    case minecraft::green_concrete_powder:
    case minecraft::green_glazed_terracotta:
    case minecraft::green_stained_glass:
    case minecraft::green_stained_glass_pane:
    case minecraft::green_terracotta:
    case minecraft::green_wool:
    case minecraft::hay_block:
    case minecraft::honeycomb_block:
    case minecraft::horn_coral_block:
    case minecraft::ice:
    case minecraft::infested_chiseled_stone_bricks:
    case minecraft::infested_cobblestone:
    case minecraft::infested_cracked_stone_bricks:
    case minecraft::infested_deepslate:
    case minecraft::infested_mossy_stone_bricks:
    case minecraft::infested_stone:
    case minecraft::infested_stone_bricks:
    case minecraft::iron_bars:
    case minecraft::iron_block:
    case minecraft::iron_ore:
    case minecraft::jigsaw:
    case minecraft::jukebox:
    case minecraft::jungle_log:
    case minecraft::jungle_planks:
    case minecraft::jungle_wood:
    case minecraft::lapis_block:
    case minecraft::lapis_ore:
    case minecraft::light_blue_concrete:
    case minecraft::light_blue_concrete_powder:
    case minecraft::light_blue_glazed_terracotta:
    case minecraft::light_blue_stained_glass:
    case minecraft::light_blue_stained_glass_pane:
    case minecraft::light_blue_terracotta:
    case minecraft::light_blue_wool:
    case minecraft::light_gray_concrete:
    case minecraft::light_gray_concrete_powder:
    case minecraft::light_gray_glazed_terracotta:
    case minecraft::light_gray_stained_glass:
    case minecraft::light_gray_stained_glass_pane:
    case minecraft::light_gray_terracotta:
    case minecraft::light_gray_wool:
    case minecraft::lime_concrete:
    case minecraft::lime_concrete_powder:
    case minecraft::lime_glazed_terracotta:
    case minecraft::lime_stained_glass:
    case minecraft::lime_stained_glass_pane:
    case minecraft::lime_terracotta:
    case minecraft::lime_wool:
    case minecraft::lodestone:
    case minecraft::loom:
    case minecraft::magenta_concrete:
    case minecraft::magenta_concrete_powder:
    case minecraft::magenta_glazed_terracotta:
    case minecraft::magenta_stained_glass:
    case minecraft::magenta_stained_glass_pane:
    case minecraft::magenta_terracotta:
    case minecraft::magenta_wool:
    case minecraft::magma_block:
    case minecraft::mangrove_log:
    case minecraft::mangrove_planks:
    case minecraft::mangrove_roots:
    case minecraft::mangrove_wood:
    case minecraft::moss_block:
    case minecraft::mossy_cobblestone:
    case minecraft::mossy_cobblestone_wall:
    case minecraft::mossy_stone_brick_wall:
    case minecraft::mossy_stone_bricks:
    case minecraft::mud:
    case minecraft::mud_brick_wall:
    case minecraft::mud_bricks:
    case minecraft::muddy_mangrove_roots:
    case minecraft::mushroom_stem:
    case minecraft::mycelium:
    case minecraft::nether_brick_wall:
    case minecraft::nether_bricks:
    case minecraft::nether_gold_ore:
    case minecraft::nether_quartz_ore:
    case minecraft::nether_wart_block:
    case minecraft::netherite_block:
    case minecraft::netherrack:
    case minecraft::note_block:
    case minecraft::oak_log:
    case minecraft::oak_planks:
    case minecraft::oak_wood:
    case minecraft::observer:
    case minecraft::obsidian:
    case minecraft::ochre_froglight:
    case minecraft::orange_concrete:
    case minecraft::orange_concrete_powder:
    case minecraft::orange_glazed_terracotta:
    case minecraft::orange_stained_glass:
    case minecraft::orange_stained_glass_pane:
    case minecraft::orange_terracotta:
    case minecraft::orange_wool:
    case minecraft::oxidized_copper:
    case minecraft::oxidized_cut_copper:
    case minecraft::packed_ice:
    case minecraft::packed_mud:
    case minecraft::pearlescent_froglight:
    case minecraft::pink_concrete:
    case minecraft::pink_concrete_powder:
    case minecraft::pink_glazed_terracotta:
    case minecraft::pink_stained_glass:
    case minecraft::pink_stained_glass_pane:
    case minecraft::pink_terracotta:
    case minecraft::pink_wool:
    case minecraft::podzol:
    case minecraft::polished_andesite:
    case minecraft::polished_basalt:
    case minecraft::polished_blackstone:
    case minecraft::polished_blackstone_brick_wall:
    case minecraft::polished_blackstone_bricks:
    case minecraft::polished_blackstone_wall:
    case minecraft::polished_deepslate:
    case minecraft::polished_deepslate_wall:
    case minecraft::polished_diorite:
    case minecraft::polished_granite:
    case minecraft::prismarine:
    case minecraft::prismarine_bricks:
    case minecraft::prismarine_wall:
    case minecraft::purple_concrete:
    case minecraft::purple_concrete_powder:
    case minecraft::purple_glazed_terracotta:
    case minecraft::purple_stained_glass:
    case minecraft::purple_stained_glass_pane:
    case minecraft::purple_terracotta:
    case minecraft::purple_wool:
    case minecraft::purpur_block:
    case minecraft::purpur_pillar:
    case minecraft::quartz_block:
    case minecraft::quartz_bricks:
    case minecraft::quartz_pillar:
    case minecraft::raw_copper_block:
    case minecraft::raw_gold_block:
    case minecraft::raw_iron_block:
    case minecraft::red_concrete:
    case minecraft::red_concrete_powder:
    case minecraft::red_glazed_terracotta:
    case minecraft::red_mushroom_block:
    case minecraft::red_nether_brick_wall:
    case minecraft::red_nether_bricks:
    case minecraft::red_sand:
    case minecraft::red_sandstone:
    case minecraft::red_sandstone_wall:
    case minecraft::red_stained_glass:
    case minecraft::red_stained_glass_pane:
    case minecraft::red_terracotta:
    case minecraft::red_wool:
    case minecraft::redstone_block:
    case minecraft::redstone_lamp:
    case minecraft::redstone_ore:
    case minecraft::repeating_command_block:
    case minecraft::respawn_anchor:
    case minecraft::rooted_dirt:
    case minecraft::sand:
    case minecraft::sandstone:
    case minecraft::sandstone_wall:
    case minecraft::sea_lantern:
    case minecraft::shroomlight:
    case minecraft::slime_block:
    case minecraft::smithing_table:
    case minecraft::smoker:
    case minecraft::smooth_basalt:
    case minecraft::smooth_quartz:
    case minecraft::smooth_red_sandstone:
    case minecraft::smooth_sandstone:
    case minecraft::smooth_stone:
    case minecraft::snow_block:
    case minecraft::soul_sand:
    case minecraft::soul_soil:
    case minecraft::spawner:
    case minecraft::sponge:
    case minecraft::spruce_log:
    case minecraft::spruce_planks:
    case minecraft::spruce_wood:
    case minecraft::stone:
    case minecraft::stone_brick_wall:
    case minecraft::stone_bricks:
    case minecraft::stripped_acacia_log:
    case minecraft::stripped_acacia_wood:
    case minecraft::stripped_birch_log:
    case minecraft::stripped_birch_wood:
    case minecraft::stripped_crimson_hyphae:
    case minecraft::stripped_crimson_stem:
    case minecraft::stripped_dark_oak_log:
    case minecraft::stripped_dark_oak_wood:
    case minecraft::stripped_jungle_log:
    case minecraft::stripped_jungle_wood:
    case minecraft::stripped_mangrove_log:
    case minecraft::stripped_mangrove_wood:
    case minecraft::stripped_oak_log:
    case minecraft::stripped_oak_wood:
    case minecraft::stripped_spruce_log:
    case minecraft::stripped_spruce_wood:
    case minecraft::stripped_warped_hyphae:
    case minecraft::stripped_warped_stem:
    case minecraft::structure_block:
    case minecraft::target:
    case minecraft::terracotta:
    case minecraft::tinted_glass:
    case minecraft::tnt:
    case minecraft::tube_coral_block:
    case minecraft::tuff:
    case minecraft::verdant_froglight:
    case minecraft::warped_hyphae:
    case minecraft::warped_nylium:
    case minecraft::warped_planks:
    case minecraft::warped_stem:
    case minecraft::warped_wart_block:
    case minecraft::waxed_copper_block:
    case minecraft::waxed_cut_copper:
    case minecraft::waxed_exposed_copper:
    case minecraft::waxed_exposed_cut_copper:
    case minecraft::waxed_oxidized_copper:
    case minecraft::waxed_oxidized_cut_copper:
    case minecraft::waxed_weathered_copper:
    case minecraft::waxed_weathered_cut_copper:
    case minecraft::weathered_copper:
    case minecraft::weathered_cut_copper:
    case minecraft::wet_sponge:
    case minecraft::white_concrete:
    case minecraft::white_concrete_powder:
    case minecraft::white_glazed_terracotta:
    case minecraft::white_stained_glass:
    case minecraft::white_stained_glass_pane:
    case minecraft::white_terracotta:
    case minecraft::white_wool:
    case minecraft::yellow_concrete:
    case minecraft::yellow_concrete_powder:
    case minecraft::yellow_glazed_terracotta:
    case minecraft::yellow_stained_glass:
    case minecraft::yellow_stained_glass_pane:
    case minecraft::yellow_terracotta:
    case minecraft::yellow_wool:
      return true;
    default:
      return false;
    }
  }

  static bool IsBlockAlwaysMakeWallTallShape(mcfile::blocks::BlockId id) {
    using namespace mcfile::blocks;
    switch (id) {
    case minecraft::acacia_leaves:
    case minecraft::acacia_log:
    case minecraft::acacia_planks:
    case minecraft::acacia_wood:
    case minecraft::amethyst_block:
    case minecraft::ancient_debris:
    case minecraft::andesite:
    case minecraft::azalea_leaves:
    case minecraft::barrel:
    case minecraft::barrier:
    case minecraft::basalt:
    case minecraft::beacon:
    case minecraft::bedrock:
    case minecraft::bee_nest:
    case minecraft::beehive:
    case minecraft::birch_leaves:
    case minecraft::birch_log:
    case minecraft::birch_planks:
    case minecraft::birch_wood:
    case minecraft::black_concrete:
    case minecraft::black_concrete_powder:
    case minecraft::black_glazed_terracotta:
    case minecraft::black_shulker_box:
    case minecraft::black_stained_glass:
    case minecraft::black_terracotta:
    case minecraft::black_wool:
    case minecraft::blackstone:
    case minecraft::blast_furnace:
    case minecraft::blue_concrete:
    case minecraft::blue_concrete_powder:
    case minecraft::blue_glazed_terracotta:
    case minecraft::blue_ice:
    case minecraft::blue_shulker_box:
    case minecraft::blue_stained_glass:
    case minecraft::blue_terracotta:
    case minecraft::blue_wool:
    case minecraft::bone_block:
    case minecraft::bookshelf:
    case minecraft::brain_coral_block:
    case minecraft::bricks:
    case minecraft::brown_concrete:
    case minecraft::brown_concrete_powder:
    case minecraft::brown_glazed_terracotta:
    case minecraft::brown_mushroom_block:
    case minecraft::brown_shulker_box:
    case minecraft::brown_stained_glass:
    case minecraft::brown_terracotta:
    case minecraft::brown_wool:
    case minecraft::bubble_coral_block:
    case minecraft::budding_amethyst:
    case minecraft::calcite:
    case minecraft::campfire:
    case minecraft::cartography_table:
    case minecraft::carved_pumpkin:
    case minecraft::chain_command_block:
    case minecraft::chiseled_deepslate:
    case minecraft::chiseled_nether_bricks:
    case minecraft::chiseled_polished_blackstone:
    case minecraft::chiseled_quartz_block:
    case minecraft::chiseled_red_sandstone:
    case minecraft::chiseled_sandstone:
    case minecraft::chiseled_stone_bricks:
    case minecraft::clay:
    case minecraft::coal_block:
    case minecraft::coal_ore:
    case minecraft::coarse_dirt:
    case minecraft::cobbled_deepslate:
    case minecraft::cobblestone:
    case minecraft::command_block:
    case minecraft::composter:
    case minecraft::copper_block:
    case minecraft::copper_ore:
    case minecraft::cracked_deepslate_bricks:
    case minecraft::cracked_deepslate_tiles:
    case minecraft::cracked_nether_bricks:
    case minecraft::cracked_polished_blackstone_bricks:
    case minecraft::cracked_stone_bricks:
    case minecraft::crafting_table:
    case minecraft::crimson_hyphae:
    case minecraft::crimson_nylium:
    case minecraft::crimson_planks:
    case minecraft::crimson_stem:
    case minecraft::crying_obsidian:
    case minecraft::cut_copper:
    case minecraft::cut_red_sandstone:
    case minecraft::cut_sandstone:
    case minecraft::cyan_concrete:
    case minecraft::cyan_concrete_powder:
    case minecraft::cyan_glazed_terracotta:
    case minecraft::cyan_shulker_box:
    case minecraft::cyan_stained_glass:
    case minecraft::cyan_terracotta:
    case minecraft::cyan_wool:
    case minecraft::dark_oak_leaves:
    case minecraft::dark_oak_log:
    case minecraft::dark_oak_planks:
    case minecraft::dark_oak_wood:
    case minecraft::dark_prismarine:
    case minecraft::daylight_detector:
    case minecraft::dead_brain_coral_block:
    case minecraft::dead_bubble_coral_block:
    case minecraft::dead_fire_coral_block:
    case minecraft::dead_horn_coral_block:
    case minecraft::dead_tube_coral_block:
    case minecraft::deepslate:
    case minecraft::deepslate_bricks:
    case minecraft::deepslate_coal_ore:
    case minecraft::deepslate_copper_ore:
    case minecraft::deepslate_diamond_ore:
    case minecraft::deepslate_emerald_ore:
    case minecraft::deepslate_gold_ore:
    case minecraft::deepslate_iron_ore:
    case minecraft::deepslate_lapis_ore:
    case minecraft::deepslate_redstone_ore:
    case minecraft::deepslate_tiles:
    case minecraft::diamond_block:
    case minecraft::diamond_ore:
    case minecraft::diorite:
    case minecraft::dirt:
    case minecraft::dirt_path:
    case minecraft::dispenser:
    case minecraft::dried_kelp_block:
    case minecraft::dripstone_block:
    case minecraft::dropper:
    case minecraft::emerald_block:
    case minecraft::emerald_ore:
    case minecraft::enchanting_table:
    case minecraft::end_portal_frame:
    case minecraft::end_stone:
    case minecraft::end_stone_bricks:
    case minecraft::exposed_copper:
    case minecraft::exposed_cut_copper:
    case minecraft::farmland:
    case minecraft::fire_coral_block:
    case minecraft::fletching_table:
    case minecraft::flowering_azalea_leaves:
    case minecraft::frosted_ice:
    case minecraft::furnace:
    case minecraft::gilded_blackstone:
    case minecraft::glass:
    case minecraft::glowstone:
    case minecraft::gold_block:
    case minecraft::gold_ore:
    case minecraft::granite:
    case minecraft::grass_block:
    case minecraft::gravel:
    case minecraft::gray_concrete:
    case minecraft::gray_concrete_powder:
    case minecraft::gray_glazed_terracotta:
    case minecraft::gray_shulker_box:
    case minecraft::gray_stained_glass:
    case minecraft::gray_terracotta:
    case minecraft::gray_wool:
    case minecraft::green_concrete:
    case minecraft::green_concrete_powder:
    case minecraft::green_glazed_terracotta:
    case minecraft::green_shulker_box:
    case minecraft::green_stained_glass:
    case minecraft::green_terracotta:
    case minecraft::green_wool:
    case minecraft::hay_block:
    case minecraft::honeycomb_block:
    case minecraft::horn_coral_block:
    case minecraft::ice:
    case minecraft::infested_chiseled_stone_bricks:
    case minecraft::infested_cobblestone:
    case minecraft::infested_cracked_stone_bricks:
    case minecraft::infested_deepslate:
    case minecraft::infested_mossy_stone_bricks:
    case minecraft::infested_stone:
    case minecraft::infested_stone_bricks:
    case minecraft::iron_block:
    case minecraft::iron_ore:
    case minecraft::jack_o_lantern:
    case minecraft::jigsaw:
    case minecraft::jukebox:
    case minecraft::jungle_leaves:
    case minecraft::jungle_log:
    case minecraft::jungle_planks:
    case minecraft::jungle_wood:
    case minecraft::lapis_block:
    case minecraft::lapis_ore:
    case minecraft::lectern:
    case minecraft::light_blue_concrete:
    case minecraft::light_blue_concrete_powder:
    case minecraft::light_blue_glazed_terracotta:
    case minecraft::light_blue_shulker_box:
    case minecraft::light_blue_stained_glass:
    case minecraft::light_blue_terracotta:
    case minecraft::light_blue_wool:
    case minecraft::light_gray_concrete:
    case minecraft::light_gray_concrete_powder:
    case minecraft::light_gray_glazed_terracotta:
    case minecraft::light_gray_shulker_box:
    case minecraft::light_gray_stained_glass:
    case minecraft::light_gray_terracotta:
    case minecraft::light_gray_wool:
    case minecraft::lime_concrete:
    case minecraft::lime_concrete_powder:
    case minecraft::lime_glazed_terracotta:
    case minecraft::lime_shulker_box:
    case minecraft::lime_stained_glass:
    case minecraft::lime_terracotta:
    case minecraft::lime_wool:
    case minecraft::lodestone:
    case minecraft::loom:
    case minecraft::magenta_concrete:
    case minecraft::magenta_concrete_powder:
    case minecraft::magenta_glazed_terracotta:
    case minecraft::magenta_shulker_box:
    case minecraft::magenta_stained_glass:
    case minecraft::magenta_terracotta:
    case minecraft::magenta_wool:
    case minecraft::magma_block:
    case minecraft::mangrove_leaves:
    case minecraft::mangrove_log:
    case minecraft::mangrove_planks:
    case minecraft::mangrove_roots:
    case minecraft::mangrove_wood:
    case minecraft::melon:
    case minecraft::moss_block:
    case minecraft::mossy_cobblestone:
    case minecraft::mossy_stone_bricks:
    case minecraft::mud:
    case minecraft::mud_bricks:
    case minecraft::muddy_mangrove_roots:
    case minecraft::mushroom_stem:
    case minecraft::mycelium:
    case minecraft::nether_bricks:
    case minecraft::nether_gold_ore:
    case minecraft::nether_quartz_ore:
    case minecraft::nether_wart_block:
    case minecraft::netherite_block:
    case minecraft::netherrack:
    case minecraft::note_block:
    case minecraft::oak_leaves:
    case minecraft::oak_log:
    case minecraft::oak_planks:
    case minecraft::oak_wood:
    case minecraft::observer:
    case minecraft::obsidian:
    case minecraft::ochre_froglight:
    case minecraft::orange_concrete:
    case minecraft::orange_concrete_powder:
    case minecraft::orange_glazed_terracotta:
    case minecraft::orange_shulker_box:
    case minecraft::orange_stained_glass:
    case minecraft::orange_terracotta:
    case minecraft::orange_wool:
    case minecraft::oxidized_copper:
    case minecraft::oxidized_cut_copper:
    case minecraft::packed_ice:
    case minecraft::packed_mud:
    case minecraft::pearlescent_froglight:
    case minecraft::pink_concrete:
    case minecraft::pink_concrete_powder:
    case minecraft::pink_glazed_terracotta:
    case minecraft::pink_shulker_box:
    case minecraft::pink_stained_glass:
    case minecraft::pink_terracotta:
    case minecraft::pink_wool:
    case minecraft::podzol:
    case minecraft::polished_andesite:
    case minecraft::polished_basalt:
    case minecraft::polished_blackstone:
    case minecraft::polished_blackstone_bricks:
    case minecraft::polished_deepslate:
    case minecraft::polished_diorite:
    case minecraft::polished_granite:
    case minecraft::prismarine:
    case minecraft::prismarine_bricks:
    case minecraft::pumpkin:
    case minecraft::purple_concrete:
    case minecraft::purple_concrete_powder:
    case minecraft::purple_glazed_terracotta:
    case minecraft::purple_shulker_box:
    case minecraft::purple_stained_glass:
    case minecraft::purple_terracotta:
    case minecraft::purple_wool:
    case minecraft::purpur_block:
    case minecraft::purpur_pillar:
    case minecraft::quartz_block:
    case minecraft::quartz_bricks:
    case minecraft::quartz_pillar:
    case minecraft::raw_copper_block:
    case minecraft::raw_gold_block:
    case minecraft::raw_iron_block:
    case minecraft::red_concrete:
    case minecraft::red_concrete_powder:
    case minecraft::red_glazed_terracotta:
    case minecraft::red_mushroom_block:
    case minecraft::red_nether_bricks:
    case minecraft::red_sand:
    case minecraft::red_sandstone:
    case minecraft::red_shulker_box:
    case minecraft::red_stained_glass:
    case minecraft::red_terracotta:
    case minecraft::red_wool:
    case minecraft::redstone_block:
    case minecraft::redstone_lamp:
    case minecraft::redstone_ore:
    case minecraft::repeating_command_block:
    case minecraft::respawn_anchor:
    case minecraft::rooted_dirt:
    case minecraft::sand:
    case minecraft::sandstone:
    case minecraft::sculk_sensor:
    case minecraft::sea_lantern:
    case minecraft::shroomlight:
    case minecraft::shulker_box:
    case minecraft::slime_block:
    case minecraft::smithing_table:
    case minecraft::smoker:
    case minecraft::smooth_basalt:
    case minecraft::smooth_quartz:
    case minecraft::smooth_red_sandstone:
    case minecraft::smooth_sandstone:
    case minecraft::smooth_stone:
    case minecraft::snow_block:
    case minecraft::soul_campfire:
    case minecraft::soul_sand:
    case minecraft::soul_soil:
    case minecraft::spawner:
    case minecraft::sponge:
    case minecraft::spruce_leaves:
    case minecraft::spruce_log:
    case minecraft::spruce_planks:
    case minecraft::spruce_wood:
    case minecraft::stone:
    case minecraft::stone_bricks:
    case minecraft::stonecutter:
    case minecraft::stripped_acacia_log:
    case minecraft::stripped_acacia_wood:
    case minecraft::stripped_birch_log:
    case minecraft::stripped_birch_wood:
    case minecraft::stripped_crimson_hyphae:
    case minecraft::stripped_crimson_stem:
    case minecraft::stripped_dark_oak_log:
    case minecraft::stripped_dark_oak_wood:
    case minecraft::stripped_jungle_log:
    case minecraft::stripped_jungle_wood:
    case minecraft::stripped_mangrove_log:
    case minecraft::stripped_mangrove_wood:
    case minecraft::stripped_oak_log:
    case minecraft::stripped_oak_wood:
    case minecraft::stripped_spruce_log:
    case minecraft::stripped_spruce_wood:
    case minecraft::stripped_warped_hyphae:
    case minecraft::stripped_warped_stem:
    case minecraft::structure_block:
    case minecraft::target:
    case minecraft::terracotta:
    case minecraft::tinted_glass:
    case minecraft::tnt:
    case minecraft::tube_coral_block:
    case minecraft::tuff:
    case minecraft::verdant_froglight:
    case minecraft::warped_hyphae:
    case minecraft::warped_nylium:
    case minecraft::warped_planks:
    case minecraft::warped_stem:
    case minecraft::warped_wart_block:
    case minecraft::waxed_copper_block:
    case minecraft::waxed_cut_copper:
    case minecraft::waxed_exposed_copper:
    case minecraft::waxed_exposed_cut_copper:
    case minecraft::waxed_oxidized_copper:
    case minecraft::waxed_oxidized_cut_copper:
    case minecraft::waxed_weathered_copper:
    case minecraft::waxed_weathered_cut_copper:
    case minecraft::weathered_copper:
    case minecraft::weathered_cut_copper:
    case minecraft::wet_sponge:
    case minecraft::white_concrete:
    case minecraft::white_concrete_powder:
    case minecraft::white_glazed_terracotta:
    case minecraft::white_shulker_box:
    case minecraft::white_stained_glass:
    case minecraft::white_terracotta:
    case minecraft::white_wool:
    case minecraft::yellow_concrete:
    case minecraft::yellow_concrete_powder:
    case minecraft::yellow_glazed_terracotta:
    case minecraft::yellow_shulker_box:
    case minecraft::yellow_stained_glass:
    case minecraft::yellow_terracotta:
    case minecraft::yellow_wool:
      return true;
    default:
      return false;
    }
  }
};

} // namespace je2be::terraform
