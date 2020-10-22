#pragma once

namespace j2b {

class BlockData {
public:
    static std::shared_ptr<mcfile::nbt::CompoundTag> From(std::shared_ptr<mcfile::Block const> const& block) {
        using namespace std;
        static unique_ptr<unordered_map<string, Converter> const> const table(CreateConverterTable());
        
        auto found = table->find(block->fName);
        if (found == table->end()) {
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

    using BlockDataType = std::shared_ptr<mcfile::nbt::CompoundTag>;
    using StatesType = std::shared_ptr<mcfile::nbt::CompoundTag>;
    using Block = mcfile::Block;

    using PropertyType = std::shared_ptr<mcfile::nbt::Tag>;
    using PropertySpec = std::pair<std::string, PropertyType>;

    using NamingFunction = std::function<std::string(Block const&)>;
    using PropertyMapFunction = std::function<std::pair<std::string, PropertyType>(Block const&)>;

    struct Converter {
        template<class... A>
        Converter(NamingFunction name, A... args)
            : fName(name)
            , fProperties(std::initializer_list<PropertyMapFunction>{args...}) {}
        
        BlockDataType operator() (Block const& block) const {
            using namespace std;
            string name = fName(block);
            auto tag = New(name, true);
            auto states = States();
            for (auto const& p : fProperties) {
                auto kv = p(block);
                states->fValue.insert(kv);
            }
            tag->fValue.emplace("states", states);
            return tag;
        }
        
        NamingFunction const fName;
        std::vector<PropertyMapFunction> fProperties;
    };

    static BlockDataType Identity(mcfile::Block const& block) {
        auto tag = New(block.fName, true);
        auto states = States();
        tag->fValue.emplace("states", states);
        return tag;
    }

    static NamingFunction Fixed(std::string const& name) {
        return [=](Block const&) { return "minecraft:" + name; };
    }
    
    static std::string Same(Block const& block) {
        return block.fName;
    }
    
    static NamingFunction SlabName(std::string const& name, std::string const& tail) {
        return [=](Block const& block) {
            auto t = block.property("type", "bottom");
            return t == "double" ? ("minecraft:double_" + name + "_slab" + tail) : ("minecraft:" + name + "_slab" + tail);
        };
    }
    
    static NamingFunction ChangeWhenDoubleType(std::string const& doubleName) {
        return [=](Block const& block) {
            auto t = block.property("type", "bottom");
            return t == "double" ? "minecraft:" + doubleName : block.fName;
        };
    }
    
    static NamingFunction FlowingLiquidName(std::string const& name) {
        return [=](Block const& block) {
            auto levelString = block.property("level", "0");
            auto level = std::stoi(levelString);
            if (level == 0) {
                return "minecraft:" + name;
            } else {
                return "minecraft:flowing_" + name;
            }
        };
    }
    
    static PropertyMapFunction AddStringProperty(std::string const& name, std::string const& value) {
        return [=](Block const&) { return std::make_pair(name, props::String(value)); };
    }

    static PropertyMapFunction AddBoolProperty(std::string const& name, bool v) {
        return [=](Block const&) { return std::make_pair(name, props::Bool(v)); };
    }
    
    static PropertyMapFunction AddIntProperty(std::string const& name, int32_t v) {
        return [=](Block const&) { return std::make_pair(name, props::Int(v)); };
    }

    static PropertySpec AxisToPillarAxis(Block const& block) {
        auto v = block.property("axis", "y");
        return std::make_pair("pillar_axis", props::String(v));
    }
    
    static PropertySpec PersistentToPersistentBit(Block const& block) {
        auto persistent = block.property("persistent", "false");
        bool persistentV = persistent == "true";
        return std::make_pair("persistent_bit", props::Bool(persistentV));
    }

    static PropertySpec DistanceToUpdateBit(Block const& block) {
        auto distance = block.property("distance", "7");
        int distanceV = std::stoi(distance);
        return std::make_pair("update_bit", props::Bool(distanceV > 4));
    }

    static PropertySpec TypeToTopSlotBit(Block const& block) {
        auto t = block.property("type", "bottom");
        return std::make_pair("top_slot_bit", props::Bool(t == "top"));
    }
    
    static PropertyMapFunction AddStoneSlabType(std::string const& number, std::string const& type) {
        auto typeKey = number.empty() ? "stone_slab_type" : "stone_slab_type_" + number;
        return [=](Block const&) {
            return std::make_pair(typeKey, props::String(type));
        };
    }
    
    static PropertySpec HalfToUpperBlockBit(Block const& block) {
        auto half = block.property("half", "lower");
        return std::make_pair("upper_block_bit", props::Bool(half == "upper"));
    }
    
    static PropertySpec WaterloggedToDeadBit(Block const& block) {
        auto waterlogged = block.property("waterlogged", "false");
        return std::make_pair("dead_bit", props::Bool(waterlogged == "false"));
    }
    
    static PropertySpec PicklesToClusterCount(Block const& block) {
        auto pickles = block.property("pickles", "1");
        auto cluster = std::stoi(pickles) - 1;
        return std::make_pair("cluster_count", props::Int(cluster));
    }

    static PropertySpec HalfToSeagrassType(Block const& block) {
        auto half = block.property("half", "bottom");
        auto type = half == "bottom" ? "double_bot" : "double_top";
        return std::make_pair("sea_grass_type", props::String(type));
    }
    
    static PropertyType Age(Block const& block) {
        auto age = std::stoi(block.property("age", "0"));
        return props::Int(age);
    }
    
    static PropertyMapFunction WithName(std::string const& name, std::function<PropertyType(Block const&)> func) {
        return [=](Block const& block) {
            return std::make_pair(name, func(block));
        };
    }

    static PropertyType Level(Block const& block) {
        auto level = std::stoi(block.property("level", "0"));
        return props::Int(level);
    }
    

    static PropertySpec VineDirectionBits(Block const& block) {
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
        return std::make_pair("vine_direction_bits", props::Int(direction));
    }

    static Converter Subtype(std::string const& name, std::string const& subtypeTitle, std::string const& subtype) {
        return Converter(Fixed(name), AddStringProperty(subtypeTitle, subtype));
    }

    static Converter Stone(std::string const& stoneType) {
        return Subtype("stone", "stone_type", stoneType);
    }

    static Converter Dirt(std::string const& dirtType) {
        return Subtype("dirt", "dirt_type", dirtType);
    }
    
    static Converter Log(std::string const& type) {
        return Converter(Fixed("log"), AddStringProperty("old_log_type", type), AxisToPillarAxis);
    }

    static Converter Log2(std::string const& type) {
        return Converter(Fixed("log2"), AddStringProperty("new_log_type", type), AxisToPillarAxis);
    }

    static Converter Wood(std::string const& type, bool stripped) {
        return Converter(Fixed("wood"), AxisToPillarAxis, AddStringProperty("wood_type", type), AddBoolProperty("stripped_bit", stripped));
    }

    static Converter Leaves(std::string const& type) {
        return Converter(Fixed("leaves"), AddStringProperty("old_leaf_type", type), PersistentToPersistentBit, DistanceToUpdateBit);
    }

    static Converter Leaves2(std::string const& type) {
        return Converter(Fixed("leaves2"), AddStringProperty("new_leaf_type", type), PersistentToPersistentBit, DistanceToUpdateBit);
    }

    static Converter WoodenSlab(std::string const& type) {
        return Converter(SlabName("wooden", ""), TypeToTopSlotBit, AddStringProperty("wood_type", type));
    }

    static Converter StoneSlab(std::string const& type) {
        return StoneSlabNumbered("", type);
    }

    static Converter StoneSlab2(std::string const& type) {
        return StoneSlabNumbered("2", type);
    }

    static Converter StoneSlab3(std::string const& type) {
        return StoneSlabNumbered("3", type);
    }

    static Converter StoneSlab4(std::string const& type) {
        return StoneSlabNumbered("4", type);
    }

    static Converter StoneSlabNumbered(std::string const& number, std::string const& type) {
        return Converter(SlabName("stone", number), TypeToTopSlotBit, AddStoneSlabType(number, type));
    }

    static Converter StoneSlabNT(std::string const& doubledName) {
        return Converter(ChangeWhenDoubleType(doubledName), TypeToTopSlotBit);
    }

    static Converter TallGrass(std::string const& type) {
        return Converter(Fixed("tallgrass"), AddStringProperty("tall_grass_type", type));
    }

    static Converter DoublePlant(std::string const& type) {
        return Converter(Fixed("double_plant"), AddStringProperty("double_plant_type", type), HalfToUpperBlockBit);
    }
    
    static Converter Rename(std::string const& name) {
        return Converter(Fixed(name));
    }
    
    static Converter SeaPickle() {
        return Converter(Fixed("sea_pickle"), WaterloggedToDeadBit, PicklesToClusterCount);
    }
    
    static Converter RedFlower(std::string const& type) {
        return Converter(Fixed("red_flower"), AddStringProperty("flower_type", type));
    }
     
    static Converter Kelp(std::optional<int32_t> age = std::nullopt) {
        if (age) {
            return Converter(Fixed("kelp"), AddIntProperty("kelp_age", *age));
        } else {
            return Converter(Fixed("kelp"), WithName("kelp_age", Age));
        }
    }

    static Converter Liquid(std::string const& type) {
        return Converter(FlowingLiquidName(type), WithName("liquid_depth", Level));
    }

    static Converter NetherVines(std::string const& type, std::optional<int> age = std::nullopt) {
        if (age) {
            return Converter(Fixed(type + "_vines"), AddIntProperty(type + "_vines_age", *age));
        } else {
            return Converter(Fixed(type + "_vines"), WithName(type + "_vines_age", Age));
        }
    }

    static PropertySpec StairsDirectionFromFacing(Block const& block) {
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
        return std::make_pair("weirdo_direction", props::Int(direction));
    }
    
    static PropertySpec HalfToUpsideDownBit(Block const& block) {
        auto half = block.property("half", "bottom");
        return std::make_pair("upside_down_bit", props::Bool(half == "top"));
    }
    
    static Converter Stairs(std::optional<std::string> name = std::nullopt) {
        NamingFunction naming = name ? Fixed(*name) : Same;
        return Converter(naming, HalfToUpsideDownBit, StairsDirectionFromFacing);
    }

    static Converter Sponge(std::string const& type) {
        return Subtype("sponge", "sponge_type", type);
    }

    static Converter Wool(std::string const& color) {
        return Subtype("wool", "color", color);
    }

    static Converter RedSandstone(std::string const& type) {
        return Subtype("red_sandstone", "sand_stone_type", type);
    }

    static Converter Sandstone(std::string const& type) {
        return Subtype("sandstone", "sand_stone_type", type);
    }

    static Converter Sand(std::string const& type) {
        return Subtype("sand", "sand_type", type);
    }

    static Converter QuartzBlock(std::string const& type) {
        return Converter(Fixed("quartz_block"), AxisToPillarAxis, AddStringProperty("chisel_type", type));
    }

    static Converter PurpurBlock(std::string const& type) {
        return Subtype("purpur_block", "chisel_type", type);
    }

    static Converter Planks(std::string const& type) {
        return Subtype("planks", "wood_type", type);
    }

    static PropertyType Facing(Block const& block) {
        auto facing = block.property("facing", "north");
        int32_t direction = 2;
        if (facing == "south") {
            direction = 0;
        } else if (facing == "east") {
            direction = 3;
        } else if (facing == "west") {
            direction = 1;
        } else {
            direction = 2;
        }
        return props::Int(direction);
    }

    static PropertySpec DirectionFromFacing(Block const& block) {
        return std::make_pair("direction", Facing(block));
    }
    
    static Converter LitPumpkin() {
        return Converter(Fixed("lit_pumpkin"), DirectionFromFacing);
    }

    static Converter StainedGlass(std::string const& color) {
        return Subtype("stained_glass", "color", color);
    }

    static Converter Prismarine(std::string const& type) {
        return Subtype("prismarine", "prismarine_block_type", type);
    }

    static Converter Terracotta(std::string const& color) {
        return Subtype("stained_hardened_clay", "color", color);
    }

    static Converter Concrete(std::string const& color) {
        return Subtype("concrete", "color", color);
    }

    static Converter ConcretePowder(std::string const& color) {
        return Subtype("concretePowder", "color", color);
    }

    static Converter CoralBlock(std::string const& color, bool dead) {
        return Converter(Fixed("coral_block"), AddStringProperty("coral_color", color), AddBoolProperty("dead_bit", dead));
    }

    static PropertySpec StageToAgeBit(Block const& block) {
        auto stage = block.property("stage", "0");
        return std::make_pair("age_bit", props::Bool(stage == "1"));
    }
    
    static Converter Sapling(std::string const& type) {
        return Converter(Fixed("sapling"), AddStringProperty("sapling_type", type), StageToAgeBit);
    }

    static Converter StoneBrick(std::string const& type) {
        return Subtype("stonebrick", "stone_brick_type", type);
    }

    static PropertySpec LayersToHeight(Block const& block) {
        auto layers = std::stoi(block.property("layers", "1"));
        return std::make_pair("height", props::Int(layers - 1));
    }
    
    static Converter SnowLayer() {
        return Converter(Fixed("snow_layer"), LayersToHeight, AddBoolProperty("covered_bit", false));
    }

    static PropertySpec EndRodFacingDirectionFromFacing(Block const& block) {
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
        return std::make_pair("facing_direction", props::Int(direction));
    }

    static Converter AnyTorch(std::string const& prefix) {
        return Converter(Fixed(prefix + "torch"), AddStringProperty("torch_facing_direction", "top"));
    }

    static std::string GetTorchFacingDirectionFromFacing(std::string const& facing) {
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

    static PropertySpec TorchFacingDirectionFromFacing(Block const& block) {
        auto facing = block.property("facing", "north");
        auto direction = GetTorchFacingDirectionFromFacing(facing);
        return std::make_pair("torch_facing_direction", props::String(direction));
    }
    
    static Converter AnyWallTorch(std::string const& prefix) {
        return Converter(Fixed(prefix + "torch"), TorchFacingDirectionFromFacing);
    }

    static Converter InfestedStone(std::string const& type) {
        return Subtype("monster_egg", "monster_egg_stone_type", type);
    }

    static Converter Fence(std::string const& type) {
        return Subtype("fence", "wood_type", type);
    }

    static PropertyType Moisture(Block const& block) {
        auto v = std::stoi(block.property("moisture", "0"));
        return props::Int(v);
    }
    
    static PropertyMapFunction HugeMushroomBits(bool stem) {
        return [=](Block const& block) {
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
            return std::make_pair("huge_mushroom_bits", props::Int(bits));
        };
    }

    static Converter AnyMushroomBlock(std::string const& name, bool stem) {
        return Converter(Fixed(name), HugeMushroomBits(stem));
    }

    static Converter ShulkerBox(std::string const& color) {
        return Subtype("shulker_box", "color", color);
    }

    static PropertySpec EyeToEndPortalEyeBit(Block const& block) {
        auto eye = block.property("eye", "false") == "true";
        return std::make_pair("end_portal_eye_bit", props::Bool(eye));
    }
    
    static PropertySpec FacingDirectionFromFacing(Block const& block) {
        auto facing = block.property("facing", "north");
        int32_t direction = GetFacingDirectionFromFacing(block);
        return std::make_pair("facing_direction", props::Int(direction));
    }
    
    static std::string GetWallConnectionType(std::string const& type) {
        if (type == "low") {
            return "short";
        } else if (type == "tall") {
            return "tall";
        } else {
            return "none";
        }
    }

    static PropertyMapFunction WallConnectionType(std::string const& jeName, std::string const& beName) {
        return [=](Block const& block) {
            auto v = block.property(jeName, "none");
            auto type = GetWallConnectionType(v);
            return std::make_pair(beName, props::String(type));
        };
    }
    
    static Converter Wall(std::string const& type) {
        return Converter(Fixed("cobblestone_wall"),
                         AddStringProperty("wall_block_type", type),
                         WallConnectionType("east", "wall_connection_type_east"),
                         WallConnectionType("north", "wall_connection_type_north"),
                         WallConnectionType("south", "wall_connection_type_south"),
                         WallConnectionType("west", "wall_connection_type_west"));
    }

    static Converter Carpet(std::string const& color) {
        return Subtype("carpet", "color", color);
    }

    static Converter StainedGlassPane(std::string const& color) {
        return Subtype("stained_glass_pane", "color", color);
    }

    static Converter Anvil(std::string const& damage) {
        return Converter(Fixed("anvil"), AddStringProperty("damage", damage), DirectionFromFacing);
    }

    static Converter Coral(std::string const& type, bool dead) {
        return Converter(Fixed("coral"), AddStringProperty("coral_color", type), AddBoolProperty("dead_bit", dead));
    }

    static Converter CoralFan(std::string const& type, bool dead) {
        return Converter(Fixed(dead ? "coral_fan_dead" : "coral_fan"), AddStringProperty("coral_color", type), AddIntProperty("coral_fan_direction", 0));
    }

    static Converter WallSign(std::optional<std::string> prefix = std::nullopt) {
        std::string name = prefix ? *prefix + "_wall_sign" : "wall_sign";
        return Converter(Fixed(name), FacingDirectionFromFacing);
    }

    static PropertyType Rotation(Block const& block) {
        auto r = block.property("rotation", "0");
        return props::Int(std::stoi(r));
    }
    
    static Converter Sign(std::optional<std::string> prefix = std::nullopt) {
        std::string name = prefix ? *prefix + "_standing_sign" : "standing_sign";
        return Converter(Fixed(name), WithName("ground_sign_direction", Rotation));
    }

    static PropertySpec PartToHeadPieceBit(Block const& block) {
        auto head = block.property("part", "foot") == "head";
        return std::make_pair("head_piece_bit", props::Bool(head));
    }
    
    static PropertySpec OccupiedToOccupiedBit(Block const& block) {
        auto occupied = block.property("occupied", "false") == "true";
        return std::make_pair("occupied_bit", props::Bool(occupied));
    }
    
    static PropertyType Open(Block const& block) {
        return props::Bool(block.property("open", "false") == "true");
    }
    
    static PropertySpec GrindstoneFaceToAttachment(Block const& block) {
        auto face = block.property("face", "wall");
        std::string attachment;
        if (face == "wall") {
            attachment = "side";
        } else if (face == "floor") {
            attachment = "standing";
        } else {
            attachment = "hanging";
        }
        return std::make_pair("attachment", props::String(attachment));
    }
    
    static PropertyType Hanging(Block const& block) {
        auto hanging = block.property("hanging", "false") == "true";
        return props::Bool(hanging);
    }
    
    static PropertySpec BellAttachmentFromAttachment(Block const& block) {
        auto attachment = block.property("attachment", "floor");
        return std::make_pair("attachment", props::String(GetAttachment(attachment)));
    }

    static PropertySpec BellDirectionFromFacing(Block const& block) {
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
        return std::make_pair("direction", props::Int(direction));
    }
    
    static PropertyType Powered(Block const& block) {
        auto p = block.property("powered", "false") == "true";
        return props::Bool(p);
    }
    
    static PropertyType Lit(Block const& block) {
        auto l = block.property("lit", "flase") == "true";
        return props::Bool(l);
    }
    
    static PropertySpec LitToExtinguished(Block const& block) {
        auto l = block.property("lit", "flase") == "true";
        return std::make_pair("extinguished", props::Bool(!l));
    }
    
    static std::unordered_map<std::string, Converter>* CreateConverterTable() {
        using namespace std;
        Converter axisToPillarAxis(Same, AxisToPillarAxis);
        Converter directionFromFacing(Same, DirectionFromFacing);
        Converter facingDirectionFromFacing(Same, FacingDirectionFromFacing);
        Converter wall(Same,
                       AddBoolProperty("wall_post_bit", false),
                       WallConnectionType("east", "wall_connection_type_east"),
                       WallConnectionType("north", "wall_connection_type_north"),
                       WallConnectionType("south", "wall_connection_type_south"),
                       WallConnectionType("west", "wall_connection_type_west"));
        Converter bed(Fixed("bed"), DirectionFromFacing, PartToHeadPieceBit, OccupiedToOccupiedBit);
        Converter pottedFlowerPot(Fixed("flower_pot"), AddBoolProperty("update_bit", true));
        Converter wallBanner(Fixed("wall_banner"), FacingDirectionFromFacing);
        Converter banner(Fixed("standing_banner"), WithName("ground_sign_direction", Rotation));
        Converter lantern(Same, WithName("hanging", Hanging));
        Converter campfire(Same, DirectionFromFacing, LitToExtinguished);

        auto table = new unordered_map<string, Converter>();
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
        E("crimson_stem", axisToPillarAxis);
        E("warped_stem", axisToPillarAxis);

        E("stripped_oak_log", axisToPillarAxis);
        E("stripped_spruce_log", axisToPillarAxis);
        E("stripped_birch_log", axisToPillarAxis);
        E("stripped_acacia_log", axisToPillarAxis);
        E("stripped_jungle_log", axisToPillarAxis);
        E("stripped_dark_oak_log", axisToPillarAxis);

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
        E("crimson_hyphae", axisToPillarAxis);
        E("warped_hyphae", axisToPillarAxis);
        E("stripped_crimson_hyphae", axisToPillarAxis);
        E("stripped_warped_hyphae", axisToPillarAxis);
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
        E("sea_pickle", SeaPickle());
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
        E("seagrass", Converter(Fixed("seagrass"), AddStringProperty("sea_grass_type", "default")));
        E("tall_seagrass", Converter(Fixed("seagrass"), HalfToSeagrassType));
        E("kelp", Kelp());
        E("kelp_plant", Kelp(16));
        E("water", Liquid("water"));
        E("lava", Liquid("lava"));
        E("weeping_vines_plant", NetherVines("weeping", 25)); //TODO(kbinani): is 25 correct?
        E("weeping_vines", NetherVines("weeping"));
        E("twisting_vines_plant", NetherVines("twisting", 25)); //TODO(kbinani): is 25 correct?
        E("twisting_vines", NetherVines("twisting"));
        E("vine", Converter(Fixed("vine"), VineDirectionBits));
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
        E("jack_o_lantern", LitPumpkin());
        E("carved_pumpkin", directionFromFacing);
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
        E("snow", SnowLayer());
        E("sugar_cane", Rename("reeds"));
        E("end_rod", Converter(Same, EndRodFacingDirectionFromFacing));
        E("oak_fence", Fence("oak"));
        E("spruce_fence", Fence("spruce"));
        E("birch_fence", Fence("birch"));
        E("jungle_fence", Fence("jungle"));
        E("acacia_fence", Fence("acacia"));
        E("dark_oak_fence", Fence("dark_oak"));
        E("ladder", facingDirectionFromFacing);
        E("chest", facingDirectionFromFacing);
        E("furnace", facingDirectionFromFacing);
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
        E("farmland", Converter(Same, WithName("moisturized_amount", Moisture)));
        E("red_mushroom_block", AnyMushroomBlock("red_mushroom_block", false));
        E("brown_mushroom_block", AnyMushroomBlock("brown_mushroom_block", false));
        E("mushroom_stem", AnyMushroomBlock("brown_mushroom_block", true));
        E("end_portal_frame", Converter(Same, DirectionFromFacing, EyeToEndPortalEyeBit));
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
        E("blackstone_wall", wall);
        E("polished_blackstone_wall", wall);
        E("polished_blackstone_brick_wall", wall);
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
        E("white_glazed_terracotta", facingDirectionFromFacing);
        E("orange_glazed_terracotta", facingDirectionFromFacing);
        E("magenta_glazed_terracotta", facingDirectionFromFacing);
        E("light_blue_glazed_terracotta", facingDirectionFromFacing);
        E("yellow_glazed_terracotta", facingDirectionFromFacing);
        E("lime_glazed_terracotta", facingDirectionFromFacing);
        E("pink_glazed_terracotta", facingDirectionFromFacing);
        E("gray_glazed_terracotta", facingDirectionFromFacing);
        E("light_gray_glazed_terracotta", Converter(Fixed("silver_glazed_terracotta"), FacingDirectionFromFacing));
        E("cyan_glazed_terracotta", facingDirectionFromFacing);
        E("purple_glazed_terracotta", facingDirectionFromFacing);
        E("blue_glazed_terracotta", facingDirectionFromFacing);
        E("brown_glazed_terracotta", facingDirectionFromFacing);
        E("green_glazed_terracotta", facingDirectionFromFacing);
        E("red_glazed_terracotta", facingDirectionFromFacing);
        E("black_glazed_terracotta", facingDirectionFromFacing);

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

        E("white_bed", bed);
        E("orange_bed", bed);
        E("magenta_bed", bed);
        E("light_blue_bed", bed);
        E("yellow_bed", bed);
        E("lime_bed", bed);
        E("pink_bed", bed);
        E("gray_bed", bed);
        E("light_gray_bed", bed);
        E("cyan_bed", bed);
        E("purple_bed", bed);
        E("blue_bed", bed);
        E("brown_bed", bed);
        E("green_bed", bed);
        E("red_bed", bed);
        E("black_bed", bed);

        E("potted_oak_sapling", pottedFlowerPot);
        E("potted_spruce_sapling", pottedFlowerPot);
        E("potted_birch_sapling", pottedFlowerPot);
        E("potted_jungle_sapling", pottedFlowerPot);
        E("potted_acacia_sapling", pottedFlowerPot);
        E("potted_dark_oak_sapling", pottedFlowerPot);
        E("potted_fern", pottedFlowerPot);
        E("potted_dead_bush", pottedFlowerPot);
        E("potted_dandelion", pottedFlowerPot);
        E("potted_poppy", pottedFlowerPot);
        E("potted_blue_orchid", pottedFlowerPot);
        E("potted_allium", pottedFlowerPot);
        E("potted_azure_bluet", pottedFlowerPot);
        E("potted_red_tulip", pottedFlowerPot);
        E("potted_orange_tulip", pottedFlowerPot);
        E("potted_white_tulip", pottedFlowerPot);
        E("potted_pink_tulip", pottedFlowerPot);
        E("potted_oxeye_daisy", pottedFlowerPot);
        E("potted_cornflower", pottedFlowerPot);
        E("potted_lily_of_the_valley", pottedFlowerPot);
        E("potted_wither_rose", pottedFlowerPot);
        E("potted_brown_mushroom", pottedFlowerPot);
        E("potted_red_mushroom", pottedFlowerPot);
        E("potted_crimson_fungus", pottedFlowerPot);
        E("potted_warped_fungus", pottedFlowerPot);
        E("potted_crimson_roots", pottedFlowerPot);
        E("potted_warped_roots", pottedFlowerPot);
        E("potted_bamboo", pottedFlowerPot);

        E("skeleton_skull", Converter(Fixed("skull"), AddIntProperty("facing_direction", 1), AddBoolProperty("no_drop_bit", false)));

        E("white_banner", banner);
        E("orange_banner", banner);
        E("magenta_banner", banner);
        E("light_blue_banner", banner);
        E("yellow_banner", banner);
        E("lime_banner", banner);
        E("pink_banner", banner);
        E("gray_banner", banner);
        E("light_gray_banner", banner);
        E("cyan_banner", banner);
        E("purple_banner", banner);
        E("blue_banner", banner);
        E("brown_banner", banner);
        E("green_banner", banner);
        E("red_banner", banner);
        E("black_banner", banner);

        E("white_wall_banner", wallBanner);
        E("orange_wall_banner", wallBanner);
        E("magenta_wall_banner", wallBanner);
        E("light_blue_wall_banner", wallBanner);
        E("yellow_wall_banner", wallBanner);
        E("lime_wall_banner", wallBanner);
        E("pink_wall_banner", wallBanner);
        E("gray_wall_banner", wallBanner);
        E("light_gray_wall_banner", wallBanner);
        E("cyan_wall_banner", wallBanner);
        E("purple_wall_banner", wallBanner);
        E("blue_wall_banner", wallBanner);
        E("brown_wall_banner", wallBanner);
        E("green_wall_banner", wallBanner);
        E("red_wall_banner", wallBanner);
        E("black_wall_banner", wallBanner);

        E("stonecutter", Converter(Fixed("stonecutter_block"), FacingDirectionFromFacing));
        E("loom", directionFromFacing);
        E("grindstone", Converter(Fixed("grindstone"), DirectionFromFacing, GrindstoneFaceToAttachment));
        E("smoker", facingDirectionFromFacing);
        E("blast_furnace", facingDirectionFromFacing);
        E("barrel", Converter(Fixed("barrel"), FacingDirectionFromFacing, WithName("open_bit", Open)));
        E("lantern", lantern);
        E("soul_lantern", lantern);
        E("bell", Converter(Fixed("bell"), BellDirectionFromFacing, BellAttachmentFromAttachment, WithName("toggle_bit", Powered)));
        E("campfire", campfire);
        E("soul_campfire", campfire);
#undef E
        return table;
    }

    static std::string GetAttachment(std::string const& attachment) {
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

    static int32_t GetFacingDirectionFromFacing(Block const& block) {
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
    
private:
    static int32_t const kBlockDataVersion = 17825808;
};

}
