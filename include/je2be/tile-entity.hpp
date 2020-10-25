#pragma once

namespace j2b {

class TileEntity {
public:
    static std::shared_ptr<mcfile::nbt::CompoundTag> From(Pos const& pos, mcfile::Block const& block, std::shared_ptr<mcfile::nbt::CompoundTag> const& tag) {
        using namespace std;
        string const& name = block.fName;
        if (name == "minecraft:chest" || name == "minecraft:trapped_chest") {
            return Chest(pos, block, tag);
        }
        return nullptr;
    }

private:
    TileEntity() = delete;

    static std::shared_ptr<mcfile::nbt::CompoundTag> Chest(Pos const& pos, mcfile::Block const& b, std::shared_ptr<mcfile::nbt::CompoundTag> const& comp) {
        using namespace props;
        using namespace mcfile::nbt;
        using namespace std;

        auto type = b.property("type", "single");
        auto facing = b.property("facing", "north");
        optional<pair<int, int>> pair;
        if (type == "left" && facing == "north") {
            pair = make_pair(pos.fX + 1, pos.fZ);
        } else if (type == "right" && facing == "south") {
            pair = make_pair(pos.fX + 1, pos.fZ);
        } else if (type == "right" && facing == "west") {
            pair = make_pair(pos.fX, pos.fZ + 1);
        } else if (type == "left" && facing == "east") {
            pair = make_pair(pos.fX, pos.fZ + 1);
        }

        auto tag = std::make_shared<CompoundTag>();
        auto items = std::make_shared<ListTag>();
        items->fType = Tag::TAG_Compound;

        tag->fValue = {
            {"Items", items},
            {"Findable", Bool(false)},
            {"id", String(string("Chest"))},
            {"isMovable", Bool(true)},
            {"x", Int(pos.fX)},
            {"y", Int(pos.fY)},
            {"z", Int(pos.fZ)},
        };
        if (pair) {
            tag->fValue.emplace("pairlead", Bool(true));
            tag->fValue.emplace("pairx", Int(pair->first));
            tag->fValue.emplace("pairz", Int(pair->second));
        }
        return tag;
    }
};

}
