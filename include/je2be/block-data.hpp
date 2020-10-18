#pragma once

namespace j2e {

class BlockData {
public:
    static std::shared_ptr<mcfile::nbt::CompoundTag> From(std::shared_ptr<mcfile::Block const> const& block) {
        using namespace std;
        using namespace props;
        using namespace mcfile::nbt;

        static unordered_map<string, ConverterFunction> const converterTable = {
            {"minecraft:stone", StoneSubtype("stone") },
            {"minecraft:granite", StoneSubtype("granite") },
            {"minecraft:polished_granite", StoneSubtype("granite_smooth") },
            {"minecraft:andesite", StoneSubtype("andesite") },
            {"minecraft:polished_andesite", StoneSubtype("andesite_smooth" ) },
            {"minecraft:diorite", StoneSubtype("diorite") },
            {"minecraft:polished_diorite", StoneSubtype("diorite_smooth") },
            {"minecraft:dirt", DirtSubtype("normal") },
            {"minecraft:coarse_dirt", DirtSubtype("coarse") },
            {"minecraft:grass_block", Rename("minecraft:grass") },
            {"minecraft:oak_log", Log("oak") },
            {"minecraft:spruce_log", Log("spruce") },
            {"minecraft:birch_log", Log("birch") },
            {"minecraft:jungle_log", Log("jungle") },
            {"minecraft:acacia_log", Log2("acacia") },
            {"minecraft:dark_oak_log", Log2("dark_oak") },
            {"minecraft:oak_wood", Wood("oak", false) },
            {"minecraft:spruce_wood", Wood("spruce", false) },
            {"minecraft:birch_wood", Wood("birch", false) },
            {"minecraft:acacia_wood", Wood("acacia", false) },
            {"minecraft:jungle_wood", Wood("jungle", false) },
            {"minecraft:dark_oak_wood", Wood("dark_oak", false) },
            {"minecraft:stripped_oak_wood", Wood("oak", true) },
            {"minecraft:stripped_spruce_wood", Wood("spruce", true) },
            {"minecraft:stripped_birch_wood", Wood("birch", true) },
            {"minecraft:stripped_acacia_wood", Wood("acacia", true) },
            {"minecraft:stripped_jungle_wood", Wood("jungle", true) },
            {"minecraft:stripped_dark_oak_wood", Wood("dark_oak", true) },
        };

        auto found = converterTable.find(block->fName);
        if (found == converterTable.end()) {
            return Identity(*block);
        } else {
            return found->second(*block);
        }
    }

    static std::shared_ptr<mcfile::nbt::CompoundTag> Air() {
        static std::shared_ptr<mcfile::nbt::CompoundTag> const air = MakeAir();
        return air;
    }

private:
    BlockData() = delete;

    using ConverterFunction = std::function<std::shared_ptr<mcfile::nbt::CompoundTag>(mcfile::Block const&)>;

    static std::shared_ptr<mcfile::nbt::CompoundTag> MakeAir() {
        using namespace std;
        using namespace mcfile;
        using namespace mcfile::nbt;
        using namespace props;

        auto tag = make_shared<CompoundTag>();
        auto states = make_shared<CompoundTag>();
        tag->fValue.insert(make_pair("name", String("minecraft:air"s)));
        tag->fValue.insert(make_pair("version", Int(kBlockDataVersion)));
        tag->fValue.insert(make_pair("states", states));

        return tag;
    }

    static std::shared_ptr<mcfile::nbt::CompoundTag> Identity(mcfile::Block const& block) {
        using namespace std;
        using namespace mcfile;
        using namespace mcfile::nbt;
        using namespace props;

        auto tag = New(block.fName);
        auto states = make_shared<CompoundTag>();
        static set<string> const ignore = {};
        MergeProperties(block, *states, ignore);
        tag->fValue.insert(make_pair("states", states));
        return tag;
    }

    static ConverterFunction Subtype(std::string const& name, std::string const& subtypeTitle, std::string const& subtype) {
        using namespace std;
        using namespace mcfile;
        using namespace mcfile::nbt;
        using namespace props;

        return [name, subtypeTitle, subtype](Block const& block) -> shared_ptr<CompoundTag> {
            auto tag = New(name);
            auto states = make_shared<CompoundTag>();
            states->fValue.insert(make_pair(subtypeTitle, String(subtype)));
            static set<string> const ignore = { subtypeTitle };
            MergeProperties(block, *states, ignore);
            tag->fValue.insert(make_pair("states", states));
            return tag;
        };
    }

    static ConverterFunction StoneSubtype(std::string const& stoneType) {
        return Subtype("minecraft:stone", "stone_type", stoneType);
    }

    static ConverterFunction DirtSubtype(std::string const& dirtType) {
        return Subtype("minecraft:dirt", "dirt_type", dirtType);
    }

    static ConverterFunction Rename(std::string const& to) {
        using namespace std;
        using namespace mcfile;
        using namespace mcfile::nbt;
        using namespace props;

        return [to](Block const& block) -> shared_ptr<CompoundTag> {
            auto tag = New(to);
            auto states = make_shared<CompoundTag>();
            static set<string> const ignore = {};
            MergeProperties(block, *states, ignore);
            tag->fValue.insert(make_pair("states", states));
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
            states.fValue.insert(std::make_pair(name, props::String(it->second)));
        }
    }

    static std::shared_ptr<mcfile::nbt::CompoundTag> New(std::string const& name) {
        using namespace std;
        using namespace mcfile::nbt;
        auto tag = make_shared<mcfile::nbt::CompoundTag>();
        auto states = make_shared<CompoundTag>();
        tag->fValue.insert(make_pair("name", props::String(name)));
        tag->fValue.insert(make_pair("version", props::Int(kBlockDataVersion)));
        return tag;
    }

    static ConverterFunction Log(std::string const& type) {
        using namespace std;
        using namespace mcfile;
        using namespace mcfile::nbt;
        using namespace props;

        return [type](Block const& block) -> shared_ptr<CompoundTag> {
            auto tag = New("minecraft:log");
            auto states = make_shared<CompoundTag>();
            states->fValue.emplace("old_log_type", String(type));
            auto axis = block.fProperties.find("axis");
            string pillarAxis = "y";
            if (axis != block.fProperties.end()) {
                pillarAxis = axis->second;
            }
            states->fValue.emplace("pillar_axis", String(pillarAxis));
            static set<string> const ignore = { "axis", "pillar_axis", "old_log_type" };
            MergeProperties(block, *states, ignore);
            tag->fValue.emplace("states", states);
            return tag;
        };
    }

    static ConverterFunction Log2(std::string const& type) {
        using namespace std;
        using namespace mcfile;
        using namespace mcfile::nbt;
        using namespace props;

        return [type](Block const& block) -> shared_ptr<CompoundTag> {
            auto tag = New("minecraft:log2");
            auto states = make_shared<CompoundTag>();
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
        using namespace mcfile;
        using namespace mcfile::nbt;
        using namespace props;

        return [=](Block const& block) -> shared_ptr<CompoundTag> {
            auto tag = New("minecraft:wood");
            auto states = make_shared<CompoundTag>();
            auto axis = block.property("axis", "y");
            states->fValue.emplace("pillar_axis", String(axis));
            states->fValue.emplace("wood_type", String(type));
            states->fValue.emplace("stripped_bit", Bool(stripped));
            static set<string> const ignore = { "axis" };
            MergeProperties(block, *states, ignore);
            tag->fValue.insert(make_pair("states", states));
            return tag;
        };
    }

private:
    static int32_t const kBlockDataVersion = 17825808;
};

}
