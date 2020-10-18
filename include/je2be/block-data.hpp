#pragma once

namespace j2b {

class BlockData {
public:
    static std::shared_ptr<mcfile::nbt::CompoundTag> From(std::shared_ptr<mcfile::Block const> const& block) {
        using namespace std;
        using namespace props;
        using namespace mcfile::nbt;

        static unordered_map<string, ConverterFunction> const converterTable = {
            {"minecraft:stone", Stone("stone")},
            {"minecraft:granite", Stone("granite")},
            {"minecraft:polished_granite", Stone("granite_smooth")},
            {"minecraft:andesite", Stone("andesite")},
            {"minecraft:polished_andesite", Stone("andesite_smooth")},
            {"minecraft:diorite", Stone("diorite")},
            {"minecraft:polished_diorite", Stone("diorite_smooth")},
            {"minecraft:dirt", Dirt("normal")},
            {"minecraft:coarse_dirt", Dirt("coarse")},
            {"minecraft:grass_block", Rename("grass")},
            {"minecraft:oak_log", Log("oak")},
            {"minecraft:spruce_log", Log("spruce")},
            {"minecraft:birch_log", Log("birch")},
            {"minecraft:jungle_log", Log("jungle")},
            {"minecraft:acacia_log", Log2("acacia")},
            {"minecraft:dark_oak_log", Log2("dark_oak")},
            {"minecraft:oak_wood", Wood("oak", false)},
            {"minecraft:spruce_wood", Wood("spruce", false)},
            {"minecraft:birch_wood", Wood("birch", false)},
            {"minecraft:acacia_wood", Wood("acacia", false)},
            {"minecraft:jungle_wood", Wood("jungle", false)},
            {"minecraft:dark_oak_wood", Wood("dark_oak", false)},
            {"minecraft:stripped_oak_wood", Wood("oak", true)},
            {"minecraft:stripped_spruce_wood", Wood("spruce", true)},
            {"minecraft:stripped_birch_wood", Wood("birch", true)},
            {"minecraft:stripped_acacia_wood", Wood("acacia", true)},
            {"minecraft:stripped_jungle_wood", Wood("jungle", true)},
            {"minecraft:stripped_dark_oak_wood", Wood("dark_oak", true)},
            {"minecraft:oak_leaves", Leaves("oak")},
            {"minecraft:spruce_leaves", Leaves("spruce")},
            {"minecraft:birch_leaves", Leaves("birch")},
            {"minecraft:jungle_leaves", Leaves("jungle")},
            {"minecraft:acacia_leaves", Leaves2("acacia")},
            {"minecraft:dark_oak_leaves", Leaves2("dark_oak")},
            {"minecraft:crimson_hyphae", AxisToPillarAxis()},
            {"minecraft:warped_hyphae", AxisToPillarAxis()},
            {"minecraft:stripped_crimson_hyphae", AxisToPillarAxis()},
            {"minecraft:stripped_warped_hyphae", AxisToPillarAxis()},
            {"minecraft:oak_slab", WoodenSlab("oak")},
            {"minecraft:birch_slab", WoodenSlab("birch")},
            {"minecraft:spruce_slab", WoodenSlab("spruce")},
            {"minecraft:jungle_slab", WoodenSlab("jungle")},
            {"minecraft:acacia_slab", WoodenSlab("acacia")},
            {"minecraft:dark_oak_slab", WoodenSlab("dark_oak")},
            {"minecraft:petrified_oak_slab", WoodenSlab("oak")},
            {"minecraft:stone_slab", StoneSlab4("stone")},
            {"minecraft:granite_slab", StoneSlab3("granite")},
            {"minecraft:andesite_slab", StoneSlab3("andesite")},
            {"minecraft:diorite_slab", StoneSlab3("diorite")},
            {"minecraft:cobblestone_slab", StoneSlab("cobblestone")},
            {"minecraft:stone_brick_slab", StoneSlab("stone_brick")},
            {"minecraft:brick_slab", StoneSlab("brick")},
            {"minecraft:sandstone_slab", StoneSlab("sandstone")},
            {"minecraft:smooth_sandstone_slab", StoneSlab2("smooth_sandstone")},
            {"minecraft:smooth_stone_slab", StoneSlab("smooth_stone")},
            {"minecraft:nether_brick_slab", StoneSlab("nether_brick")},
            {"minecraft:quartz_slab", StoneSlab("quartz")},
            {"minecraft:smooth_quartz_slab", StoneSlab4("smooth_quartz")},
            {"minecraft:red_sandstone_slab", StoneSlab2("red_sandstone")},
            {"minecraft:smooth_red_sandstone_slab", StoneSlab3("smooth_red_sandstone")},
            {"minecraft:cut_red_sandstone_slab", StoneSlab4("cut_red_sandstone")},
            {"minecraft:mossy_cobblestone_slab", StoneSlab2("mossy_cobblestone")},
            {"minecraft:polished_diorite_slab", StoneSlab3("polished_diorite")},
            {"minecraft:mossy_stone_brick_slab", StoneSlab4("mossy_stone_brick")},
            {"minecraft:polished_granite_slab", StoneSlab3("polished_granite")},
            {"minecraft:dark_prismarine_slab", StoneSlab2("prismarine_dark")},
            {"minecraft:prismarine_brick_slab", StoneSlab2("prismarine_brick")},
            {"minecraft:prismarine_slab", StoneSlab2("prismarine_rough")},
            {"minecraft:purpur_slab", StoneSlab2("purpur")},
            {"minecraft:cut_sandstone_slab", StoneSlab4("cut_sandstone")},
            {"minecraft:polished_blackstone_brick_slab", StoneSlabNT("polished_blackstone_brick_double_slab")},
            {"minecraft:polished_blackstone_slab", StoneSlabNT("polished_blackstone_double_slab")},
            {"minecraft:blackstone_slab", StoneSlabNT("blackstone_double_slab")},
            {"minecraft:polished_andesite_slab", StoneSlab3("polished_andesite")},
            {"minecraft:red_nether_brick_slab", StoneSlab2("red_nether_brick")},
            {"minecraft:end_stone_brick_slab", StoneSlab3("end_stone_brick")},
            {"minecraft:warped_slab", StoneSlabNT("warped_double_slab")},
            {"minecraft:crimson_slab", StoneSlabNT("crimson_double_slab")},
            {"minecraft:grass", TallGrass("tall")},
            {"minecraft:tall_grass", DoublePlant("grass")},
            {"minecraft:large_fern", DoublePlant("fern")},
            {"minecraft:fern", TallGrass("fern")},
            {"minecraft:lilac", DoublePlant("syringa")},
            {"minecraft:rose_bush", DoublePlant("rose")},
            {"minecraft:peony", DoublePlant("paeonia")},
            {"minecraft:sunflower", DoublePlant("sunflower")},
            {"minecraft:dead_bush", Rename("deadbush")},
            {"minecraft:sea_pickle", SeaPickle},
            {"minecraft:dandelion", Rename("yellow_flower")},
            {"minecraft:poppy", RedFlower("poppy")},
            {"minecraft:blue_orchid", RedFlower("orchid")},
            {"minecraft:allium", RedFlower("allium")},
            {"minecraft:azure_bluet", RedFlower("houstonia")},
            {"minecraft:red_tulip", RedFlower("tulip_red")},
            {"minecraft:orange_tulip", RedFlower("tulip_orange")},
            {"minecraft:white_tulip", RedFlower("tulip_white")},
            {"minecraft:pink_tulip", RedFlower("tulip_pink")},
            {"minecraft:oxeye_daisy", RedFlower("oxeye")},
            {"minecraft:cornflower", RedFlower("cornflower")},
            {"minecraft:lily_of_the_valley", RedFlower("lily_of_the_valley")},
            {"minecraft:seagrass", Seagrass},
            {"minecraft:tall_seagrass", TallSeagrass},
            {"minecraft:kelp", Kelp()},
            {"minecraft:kelp_plant", Kelp(16)},
            {"minecraft:water", Liquid("water")},
            {"minecraft:lava", Liquid("lava")},
            {"minecraft:weeping_vines_plant", NetherVines("weeping", 25)}, //TODO(kbinani): is 25 correct?
            {"minecraft:weeping_vines", NetherVines("weeping")},
            {"minecraft:twisting_vines_plant", NetherVines("twisting", 25)}, //TODO(kbinani): is 25 correct?
            {"minecraft:twisting_vines", NetherVines("twisting")},
            {"minecraft:vine", Vine},
            {"minecraft:cobblestone_stairs", Stairs("stone_stairs")},
            {"minecraft:stone_stairs", Stairs("normal_stone_stairs")},
            {"minecraft:end_stone_brick_stairs", Stairs("end_brick_stairs")},
            {"minecraft:prismarine_brick_stairs", Stairs("prismarine_bricks_stairs")},
            {"minecraft:oak_stairs", Stairs()},
            {"minecraft:spruce_stairs", Stairs()},
            {"minecraft:birch_stairs", Stairs()},
            {"minecraft:jungle_stairs", Stairs()},
            {"minecraft:acacia_stairs", Stairs()},
            {"minecraft:dark_oak_stairs", Stairs()},
            {"minecraft:crimson_stairs", Stairs()},
            {"minecraft:warped_stairs", Stairs()},
            {"minecraft:granite_stairs", Stairs()},
            {"minecraft:polished_granite_stairs", Stairs()},
            {"minecraft:diorite_stairs", Stairs()},
            {"minecraft:polished_diorite_stairs", Stairs()},
            {"minecraft:andesite_stairs", Stairs()},
            {"minecraft:polished_andesite_stairs", Stairs()},
            {"minecraft:cobblestone_stairs", Stairs()},
            {"minecraft:mossy_cobblestone_stairs", Stairs()},
            {"minecraft:stone_brick_stairs", Stairs()},
            {"minecraft:mossy_stone_brick_stairs", Stairs()},
            {"minecraft:brick_stairs", Stairs()},
            {"minecraft:end_stone_brick_stairs", Stairs()},
            {"minecraft:nether_brick_stairs", Stairs()},
            {"minecraft:red_nether_brick_stairs", Stairs()},
            {"minecraft:sandstone_stairs", Stairs()},
            {"minecraft:smooth_sandstone_stairs", Stairs()},
            {"minecraft:red_sandstone_stairs", Stairs()},
            {"minecraft:smooth_red_sandstone_stairs", Stairs()},
            {"minecraft:quartz_stairs", Stairs()},
            {"minecraft:smooth_quartz_stairs", Stairs()},
            {"minecraft:purpur_stairs", Stairs()},
            {"minecraft:prismarine_stairs", Stairs()},
            {"minecraft:prismarine_brick_stairs", Stairs()},
            {"minecraft:dark_prismarine_stairs", Stairs()},
            {"minecraft:blackstone_stairs", Stairs()},
            {"minecraft:polished_blackstone_stairs", Stairs()},
            {"minecraft:polished_blackstone_brick_stairs", Stairs()},
        };

        auto found = converterTable.find(block->fName);
        if (found == converterTable.end()) {
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

            MergeProperties(block, *states, { "facing", "half", "shape" });
            tag->fValue.emplace("states", states);

            return tag;
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
        MergeProperties(block, *states, { "up", "east", "west", "north", "south" });
        tag->fValue.emplace("states", states);
        return tag;
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
            MergeProperties(block, *states, { "age" });
            tag->fValue.emplace("states", states);
            return tag;
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
            MergeProperties(block, *states, { "level" });
            tag->fValue.emplace("states", states);
            return tag;
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
            MergeProperties(block, *states, {});
            tag->fValue.emplace("states", states);
            return tag;
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
        MergeProperties(block, *states, {});
        tag->fValue.emplace("states", states);
        return tag;
    }

    static BlockDataType Seagrass(Block const& block) {
        using namespace std;
        using namespace props;
        auto tag = New("seagrass");
        auto states = States();
        states->fValue.emplace("sea_grass_type", String("default"));
        MergeProperties(block, *states, {});
        tag->fValue.emplace("states", states);
        return tag;
    }

    static ConverterFunction RedFlower(std::string const& type) {
        using namespace props;
        return [=](Block const& block) -> BlockDataType {
            auto tag = New("red_flower");
            auto states = States();
            states->fValue.emplace("flower_type", String(type));
            MergeProperties(block, *states, {});
            tag->fValue.emplace("states", states);
            return tag;
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
        static set<string> const ignore = {"pickles"};
        MergeProperties(block, *states, ignore);
        tag->fValue.emplace("states", states);
        return tag;
    }
    
    static ConverterFunction DoublePlant(std::string const& type) {
        using namespace props;
        return [=](Block const& block) -> BlockDataType {
            auto tag = New("double_plant");
            auto states = States();
            states->fValue.emplace("double_plant_type", String(type));
            auto half = block.property("half", "lower");
            states->fValue.emplace("upper_block_bit", Bool(half == "upper"));
            MergeProperties(block, *states, {});
            tag->fValue.emplace("states", states);
            return tag;
        };
    }
    
    static ConverterFunction TallGrass(std::string const& type) {
        using namespace props;
        return [=](Block const& block) -> BlockDataType {
            auto tag = New("tallgrass");
            auto states = States();
            states->fValue.emplace("tall_grass_type", String(type));
            MergeProperties(block, *states, {});
            tag->fValue.emplace("states", states);
            return tag;
        };
    }
    
    static BlockDataType Identity(mcfile::Block const& block) {
        using namespace std;

        auto tag = New(block.fName, true);
        auto states = States();
        static set<string> const ignore = {};
        MergeProperties(block, *states, ignore);
        tag->fValue.emplace("states", states);
        return tag;
    }

    static ConverterFunction Subtype(std::string const& name, std::string const& subtypeTitle, std::string const& subtype) {
        using namespace std;
        using namespace props;

        return [=](Block const& block) -> BlockDataType {
            auto tag = New(name);
            auto states = States();
            states->fValue.emplace(subtypeTitle, String(subtype));
            static set<string> const ignore = { subtypeTitle };
            MergeProperties(block, *states, ignore);
            tag->fValue.emplace("states", states);
            return tag;
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
            MergeProperties(block, *states, ignore);
            tag->fValue.emplace("states", states);
            return tag;
        };
    }

    static void MergeProperties(mcfile::Block const& block, mcfile::nbt::CompoundTag& states, std::set<std::string> const& ignore) {
        for (auto it = block.fProperties.begin(); it != block.fProperties.end(); it++) {
            auto const& name = it->first;
            if (!ignore.empty()) {
                auto found = ignore.find(name);
                if (found != ignore.end()) {
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
            static set<string> const ignore = { "axis", "pillar_axis", "old_log_type" };
            MergeProperties(block, *states, ignore);
            tag->fValue.emplace("states", states);
            return tag;
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
            static set<string> const ignore = { "axis", "pillar_axis", "new_log_type" };
            MergeProperties(block, *states, ignore);
            tag->fValue.emplace("states", states);
            return tag;
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
            static set<string> const ignore = { "axis" };
            MergeProperties(block, *states, ignore);
            tag->fValue.emplace("states", states);
            return tag;
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

            static set<string> const ignore = { "persistent", "distance" };
            MergeProperties(block, *states, ignore);
            tag->fValue.emplace("states", states);
            return tag;
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

            static set<string> const ignore = { "persistent", "distance" };
            MergeProperties(block, *states, ignore);
            tag->fValue.emplace("states", states);
            return tag;
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
            static set<string> const ignore = { "type", "waterlogged" };
            MergeProperties(block, *states, ignore);
            auto tag = t == "double" ? New("double_wooden_slab") : New("wooden_slab");
            tag->fValue.emplace("states", states);
            return tag;
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
            static set<string> const ignore = { "type", "waterlogged" };
            MergeProperties(block, *states, ignore);
            auto tag = t == "double" ? New("double_stone_slab" + number) : New("stone_slab" + number);
            tag->fValue.emplace("states", states);
            return tag;
        };
    }

    static ConverterFunction StoneSlabNT(std::string const& doubledName) {
        using namespace std;
        using namespace props;
        return [=](Block const& block) -> BlockDataType {
            auto states = States();
            auto t = block.property("type", "bottom");
            states->fValue.emplace("top_slot_bit", Bool(t == "top"));
            static set<string> const ignore = { "type", "waterlogged" };
            MergeProperties(block, *states, ignore);
            auto tag = t == "double" ? New(doubledName) : New(block.fName, true);
            tag->fValue.emplace("states", states);
            return tag;
        };
    }

private:
    static int32_t const kBlockDataVersion = 17825808;
};

}
