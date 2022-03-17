#pragma once

namespace je2be {

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
    switch (id) {
    case mcfile::blocks::minecraft::acacia_log:
    case mcfile::blocks::minecraft::acacia_planks:
    case mcfile::blocks::minecraft::acacia_wood:
    case mcfile::blocks::minecraft::amethyst_block:
    case mcfile::blocks::minecraft::ancient_debris:
    case mcfile::blocks::minecraft::andesite:
    case mcfile::blocks::minecraft::andesite_wall:
    case mcfile::blocks::minecraft::barrel:
    case mcfile::blocks::minecraft::basalt:
    case mcfile::blocks::minecraft::beacon:
    case mcfile::blocks::minecraft::bedrock:
    case mcfile::blocks::minecraft::bee_nest:
    case mcfile::blocks::minecraft::beehive:
    case mcfile::blocks::minecraft::birch_log:
    case mcfile::blocks::minecraft::birch_planks:
    case mcfile::blocks::minecraft::birch_wood:
    case mcfile::blocks::minecraft::black_concrete:
    case mcfile::blocks::minecraft::black_concrete_powder:
    case mcfile::blocks::minecraft::black_glazed_terracotta:
    case mcfile::blocks::minecraft::black_stained_glass:
    case mcfile::blocks::minecraft::black_stained_glass_pane:
    case mcfile::blocks::minecraft::black_terracotta:
    case mcfile::blocks::minecraft::black_wool:
    case mcfile::blocks::minecraft::blackstone:
    case mcfile::blocks::minecraft::blackstone_wall:
    case mcfile::blocks::minecraft::blast_furnace:
    case mcfile::blocks::minecraft::blue_concrete:
    case mcfile::blocks::minecraft::blue_concrete_powder:
    case mcfile::blocks::minecraft::blue_glazed_terracotta:
    case mcfile::blocks::minecraft::blue_ice:
    case mcfile::blocks::minecraft::blue_stained_glass:
    case mcfile::blocks::minecraft::blue_stained_glass_pane:
    case mcfile::blocks::minecraft::blue_terracotta:
    case mcfile::blocks::minecraft::blue_wool:
    case mcfile::blocks::minecraft::bone_block:
    case mcfile::blocks::minecraft::bookshelf:
    case mcfile::blocks::minecraft::brain_coral_block:
    case mcfile::blocks::minecraft::brick_wall:
    case mcfile::blocks::minecraft::bricks:
    case mcfile::blocks::minecraft::brown_concrete:
    case mcfile::blocks::minecraft::brown_concrete_powder:
    case mcfile::blocks::minecraft::brown_glazed_terracotta:
    case mcfile::blocks::minecraft::brown_mushroom_block:
    case mcfile::blocks::minecraft::brown_stained_glass:
    case mcfile::blocks::minecraft::brown_stained_glass_pane:
    case mcfile::blocks::minecraft::brown_terracotta:
    case mcfile::blocks::minecraft::brown_wool:
    case mcfile::blocks::minecraft::bubble_coral_block:
    case mcfile::blocks::minecraft::budding_amethyst:
    case mcfile::blocks::minecraft::calcite:
    case mcfile::blocks::minecraft::cartography_table:
    case mcfile::blocks::minecraft::chain_command_block:
    case mcfile::blocks::minecraft::chiseled_deepslate:
    case mcfile::blocks::minecraft::chiseled_nether_bricks:
    case mcfile::blocks::minecraft::chiseled_polished_blackstone:
    case mcfile::blocks::minecraft::chiseled_quartz_block:
    case mcfile::blocks::minecraft::chiseled_red_sandstone:
    case mcfile::blocks::minecraft::chiseled_sandstone:
    case mcfile::blocks::minecraft::chiseled_stone_bricks:
    case mcfile::blocks::minecraft::clay:
    case mcfile::blocks::minecraft::coal_block:
    case mcfile::blocks::minecraft::coal_ore:
    case mcfile::blocks::minecraft::coarse_dirt:
    case mcfile::blocks::minecraft::cobbled_deepslate:
    case mcfile::blocks::minecraft::cobbled_deepslate_wall:
    case mcfile::blocks::minecraft::cobblestone:
    case mcfile::blocks::minecraft::cobblestone_wall:
    case mcfile::blocks::minecraft::command_block:
    case mcfile::blocks::minecraft::composter:
    case mcfile::blocks::minecraft::copper_block:
    case mcfile::blocks::minecraft::copper_ore:
    case mcfile::blocks::minecraft::cracked_deepslate_bricks:
    case mcfile::blocks::minecraft::cracked_deepslate_tiles:
    case mcfile::blocks::minecraft::cracked_nether_bricks:
    case mcfile::blocks::minecraft::cracked_polished_blackstone_bricks:
    case mcfile::blocks::minecraft::cracked_stone_bricks:
    case mcfile::blocks::minecraft::crafting_table:
    case mcfile::blocks::minecraft::crimson_hyphae:
    case mcfile::blocks::minecraft::crimson_nylium:
    case mcfile::blocks::minecraft::crimson_planks:
    case mcfile::blocks::minecraft::crimson_stem:
    case mcfile::blocks::minecraft::crying_obsidian:
    case mcfile::blocks::minecraft::cut_copper:
    case mcfile::blocks::minecraft::cut_red_sandstone:
    case mcfile::blocks::minecraft::cut_sandstone:
    case mcfile::blocks::minecraft::cyan_concrete:
    case mcfile::blocks::minecraft::cyan_concrete_powder:
    case mcfile::blocks::minecraft::cyan_glazed_terracotta:
    case mcfile::blocks::minecraft::cyan_stained_glass:
    case mcfile::blocks::minecraft::cyan_stained_glass_pane:
    case mcfile::blocks::minecraft::cyan_terracotta:
    case mcfile::blocks::minecraft::cyan_wool:
    case mcfile::blocks::minecraft::dark_oak_log:
    case mcfile::blocks::minecraft::dark_oak_planks:
    case mcfile::blocks::minecraft::dark_oak_wood:
    case mcfile::blocks::minecraft::dark_prismarine:
    case mcfile::blocks::minecraft::dead_brain_coral_block:
    case mcfile::blocks::minecraft::dead_bubble_coral_block:
    case mcfile::blocks::minecraft::dead_fire_coral_block:
    case mcfile::blocks::minecraft::dead_horn_coral_block:
    case mcfile::blocks::minecraft::dead_tube_coral_block:
    case mcfile::blocks::minecraft::deepslate:
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
    case mcfile::blocks::minecraft::deepslate_tile_wall:
    case mcfile::blocks::minecraft::deepslate_tiles:
    case mcfile::blocks::minecraft::diamond_block:
    case mcfile::blocks::minecraft::diamond_ore:
    case mcfile::blocks::minecraft::diorite:
    case mcfile::blocks::minecraft::diorite_wall:
    case mcfile::blocks::minecraft::dirt:
    case mcfile::blocks::minecraft::dispenser:
    case mcfile::blocks::minecraft::dried_kelp_block:
    case mcfile::blocks::minecraft::dripstone_block:
    case mcfile::blocks::minecraft::dropper:
    case mcfile::blocks::minecraft::emerald_block:
    case mcfile::blocks::minecraft::emerald_ore:
    case mcfile::blocks::minecraft::end_stone:
    case mcfile::blocks::minecraft::end_stone_brick_wall:
    case mcfile::blocks::minecraft::end_stone_bricks:
    case mcfile::blocks::minecraft::exposed_copper:
    case mcfile::blocks::minecraft::exposed_cut_copper:
    case mcfile::blocks::minecraft::fire_coral_block:
    case mcfile::blocks::minecraft::fletching_table:
    case mcfile::blocks::minecraft::frosted_ice:
    case mcfile::blocks::minecraft::furnace:
    case mcfile::blocks::minecraft::gilded_blackstone:
    case mcfile::blocks::minecraft::glass:
    case mcfile::blocks::minecraft::glass_pane:
    case mcfile::blocks::minecraft::glowstone:
    case mcfile::blocks::minecraft::gold_block:
    case mcfile::blocks::minecraft::gold_ore:
    case mcfile::blocks::minecraft::granite:
    case mcfile::blocks::minecraft::granite_wall:
    case mcfile::blocks::minecraft::grass_block:
    case mcfile::blocks::minecraft::gravel:
    case mcfile::blocks::minecraft::gray_concrete:
    case mcfile::blocks::minecraft::gray_concrete_powder:
    case mcfile::blocks::minecraft::gray_glazed_terracotta:
    case mcfile::blocks::minecraft::gray_stained_glass:
    case mcfile::blocks::minecraft::gray_stained_glass_pane:
    case mcfile::blocks::minecraft::gray_terracotta:
    case mcfile::blocks::minecraft::gray_wool:
    case mcfile::blocks::minecraft::green_concrete:
    case mcfile::blocks::minecraft::green_concrete_powder:
    case mcfile::blocks::minecraft::green_glazed_terracotta:
    case mcfile::blocks::minecraft::green_stained_glass:
    case mcfile::blocks::minecraft::green_stained_glass_pane:
    case mcfile::blocks::minecraft::green_terracotta:
    case mcfile::blocks::minecraft::green_wool:
    case mcfile::blocks::minecraft::hay_block:
    case mcfile::blocks::minecraft::honeycomb_block:
    case mcfile::blocks::minecraft::horn_coral_block:
    case mcfile::blocks::minecraft::ice:
    case mcfile::blocks::minecraft::infested_chiseled_stone_bricks:
    case mcfile::blocks::minecraft::infested_cobblestone:
    case mcfile::blocks::minecraft::infested_cracked_stone_bricks:
    case mcfile::blocks::minecraft::infested_deepslate:
    case mcfile::blocks::minecraft::infested_mossy_stone_bricks:
    case mcfile::blocks::minecraft::infested_stone:
    case mcfile::blocks::minecraft::infested_stone_bricks:
    case mcfile::blocks::minecraft::iron_bars:
    case mcfile::blocks::minecraft::iron_block:
    case mcfile::blocks::minecraft::iron_ore:
    case mcfile::blocks::minecraft::jigsaw:
    case mcfile::blocks::minecraft::jukebox:
    case mcfile::blocks::minecraft::jungle_log:
    case mcfile::blocks::minecraft::jungle_planks:
    case mcfile::blocks::minecraft::jungle_wood:
    case mcfile::blocks::minecraft::lapis_block:
    case mcfile::blocks::minecraft::lapis_ore:
    case mcfile::blocks::minecraft::light_blue_concrete:
    case mcfile::blocks::minecraft::light_blue_concrete_powder:
    case mcfile::blocks::minecraft::light_blue_glazed_terracotta:
    case mcfile::blocks::minecraft::light_blue_stained_glass:
    case mcfile::blocks::minecraft::light_blue_stained_glass_pane:
    case mcfile::blocks::minecraft::light_blue_terracotta:
    case mcfile::blocks::minecraft::light_blue_wool:
    case mcfile::blocks::minecraft::light_gray_concrete:
    case mcfile::blocks::minecraft::light_gray_concrete_powder:
    case mcfile::blocks::minecraft::light_gray_glazed_terracotta:
    case mcfile::blocks::minecraft::light_gray_stained_glass:
    case mcfile::blocks::minecraft::light_gray_stained_glass_pane:
    case mcfile::blocks::minecraft::light_gray_terracotta:
    case mcfile::blocks::minecraft::light_gray_wool:
    case mcfile::blocks::minecraft::lime_concrete:
    case mcfile::blocks::minecraft::lime_concrete_powder:
    case mcfile::blocks::minecraft::lime_glazed_terracotta:
    case mcfile::blocks::minecraft::lime_stained_glass:
    case mcfile::blocks::minecraft::lime_stained_glass_pane:
    case mcfile::blocks::minecraft::lime_terracotta:
    case mcfile::blocks::minecraft::lime_wool:
    case mcfile::blocks::minecraft::lodestone:
    case mcfile::blocks::minecraft::loom:
    case mcfile::blocks::minecraft::magenta_concrete:
    case mcfile::blocks::minecraft::magenta_concrete_powder:
    case mcfile::blocks::minecraft::magenta_glazed_terracotta:
    case mcfile::blocks::minecraft::magenta_stained_glass:
    case mcfile::blocks::minecraft::magenta_stained_glass_pane:
    case mcfile::blocks::minecraft::magenta_terracotta:
    case mcfile::blocks::minecraft::magenta_wool:
    case mcfile::blocks::minecraft::magma_block:
    case mcfile::blocks::minecraft::moss_block:
    case mcfile::blocks::minecraft::mossy_cobblestone:
    case mcfile::blocks::minecraft::mossy_cobblestone_wall:
    case mcfile::blocks::minecraft::mossy_stone_brick_wall:
    case mcfile::blocks::minecraft::mossy_stone_bricks:
    case mcfile::blocks::minecraft::mushroom_stem:
    case mcfile::blocks::minecraft::mycelium:
    case mcfile::blocks::minecraft::nether_brick_wall:
    case mcfile::blocks::minecraft::nether_bricks:
    case mcfile::blocks::minecraft::nether_gold_ore:
    case mcfile::blocks::minecraft::nether_quartz_ore:
    case mcfile::blocks::minecraft::nether_wart_block:
    case mcfile::blocks::minecraft::netherite_block:
    case mcfile::blocks::minecraft::netherrack:
    case mcfile::blocks::minecraft::note_block:
    case mcfile::blocks::minecraft::oak_log:
    case mcfile::blocks::minecraft::oak_planks:
    case mcfile::blocks::minecraft::oak_wood:
    case mcfile::blocks::minecraft::observer:
    case mcfile::blocks::minecraft::obsidian:
    case mcfile::blocks::minecraft::orange_concrete:
    case mcfile::blocks::minecraft::orange_concrete_powder:
    case mcfile::blocks::minecraft::orange_glazed_terracotta:
    case mcfile::blocks::minecraft::orange_stained_glass:
    case mcfile::blocks::minecraft::orange_stained_glass_pane:
    case mcfile::blocks::minecraft::orange_terracotta:
    case mcfile::blocks::minecraft::orange_wool:
    case mcfile::blocks::minecraft::oxidized_copper:
    case mcfile::blocks::minecraft::oxidized_cut_copper:
    case mcfile::blocks::minecraft::packed_ice:
    case mcfile::blocks::minecraft::pink_concrete:
    case mcfile::blocks::minecraft::pink_concrete_powder:
    case mcfile::blocks::minecraft::pink_glazed_terracotta:
    case mcfile::blocks::minecraft::pink_stained_glass:
    case mcfile::blocks::minecraft::pink_stained_glass_pane:
    case mcfile::blocks::minecraft::pink_terracotta:
    case mcfile::blocks::minecraft::pink_wool:
    case mcfile::blocks::minecraft::podzol:
    case mcfile::blocks::minecraft::polished_andesite:
    case mcfile::blocks::minecraft::polished_basalt:
    case mcfile::blocks::minecraft::polished_blackstone:
    case mcfile::blocks::minecraft::polished_blackstone_brick_wall:
    case mcfile::blocks::minecraft::polished_blackstone_bricks:
    case mcfile::blocks::minecraft::polished_blackstone_wall:
    case mcfile::blocks::minecraft::polished_deepslate:
    case mcfile::blocks::minecraft::polished_deepslate_wall:
    case mcfile::blocks::minecraft::polished_diorite:
    case mcfile::blocks::minecraft::polished_granite:
    case mcfile::blocks::minecraft::prismarine:
    case mcfile::blocks::minecraft::prismarine_bricks:
    case mcfile::blocks::minecraft::prismarine_wall:
    case mcfile::blocks::minecraft::purple_concrete:
    case mcfile::blocks::minecraft::purple_concrete_powder:
    case mcfile::blocks::minecraft::purple_glazed_terracotta:
    case mcfile::blocks::minecraft::purple_stained_glass:
    case mcfile::blocks::minecraft::purple_stained_glass_pane:
    case mcfile::blocks::minecraft::purple_terracotta:
    case mcfile::blocks::minecraft::purple_wool:
    case mcfile::blocks::minecraft::purpur_block:
    case mcfile::blocks::minecraft::purpur_pillar:
    case mcfile::blocks::minecraft::quartz_block:
    case mcfile::blocks::minecraft::quartz_bricks:
    case mcfile::blocks::minecraft::quartz_pillar:
    case mcfile::blocks::minecraft::raw_copper_block:
    case mcfile::blocks::minecraft::raw_gold_block:
    case mcfile::blocks::minecraft::raw_iron_block:
    case mcfile::blocks::minecraft::red_concrete:
    case mcfile::blocks::minecraft::red_concrete_powder:
    case mcfile::blocks::minecraft::red_glazed_terracotta:
    case mcfile::blocks::minecraft::red_mushroom_block:
    case mcfile::blocks::minecraft::red_nether_brick_wall:
    case mcfile::blocks::minecraft::red_nether_bricks:
    case mcfile::blocks::minecraft::red_sand:
    case mcfile::blocks::minecraft::red_sandstone:
    case mcfile::blocks::minecraft::red_sandstone_wall:
    case mcfile::blocks::minecraft::red_stained_glass:
    case mcfile::blocks::minecraft::red_stained_glass_pane:
    case mcfile::blocks::minecraft::red_terracotta:
    case mcfile::blocks::minecraft::red_wool:
    case mcfile::blocks::minecraft::redstone_block:
    case mcfile::blocks::minecraft::redstone_lamp:
    case mcfile::blocks::minecraft::redstone_ore:
    case mcfile::blocks::minecraft::repeating_command_block:
    case mcfile::blocks::minecraft::respawn_anchor:
    case mcfile::blocks::minecraft::rooted_dirt:
    case mcfile::blocks::minecraft::sand:
    case mcfile::blocks::minecraft::sandstone:
    case mcfile::blocks::minecraft::sandstone_wall:
    case mcfile::blocks::minecraft::sea_lantern:
    case mcfile::blocks::minecraft::shroomlight:
    case mcfile::blocks::minecraft::slime_block:
    case mcfile::blocks::minecraft::smithing_table:
    case mcfile::blocks::minecraft::smoker:
    case mcfile::blocks::minecraft::smooth_basalt:
    case mcfile::blocks::minecraft::smooth_quartz:
    case mcfile::blocks::minecraft::smooth_red_sandstone:
    case mcfile::blocks::minecraft::smooth_sandstone:
    case mcfile::blocks::minecraft::smooth_stone:
    case mcfile::blocks::minecraft::snow_block:
    case mcfile::blocks::minecraft::soul_sand:
    case mcfile::blocks::minecraft::soul_soil:
    case mcfile::blocks::minecraft::spawner:
    case mcfile::blocks::minecraft::sponge:
    case mcfile::blocks::minecraft::spruce_log:
    case mcfile::blocks::minecraft::spruce_planks:
    case mcfile::blocks::minecraft::spruce_wood:
    case mcfile::blocks::minecraft::stone:
    case mcfile::blocks::minecraft::stone_brick_wall:
    case mcfile::blocks::minecraft::stone_bricks:
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
    case mcfile::blocks::minecraft::structure_block:
    case mcfile::blocks::minecraft::target:
    case mcfile::blocks::minecraft::terracotta:
    case mcfile::blocks::minecraft::tinted_glass:
    case mcfile::blocks::minecraft::tnt:
    case mcfile::blocks::minecraft::tube_coral_block:
    case mcfile::blocks::minecraft::tuff:
    case mcfile::blocks::minecraft::warped_hyphae:
    case mcfile::blocks::minecraft::warped_nylium:
    case mcfile::blocks::minecraft::warped_planks:
    case mcfile::blocks::minecraft::warped_stem:
    case mcfile::blocks::minecraft::warped_wart_block:
    case mcfile::blocks::minecraft::waxed_copper_block:
    case mcfile::blocks::minecraft::waxed_cut_copper:
    case mcfile::blocks::minecraft::waxed_exposed_copper:
    case mcfile::blocks::minecraft::waxed_exposed_cut_copper:
    case mcfile::blocks::minecraft::waxed_oxidized_copper:
    case mcfile::blocks::minecraft::waxed_oxidized_cut_copper:
    case mcfile::blocks::minecraft::waxed_weathered_copper:
    case mcfile::blocks::minecraft::waxed_weathered_cut_copper:
    case mcfile::blocks::minecraft::weathered_copper:
    case mcfile::blocks::minecraft::weathered_cut_copper:
    case mcfile::blocks::minecraft::wet_sponge:
    case mcfile::blocks::minecraft::white_concrete:
    case mcfile::blocks::minecraft::white_concrete_powder:
    case mcfile::blocks::minecraft::white_glazed_terracotta:
    case mcfile::blocks::minecraft::white_stained_glass:
    case mcfile::blocks::minecraft::white_stained_glass_pane:
    case mcfile::blocks::minecraft::white_terracotta:
    case mcfile::blocks::minecraft::white_wool:
    case mcfile::blocks::minecraft::yellow_concrete:
    case mcfile::blocks::minecraft::yellow_concrete_powder:
    case mcfile::blocks::minecraft::yellow_glazed_terracotta:
    case mcfile::blocks::minecraft::yellow_stained_glass:
    case mcfile::blocks::minecraft::yellow_stained_glass_pane:
    case mcfile::blocks::minecraft::yellow_terracotta:
    case mcfile::blocks::minecraft::yellow_wool:
      return true;
    default:
      return false;
    }
  }

  static bool IsBlockAlwaysMakeWallTallShape(mcfile::blocks::BlockId id) {
    switch (id) {
    case mcfile::blocks::minecraft::acacia_leaves:
    case mcfile::blocks::minecraft::acacia_log:
    case mcfile::blocks::minecraft::acacia_planks:
    case mcfile::blocks::minecraft::acacia_wood:
    case mcfile::blocks::minecraft::amethyst_block:
    case mcfile::blocks::minecraft::ancient_debris:
    case mcfile::blocks::minecraft::andesite:
    case mcfile::blocks::minecraft::azalea_leaves:
    case mcfile::blocks::minecraft::barrel:
    case mcfile::blocks::minecraft::barrier:
    case mcfile::blocks::minecraft::basalt:
    case mcfile::blocks::minecraft::beacon:
    case mcfile::blocks::minecraft::bedrock:
    case mcfile::blocks::minecraft::bee_nest:
    case mcfile::blocks::minecraft::beehive:
    case mcfile::blocks::minecraft::birch_leaves:
    case mcfile::blocks::minecraft::birch_log:
    case mcfile::blocks::minecraft::birch_planks:
    case mcfile::blocks::minecraft::birch_wood:
    case mcfile::blocks::minecraft::black_concrete:
    case mcfile::blocks::minecraft::black_concrete_powder:
    case mcfile::blocks::minecraft::black_glazed_terracotta:
    case mcfile::blocks::minecraft::black_shulker_box:
    case mcfile::blocks::minecraft::black_stained_glass:
    case mcfile::blocks::minecraft::black_terracotta:
    case mcfile::blocks::minecraft::black_wool:
    case mcfile::blocks::minecraft::blackstone:
    case mcfile::blocks::minecraft::blast_furnace:
    case mcfile::blocks::minecraft::blue_concrete:
    case mcfile::blocks::minecraft::blue_concrete_powder:
    case mcfile::blocks::minecraft::blue_glazed_terracotta:
    case mcfile::blocks::minecraft::blue_ice:
    case mcfile::blocks::minecraft::blue_shulker_box:
    case mcfile::blocks::minecraft::blue_stained_glass:
    case mcfile::blocks::minecraft::blue_terracotta:
    case mcfile::blocks::minecraft::blue_wool:
    case mcfile::blocks::minecraft::bone_block:
    case mcfile::blocks::minecraft::bookshelf:
    case mcfile::blocks::minecraft::brain_coral_block:
    case mcfile::blocks::minecraft::bricks:
    case mcfile::blocks::minecraft::brown_concrete:
    case mcfile::blocks::minecraft::brown_concrete_powder:
    case mcfile::blocks::minecraft::brown_glazed_terracotta:
    case mcfile::blocks::minecraft::brown_mushroom_block:
    case mcfile::blocks::minecraft::brown_shulker_box:
    case mcfile::blocks::minecraft::brown_stained_glass:
    case mcfile::blocks::minecraft::brown_terracotta:
    case mcfile::blocks::minecraft::brown_wool:
    case mcfile::blocks::minecraft::bubble_coral_block:
    case mcfile::blocks::minecraft::budding_amethyst:
    case mcfile::blocks::minecraft::calcite:
    case mcfile::blocks::minecraft::campfire:
    case mcfile::blocks::minecraft::cartography_table:
    case mcfile::blocks::minecraft::carved_pumpkin:
    case mcfile::blocks::minecraft::chain_command_block:
    case mcfile::blocks::minecraft::chiseled_deepslate:
    case mcfile::blocks::minecraft::chiseled_nether_bricks:
    case mcfile::blocks::minecraft::chiseled_polished_blackstone:
    case mcfile::blocks::minecraft::chiseled_quartz_block:
    case mcfile::blocks::minecraft::chiseled_red_sandstone:
    case mcfile::blocks::minecraft::chiseled_sandstone:
    case mcfile::blocks::minecraft::chiseled_stone_bricks:
    case mcfile::blocks::minecraft::clay:
    case mcfile::blocks::minecraft::coal_block:
    case mcfile::blocks::minecraft::coal_ore:
    case mcfile::blocks::minecraft::coarse_dirt:
    case mcfile::blocks::minecraft::cobbled_deepslate:
    case mcfile::blocks::minecraft::cobblestone:
    case mcfile::blocks::minecraft::command_block:
    case mcfile::blocks::minecraft::composter:
    case mcfile::blocks::minecraft::copper_block:
    case mcfile::blocks::minecraft::copper_ore:
    case mcfile::blocks::minecraft::cracked_deepslate_bricks:
    case mcfile::blocks::minecraft::cracked_deepslate_tiles:
    case mcfile::blocks::minecraft::cracked_nether_bricks:
    case mcfile::blocks::minecraft::cracked_polished_blackstone_bricks:
    case mcfile::blocks::minecraft::cracked_stone_bricks:
    case mcfile::blocks::minecraft::crafting_table:
    case mcfile::blocks::minecraft::crimson_hyphae:
    case mcfile::blocks::minecraft::crimson_nylium:
    case mcfile::blocks::minecraft::crimson_planks:
    case mcfile::blocks::minecraft::crimson_stem:
    case mcfile::blocks::minecraft::crying_obsidian:
    case mcfile::blocks::minecraft::cut_copper:
    case mcfile::blocks::minecraft::cut_red_sandstone:
    case mcfile::blocks::minecraft::cut_sandstone:
    case mcfile::blocks::minecraft::cyan_concrete:
    case mcfile::blocks::minecraft::cyan_concrete_powder:
    case mcfile::blocks::minecraft::cyan_glazed_terracotta:
    case mcfile::blocks::minecraft::cyan_shulker_box:
    case mcfile::blocks::minecraft::cyan_stained_glass:
    case mcfile::blocks::minecraft::cyan_terracotta:
    case mcfile::blocks::minecraft::cyan_wool:
    case mcfile::blocks::minecraft::dark_oak_leaves:
    case mcfile::blocks::minecraft::dark_oak_log:
    case mcfile::blocks::minecraft::dark_oak_planks:
    case mcfile::blocks::minecraft::dark_oak_wood:
    case mcfile::blocks::minecraft::dark_prismarine:
    case mcfile::blocks::minecraft::daylight_detector:
    case mcfile::blocks::minecraft::dead_brain_coral_block:
    case mcfile::blocks::minecraft::dead_bubble_coral_block:
    case mcfile::blocks::minecraft::dead_fire_coral_block:
    case mcfile::blocks::minecraft::dead_horn_coral_block:
    case mcfile::blocks::minecraft::dead_tube_coral_block:
    case mcfile::blocks::minecraft::deepslate:
    case mcfile::blocks::minecraft::deepslate_bricks:
    case mcfile::blocks::minecraft::deepslate_coal_ore:
    case mcfile::blocks::minecraft::deepslate_copper_ore:
    case mcfile::blocks::minecraft::deepslate_diamond_ore:
    case mcfile::blocks::minecraft::deepslate_emerald_ore:
    case mcfile::blocks::minecraft::deepslate_gold_ore:
    case mcfile::blocks::minecraft::deepslate_iron_ore:
    case mcfile::blocks::minecraft::deepslate_lapis_ore:
    case mcfile::blocks::minecraft::deepslate_redstone_ore:
    case mcfile::blocks::minecraft::deepslate_tiles:
    case mcfile::blocks::minecraft::diamond_block:
    case mcfile::blocks::minecraft::diamond_ore:
    case mcfile::blocks::minecraft::diorite:
    case mcfile::blocks::minecraft::dirt_path:
    case mcfile::blocks::minecraft::dispenser:
    case mcfile::blocks::minecraft::dried_kelp_block:
    case mcfile::blocks::minecraft::dripstone_block:
    case mcfile::blocks::minecraft::dropper:
    case mcfile::blocks::minecraft::emerald_block:
    case mcfile::blocks::minecraft::emerald_ore:
    case mcfile::blocks::minecraft::enchanting_table:
    case mcfile::blocks::minecraft::end_portal_frame:
    case mcfile::blocks::minecraft::end_stone:
    case mcfile::blocks::minecraft::end_stone_bricks:
    case mcfile::blocks::minecraft::exposed_copper:
    case mcfile::blocks::minecraft::exposed_cut_copper:
    case mcfile::blocks::minecraft::farmland:
    case mcfile::blocks::minecraft::fire_coral_block:
    case mcfile::blocks::minecraft::fletching_table:
    case mcfile::blocks::minecraft::furnace:
    case mcfile::blocks::minecraft::gilded_blackstone:
    case mcfile::blocks::minecraft::glass:
    case mcfile::blocks::minecraft::glowstone:
    case mcfile::blocks::minecraft::gold_block:
    case mcfile::blocks::minecraft::gold_ore:
    case mcfile::blocks::minecraft::granite:
    case mcfile::blocks::minecraft::grass_block:
    case mcfile::blocks::minecraft::gravel:
    case mcfile::blocks::minecraft::gray_concrete:
    case mcfile::blocks::minecraft::gray_concrete_powder:
    case mcfile::blocks::minecraft::gray_glazed_terracotta:
    case mcfile::blocks::minecraft::gray_shulker_box:
    case mcfile::blocks::minecraft::gray_stained_glass:
    case mcfile::blocks::minecraft::gray_terracotta:
    case mcfile::blocks::minecraft::gray_wool:
    case mcfile::blocks::minecraft::green_concrete:
    case mcfile::blocks::minecraft::green_concrete_powder:
    case mcfile::blocks::minecraft::green_glazed_terracotta:
    case mcfile::blocks::minecraft::green_shulker_box:
    case mcfile::blocks::minecraft::green_stained_glass:
    case mcfile::blocks::minecraft::green_terracotta:
    case mcfile::blocks::minecraft::green_wool:
    case mcfile::blocks::minecraft::hay_block:
    case mcfile::blocks::minecraft::honeycomb_block:
    case mcfile::blocks::minecraft::horn_coral_block:
    case mcfile::blocks::minecraft::ice:
    case mcfile::blocks::minecraft::infested_chiseled_stone_bricks:
    case mcfile::blocks::minecraft::infested_cobblestone:
    case mcfile::blocks::minecraft::infested_cracked_stone_bricks:
    case mcfile::blocks::minecraft::infested_deepslate:
    case mcfile::blocks::minecraft::infested_mossy_stone_bricks:
    case mcfile::blocks::minecraft::infested_stone:
    case mcfile::blocks::minecraft::infested_stone_bricks:
    case mcfile::blocks::minecraft::iron_block:
    case mcfile::blocks::minecraft::iron_ore:
    case mcfile::blocks::minecraft::jack_o_lantern:
    case mcfile::blocks::minecraft::jigsaw:
    case mcfile::blocks::minecraft::jukebox:
    case mcfile::blocks::minecraft::jungle_leaves:
    case mcfile::blocks::minecraft::jungle_log:
    case mcfile::blocks::minecraft::jungle_planks:
    case mcfile::blocks::minecraft::jungle_wood:
    case mcfile::blocks::minecraft::lapis_block:
    case mcfile::blocks::minecraft::lapis_ore:
    case mcfile::blocks::minecraft::lectern:
    case mcfile::blocks::minecraft::light_blue_concrete:
    case mcfile::blocks::minecraft::light_blue_concrete_powder:
    case mcfile::blocks::minecraft::light_blue_glazed_terracotta:
    case mcfile::blocks::minecraft::light_blue_shulker_box:
    case mcfile::blocks::minecraft::light_blue_stained_glass:
    case mcfile::blocks::minecraft::light_blue_terracotta:
    case mcfile::blocks::minecraft::light_blue_wool:
    case mcfile::blocks::minecraft::light_gray_concrete:
    case mcfile::blocks::minecraft::light_gray_concrete_powder:
    case mcfile::blocks::minecraft::light_gray_glazed_terracotta:
    case mcfile::blocks::minecraft::light_gray_shulker_box:
    case mcfile::blocks::minecraft::light_gray_stained_glass:
    case mcfile::blocks::minecraft::light_gray_terracotta:
    case mcfile::blocks::minecraft::light_gray_wool:
    case mcfile::blocks::minecraft::lime_concrete:
    case mcfile::blocks::minecraft::lime_concrete_powder:
    case mcfile::blocks::minecraft::lime_glazed_terracotta:
    case mcfile::blocks::minecraft::lime_shulker_box:
    case mcfile::blocks::minecraft::lime_stained_glass:
    case mcfile::blocks::minecraft::lime_terracotta:
    case mcfile::blocks::minecraft::lime_wool:
    case mcfile::blocks::minecraft::lodestone:
    case mcfile::blocks::minecraft::loom:
    case mcfile::blocks::minecraft::magenta_concrete:
    case mcfile::blocks::minecraft::magenta_concrete_powder:
    case mcfile::blocks::minecraft::magenta_glazed_terracotta:
    case mcfile::blocks::minecraft::magenta_shulker_box:
    case mcfile::blocks::minecraft::magenta_stained_glass:
    case mcfile::blocks::minecraft::magenta_terracotta:
    case mcfile::blocks::minecraft::magenta_wool:
    case mcfile::blocks::minecraft::magma_block:
    case mcfile::blocks::minecraft::melon:
    case mcfile::blocks::minecraft::moss_block:
    case mcfile::blocks::minecraft::mossy_cobblestone:
    case mcfile::blocks::minecraft::mossy_stone_bricks:
    case mcfile::blocks::minecraft::mushroom_stem:
    case mcfile::blocks::minecraft::mycelium:
    case mcfile::blocks::minecraft::nether_bricks:
    case mcfile::blocks::minecraft::nether_gold_ore:
    case mcfile::blocks::minecraft::nether_quartz_ore:
    case mcfile::blocks::minecraft::nether_wart_block:
    case mcfile::blocks::minecraft::netherite_block:
    case mcfile::blocks::minecraft::netherrack:
    case mcfile::blocks::minecraft::note_block:
    case mcfile::blocks::minecraft::oak_leaves:
    case mcfile::blocks::minecraft::oak_log:
    case mcfile::blocks::minecraft::oak_planks:
    case mcfile::blocks::minecraft::oak_wood:
    case mcfile::blocks::minecraft::observer:
    case mcfile::blocks::minecraft::obsidian:
    case mcfile::blocks::minecraft::orange_concrete:
    case mcfile::blocks::minecraft::orange_concrete_powder:
    case mcfile::blocks::minecraft::orange_glazed_terracotta:
    case mcfile::blocks::minecraft::orange_shulker_box:
    case mcfile::blocks::minecraft::orange_stained_glass:
    case mcfile::blocks::minecraft::orange_terracotta:
    case mcfile::blocks::minecraft::orange_wool:
    case mcfile::blocks::minecraft::oxidized_copper:
    case mcfile::blocks::minecraft::oxidized_cut_copper:
    case mcfile::blocks::minecraft::packed_ice:
    case mcfile::blocks::minecraft::pink_concrete:
    case mcfile::blocks::minecraft::pink_concrete_powder:
    case mcfile::blocks::minecraft::pink_glazed_terracotta:
    case mcfile::blocks::minecraft::pink_shulker_box:
    case mcfile::blocks::minecraft::pink_stained_glass:
    case mcfile::blocks::minecraft::pink_terracotta:
    case mcfile::blocks::minecraft::pink_wool:
    case mcfile::blocks::minecraft::podzol:
    case mcfile::blocks::minecraft::polished_andesite:
    case mcfile::blocks::minecraft::polished_basalt:
    case mcfile::blocks::minecraft::polished_blackstone:
    case mcfile::blocks::minecraft::polished_blackstone_bricks:
    case mcfile::blocks::minecraft::polished_deepslate:
    case mcfile::blocks::minecraft::polished_diorite:
    case mcfile::blocks::minecraft::polished_granite:
    case mcfile::blocks::minecraft::prismarine:
    case mcfile::blocks::minecraft::prismarine_bricks:
    case mcfile::blocks::minecraft::pumpkin:
    case mcfile::blocks::minecraft::purple_concrete:
    case mcfile::blocks::minecraft::purple_concrete_powder:
    case mcfile::blocks::minecraft::purple_glazed_terracotta:
    case mcfile::blocks::minecraft::purple_shulker_box:
    case mcfile::blocks::minecraft::purple_stained_glass:
    case mcfile::blocks::minecraft::purple_terracotta:
    case mcfile::blocks::minecraft::purple_wool:
    case mcfile::blocks::minecraft::purpur_block:
    case mcfile::blocks::minecraft::purpur_pillar:
    case mcfile::blocks::minecraft::quartz_block:
    case mcfile::blocks::minecraft::quartz_bricks:
    case mcfile::blocks::minecraft::quartz_pillar:
    case mcfile::blocks::minecraft::raw_copper_block:
    case mcfile::blocks::minecraft::raw_gold_block:
    case mcfile::blocks::minecraft::raw_iron_block:
    case mcfile::blocks::minecraft::red_concrete:
    case mcfile::blocks::minecraft::red_concrete_powder:
    case mcfile::blocks::minecraft::red_glazed_terracotta:
    case mcfile::blocks::minecraft::red_mushroom_block:
    case mcfile::blocks::minecraft::red_nether_bricks:
    case mcfile::blocks::minecraft::red_sand:
    case mcfile::blocks::minecraft::red_sandstone:
    case mcfile::blocks::minecraft::red_shulker_box:
    case mcfile::blocks::minecraft::red_stained_glass:
    case mcfile::blocks::minecraft::red_terracotta:
    case mcfile::blocks::minecraft::red_wool:
    case mcfile::blocks::minecraft::redstone_block:
    case mcfile::blocks::minecraft::redstone_lamp:
    case mcfile::blocks::minecraft::redstone_ore:
    case mcfile::blocks::minecraft::repeating_command_block:
    case mcfile::blocks::minecraft::respawn_anchor:
    case mcfile::blocks::minecraft::rooted_dirt:
    case mcfile::blocks::minecraft::sand:
    case mcfile::blocks::minecraft::sandstone:
    case mcfile::blocks::minecraft::sculk_sensor:
    case mcfile::blocks::minecraft::sea_lantern:
    case mcfile::blocks::minecraft::shroomlight:
    case mcfile::blocks::minecraft::shulker_box:
    case mcfile::blocks::minecraft::slime_block:
    case mcfile::blocks::minecraft::smithing_table:
    case mcfile::blocks::minecraft::smoker:
    case mcfile::blocks::minecraft::smooth_basalt:
    case mcfile::blocks::minecraft::smooth_quartz:
    case mcfile::blocks::minecraft::smooth_red_sandstone:
    case mcfile::blocks::minecraft::smooth_sandstone:
    case mcfile::blocks::minecraft::smooth_stone:
    case mcfile::blocks::minecraft::snow_block:
    case mcfile::blocks::minecraft::soul_campfire:
    case mcfile::blocks::minecraft::soul_sand:
    case mcfile::blocks::minecraft::soul_soil:
    case mcfile::blocks::minecraft::spawner:
    case mcfile::blocks::minecraft::sponge:
    case mcfile::blocks::minecraft::spruce_leaves:
    case mcfile::blocks::minecraft::spruce_log:
    case mcfile::blocks::minecraft::spruce_planks:
    case mcfile::blocks::minecraft::spruce_wood:
    case mcfile::blocks::minecraft::stone:
    case mcfile::blocks::minecraft::stone_bricks:
    case mcfile::blocks::minecraft::stonecutter:
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
    case mcfile::blocks::minecraft::structure_block:
    case mcfile::blocks::minecraft::target:
    case mcfile::blocks::minecraft::terracotta:
    case mcfile::blocks::minecraft::tinted_glass:
    case mcfile::blocks::minecraft::tnt:
    case mcfile::blocks::minecraft::tube_coral_block:
    case mcfile::blocks::minecraft::tuff:
    case mcfile::blocks::minecraft::warped_hyphae:
    case mcfile::blocks::minecraft::warped_nylium:
    case mcfile::blocks::minecraft::warped_planks:
    case mcfile::blocks::minecraft::warped_stem:
    case mcfile::blocks::minecraft::warped_wart_block:
    case mcfile::blocks::minecraft::waxed_copper_block:
    case mcfile::blocks::minecraft::waxed_cut_copper:
    case mcfile::blocks::minecraft::waxed_exposed_copper:
    case mcfile::blocks::minecraft::waxed_exposed_cut_copper:
    case mcfile::blocks::minecraft::waxed_oxidized_copper:
    case mcfile::blocks::minecraft::waxed_oxidized_cut_copper:
    case mcfile::blocks::minecraft::waxed_weathered_copper:
    case mcfile::blocks::minecraft::waxed_weathered_cut_copper:
    case mcfile::blocks::minecraft::weathered_copper:
    case mcfile::blocks::minecraft::weathered_cut_copper:
    case mcfile::blocks::minecraft::wet_sponge:
    case mcfile::blocks::minecraft::white_concrete:
    case mcfile::blocks::minecraft::white_concrete_powder:
    case mcfile::blocks::minecraft::white_glazed_terracotta:
    case mcfile::blocks::minecraft::white_shulker_box:
    case mcfile::blocks::minecraft::white_stained_glass:
    case mcfile::blocks::minecraft::white_terracotta:
    case mcfile::blocks::minecraft::white_wool:
    case mcfile::blocks::minecraft::yellow_concrete:
    case mcfile::blocks::minecraft::yellow_concrete_powder:
    case mcfile::blocks::minecraft::yellow_glazed_terracotta:
    case mcfile::blocks::minecraft::yellow_shulker_box:
    case mcfile::blocks::minecraft::yellow_stained_glass:
    case mcfile::blocks::minecraft::yellow_terracotta:
    case mcfile::blocks::minecraft::yellow_wool:
      return true;
    default:
      return false;
    }
  }
};

} // namespace je2be
