#pragma once

namespace j2b {

class BlockData {
public:
    static std::shared_ptr<mcfile::nbt::CompoundTag> From(std::shared_ptr<mcfile::Block const> const& block) {
        using namespace std;
        using namespace props;
        using namespace mcfile::nbt;

        static unordered_map<string, ConverterFunction> const* const converterTable = PrepareConverterTable();

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

    static std::unordered_map<std::string, ConverterFunction>* PrepareConverterTable() {
        using namespace std;
        using TableType = unordered_map<string, ConverterFunction>;
        TableType* base = new TableType({
            {"stone", Stone("stone")},
            {"granite", Stone("granite")},
            {"polished_granite", Stone("granite_smooth")},
            {"andesite", Stone("andesite")},
            {"polished_andesite", Stone("andesite_smooth")},
            {"diorite", Stone("diorite")},
            {"polished_diorite", Stone("diorite_smooth")},
            {"dirt", Dirt("normal")},
            {"coarse_dirt", Dirt("coarse")},
            {"grass_block", Rename("grass")},
            {"oak_log", Log("oak")},
            {"spruce_log", Log("spruce")},
            {"birch_log", Log("birch")},
            {"jungle_log", Log("jungle")},
            {"acacia_log", Log2("acacia")},
            {"dark_oak_log", Log2("dark_oak")},
            {"oak_wood", Wood("oak", false)},
            {"spruce_wood", Wood("spruce", false)},
            {"birch_wood", Wood("birch", false)},
            {"acacia_wood", Wood("acacia", false)},
            {"jungle_wood", Wood("jungle", false)},
            {"dark_oak_wood", Wood("dark_oak", false)},
            {"stripped_oak_wood", Wood("oak", true)},
            {"stripped_spruce_wood", Wood("spruce", true)},
            {"stripped_birch_wood", Wood("birch", true)},
            {"stripped_acacia_wood", Wood("acacia", true)},
            {"stripped_jungle_wood", Wood("jungle", true)},
            {"stripped_dark_oak_wood", Wood("dark_oak", true)},
            {"oak_leaves", Leaves("oak")},
            {"spruce_leaves", Leaves("spruce")},
            {"birch_leaves", Leaves("birch")},
            {"jungle_leaves", Leaves("jungle")},
            {"acacia_leaves", Leaves2("acacia")},
            {"dark_oak_leaves", Leaves2("dark_oak")},
            {"crimson_hyphae", AxisToPillarAxis()},
            {"warped_hyphae", AxisToPillarAxis()},
            {"stripped_crimson_hyphae", AxisToPillarAxis()},
            {"stripped_warped_hyphae", AxisToPillarAxis()},
            {"oak_slab", WoodenSlab("oak")},
            {"birch_slab", WoodenSlab("birch")},
            {"spruce_slab", WoodenSlab("spruce")},
            {"jungle_slab", WoodenSlab("jungle")},
            {"acacia_slab", WoodenSlab("acacia")},
            {"dark_oak_slab", WoodenSlab("dark_oak")},
            {"petrified_oak_slab", WoodenSlab("oak")},
            {"stone_slab", StoneSlab4("stone")},
            {"granite_slab", StoneSlab3("granite")},
            {"andesite_slab", StoneSlab3("andesite")},
            {"diorite_slab", StoneSlab3("diorite")},
            {"cobblestone_slab", StoneSlab("cobblestone")},
            {"stone_brick_slab", StoneSlab("stone_brick")},
            {"brick_slab", StoneSlab("brick")},
            {"sandstone_slab", StoneSlab("sandstone")},
            {"smooth_sandstone_slab", StoneSlab2("smooth_sandstone")},
            {"smooth_stone_slab", StoneSlab("smooth_stone")},
            {"nether_brick_slab", StoneSlab("nether_brick")},
            {"quartz_slab", StoneSlab("quartz")},
            {"smooth_quartz_slab", StoneSlab4("smooth_quartz")},
            {"red_sandstone_slab", StoneSlab2("red_sandstone")},
            {"smooth_red_sandstone_slab", StoneSlab3("smooth_red_sandstone")},
            {"cut_red_sandstone_slab", StoneSlab4("cut_red_sandstone")},
            {"mossy_cobblestone_slab", StoneSlab2("mossy_cobblestone")},
            {"polished_diorite_slab", StoneSlab3("polished_diorite")},
            {"mossy_stone_brick_slab", StoneSlab4("mossy_stone_brick")},
            {"polished_granite_slab", StoneSlab3("polished_granite")},
            {"dark_prismarine_slab", StoneSlab2("prismarine_dark")},
            {"prismarine_brick_slab", StoneSlab2("prismarine_brick")},
            {"prismarine_slab", StoneSlab2("prismarine_rough")},
            {"purpur_slab", StoneSlab2("purpur")},
            {"cut_sandstone_slab", StoneSlab4("cut_sandstone")},
            {"polished_blackstone_brick_slab", StoneSlabNT("polished_blackstone_brick_double_slab")},
            {"polished_blackstone_slab", StoneSlabNT("polished_blackstone_double_slab")},
            {"blackstone_slab", StoneSlabNT("blackstone_double_slab")},
            {"polished_andesite_slab", StoneSlab3("polished_andesite")},
            {"red_nether_brick_slab", StoneSlab2("red_nether_brick")},
            {"end_stone_brick_slab", StoneSlab3("end_stone_brick")},
            {"warped_slab", StoneSlabNT("warped_double_slab")},
            {"crimson_slab", StoneSlabNT("crimson_double_slab")},
            {"grass", TallGrass("tall")},
            {"tall_grass", DoublePlant("grass")},
            {"large_fern", DoublePlant("fern")},
            {"fern", TallGrass("fern")},
            {"lilac", DoublePlant("syringa")},
            {"rose_bush", DoublePlant("rose")},
            {"peony", DoublePlant("paeonia")},
            {"sunflower", DoublePlant("sunflower")},
            {"dead_bush", Rename("deadbush")},
            {"sea_pickle", SeaPickle},
            {"dandelion", Rename("yellow_flower")},
            {"poppy", RedFlower("poppy")},
            {"blue_orchid", RedFlower("orchid")},
            {"allium", RedFlower("allium")},
            {"azure_bluet", RedFlower("houstonia")},
            {"red_tulip", RedFlower("tulip_red")},
            {"orange_tulip", RedFlower("tulip_orange")},
            {"white_tulip", RedFlower("tulip_white")},
            {"pink_tulip", RedFlower("tulip_pink")},
            {"oxeye_daisy", RedFlower("oxeye")},
            {"cornflower", RedFlower("cornflower")},
            {"lily_of_the_valley", RedFlower("lily_of_the_valley")},
            {"seagrass", Seagrass},
            {"tall_seagrass", TallSeagrass},
            {"kelp", Kelp()},
            {"kelp_plant", Kelp(16)},
            {"water", Liquid("water")},
            {"lava", Liquid("lava")},
            {"weeping_vines_plant", NetherVines("weeping", 25)}, //TODO(kbinani): is 25 correct?
            {"weeping_vines", NetherVines("weeping")},
            {"twisting_vines_plant", NetherVines("twisting", 25)}, //TODO(kbinani): is 25 correct?
            {"twisting_vines", NetherVines("twisting")},
            {"vine", Vine},
            {"cobblestone_stairs", Stairs("stone_stairs")},
            {"stone_stairs", Stairs("normal_stone_stairs")},
            {"end_stone_brick_stairs", Stairs("end_brick_stairs")},
            {"prismarine_brick_stairs", Stairs("prismarine_bricks_stairs")},
            {"oak_stairs", Stairs()},
            {"spruce_stairs", Stairs()},
            {"birch_stairs", Stairs()},
            {"jungle_stairs", Stairs()},
            {"acacia_stairs", Stairs()},
            {"dark_oak_stairs", Stairs()},
            {"crimson_stairs", Stairs()},
            {"warped_stairs", Stairs()},
            {"granite_stairs", Stairs()},
            {"polished_granite_stairs", Stairs()},
            {"diorite_stairs", Stairs()},
            {"polished_diorite_stairs", Stairs()},
            {"andesite_stairs", Stairs()},
            {"polished_andesite_stairs", Stairs()},
            {"cobblestone_stairs", Stairs()},
            {"mossy_cobblestone_stairs", Stairs()},
            {"stone_brick_stairs", Stairs()},
            {"mossy_stone_brick_stairs", Stairs()},
            {"brick_stairs", Stairs()},
            {"end_stone_brick_stairs", Stairs()},
            {"nether_brick_stairs", Stairs()},
            {"red_nether_brick_stairs", Stairs()},
            {"sandstone_stairs", Stairs()},
            {"smooth_sandstone_stairs", Stairs()},
            {"red_sandstone_stairs", Stairs()},
            {"smooth_red_sandstone_stairs", Stairs()},
            {"quartz_stairs", Stairs()},
            {"smooth_quartz_stairs", Stairs()},
            {"purpur_stairs", Stairs()},
            {"prismarine_stairs", Stairs()},
            {"prismarine_brick_stairs", Stairs()},
            {"dark_prismarine_stairs", Stairs()},
            {"blackstone_stairs", Stairs()},
            {"polished_blackstone_stairs", Stairs()},
            {"polished_blackstone_brick_stairs", Stairs()},
            {"sponge", Sponge("dry")},
            {"wet_sponge", Sponge("wet")},
            {"sandstone", Sandstone("default")},
            {"chiseled_sandstone", Sandstone("heiroglyphs")},
            {"cut_sandstone", Sandstone("cut")},
            {"smooth_sandstone", Sandstone("smooth")},
            {"red_sandstone", RedSandstone("default")},
            {"chiseled_red_sandstone", RedSandstone("heiroglyphs")},
            {"cut_red_sandstone", RedSandstone("cut")},
            {"smooth_red_sandstone", RedSandstone("smooth")},
            {"white_wool", Wool("white")},
            {"orange_wool", Wool("orange")},
            {"magenta_wool", Wool("magenta")},
            {"light_blue_wool", Wool("light_blue")},
            {"yellow_wool", Wool("yellow")},
            {"lime_wool", Wool("lime")},
            {"pink_wool", Wool("pink")},
            {"gray_wool", Wool("gray")},
            {"light_gray_wool", Wool("silver")},
            {"cyan_wool", Wool("cyan")},
            {"purple_wool", Wool("purple")},
            {"blue_wool", Wool("blue")},
            {"brown_wool", Wool("brown")},
            {"green_wool", Wool("green")},
            {"red_wool", Wool("red")},
            {"black_wool", Wool("black")},
            {"snow_block", Rename("snow")},
            {"quartz_block", QuartzBlock("default")},
            {"smooth_quartz", QuartzBlock("smooth")},
            {"quartz_pillar", QuartzBlock("lines")},
            {"chiseled_quartz_block", QuartzBlock("chiseled")},
            {"bricks", Rename("brick_block")},
            {"sand", Sand("normal")},
            {"red_sand", Sand("red")},
            {"oak_planks", Planks("oak")},
            {"spruce_planks", Planks("spruce")},
            {"birch_planks", Planks("birch")},
            {"jungle_planks", Planks("jungle")},
            {"acacia_planks", Planks("acacia")},
            {"dark_oak_planks", Planks("dark_oak")},
            {"purpur_block", PurpurBlock("default")},
            {"purpur_pillar", PurpurBlock("lines")},
            {"jack_o_lantern", LitPumpkin},
            {"carved_pumpkin", FacingToDirection},
            {"white_stained_glass", StainedGlass("white")},
            {"orange_stained_glass", StainedGlass("orange")},
            {"magenta_stained_glass", StainedGlass("magenta")},
            {"light_blue_stained_glass", StainedGlass("light_blue")},
            {"yellow_stained_glass", StainedGlass("yellow")},
            {"lime_stained_glass", StainedGlass("lime")},
            {"pink_stained_glass", StainedGlass("pink")},
            {"gray_stained_glass", StainedGlass("gray")},
            {"light_gray_stained_glass", StainedGlass("silver")},
            {"cyan_stained_glass", StainedGlass("cyan")},
            {"purple_stained_glass", StainedGlass("purple")},
            {"blue_stained_glass", StainedGlass("blue")},
            {"brown_stained_glass", StainedGlass("brown")},
            {"green_stained_glass", StainedGlass("green")},
            {"red_stained_glass", StainedGlass("red")},
            {"black_stained_glass", StainedGlass("black")},
            {"white_concrete_powder", ConcretePowder("white")},
            {"orange_concrete_powder", ConcretePowder("orange")},
            {"magenta_concrete_powder", ConcretePowder("magenta")},
            {"light_blue_concrete_powder", ConcretePowder("light_blue")},
            {"yellow_concrete_powder", ConcretePowder("yellow")},
            {"lime_concrete_powder", ConcretePowder("lime")},
            {"pink_concrete_powder", ConcretePowder("pink")},
            {"gray_concrete_powder", ConcretePowder("gray")},
            {"light_gray_concrete_powder", ConcretePowder("silver")},
            {"cyan_concrete_powder", ConcretePowder("cyan")},
            {"purple_concrete_powder", ConcretePowder("purple")},
            {"blue_concrete_powder", ConcretePowder("blue")},
            {"brown_concrete_powder", ConcretePowder("brown")},
            {"green_concrete_powder", ConcretePowder("green")},
            {"red_concrete_powder", ConcretePowder("red")},
            {"black_concrete_powder", ConcretePowder("black")},
            {"white_concrete", Concrete("white")},
            {"orange_concrete", Concrete("orange")},
            {"magenta_concrete", Concrete("magenta")},
            {"light_blue_concrete", Concrete("light_blue")},
            {"yellow_concrete", Concrete("yellow")},
            {"lime_concrete", Concrete("lime")},
            {"pink_concrete", Concrete("pink")},
            {"gray_concrete", Concrete("gray")},
            {"light_gray_concrete", Concrete("silver")},
            {"cyan_concrete", Concrete("cyan")},
            {"purple_concrete", Concrete("purple")},
            {"blue_concrete", Concrete("blue")},
            {"brown_concrete", Concrete("brown")},
            {"green_concrete", Concrete("green")},
            {"red_concrete", Concrete("red")},
            {"black_concrete", Concrete("black")},
            {"white_terracotta", Terracotta("white")},
            {"orange_terracotta", Terracotta("orange")},
            {"magenta_terracotta", Terracotta("magenta")},
            {"light_blue_terracotta", Terracotta("light_blue")},
            {"yellow_terracotta", Terracotta("yellow")},
            {"lime_terracotta", Terracotta("lime")},
            {"pink_terracotta", Terracotta("pink")},
            {"gray_terracotta", Terracotta("gray")},
            {"light_gray_terracotta", Terracotta("silver")},
            {"cyan_terracotta", Terracotta("cyan")},
            {"purple_terracotta", Terracotta("purple")},
            {"blue_terracotta", Terracotta("blue")},
            {"brown_terracotta", Terracotta("brown")},
            {"green_terracotta", Terracotta("green")},
            {"red_terracotta", Terracotta("red")},
            {"black_terracotta", Terracotta("black")},
            {"nether_quartz_ore", Rename("quartz_ore")},
            {"red_nether_bricks", Rename("red_nether_brick")},
            {"magma_block", Rename("magma")},
            {"sea_lantern", Rename("seaLantern")},
            {"prismarine_bricks", Prismarine("bricks")},
            {"dark_prismarine", Prismarine("dark")},
            {"prismarine", Prismarine("default")},
            {"terracotta", Rename("hardened_clay")},
            {"end_stone_bricks", Rename("end_bricks")},
            {"melon", Rename("melon_block")},
            {"chiseled_stone_bricks", StoneBrick("chiseled")},
            {"cracked_stone_bricks", StoneBrick("cracked")},
            {"mossy_stone_bricks", StoneBrick("mossy")},
            {"stone_bricks", StoneBrick("default")},
            {"oak_sapling", Sapling("oak")},
            {"birch_sapling", Sapling("birch")},
            {"jungle_sapling", Sapling("jungle")},
            {"acacia_sapling", Sapling("acacia")},
            {"spruce_sapling", Sapling("spruce")},
            {"dark_oak_sapling", Sapling("dark_oak")},
            {"tube_coral_block", Coral("blue", false)},
            {"brain_coral_block", Coral("pink", false)},
            {"bubble_coral_block", Coral("purple", false)},
            {"fire_coral_block", Coral("red", false)},
            {"horn_coral_block", Coral("yellow", false)},
            {"dead_tube_coral_block", Coral("blue", true)},
            {"dead_brain_coral_block", Coral("pink", true)},
            {"dead_bubble_coral_block", Coral("purple", true)},
            {"dead_fire_coral_block", Coral("red", true)},
            {"dead_horn_coral_block", Coral("yellow", true)},
            {"snow", SnowLayer},
            {"sugar_cane", Rename("reeds")},
            {"end_rod", EndRod},
            {"oak_fence", Fence("oak")},
            {"spruce_fence", Fence("spruce")},
            {"birch_fence", Fence("birch")},
            {"jungle_fence", Fence("jungle")},
            {"acacia_fence", Fence("acacia")},
            {"dark_oak_fence", Fence("dark_oak")},
            {"ladder", FacingToFacingDirection},
            {"chest", FacingToFacingDirection},
            {"furnace", FacingToFacingDirection},
            {"nether_bricks", Rename("nether_brick")},
            {"infested_stone", InfestedStone("stone")},
            {"infested_cobblestone", InfestedStone("cobblestone")},
            {"infested_stone_bricks", InfestedStone("stone_brick")},
            {"infested_mossy_stone_bricks", InfestedStone("mossy_stone_brick")},
            {"infested_cracked_stone_bricks", InfestedStone("cracked_stone_brick")},
            {"infested_chiseled_stone_bricks", InfestedStone("chiseled_stone_brick")},
            {"torch", AnyTorch("")},
            {"wall_torch", AnyWallTorch("")},
            {"soul_torch", AnyTorch("soul_")},
            {"soul_wall_torch", AnyWallTorch("soul_")},
        });
        TableType* table = new TableType();
        for (auto it = base->begin(); it != base->end(); it++) {
            string name = it->first;
            table->emplace(name, it->second);
            table->emplace("minecraft:"s + name, it->second);
        }
        delete base;
        return table;
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
        return [=](Block const& block) {
            auto tag = New(prefix + "torch");
            auto states = States();
            states->fValue.emplace("torch_facing_direction", String("top"));
            return Complete(tag, block, states);
        };
    }

    static ConverterFunction AnyWallTorch(std::string const& prefix) {
        using namespace std;
        using namespace props;
        return [=](Block const& block) {
            auto tag = New(prefix + "torch");
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

    static ConverterFunction Coral(std::string const& color, bool dead) {
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

    static BlockDataType FacingToFacingDirection(Block const& block) {
        using namespace std;
        using namespace props;
        auto tag = New(block.fName, true);
        auto states = States();
        auto facing = block.property("facing", "north");
        int32_t direction = 2;
        if (facing == "east") {
            direction = 5;
        } else if (facing == "south") {
            direction = 3;
        } else if (facing == "west") {
            direction = 4;
        } else if (facing == "north") {
            direction = 2;
        }
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
        return [=](Block const& block) -> BlockDataType {
            auto tag = New(type + "_vines");
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
            using namespace std;
            using namespace props;
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

    static ConverterFunction Rename(std::optional<std::string> to = std::nullopt , std::optional<std::unordered_map<std::string, std::string>> properties = std::nullopt) {
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
            states.fValue.emplace(name, props::String(it->second));
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
        static unordered_map<string, string> const axisToPillarAxis = {
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
