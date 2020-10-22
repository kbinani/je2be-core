#pragma once

namespace j2b {

class BlockData {
public:
    static std::shared_ptr<mcfile::nbt::CompoundTag> From(std::shared_ptr<mcfile::Block const> const& block) {
        using namespace std;
        using namespace props;
        using namespace mcfile::nbt;

        static unique_ptr<unordered_map<string, ConverterFunction> const> const converterTable(CreateConverterTable());

        auto found = converterTable->find(block->fName);
        if (found == converterTable->end()) {
            return Identity(*block);
        } else {
            return found->second(*block);
        }
    }

    static std::shared_ptr<mcfile::nbt::CompoundTag> Air() {
        static std::shared_ptr<mcfile::nbt::CompoundTag> const air = Make("air");
        return air;
    }

    static std::shared_ptr<mcfile::nbt::CompoundTag> Make(std::string const& name) {
        auto tag = New(name);
        auto states = States();
        tag->fValue.emplace("states", states);
        return tag;
    }
    
private:
    BlockData() = delete;

    using ConverterFunction = std::function<std::shared_ptr<mcfile::nbt::CompoundTag>(mcfile::Block const&)>;
    using BlockDataType = std::shared_ptr<mcfile::nbt::CompoundTag>;
    using StatesType = std::shared_ptr<mcfile::nbt::CompoundTag>;
    using Block = mcfile::Block;

    static std::unordered_map<std::string, ConverterFunction>* CreateConverterTable() {
        using namespace std;
        auto table = new unordered_map<string, ConverterFunction>();
#define E(__name, __func) table->emplace("minecraft:" __name, __func);
        E("stone", Stone("stone"));
        E("granite", Stone("granite"));
        E("polished_granite", Stone("granite_smooth"));
        E("andesite", Stone("andesite"));
        E("polished_andesite", Stone("andesite_smooth"));
        E("diorite", Stone("diorite"));
        E("polished_diorite", Stone("diorite_smooth"));
        E("dirt", Dirt("normal"));
        E("coarse_dirt", Dirt("coarse"));
        E("grass_block", Rename("grass"));
        E("oak_log", Log("oak"));
        E("spruce_log", Log("spruce"));
        E("birch_log", Log("birch"));
        E("jungle_log", Log("jungle"));
        E("acacia_log", Log2("acacia"));
        E("dark_oak_log", Log2("dark_oak"));
        E("crimson_stem", AxisToPillarAxis());
        E("warped_stem", AxisToPillarAxis());

        E("stripped_oak_log", AxisToPillarAxis());
        E("stripped_spruce_log", AxisToPillarAxis());
        E("stripped_birch_log", AxisToPillarAxis());
        E("stripped_acacia_log", AxisToPillarAxis());
        E("stripped_jungle_log", AxisToPillarAxis());
        E("stripped_dark_oak_log", AxisToPillarAxis());

        E("oak_wood", Wood("oak", false));
        E("spruce_wood", Wood("spruce", false));
        E("birch_wood", Wood("birch", false));
        E("acacia_wood", Wood("acacia", false));
        E("jungle_wood", Wood("jungle", false));
        E("dark_oak_wood", Wood("dark_oak", false));
        E("stripped_oak_wood", Wood("oak", true));
        E("stripped_spruce_wood", Wood("spruce", true));
        E("stripped_birch_wood", Wood("birch", true));
        E("stripped_acacia_wood", Wood("acacia", true));
        E("stripped_jungle_wood", Wood("jungle", true));
        E("stripped_dark_oak_wood", Wood("dark_oak", true));
        E("oak_leaves", Leaves("oak"));
        E("spruce_leaves", Leaves("spruce"));
        E("birch_leaves", Leaves("birch"));
        E("jungle_leaves", Leaves("jungle"));
        E("acacia_leaves", Leaves2("acacia"));
        E("dark_oak_leaves", Leaves2("dark_oak"));
        E("crimson_hyphae", AxisToPillarAxis());
        E("warped_hyphae", AxisToPillarAxis());
        E("stripped_crimson_hyphae", AxisToPillarAxis());
        E("stripped_warped_hyphae", AxisToPillarAxis());
        E("oak_slab", WoodenSlab("oak"));
        E("birch_slab", WoodenSlab("birch"));
        E("spruce_slab", WoodenSlab("spruce"));
        E("jungle_slab", WoodenSlab("jungle"));
        E("acacia_slab", WoodenSlab("acacia"));
        E("dark_oak_slab", WoodenSlab("dark_oak"));
        E("petrified_oak_slab", WoodenSlab("oak"));
        E("stone_slab", StoneSlab4("stone"));
        E("granite_slab", StoneSlab3("granite"));
        E("andesite_slab", StoneSlab3("andesite"));
        E("diorite_slab", StoneSlab3("diorite"));
        E("cobblestone_slab", StoneSlab("cobblestone"));
        E("stone_brick_slab", StoneSlab("stone_brick"));
        E("brick_slab", StoneSlab("brick"));
        E("sandstone_slab", StoneSlab("sandstone"));
        E("smooth_sandstone_slab", StoneSlab2("smooth_sandstone"));
        E("smooth_stone_slab", StoneSlab("smooth_stone"));
        E("nether_brick_slab", StoneSlab("nether_brick"));
        E("quartz_slab", StoneSlab("quartz"));
        E("smooth_quartz_slab", StoneSlab4("smooth_quartz"));
        E("red_sandstone_slab", StoneSlab2("red_sandstone"));
        E("smooth_red_sandstone_slab", StoneSlab3("smooth_red_sandstone"));
        E("cut_red_sandstone_slab", StoneSlab4("cut_red_sandstone"));
        E("mossy_cobblestone_slab", StoneSlab2("mossy_cobblestone"));
        E("polished_diorite_slab", StoneSlab3("polished_diorite"));
        E("mossy_stone_brick_slab", StoneSlab4("mossy_stone_brick"));
        E("polished_granite_slab", StoneSlab3("polished_granite"));
        E("dark_prismarine_slab", StoneSlab2("prismarine_dark"));
        E("prismarine_brick_slab", StoneSlab2("prismarine_brick"));
        E("prismarine_slab", StoneSlab2("prismarine_rough"));
        E("purpur_slab", StoneSlab2("purpur"));
        E("cut_sandstone_slab", StoneSlab4("cut_sandstone"));
        E("polished_blackstone_brick_slab", StoneSlabNT("polished_blackstone_brick_double_slab"));
        E("polished_blackstone_slab", StoneSlabNT("polished_blackstone_double_slab"));
        E("blackstone_slab", StoneSlabNT("blackstone_double_slab"));
        E("polished_andesite_slab", StoneSlab3("polished_andesite"));
        E("red_nether_brick_slab", StoneSlab2("red_nether_brick"));
        E("end_stone_brick_slab", StoneSlab3("end_stone_brick"));
        E("warped_slab", StoneSlabNT("warped_double_slab"));
        E("crimson_slab", StoneSlabNT("crimson_double_slab"));
        E("grass", TallGrass("tall"));
        E("tall_grass", DoublePlant("grass"));
        E("large_fern", DoublePlant("fern"));
        E("fern", TallGrass("fern"));
        E("lilac", DoublePlant("syringa"));
        E("rose_bush", DoublePlant("rose"));
        E("peony", DoublePlant("paeonia"));
        E("sunflower", DoublePlant("sunflower"));
        E("dead_bush", Rename("deadbush"));
        E("sea_pickle", SeaPickle);
        E("dandelion", Rename("yellow_flower"));
        E("poppy", RedFlower("poppy"));
        E("blue_orchid", RedFlower("orchid"));
        E("allium", RedFlower("allium"));
        E("azure_bluet", RedFlower("houstonia"));
        E("red_tulip", RedFlower("tulip_red"));
        E("orange_tulip", RedFlower("tulip_orange"));
        E("white_tulip", RedFlower("tulip_white"));
        E("pink_tulip", RedFlower("tulip_pink"));
        E("oxeye_daisy", RedFlower("oxeye"));
        E("cornflower", RedFlower("cornflower"));
        E("lily_of_the_valley", RedFlower("lily_of_the_valley"));
        E("seagrass", Seagrass);
        E("tall_seagrass", TallSeagrass);
        E("kelp", Kelp());
        E("kelp_plant", Kelp(16));
        E("water", Liquid("water"));
        E("lava", Liquid("lava"));
        E("weeping_vines_plant", NetherVines("weeping", 25)); //TODO(kbinani): is 25 correct?
        E("weeping_vines", NetherVines("weeping"));
        E("twisting_vines_plant", NetherVines("twisting", 25)); //TODO(kbinani): is 25 correct?
        E("twisting_vines", NetherVines("twisting"));
        E("vine", Vine);
        E("cobblestone_stairs", Stairs("stone_stairs"));
        E("stone_stairs", Stairs("normal_stone_stairs"));
        E("end_stone_brick_stairs", Stairs("end_brick_stairs"));
        E("prismarine_brick_stairs", Stairs("prismarine_bricks_stairs"));
        E("oak_stairs", Stairs());
        E("spruce_stairs", Stairs());
        E("birch_stairs", Stairs());
        E("jungle_stairs", Stairs());
        E("acacia_stairs", Stairs());
        E("dark_oak_stairs", Stairs());
        E("crimson_stairs", Stairs());
        E("warped_stairs", Stairs());
        E("granite_stairs", Stairs());
        E("polished_granite_stairs", Stairs());
        E("diorite_stairs", Stairs());
        E("polished_diorite_stairs", Stairs());
        E("andesite_stairs", Stairs());
        E("polished_andesite_stairs", Stairs());
        E("cobblestone_stairs", Stairs());
        E("mossy_cobblestone_stairs", Stairs());
        E("stone_brick_stairs", Stairs());
        E("mossy_stone_brick_stairs", Stairs());
        E("brick_stairs", Stairs());
        E("end_stone_brick_stairs", Stairs());
        E("nether_brick_stairs", Stairs());
        E("red_nether_brick_stairs", Stairs());
        E("sandstone_stairs", Stairs());
        E("smooth_sandstone_stairs", Stairs());
        E("red_sandstone_stairs", Stairs());
        E("smooth_red_sandstone_stairs", Stairs());
        E("quartz_stairs", Stairs());
        E("smooth_quartz_stairs", Stairs());
        E("purpur_stairs", Stairs());
        E("prismarine_stairs", Stairs());
        E("prismarine_brick_stairs", Stairs());
        E("dark_prismarine_stairs", Stairs());
        E("blackstone_stairs", Stairs());
        E("polished_blackstone_stairs", Stairs());
        E("polished_blackstone_brick_stairs", Stairs());
        E("sponge", Sponge("dry"));
        E("wet_sponge", Sponge("wet"));
        E("sandstone", Sandstone("default"));
        E("chiseled_sandstone", Sandstone("heiroglyphs"));
        E("cut_sandstone", Sandstone("cut"));
        E("smooth_sandstone", Sandstone("smooth"));
        E("red_sandstone", RedSandstone("default"));
        E("chiseled_red_sandstone", RedSandstone("heiroglyphs"));
        E("cut_red_sandstone", RedSandstone("cut"));
        E("smooth_red_sandstone", RedSandstone("smooth"));
        E("white_wool", Wool("white"));
        E("orange_wool", Wool("orange"));
        E("magenta_wool", Wool("magenta"));
        E("light_blue_wool", Wool("light_blue"));
        E("yellow_wool", Wool("yellow"));
        E("lime_wool", Wool("lime"));
        E("pink_wool", Wool("pink"));
        E("gray_wool", Wool("gray"));
        E("light_gray_wool", Wool("silver"));
        E("cyan_wool", Wool("cyan"));
        E("purple_wool", Wool("purple"));
        E("blue_wool", Wool("blue"));
        E("brown_wool", Wool("brown"));
        E("green_wool", Wool("green"));
        E("red_wool", Wool("red"));
        E("black_wool", Wool("black"));
        E("snow_block", Rename("snow"));
        E("quartz_block", QuartzBlock("default"));
        E("smooth_quartz", QuartzBlock("smooth"));
        E("quartz_pillar", QuartzBlock("lines"));
        E("chiseled_quartz_block", QuartzBlock("chiseled"));
        E("bricks", Rename("brick_block"));
        E("sand", Sand("normal"));
        E("red_sand", Sand("red"));
        E("oak_planks", Planks("oak"));
        E("spruce_planks", Planks("spruce"));
        E("birch_planks", Planks("birch"));
        E("jungle_planks", Planks("jungle"));
        E("acacia_planks", Planks("acacia"));
        E("dark_oak_planks", Planks("dark_oak"));
        E("purpur_block", PurpurBlock("default"));
        E("purpur_pillar", PurpurBlock("lines"));
        E("jack_o_lantern", LitPumpkin);
        E("carved_pumpkin", FacingToDirection);
        E("white_stained_glass", StainedGlass("white"));
        E("orange_stained_glass", StainedGlass("orange"));
        E("magenta_stained_glass", StainedGlass("magenta"));
        E("light_blue_stained_glass", StainedGlass("light_blue"));
        E("yellow_stained_glass", StainedGlass("yellow"));
        E("lime_stained_glass", StainedGlass("lime"));
        E("pink_stained_glass", StainedGlass("pink"));
        E("gray_stained_glass", StainedGlass("gray"));
        E("light_gray_stained_glass", StainedGlass("silver"));
        E("cyan_stained_glass", StainedGlass("cyan"));
        E("purple_stained_glass", StainedGlass("purple"));
        E("blue_stained_glass", StainedGlass("blue"));
        E("brown_stained_glass", StainedGlass("brown"));
        E("green_stained_glass", StainedGlass("green"));
        E("red_stained_glass", StainedGlass("red"));
        E("black_stained_glass", StainedGlass("black"));
        E("white_concrete_powder", ConcretePowder("white"));
        E("orange_concrete_powder", ConcretePowder("orange"));
        E("magenta_concrete_powder", ConcretePowder("magenta"));
        E("light_blue_concrete_powder", ConcretePowder("light_blue"));
        E("yellow_concrete_powder", ConcretePowder("yellow"));
        E("lime_concrete_powder", ConcretePowder("lime"));
        E("pink_concrete_powder", ConcretePowder("pink"));
        E("gray_concrete_powder", ConcretePowder("gray"));
        E("light_gray_concrete_powder", ConcretePowder("silver"));
        E("cyan_concrete_powder", ConcretePowder("cyan"));
        E("purple_concrete_powder", ConcretePowder("purple"));
        E("blue_concrete_powder", ConcretePowder("blue"));
        E("brown_concrete_powder", ConcretePowder("brown"));
        E("green_concrete_powder", ConcretePowder("green"));
        E("red_concrete_powder", ConcretePowder("red"));
        E("black_concrete_powder", ConcretePowder("black"));
        E("white_concrete", Concrete("white"));
        E("orange_concrete", Concrete("orange"));
        E("magenta_concrete", Concrete("magenta"));
        E("light_blue_concrete", Concrete("light_blue"));
        E("yellow_concrete", Concrete("yellow"));
        E("lime_concrete", Concrete("lime"));
        E("pink_concrete", Concrete("pink"));
        E("gray_concrete", Concrete("gray"));
        E("light_gray_concrete", Concrete("silver"));
        E("cyan_concrete", Concrete("cyan"));
        E("purple_concrete", Concrete("purple"));
        E("blue_concrete", Concrete("blue"));
        E("brown_concrete", Concrete("brown"));
        E("green_concrete", Concrete("green"));
        E("red_concrete", Concrete("red"));
        E("black_concrete", Concrete("black"));
        E("white_terracotta", Terracotta("white"));
        E("orange_terracotta", Terracotta("orange"));
        E("magenta_terracotta", Terracotta("magenta"));
        E("light_blue_terracotta", Terracotta("light_blue"));
        E("yellow_terracotta", Terracotta("yellow"));
        E("lime_terracotta", Terracotta("lime"));
        E("pink_terracotta", Terracotta("pink"));
        E("gray_terracotta", Terracotta("gray"));
        E("light_gray_terracotta", Terracotta("silver"));
        E("cyan_terracotta", Terracotta("cyan"));
        E("purple_terracotta", Terracotta("purple"));
        E("blue_terracotta", Terracotta("blue"));
        E("brown_terracotta", Terracotta("brown"));
        E("green_terracotta", Terracotta("green"));
        E("red_terracotta", Terracotta("red"));
        E("black_terracotta", Terracotta("black"));
        E("nether_quartz_ore", Rename("quartz_ore"));
        E("red_nether_bricks", Rename("red_nether_brick"));
        E("magma_block", Rename("magma"));
        E("sea_lantern", Rename("seaLantern"));
        E("prismarine_bricks", Prismarine("bricks"));
        E("dark_prismarine", Prismarine("dark"));
        E("prismarine", Prismarine("default"));
        E("terracotta", Rename("hardened_clay"));
        E("end_stone_bricks", Rename("end_bricks"));
        E("melon", Rename("melon_block"));
        E("chiseled_stone_bricks", StoneBrick("chiseled"));
        E("cracked_stone_bricks", StoneBrick("cracked"));
        E("mossy_stone_bricks", StoneBrick("mossy"));
        E("stone_bricks", StoneBrick("default"));
        E("oak_sapling", Sapling("oak"));
        E("birch_sapling", Sapling("birch"));
        E("jungle_sapling", Sapling("jungle"));
        E("acacia_sapling", Sapling("acacia"));
        E("spruce_sapling", Sapling("spruce"));
        E("dark_oak_sapling", Sapling("dark_oak"));
        E("tube_coral_block", CoralBlock("blue", false));
        E("brain_coral_block", CoralBlock("pink", false));
        E("bubble_coral_block", CoralBlock("purple", false));
        E("fire_coral_block", CoralBlock("red", false));
        E("horn_coral_block", CoralBlock("yellow", false));
        E("dead_tube_coral_block", CoralBlock("blue", true));
        E("dead_brain_coral_block", CoralBlock("pink", true));
        E("dead_bubble_coral_block", CoralBlock("purple", true));
        E("dead_fire_coral_block", CoralBlock("red", true));
        E("dead_horn_coral_block", CoralBlock("yellow", true));
        E("snow", SnowLayer);
        E("sugar_cane", Rename("reeds"));
        E("end_rod", EndRod);
        E("oak_fence", Fence("oak"));
        E("spruce_fence", Fence("spruce"));
        E("birch_fence", Fence("birch"));
        E("jungle_fence", Fence("jungle"));
        E("acacia_fence", Fence("acacia"));
        E("dark_oak_fence", Fence("dark_oak"));
        E("ladder", FacingToFacingDirection);
        E("chest", FacingToFacingDirection);
        E("furnace", FacingToFacingDirection);
        E("nether_bricks", Rename("nether_brick"));
        E("infested_stone", InfestedStone("stone"));
        E("infested_cobblestone", InfestedStone("cobblestone"));
        E("infested_stone_bricks", InfestedStone("stone_brick"));
        E("infested_mossy_stone_bricks", InfestedStone("mossy_stone_brick"));
        E("infested_cracked_stone_bricks", InfestedStone("cracked_stone_brick"));
        E("infested_chiseled_stone_bricks", InfestedStone("chiseled_stone_brick"));
        E("torch", AnyTorch(""));
        E("wall_torch", AnyWallTorch(""));
        E("soul_torch", AnyTorch("soul_"));
        E("soul_wall_torch", AnyWallTorch("soul_"));
        E("farmland", Farmland());
        E("red_mushroom_block", AnyMushroomBlock("red_mushroom_block", false));
        E("brown_mushroom_block", AnyMushroomBlock("brown_mushroom_block", false));
        E("mushroom_stem", AnyMushroomBlock("brown_mushroom_block", true));
        E("end_portal_frame", EndPortalFrame);
        E("white_shulker_box", ShulkerBox("white"));
        E("orange_shulker_box", ShulkerBox("orange"));
        E("magenta_shulker_box", ShulkerBox("magenta"));
        E("light_blue_shulker_box", ShulkerBox("light_blue"));
        E("yellow_shulker_box", ShulkerBox("yellow"));
        E("lime_shulker_box", ShulkerBox("lime"));
        E("pink_shulker_box", ShulkerBox("pink"));
        E("gray_shulker_box", ShulkerBox("gray"));
        E("light_gray_shulker_box", ShulkerBox("silver"));
        E("cyan_shulker_box", ShulkerBox("cyan"));
        E("purple_shulker_box", ShulkerBox("purple"));
        E("blue_shulker_box", ShulkerBox("blue"));
        E("brown_shulker_box", ShulkerBox("brown"));
        E("green_shulker_box", ShulkerBox("green"));
        E("red_shulker_box", ShulkerBox("red"));
        E("black_shulker_box", ShulkerBox("black"));
        E("shulker_box", Rename("undyed_shulker_box"));
        E("cobblestone_wall", Wall("cobblestone"));
        E("mossy_cobblestone_wall", Wall("mossy_cobblestone"));
        E("brick_wall", Wall("brick"));
        E("prismarine_wall", Wall("prismarine"));
        E("red_sandstone_wall", Wall("red_sandstone"));
        E("mossy_stone_brick_wall", Wall("mossy_stone_brick"));
        E("granite_wall", Wall("granite"));
        E("andesite_wall", Wall("andesite"));
        E("diorite_wall", Wall("diorite"));
        E("stone_brick_wall", Wall("stone_brick"));
        E("nether_brick_wall", Wall("nether_brick"));
        E("red_nether_brick_wall", Wall("red_nether_brick"));
        E("sandstone_wall", Wall("sandstone"));
        E("end_stone_brick_wall", Wall("end_brick"));
        E("blackstone_wall", WallNT);
        E("polished_blackstone_wall", WallNT);
        E("polished_blackstone_brick_wall", WallNT);
        E("white_carpet", Carpet("white"));
        E("orange_carpet", Carpet("orange"));
        E("magenta_carpet", Carpet("magenta"));
        E("light_blue_carpet", Carpet("light_blue"));
        E("yellow_carpet", Carpet("yellow"));
        E("lime_carpet", Carpet("lime"));
        E("pink_carpet", Carpet("pink"));
        E("gray_carpet", Carpet("gray"));
        E("light_gray_carpet", Carpet("silver"));
        E("cyan_carpet", Carpet("cyan"));
        E("purple_carpet", Carpet("purple"));
        E("blue_carpet", Carpet("blue"));
        E("brown_carpet", Carpet("brown"));
        E("green_carpet", Carpet("green"));
        E("red_carpet", Carpet("red"));
        E("black_carpet", Carpet("black"));
        E("white_stained_glass_pane", StainedGlassPane("white"));
        E("orange_stained_glass_pane", StainedGlassPane("orange"));
        E("magenta_stained_glass_pane", StainedGlassPane("magenta"));
        E("light_blue_stained_glass_pane", StainedGlassPane("light_blue"));
        E("yellow_stained_glass_pane", StainedGlassPane("yellow"));
        E("lime_stained_glass_pane", StainedGlassPane("lime"));
        E("pink_stained_glass_pane", StainedGlassPane("pink"));
        E("gray_stained_glass_pane", StainedGlassPane("gray"));
        E("light_gray_stained_glass_pane", StainedGlassPane("silver"));
        E("cyan_stained_glass_pane", StainedGlassPane("cyan"));
        E("purple_stained_glass_pane", StainedGlassPane("purple"));
        E("blue_stained_glass_pane", StainedGlassPane("blue"));
        E("brown_stained_glass_pane", StainedGlassPane("brown"));
        E("green_stained_glass_pane", StainedGlassPane("green"));
        E("red_stained_glass_pane", StainedGlassPane("red"));
        E("black_stained_glass_pane", StainedGlassPane("black"));
        E("slime_block", Rename("slime"));
        E("anvil", Anvil("undamaged"));
        E("chipped_anvil", Anvil("slightly_damaged"));
        E("damaged_anvil", Anvil("very_damaged"));
        E("white_glazed_terracotta", FacingToFacingDirection);
        E("orange_glazed_terracotta", FacingToFacingDirection);
        E("magenta_glazed_terracotta", FacingToFacingDirection);
        E("light_blue_glazed_terracotta", FacingToFacingDirection);
        E("yellow_glazed_terracotta", FacingToFacingDirection);
        E("lime_glazed_terracotta", FacingToFacingDirection);
        E("pink_glazed_terracotta", FacingToFacingDirection);
        E("gray_glazed_terracotta", FacingToFacingDirection);
        E("light_gray_glazed_terracotta", SilverGlazedTerracotta);
        E("cyan_glazed_terracotta", FacingToFacingDirection);
        E("purple_glazed_terracotta", FacingToFacingDirection);
        E("blue_glazed_terracotta", FacingToFacingDirection);
        E("brown_glazed_terracotta", FacingToFacingDirection);
        E("green_glazed_terracotta", FacingToFacingDirection);
        E("red_glazed_terracotta", FacingToFacingDirection);
        E("black_glazed_terracotta", FacingToFacingDirection);

        E("tube_coral", Coral("blue", false));
        E("brain_coral", Coral("pink", false));
        E("bubble_coral", Coral("purple", false));
        E("fire_coral", Coral("red", false));
        E("horn_coral", Coral("yellow", false));

        E("dead_tube_coral", Coral("blue", true));
        E("dead_brain_coral", Coral("pink", true));
        E("dead_bubble_coral", Coral("purple", true));
        E("dead_fire_coral", Coral("red", true));
        E("dead_horn_coral", Coral("yellow", true));

        E("tube_coral_fan", CoralFan("blue", false));
        E("brain_coral_fan", CoralFan("pink", false));
        E("bubble_coral_fan", CoralFan("purple", false));
        E("fire_coral_fan", CoralFan("red", false));
        E("horn_coral_fan", CoralFan("yellow", false));

        E("dead_tube_coral_fan", CoralFan("blue", true));
        E("dead_brain_coral_fan", CoralFan("pink", true));
        E("dead_bubble_coral_fan", CoralFan("purple", true));
        E("dead_fire_coral_fan", CoralFan("red", true));
        E("dead_horn_coral_fan", CoralFan("yellow", true));

        E("oak_sign", Sign());
        E("spruce_sign", Sign("spruce"));
        E("birch_sign", Sign("birch"));
        E("jungle_sign", Sign("jungle"));
        E("acacia_sign", Sign("acacia"));
        E("dark_oak_sign", Sign("darkoak"));
        E("crimson_sign", Sign("crimson"));
        E("warped_sign", Sign("warped"));

        E("oak_wall_sign", WallSign());
        E("spruce_wall_sign", WallSign("spruce"));
        E("birch_wall_sign", WallSign("birch"));
        E("jungle_wall_sign", WallSign("jungle"));
        E("acacia_wall_sign", WallSign("acacia"));
        E("dark_wall_oak_sign", WallSign("darkoak"));
        E("crimson_wall_sign", WallSign("crimson"));
        E("warped_wall_sign", WallSign("warped"));

        E("white_bed", Bed);
        E("orange_bed", Bed);
        E("magenta_bed", Bed);
        E("light_blue_bed", Bed);
        E("yellow_bed", Bed);
        E("lime_bed", Bed);
        E("pink_bed", Bed);
        E("gray_bed", Bed);
        E("light_gray_bed", Bed);
        E("cyan_bed", Bed);
        E("purple_bed", Bed);
        E("blue_bed", Bed);
        E("brown_bed", Bed);
        E("green_bed", Bed);
        E("red_bed", Bed);
        E("black_bed", Bed);

        E("potted_oak_sapling", PottedFlowerPot);
        E("potted_spruce_sapling", PottedFlowerPot);
        E("potted_birch_sapling", PottedFlowerPot);
        E("potted_jungle_sapling", PottedFlowerPot);
        E("potted_acacia_sapling", PottedFlowerPot);
        E("potted_dark_oak_sapling", PottedFlowerPot);
        E("potted_fern", PottedFlowerPot);
        E("potted_dead_bush", PottedFlowerPot);
        E("potted_dandelion", PottedFlowerPot);
        E("potted_poppy", PottedFlowerPot);
        E("potted_blue_orchid", PottedFlowerPot);
        E("potted_allium", PottedFlowerPot);
        E("potted_azure_bluet", PottedFlowerPot);
        E("potted_red_tulip", PottedFlowerPot);
        E("potted_orange_tulip", PottedFlowerPot);
        E("potted_white_tulip", PottedFlowerPot);
        E("potted_pink_tulip", PottedFlowerPot);
        E("potted_oxeye_daisy", PottedFlowerPot);
        E("potted_cornflower", PottedFlowerPot);
        E("potted_lily_of_the_valley", PottedFlowerPot);
        E("potted_wither_rose", PottedFlowerPot);
        E("potted_brown_mushroom", PottedFlowerPot);
        E("potted_red_mushroom", PottedFlowerPot);
        E("potted_crimson_fungus", PottedFlowerPot);
        E("potted_warped_fungus", PottedFlowerPot);
        E("potted_crimson_roots", PottedFlowerPot);
        E("potted_warped_roots", PottedFlowerPot);
        E("potted_bamboo", PottedFlowerPot);

        E("skeleton_skull", SkeletonSkull);

        E("white_banner", Banner);
        E("orange_banner", Banner);
        E("magenta_banner", Banner);
        E("light_blue_banner", Banner);
        E("yellow_banner", Banner);
        E("lime_banner", Banner);
        E("pink_banner", Banner);
        E("gray_banner", Banner);
        E("light_gray_banner", Banner);
        E("cyan_banner", Banner);
        E("purple_banner", Banner);
        E("blue_banner", Banner);
        E("brown_banner", Banner);
        E("green_banner", Banner);
        E("red_banner", Banner);
        E("black_banner", Banner);

        E("white_wall_banner", WallBanner);
        E("orange_wall_banner", WallBanner);
        E("magenta_wall_banner", WallBanner);
        E("light_blue_wall_banner", WallBanner);
        E("yellow_wall_banner", WallBanner);
        E("lime_wall_banner", WallBanner);
        E("pink_wall_banner", WallBanner);
        E("gray_wall_banner", WallBanner);
        E("light_gray_wall_banner", WallBanner);
        E("cyan_wall_banner", WallBanner);
        E("purple_wall_banner", WallBanner);
        E("blue_wall_banner", WallBanner);
        E("brown_wall_banner", WallBanner);
        E("green_wall_banner", WallBanner);
        E("red_wall_banner", WallBanner);
        E("black_wall_banner", WallBanner);

        E("stonecutter", RenameFaced("stonecutter_block"));
        E("loom", FacingToDirection);
        E("grindstone", Grindstone);
        E("smoker", FacingToFacingDirection);
        E("blast_furnace", FacingToFacingDirection);
        E("barrel", Barrel);
        E("lantern", Lantern);
        E("soul_lantern", Lantern);
        E("bell", Bell);
        E("campfire", Campfire);
        E("soul_campfire", Campfire);
#undef E

        /*
        TODO:
        - direction of shulker box
        - sign text
        - rotation of skelton skull
        */

        return table;
    }

    static BlockDataType Campfire(Block const& block) {
        using namespace std;
        using namespace props;
        auto tag = New(block.fName, true);
        auto states = States();
        auto direction = DirectionFromFacing(block);
        states->fValue.emplace("direction", Int(direction));
        auto lit = block.property("lit", "false") == "true";
        states->fValue.emplace("extinguished", Bool(!lit));
        return Complete(tag, block, states);
    }

    static std::string Attachment(std::string const& attachment) {
        if (attachment == "floor") {
            return "standing";
        } else if (attachment == "ceiling") {
            return "hanging";
        } else if (attachment == "double_wall") {
            return "multiple";
        } else if (attachment == "single_wall") {
            return "side";
        }
        return attachment;
    }

    static BlockDataType Bell(Block const& block) {
        using namespace std;
        using namespace props;
        auto tag = New("bell");
        auto states = States();
        auto facing = block.property("facing", "north");
        int32_t direction = 0;
        if (facing == "north") {
            direction = 0;
        } else if (facing == "east") {
            direction = 1;
        } else if (facing == "south") {
            direction = 2;
        } else {
            direction = 3;
        }
        states->fValue.emplace("direction", Int(direction));
        auto toggle = block.property("powered", "false") == "true";
        states->fValue.emplace("toggle_bit", Bool(toggle));
        auto attachment = block.property("attachment", "floor");
        states->fValue.emplace("attachment", String(Attachment(attachment)));
        return Complete(tag, block, states, {"facing", "attachment", "powered"});
    }

    static BlockDataType Lantern(Block const& block) {
        using namespace std;
        using namespace props;
        auto tag = New(block.fName, true);
        auto hanging = block.property("hanging", "false") == "true";
        auto states = States();
        states->fValue.emplace("hanging", Bool(hanging));
        return Complete(tag, block, states, {"hanging"});
    }

    static BlockDataType Barrel(Block const& block) {
        using namespace std;
        using namespace props;
        auto tag = New("barrel");
        auto states = States();
        auto facing = FacingDirectionFromFacing(block);
        states->fValue.emplace("facing_direction", Int(facing));
        auto open = block.property("open", "false") == "true";
        states->fValue.emplace("open_bit", Bool(open));
        return Complete(tag, block, states, {"facing", "open"});
    }

    static BlockDataType Grindstone(Block const& block) {
        using namespace std;
        using namespace props;
        auto tag = New("grindstone");
        auto states = States();
        auto direction = DirectionFromFacing(block);
        states->fValue.emplace("direction", Int(direction));
        auto face = block.property("face", "wall");
        string attachment;
        if (face == "wall") {
            attachment = "side";
        } else if (face == "floor") {
            attachment = "standing";
        } else {
            attachment = "hanging";
        }
        states->fValue.emplace("attachment", String(attachment));
        return Complete(tag, block, states, {"facing", "face"});
    }

    static ConverterFunction RenameFaced(std::string const& name) {
        using namespace std;
        using namespace props;
        return [=](Block const& block) {
            auto tag = New(name);
            auto states = States();
            auto facing = FacingDirectionFromFacing(block);
            states->fValue.emplace("facing_direction", Int(facing));
            return Complete(tag, block, states, {"facing"});
        };
    }

    static BlockDataType WallBanner(Block const& block) {
        using namespace std;
        using namespace props;
        auto tag = New("wall_banner");
        auto states = States();
        auto direction = FacingDirectionFromFacing(block);
        states->fValue.emplace("facing_direction", Int(direction));
        return Complete(tag, block, states, {"facing"});
    }

    static BlockDataType Banner(Block const& block) {
        using namespace std;
        using namespace props;
        auto tag = New("standing_banner");
        auto states = States();
        auto rotation = block.property("rotation", "0");
        states->fValue.emplace("ground_sign_direction", Int(stoi(rotation)));
        return Complete(tag, block, states, {"rotation"});
    }

    static BlockDataType SkeletonSkull(Block const& block) {
        using namespace std;
        using namespace props;
        auto tag = New("skull");
        auto states = States();
        states->fValue.emplace("facing_direction", Int(1));
        states->fValue.emplace("no_drop_bit", Bool(false));
        return Complete(tag, block, states, {"rotation"});
    }

    static BlockDataType PottedFlowerPot(Block const& block) {
        using namespace std;
        using namespace props;
        auto tag = New("flower_pot");
        auto states = States();
        states->fValue.emplace("update_bit", Bool(true));
        return Complete(tag, block, states);
    }

    static BlockDataType Bed(Block const& block) {
        using namespace std;
        using namespace props;
        auto tag = New("bed");
        auto states = States();
        auto direction = DirectionFromFacing(block);
        states->fValue.emplace("direction", Int(direction));
        auto head = block.property("part", "foot") == "head";
        states->fValue.emplace("head_piece_bit", Bool(head));
        auto occupied = block.property("occupied", "false") == "true";
        states->fValue.emplace("occupied_bit", Bool(occupied));
        return Complete(tag, block, states, {"facing", "part", "occupied"});
    }

    static ConverterFunction WallSign(std::optional<std::string> prefix = std::nullopt) {
        using namespace std;
        using namespace props;
        string name = prefix ? *prefix + "_wall_sign" : "wall_sign";
        return [=](Block const& block) {
            auto tag = New(name);
            auto direction = FacingDirectionFromFacing(block);
            auto states = States();
            states->fValue.emplace("facing_direction", Int(direction));
            return Complete(tag, block, states, {"facing"});
        };
    }

    static ConverterFunction Sign(std::optional<std::string> prefix = std::nullopt) {
        using namespace std;
        using namespace props;
        string name = prefix ? *prefix + "_standing_sign" : "standing_sign";
        return [=](Block const& block) {
            auto tag = New(name);
            auto rotation = block.property("rotation", "0");
            auto states = States();
            states->fValue.emplace("ground_sign_direction", Int(stoi(rotation)));
            return Complete(tag, block, states, {"rotation"});
        };
    }

    static ConverterFunction CoralFan(std::string const& type, bool dead) {
        using namespace std;
        using namespace props;
        auto tag = New(dead ? "coral_fan_dead" : "coral_fan");
        return [=](Block const& block) {
            auto states = States();
            states->fValue.emplace("coral_color", String(type));
            states->fValue.emplace("coral_fan_direction", Int(0));
            return Complete(tag, block, states);
        };
    }

    static ConverterFunction Coral(std::string const& type, bool dead) {
        using namespace std;
        using namespace props;
        return [=](Block const& block) {
            auto tag = New("coral");
            auto states = States();
            states->fValue.emplace("coral_color", String(type));
            states->fValue.emplace("dead_bit", Bool(dead));
            return Complete(tag, block, states);
        };
    }

    static BlockDataType SilverGlazedTerracotta(Block const& block) {
        using namespace std;
        using namespace props;
        auto tag = New("silver_glazed_terracotta");
        auto direction = FacingDirectionFromFacing(block);
        auto states = States();
        states->fValue.emplace("facing_direction", Int(direction));
        return Complete(tag, block, states, {"facing"});
    }

    static ConverterFunction Anvil(std::string const& damage) {
        using namespace props;
        return [=](Block const& block) {
            auto tag = New("anvil");
            auto states = States();
            states->fValue.emplace("damage", String(damage));
            auto direction = DirectionFromFacing(block);
            states->fValue.emplace("direction", Int(direction));
            return Complete(tag, block, states, {"facing"});
        };
    }

    static ConverterFunction StainedGlassPane(std::string const& color) {
        return Subtype("stained_glass_pane", "color", color);
    }

    static ConverterFunction Carpet(std::string const& color) {
        return Subtype("carpet", "color", color);
    }

    static std::string WallConnectionType(std::string const& type) {
        if (type == "low") {
            return "short";
        } else if (type == "tall") {
            return "tall";
        } else {
            return "none";
        }
    }

    static BlockDataType WallNT(Block const& block) {
        using namespace props;
        auto tag = New(block.fName, true);
        auto states = States();
        auto east = WallConnectionType(block.property("east", "none"));
        auto north = WallConnectionType(block.property("north", "none"));
        auto south = WallConnectionType(block.property("south", "none"));
        auto west = WallConnectionType(block.property("west", "none"));
        states->fValue.emplace("wall_connection_type_east", String(east));
        states->fValue.emplace("wall_connection_type_north", String(north));
        states->fValue.emplace("wall_connection_type_south", String(south));
        states->fValue.emplace("wall_connection_type_west", String(west));
        states->fValue.emplace("wall_post_bit", Bool(false));
        return Complete(tag, block, states, {"east", "north", "south", "west"});
    }

    static ConverterFunction Wall(std::string const& type) {
        using namespace props;
        return [=](Block const& block) {
            auto tag = New("cobblestone_wall");
            auto states = States();
            states->fValue.emplace("wall_block_type", String(type));
            auto east = WallConnectionType(block.property("east", "none"));
            auto north = WallConnectionType(block.property("north", "none"));
            auto south = WallConnectionType(block.property("south", "none"));
            auto west = WallConnectionType(block.property("west", "none"));
            states->fValue.emplace("wall_connection_type_east", String(east));
            states->fValue.emplace("wall_connection_type_north", String(north));
            states->fValue.emplace("wall_connection_type_south", String(south));
            states->fValue.emplace("wall_connection_type_west", String(west));
            return Complete(tag, block, states, {"east", "north", "south", "west"});
        };
    }

    static ConverterFunction ShulkerBox(std::string const& color) {
        return Subtype("shulker_box", "color", color);
    }

    static BlockDataType EndPortalFrame(Block const& block) {
        using namespace std;
        using namespace props;
        auto tag = New("end_portal_frame");
        auto direction = DirectionFromFacing(block);
        auto eye = block.property("eye", "false") == "true";
        auto states = States();
        states->fValue.emplace("direction", Int(direction));
        states->fValue.emplace("end_portal_eye_bit", Bool(eye));
        return Complete(tag, block, states, {"facing", "eye"});
    }

    static ConverterFunction Farmland() {
        using namespace std;
        static map<string, string> const moisture = {
            {"moisture", "moisturized_amount"},
        };
        return Rename(nullopt, moisture);
    }
    
    static ConverterFunction AnyMushroomBlock(std::string const& name, bool stem) {
        using namespace std;
        using namespace props;
        return [=](Block const& block) {
            auto tag = New(name);
            auto up = block.property("up", "false") == "true";
            auto down = block.property("down", "false") == "true";
            auto north = block.property("north", "false") == "true";
            auto east = block.property("east", "false") == "true";
            auto south = block.property("south", "false") == "true";
            auto west = block.property("west", "false") == "true";
            int32_t bits = stem ? 15 : 14;
            if (!up && !down && !north && !east && !south && !west) {
                bits = 0;
            } else if (up && west && north && !down && !east && !south) {
                bits = 1;
            } else if (up && north && !down && !east && !south && !west) {
                bits = 2;
            } else if (up && north && east && !down && !south && !west) {
                bits = 3;
            } else if (up && west && !down && !north && !east && !south) {
                bits = 4;
            } else if (up && !down && !north && !east && !south && !west) {
                bits = 5;
            } else if (up && east && !down && !north && !south && !west) {
                bits = 6;
            } else if (up && south && west && !down && !north && !east) {
                bits = 7;
            } else if (up && south && !down && !north && !east && !west) {
                bits = 8;
            } else if (up && east && south && !down && !north && !west) {
                bits = 9;
            } else if (north && east && south && west && !up && !down && stem) {
                bits = 10;
            } else if (up && down && north && east && south && west) {
                if (stem) {
                    bits = 15;
                } else {
                    bits = 14;
                }
            }
            auto states = States();
            states->fValue.emplace("huge_mushroom_bits", Int(bits));
            return Complete(tag, block, states, {"up", "down", "north", "east", "south", "west"});
        };
    }

    static std::string TorchFacingDirectionFromFacing(std::string const& facing) {
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

    static ConverterFunction AnyTorch(std::string const& prefix) {
        using namespace std;
        using namespace props;
        string name = prefix + "torch";
        return [=](Block const& block) {
            auto tag = New(name);
            auto states = States();
            states->fValue.emplace("torch_facing_direction", String("top"));
            return Complete(tag, block, states);
        };
    }

    static ConverterFunction AnyWallTorch(std::string const& prefix) {
        using namespace std;
        using namespace props;
        string name = prefix + "torch";
        return [=](Block const& block) {
            auto tag = New(name);
            auto states = States();
            auto facing = block.property("facing", "north");
            auto direction = TorchFacingDirectionFromFacing(facing);
            states->fValue.emplace("torch_facing_direction", String(direction));
            return Complete(tag, block, states, { "facing" });
        };
    }

    static ConverterFunction InfestedStone(std::string const& type) {
        return Subtype("monster_egg", "monster_egg_stone_type", type);
    }

    static ConverterFunction Fence(std::string const& type) {
        return Subtype("fence", "wood_type", type);
    }

    static BlockDataType EndRod(Block const& block) {
        using namespace std;
        using namespace props;
        auto tag = New("end_rod");
        auto states = States();
        auto facing = block.property("facing", "up");
        int32_t direction = 1;
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
        states->fValue.emplace("facing_direction", Int(direction));
        return Complete(tag, block, states, {"facing"});
    }

    static BlockDataType SnowLayer(Block const& block) {
        using namespace std;
        using namespace props;
        auto tag = New("snow_layer");
        auto states = States();
        auto layers = stoi(block.property("layers", "1"));
        states->fValue.emplace("height", Int(int32_t(layers - 1)));
        states->fValue.emplace("covered_bit", Bool(false));
        return Complete(tag, block, states, {"layers"});
    }

    static ConverterFunction CoralBlock(std::string const& color, bool dead) {
        using namespace std;
        using namespace props;
        return [=](Block const& block) {
            auto tag = New("coral_block");
            auto states = States();
            states->fValue.emplace("coral_color", String(color));
            states->fValue.emplace("dead_bit", Bool(dead));
            return Complete(tag, block, states);
        };
    }

    static ConverterFunction Sapling(std::string const& type) {
        using namespace std;
        using namespace props;
        return [=](Block const& block) {
            auto tag = New("sapling");
            auto states = States();
            states->fValue.emplace("sapling_type", String(type));
            auto stage = block.property("stage", "0");
            states->fValue.emplace("age_bit", Bool(stage == "1"));
            return Complete(tag, block, states, {"stage"});
        };
    }

    static ConverterFunction StoneBrick(std::string const& type) {
        return Subtype("stonebrick", "stone_brick_type", type);
    }

    static ConverterFunction Prismarine(std::string const& type) {
        return Subtype("prismarine", "prismarine_block_type", type);
    }

    static ConverterFunction Terracotta(std::string const& color) {
        return Subtype("stained_hardened_clay", "color", color);
    }

    static ConverterFunction Concrete(std::string const& color) {
        return Subtype("concrete", "color", color);
    }

    static ConverterFunction ConcretePowder(std::string const& color) {
        return Subtype("concretePowder", "color", color);
    }

    static ConverterFunction StainedGlass(std::string const& color) {
        return Subtype("stained_glass", "color", color);
    }

    static BlockDataType LitPumpkin(Block const& block) {
        using namespace std;
        using namespace props;
        auto tag = New("lit_pumpkin");
        auto states = States();
        auto direction = DirectionFromFacing(block);
        states->fValue.emplace("direction", Int(direction));
        return Complete(tag, block, states, { "facing" });
    }

    static int32_t DirectionFromFacing(Block const& block) {
        auto facing = block.property("facing", "north");
        if (facing == "south") {
            return 0;
        } else if (facing == "east") {
            return 3;
        } else if (facing == "west") {
            return 1;
        } else {
            return 2;
        }
    }

    static int32_t FacingDirectionFromFacing(Block const& block) {
        auto facing = block.property("facing", "north");
        if (facing == "east") {
            return 5;
        } else if (facing == "south") {
            return 3;
        } else if (facing == "west") {
            return 4;
        } else if (facing == "north") {
            return 2;
        } else if (facing == "up") {
            return 1;
        } else {
            return 0;
        }
    }

    static BlockDataType FacingToFacingDirection(Block const& block) {
        using namespace std;
        using namespace props;
        auto tag = New(block.fName, true);
        auto states = States();
        auto facing = block.property("facing", "north");
        int32_t direction = FacingDirectionFromFacing(block);
        states->fValue.emplace("facing_direction", Int(direction));
        return Complete(tag, block, states, { "facing" });
    }

    static BlockDataType FacingToDirection(Block const& block) {
        using namespace std;
        using namespace props;
        auto tag = New(block.fName, true);
        auto states = States();
        auto direction = DirectionFromFacing(block);
        states->fValue.emplace("direction", Int(direction));
        return Complete(tag, block, states, { "facing" });
    }

    static ConverterFunction PurpurBlock(std::string const& type) {
        return Subtype("purpur_block", "chisel_type", type);
    }

    static ConverterFunction Planks(std::string const& type) {
        return Subtype("planks", "wood_type", type);
    }

    static ConverterFunction Sand(std::string const& type) {
        return Subtype("sand", "sand_type", type);
    }

    static ConverterFunction QuartzBlock(std::string const& type) {
        using namespace std;
        using namespace props;
        return [=](Block const& block) {
            auto tag = New("quartz_block");
            auto states = States();
            auto axis = block.property("axis", "y");
            states->fValue.emplace("pillar_axis", String(axis));
            states->fValue.emplace("chisel_type", String(type));
            return Complete(tag, block, states);
        };
    }

    static ConverterFunction Wool(std::string const& color) {
        return Subtype("wool", "color", color);
    }

    static ConverterFunction RedSandstone(std::string const& type) {
        return Subtype("red_sandstone", "sand_stone_type", type);
    }

    static ConverterFunction Sandstone(std::string const& type) {
        return Subtype("sandstone", "sand_stone_type", type);
    }

    static ConverterFunction Sponge(std::string const& type) {
        return Subtype("sponge", "sponge_type", type);
    }

    static ConverterFunction Stairs(std::optional<std::string> name = std::nullopt) {
        using namespace std;
        using namespace props;
        return [=](Block const& block) -> BlockDataType {
            string n = name ? "minecraft:"s + *name : block.fName;
            auto tag = New(n, true);
            auto states = States();

            auto facing = block.property("facing", "north");
            int32_t direction = 0;
            if (facing == "east") {
                direction = 0;
            } else if (facing == "south") {
                direction = 2;
            } else if (facing == "north") {
                direction = 3;
            } else if (facing == "west") {
                direction = 1;
            }
            states->fValue.emplace("weirdo_direction", Int(direction));

            auto half = block.property("half", "bottom");
            states->fValue.emplace("upside_down_bit", Bool(half == "top"));

            return Complete(tag, block, states, { "facing", "half", "shape" });
        };
    }

    static BlockDataType Vine(Block const& block) {
        using namespace std;
        using namespace props;
        auto tag = New("vine");
        auto east = block.property("east", "false") == "true";
        auto north = block.property("north", "false") == "true";
        auto south = block.property("south", "false") == "true";
        auto up = block.property("up", "false") == "true";
        auto west = block.property("west", "false") == "true";
        int32_t direction = 0;
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
        auto states = States();
        states->fValue.emplace("vine_direction_bits", Int(direction));
        return Complete(tag, block, states, { "up", "east", "west", "north", "south" });
    }

    static ConverterFunction NetherVines(std::string const& type, int32_t age = -1) {
        using namespace std;
        using namespace props;
        string name = type + "_vines";
        return [=](Block const& block) -> BlockDataType {
            auto tag = New(name);
            auto states = States();
            int32_t a = age;
            if (a < 0) {
                auto ageString = block.property("age", "0");
                a = stoi(ageString);
            }
            states->fValue.emplace(type + "_vines_age", Int(a));
            return Complete(tag, block, states, { "age" });
        };
    }

    static ConverterFunction Liquid(std::string const& type) {
        using namespace std;
        using namespace props;
        return [=](Block const& block) -> BlockDataType {
            auto levelString = block.property("level", "0");
            auto level = stoi(levelString);
            BlockDataType tag;
            if (level == 0) {
                tag = New(type);
            } else {
                tag = New("flowing_"s + type);
            }
            auto states = States();
            states->fValue.emplace("liquid_depth", Int(level));
            return Complete(tag, block, states, { "level" });
        };
    }

    static ConverterFunction Kelp(int32_t age = -1) {
        using namespace std;
        using namespace props;
        return [=](Block const& block) -> BlockDataType {
            auto tag = New("kelp");
            auto states = States();
            int32_t a = age;
            if (age < 0) {
                auto ageString = block.property("age", "0");
                a = stoi(ageString);
            }
            states->fValue.emplace("kelp_age", Int(a));
            return Complete(tag, block, states, {"age"});
        };
    }

    static BlockDataType TallSeagrass(Block const& block) {
        using namespace std;
        using namespace props;
        auto tag = New("seagrass");
        auto states = States();
        auto half = block.property("half", "bottom");
        string type = half == "bottom" ? "double_bot" : "double_top";
        states->fValue.emplace("sea_grass_type", String(type));
        return Complete(tag, block, states, {"half"});
    }

    static BlockDataType Seagrass(Block const& block) {
        using namespace std;
        using namespace props;
        auto tag = New("seagrass");
        auto states = States();
        states->fValue.emplace("sea_grass_type", String("default"));
        return Complete(tag, block, states);
    }

    static ConverterFunction RedFlower(std::string const& type) {
        using namespace props;
        return [=](Block const& block) -> BlockDataType {
            auto tag = New("red_flower");
            auto states = States();
            states->fValue.emplace("flower_type", String(type));
            return Complete(tag, block, states);
        };
    }
    
    static BlockDataType SeaPickle(Block const& block) {
        using namespace std;
        using namespace props;
        auto tag = New("sea_pickle");
        auto states = States();
        auto waterlogged = block.property("waterlogged", "false");
        states->fValue.emplace("dead_bit", Bool(waterlogged == "false"));
        auto pickles = block.property("pickles", "1");
        auto cluster = (min)((max)(stoi(pickles), 1), 4) - 1;
        states->fValue.emplace("cluster_count", Int(cluster));
        return Complete(tag, block, states, {"pickles"});
    }
    
    static ConverterFunction DoublePlant(std::string const& type) {
        using namespace props;
        return [=](Block const& block) -> BlockDataType {
            auto tag = New("double_plant");
            auto states = States();
            states->fValue.emplace("double_plant_type", String(type));
            auto half = block.property("half", "lower");
            states->fValue.emplace("upper_block_bit", Bool(half == "upper"));
            return Complete(tag, block, states);
        };
    }
    
    static ConverterFunction TallGrass(std::string const& type) {
        using namespace props;
        return [=](Block const& block) -> BlockDataType {
            auto tag = New("tallgrass");
            auto states = States();
            states->fValue.emplace("tall_grass_type", String(type));
            return Complete(tag, block, states);
        };
    }
    
    static BlockDataType Identity(mcfile::Block const& block) {
        using namespace std;

        auto tag = New(block.fName, true);
        auto states = States();
        return Complete(tag, block, states);
    }

    static ConverterFunction Subtype(std::string const& name, std::string const& subtypeTitle, std::string const& subtype) {
        using namespace std;
        using namespace props;

        return [=](Block const& block) -> BlockDataType {
            auto tag = New(name);
            auto states = States();
            states->fValue.emplace(subtypeTitle, String(subtype));
            return Complete(tag, block, states, {subtypeTitle});
        };
    }

    static ConverterFunction Stone(std::string const& stoneType) {
        return Subtype("stone", "stone_type", stoneType);
    }

    static ConverterFunction Dirt(std::string const& dirtType) {
        return Subtype("dirt", "dirt_type", dirtType);
    }

    static ConverterFunction Rename(std::optional<std::string> to = std::nullopt , std::optional<std::map<std::string, std::string>> properties = std::nullopt) {
        using namespace std;
        using namespace props;

        set<string> ignore;
        if (properties) {
            for_each(properties->begin(), properties->end(), [&ignore](auto& it) {
                ignore.insert(it.first);
            });
        }

        return [=](Block const& block) -> BlockDataType {
            string name = to ? ("minecraft:"s + *to) : block.fName;
            auto tag = New(name, true);
            auto states = States();
            if (properties) {
                for_each(properties->begin(), properties->end(), [&block, &states](auto& it) {
                    auto found = block.fProperties.find(it.first);
                    if (found == block.fProperties.end()) {
                        return;
                    }
                    states->fValue.emplace(it.second, props::String(found->second));
                });
            }
            return Complete(tag, block, states, ignore);
        };
    }

    static BlockDataType Complete(BlockDataType tag, Block const& block, StatesType states) {
        MergeProperties(block, *states, std::nullopt);
        tag->fValue.emplace("states", states);
        return tag;
    }

    static BlockDataType Complete(BlockDataType tag, Block const& block, StatesType states, std::set<std::string> const& ignore) {
        MergeProperties(block, *states, ignore);
        tag->fValue.emplace("states", states);
        return tag;
    }

    static void MergeProperties(mcfile::Block const& block, mcfile::nbt::CompoundTag& states, std::optional<std::set<std::string>> ignore) {
        for (auto it = block.fProperties.begin(); it != block.fProperties.end(); it++) {
            auto const& name = it->first;
            if (name == "waterlogged") {
                continue;
            }
            if (ignore && !ignore->empty()) {
                auto found = ignore->find(name);
                if (found != ignore->end()) {
                    continue;
                }
            }
#ifdef _DEBUG
            std::cout << "Warning: Unhandled property " << block.fName << "[" << it->first << "=" << it->second << "]" << std::endl;
#endif
        }
    }

    static BlockDataType New(std::string const& name, bool nameIsFull = false) {
        using namespace std;
        using namespace mcfile::nbt;
        using namespace props;
        auto tag = make_shared<CompoundTag>();
        string fullName = nameIsFull ? name : "minecraft:"s + name;
        tag->fValue.emplace("name", String(fullName));
        tag->fValue.emplace("version", Int(kBlockDataVersion));
        return tag;
    }

    static StatesType States() {
        return std::make_shared<mcfile::nbt::CompoundTag>();
    }
    
    static ConverterFunction Log(std::string const& type) {
        using namespace std;
        using namespace props;

        return [type](Block const& block) -> BlockDataType {
            auto tag = New("log");
            auto states = States();
            states->fValue.emplace("old_log_type", String(type));
            auto axis = block.property("axis", "y");
            states->fValue.emplace("pillar_axis", String(axis));
            return Complete(tag, block, states, {"axis", "pillar_axis", "old_log_type"});
        };
    }

    static ConverterFunction Log2(std::string const& type) {
        using namespace std;
        using namespace props;

        return [=](Block const& block) -> BlockDataType {
            auto tag = New("log2");
            auto states = States();
            states->fValue.emplace("new_log_type", String(type));
            string axis = block.property("axis", "y");
            states->fValue.emplace("pillar_axis", String(axis));
            return Complete(tag, block, states, {"axis", "pillar_axis", "new_log_type"});
        };
    }

    static ConverterFunction Wood(std::string const& type, bool stripped) {
        using namespace std;
        using namespace props;

        return [=](Block const& block) -> BlockDataType {
            auto tag = New("wood");
            auto states = States();
            auto axis = block.property("axis", "y");
            states->fValue.emplace("pillar_axis", String(axis));
            states->fValue.emplace("wood_type", String(type));
            states->fValue.emplace("stripped_bit", Bool(stripped));
            return Complete(tag, block, states, {"axis"});
        };
    }

    static ConverterFunction Leaves(std::string const& type) {
        using namespace std;
        using namespace props;

        return [type](Block const& block) -> BlockDataType {
            auto tag = New("leaves");
            auto states = States();
            states->fValue.emplace("old_leaf_type", String(type));

            auto persistent = block.property("persistent", "false");
            bool persistentV = persistent == "true";
            states->fValue.emplace("persistent_bit", Bool(persistentV));

            auto distance = block.property("distance", "7");
            int distanceV = stoi(distance);
            states->fValue.emplace("update_bit", Bool(distanceV > 4));

            return Complete(tag, block, states, {"persistent", "distance"});
        };
    }

    static ConverterFunction Leaves2(std::string const& type) {
        using namespace std;
        using namespace props;

        return [type](Block const& block) -> BlockDataType {
            auto tag = New("leaves2");
            auto states = States();
            states->fValue.emplace("new_leaf_type", String(type));

            auto persistent = block.property("persistent", "false");
            bool persistentV = persistent == "true";
            states->fValue.emplace("persistent_bit", Bool(persistentV));

            auto distance = block.property("distance", "7");
            int distanceV = stoi(distance);
            states->fValue.emplace("update_bit", Bool(distanceV > 4));

            return Complete(tag, block, states, {"persistent", "distance"});
        };
    }

    static ConverterFunction AxisToPillarAxis() {
        using namespace std;
        static map<string, string> const axisToPillarAxis = {
            {"axis", "pillar_axis"}
        };
        return Rename(std::nullopt, axisToPillarAxis);
    }

    static ConverterFunction WoodenSlab(std::string const& type) {
        using namespace std;
        using namespace props;
        return [=](Block const& block) -> BlockDataType {
            auto states = States();
            auto t = block.property("type", "bottom");
            states->fValue.emplace("top_slot_bit", Bool(t == "top"));
            states->fValue.emplace("wood_type", String(type));
            auto tag = t == "double" ? New("double_wooden_slab") : New("wooden_slab");
            return Complete(tag, block, states, {"type"});
        };
    }

    static ConverterFunction StoneSlab(std::string const& type) {
        return StoneSlabNumbered("", type);
    }

    static ConverterFunction StoneSlab2(std::string const& type) {
        return StoneSlabNumbered("2", type);
    }

    static ConverterFunction StoneSlab3(std::string const& type) {
        return StoneSlabNumbered("3", type);
    }

    static ConverterFunction StoneSlab4(std::string const& type) {
        return StoneSlabNumbered("4", type);
    }

    static ConverterFunction StoneSlabNumbered(std::string const& number, std::string const& type) {
        using namespace std;
        using namespace props;
        return [=](Block const& block) -> BlockDataType {
            auto states = States();
            auto t = block.property("type", "bottom");
            states->fValue.emplace("top_slot_bit", Bool(t == "top"));
            auto typeKey = number.empty() ? "stone_slab_type" : "stone_slab_type_" + number;
            states->fValue.emplace(typeKey, String(type));
            auto tag = t == "double" ? New("double_stone_slab" + number) : New("stone_slab" + number);
            return Complete(tag, block, states, {"type"});
        };
    }

    static ConverterFunction StoneSlabNT(std::string const& doubledName) {
        using namespace std;
        using namespace props;
        return [=](Block const& block) -> BlockDataType {
            auto states = States();
            auto t = block.property("type", "bottom");
            states->fValue.emplace("top_slot_bit", Bool(t == "top"));
            auto tag = t == "double" ? New(doubledName) : New(block.fName, true);
            return Complete(tag, block, states, {"type"});
        };
    }

private:
    static int32_t const kBlockDataVersion = 17825808;
};

}
