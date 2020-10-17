#pragma once

namespace j2e {

class BlockData {
public:
    static std::shared_ptr<mcfile::nbt::CompoundTag> From(std::shared_ptr<mcfile::Block const> const& block) {
        using namespace std;
        using namespace props;
        using namespace mcfile::nbt;

        static map<string, ConverterFunction> const converterTable = {
            {"minecraft:stone", StoneSubtype("stone") },
            {"minecraft:granite", StoneSubtype("granite") },
            {"minecraft:polished_granite", StoneSubtype("granite_smooth") },
            {"minecraft:andesite", StoneSubtype("andesite") },
            {"minecraft:polished_andesite", StoneSubtype("andesite_smooth" ) },
            {"minecraft:diorite", StoneSubtype("diorite") },
            {"minecraft:polished_diorite", StoneSubtype("diorite_smooth") },
            {"minecraft:dirt", DirtSubtype("normal") },
            {"minecraft:coarse_dirt", DirtSubtype("coarse") },
            {"minecraft:grass_block", Rename("grass") },
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

        auto tag = make_shared<CompoundTag>();
        auto states = make_shared<CompoundTag>();
        tag->fValue.insert(make_pair("name", String(block.fName)));
        tag->fValue.insert(make_pair("version", Int(kBlockDataVersion)));
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
            auto tag = make_shared<CompoundTag>();
            auto states = make_shared<CompoundTag>();
            tag->fValue.insert(make_pair("name", String(name)));
            states->fValue.insert(make_pair(subtypeTitle, String(subtype)));
            tag->fValue.insert(make_pair("version", Int(kBlockDataVersion)));
            static set<string> const ignore = { subtypeTitle };
            MergeProperties(block, *states, ignore);
            tag->fValue.insert(make_pair("states", states));
            return tag;
        };
    }

    static ConverterFunction StoneSubtype(std::string const& stoneType) {
        return Subtype("stone", "stone_type", stoneType);
    }

    static ConverterFunction DirtSubtype(std::string const& dirtType) {
        return Subtype("dirt", "dirt_type", dirtType);
    }

    static ConverterFunction Rename(std::string const& to) {
        using namespace std;
        using namespace mcfile;
        using namespace mcfile::nbt;
        using namespace props;

        return [to](Block const& block) -> shared_ptr<CompoundTag> {
            auto tag = make_shared<CompoundTag>();
            auto states = make_shared<CompoundTag>();
            tag->fValue.insert(make_pair("name", String(to)));
            tag->fValue.insert(make_pair("version", Int(kBlockDataVersion)));
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
    
private:
    static int32_t const kBlockDataVersion = 17825808;
};

}
