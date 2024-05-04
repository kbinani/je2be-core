#include "terraform/_wall-connectable.hpp"

#include <je2be/pos2.hpp>

#include "enums/_facing4.hpp"
#include "terraform/_block-property-accessor.hpp"

#include <minecraft-file.hpp>

namespace je2be::terraform {

class WallConnectable::Impl {
  Impl() = delete;

public:
  static void Do(mcfile::je::Chunk &out, BlockAccessor<mcfile::je::Block> &blockAccessor, BlockPropertyAccessor const &accessor) {
    using namespace std;

    if (!accessor.fHasWall) {
      return;
    }

    int cx = out.fChunkX;
    int cz = out.fChunkZ;

    vector<pair<u8string, Pos2i>> const nesw({{u8"north", Pos2i(0, -1)}, {u8"east", Pos2i(1, 0)}, {u8"south", Pos2i(0, 1)}, {u8"west", Pos2i(-1, 0)}});

    for (int y = accessor.minBlockY(); y <= accessor.maxBlockY(); y++) {
      for (int lz = 0; lz < 16; lz++) {
        int z = cz * 16 + lz;
        for (int lx = 0; lx < 16; lx++) {
          int x = cx * 16 + lx;

          auto p = accessor.property(x, y, z);
          if (p != BlockPropertyAccessor::WALL) {
            continue;
          }

          auto blockJ = out.blockAt(x, y, z);
          if (!blockJ) {
            continue;
          }
          map<u8string, optional<u8string>> props;

          auto upper = blockAccessor.blockAt(x, y + 1, z);

          for (auto d : nesw) {
            u8string direction = d.first;
            Pos2i vec = d.second;
            props[direction] = u8"none";
            auto pos = Pos2i(x, z) + vec;
            auto target = blockAccessor.blockAt(pos.fX, y, pos.fZ);
            if (!target) {
              continue;
            }
            if (IsWallConnectable(*target, vec)) {
              props[direction] = u8"low";
            }
            if (upper && IsBlockMakeWallTallShape(*upper)) {
              props[direction] = u8"tall";
            }
          }

          auto replace = blockJ->applying(props);
          out.setBlockAt(x, y, z, replace);
        }
      }
    }
  }

  static bool IsWallConnectable(mcfile::je::Block const &target, Pos2i targetDirection) {
    if (IsWallAlwaysConnectable(target.fId)) {
      return true;
    }
    if (target.fName.find(u8"trapdoor") != std::string::npos) {
      auto targetBlockFace = Pos2iFromFacing4(Facing4FromJavaName(target.property(u8"facing")));
      if (target.property(u8"open") == u8"true" && targetBlockFace == targetDirection) {
        return true;
      }
    } else if (target.fName.ends_with(u8"stairs")) {
      auto targetBlockFace = Pos2iFromFacing4(Facing4FromJavaName(target.property(u8"facing")));
      if (IsOrthogonal(targetBlockFace, targetDirection)) {
        return true;
      }
    } else if (target.fName.ends_with(u8"slab")) {
      if (target.property(u8"type") == u8"double") {
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
    if (b.fName.ends_with(u8"slab") && b.property(u8"type") != u8"top") {
      return true;
    }
    if (b.fName.ends_with(u8"trapdoor") && b.property(u8"half") == u8"bottom" && b.property(u8"open") == u8"false") {
      return true;
    }
    if (b.fName.ends_with(u8"stairs") && b.property(u8"half") == u8"bottom") {
      return true;
    }
    return false;
  }

  static bool IsWallAlwaysConnectable(mcfile::blocks::BlockId id) {
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
    case chiseled_sandstone:
    case chiseled_stone_bricks:
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
    case oxidized_copper_bulb:
    case oxidized_copper_grate:
    case oxidized_cut_copper:
    case packed_ice:
    case packed_mud:
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
    case repeating_command_block:
    case respawn_anchor:
    case rooted_dirt:
    case sand:
    case sandstone:
    case sandstone_wall:
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
    case stripped_spruce_log:
    case stripped_spruce_wood:
    case stripped_warped_hyphae:
    case stripped_warped_stem:
    case structure_block:
    case suspicious_gravel:
    case suspicious_sand:
    case target:
    case terracotta:
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
      return false;
    }
  }

  static bool IsBlockAlwaysMakeWallTallShape(mcfile::blocks::BlockId id) {
    using namespace mcfile::blocks::minecraft;
    switch (id) {
    case acacia_leaves:
    case acacia_log:
    case acacia_planks:
    case acacia_wood:
    case amethyst_block:
    case ancient_debris:
    case andesite:
    case azalea_leaves:
    case bamboo_block:
    case bamboo_mosaic:
    case bamboo_planks:
    case barrel:
    case barrier:
    case basalt:
    case beacon:
    case bedrock:
    case bee_nest:
    case beehive:
    case birch_leaves:
    case birch_log:
    case birch_planks:
    case birch_wood:
    case black_carpet:
    case black_concrete:
    case black_concrete_powder:
    case black_glazed_terracotta:
    case black_shulker_box:
    case black_stained_glass:
    case black_terracotta:
    case black_wool:
    case blackstone:
    case blast_furnace:
    case blue_carpet:
    case blue_concrete:
    case blue_concrete_powder:
    case blue_glazed_terracotta:
    case blue_ice:
    case blue_shulker_box:
    case blue_stained_glass:
    case blue_terracotta:
    case blue_wool:
    case bone_block:
    case bookshelf:
    case brain_coral_block:
    case bricks:
    case brown_carpet:
    case brown_concrete:
    case brown_concrete_powder:
    case brown_glazed_terracotta:
    case brown_mushroom_block:
    case brown_shulker_box:
    case brown_stained_glass:
    case brown_terracotta:
    case brown_wool:
    case bubble_coral_block:
    case budding_amethyst:
    case calcite:
    case campfire:
    case cartography_table:
    case carved_pumpkin:
    case chain_command_block:
    case cherry_leaves:
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
    case chiseled_sandstone:
    case chiseled_stone_bricks:
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
    case crimson_hyphae:
    case crimson_nylium:
    case crimson_planks:
    case crimson_stem:
    case crying_obsidian:
    case cut_copper:
    case cut_red_sandstone:
    case cut_sandstone:
    case cyan_carpet:
    case cyan_concrete:
    case cyan_concrete_powder:
    case cyan_glazed_terracotta:
    case cyan_shulker_box:
    case cyan_stained_glass:
    case cyan_terracotta:
    case cyan_wool:
    case dark_oak_leaves:
    case dark_oak_log:
    case dark_oak_planks:
    case dark_oak_wood:
    case dark_prismarine:
    case daylight_detector:
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
    case dirt_path:
    case dispenser:
    case dried_kelp_block:
    case dripstone_block:
    case dropper:
    case emerald_block:
    case emerald_ore:
    case enchanting_table:
    case end_portal_frame:
    case end_stone:
    case end_stone_bricks:
    case exposed_chiseled_copper:
    case exposed_copper:
    case exposed_copper_bulb:
    case exposed_copper_grate:
    case exposed_cut_copper:
    case farmland:
    case fire_coral_block:
    case fletching_table:
    case flowering_azalea_leaves:
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
    case gray_carpet:
    case gray_concrete:
    case gray_concrete_powder:
    case gray_glazed_terracotta:
    case gray_shulker_box:
    case gray_stained_glass:
    case gray_terracotta:
    case gray_wool:
    case green_carpet:
    case green_concrete:
    case green_concrete_powder:
    case green_glazed_terracotta:
    case green_shulker_box:
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
    case jack_o_lantern:
    case jigsaw:
    case jukebox:
    case jungle_leaves:
    case jungle_log:
    case jungle_planks:
    case jungle_wood:
    case lapis_block:
    case lapis_ore:
    case lectern:
    case light_blue_carpet:
    case light_blue_concrete:
    case light_blue_concrete_powder:
    case light_blue_glazed_terracotta:
    case light_blue_shulker_box:
    case light_blue_stained_glass:
    case light_blue_terracotta:
    case light_blue_wool:
    case light_gray_carpet:
    case light_gray_concrete:
    case light_gray_concrete_powder:
    case light_gray_glazed_terracotta:
    case light_gray_shulker_box:
    case light_gray_stained_glass:
    case light_gray_terracotta:
    case light_gray_wool:
    case lime_carpet:
    case lime_concrete:
    case lime_concrete_powder:
    case lime_glazed_terracotta:
    case lime_shulker_box:
    case lime_stained_glass:
    case lime_terracotta:
    case lime_wool:
    case lodestone:
    case loom:
    case magenta_carpet:
    case magenta_concrete:
    case magenta_concrete_powder:
    case magenta_glazed_terracotta:
    case magenta_shulker_box:
    case magenta_stained_glass:
    case magenta_terracotta:
    case magenta_wool:
    case magma_block:
    case mangrove_leaves:
    case mangrove_log:
    case mangrove_planks:
    case mangrove_roots:
    case mangrove_wood:
    case melon:
    case moss_block:
    case moss_carpet:
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
    case oak_leaves:
    case oak_log:
    case oak_planks:
    case oak_wood:
    case observer:
    case obsidian:
    case ochre_froglight:
    case orange_carpet:
    case orange_concrete:
    case orange_concrete_powder:
    case orange_glazed_terracotta:
    case orange_shulker_box:
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
    case pearlescent_froglight:
    case pink_carpet:
    case pink_concrete:
    case pink_concrete_powder:
    case pink_glazed_terracotta:
    case pink_shulker_box:
    case pink_stained_glass:
    case pink_terracotta:
    case pink_wool:
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
    case pumpkin:
    case purple_carpet:
    case purple_concrete:
    case purple_concrete_powder:
    case purple_glazed_terracotta:
    case purple_shulker_box:
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
    case red_carpet:
    case red_concrete:
    case red_concrete_powder:
    case red_glazed_terracotta:
    case red_mushroom_block:
    case red_nether_bricks:
    case red_sand:
    case red_sandstone:
    case red_shulker_box:
    case red_stained_glass:
    case red_terracotta:
    case red_wool:
    case redstone_block:
    case redstone_lamp:
    case redstone_ore:
    case repeater:
    case repeating_command_block:
    case respawn_anchor:
    case rooted_dirt:
    case sand:
    case sandstone:
    case sculk_sensor:
    case sea_lantern:
    case shroomlight:
    case shulker_box:
    case slime_block:
    case smithing_table:
    case smoker:
    case smooth_basalt:
    case smooth_quartz:
    case smooth_red_sandstone:
    case smooth_sandstone:
    case smooth_stone:
    case snow_block:
    case soul_campfire:
    case soul_sand:
    case soul_soil:
    case spawner:
    case sponge:
    case spruce_leaves:
    case spruce_log:
    case spruce_planks:
    case spruce_wood:
    case stone:
    case stone_bricks:
    case stonecutter:
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
    case stripped_spruce_log:
    case stripped_spruce_wood:
    case stripped_warped_hyphae:
    case stripped_warped_stem:
    case structure_block:
    case suspicious_gravel:
    case suspicious_sand:
    case target:
    case terracotta:
    case tinted_glass:
    case tnt:
    case trial_spawner:
    case tube_coral_block:
    case tuff:
    case tuff_bricks:
    case vault:
    case verdant_froglight:
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
    case white_carpet:
    case white_concrete:
    case white_concrete_powder:
    case white_glazed_terracotta:
    case white_shulker_box:
    case white_stained_glass:
    case white_terracotta:
    case white_wool:
    case yellow_carpet:
    case yellow_concrete:
    case yellow_concrete_powder:
    case yellow_glazed_terracotta:
    case yellow_shulker_box:
    case yellow_stained_glass:
    case yellow_terracotta:
    case yellow_wool:
      return true;
    default:
      return false;
    }
  }
};

void WallConnectable::Do(mcfile::je::Chunk &out, BlockAccessor<mcfile::je::Block> &blockAccessor, BlockPropertyAccessor const &accessor) {
  Impl::Do(out, blockAccessor, accessor);
}

} // namespace je2be::terraform
