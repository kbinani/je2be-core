#pragma once

namespace j2b::entities {

class Item {
public:
    std::shared_ptr<mcfile::nbt::CompoundTag> toCompoundTag() const {
        using namespace std;
        using namespace props;
        auto tag = make_shared<mcfile::nbt::CompoundTag>();
        tag->fValue = {
            {"Block", fBlock},
            {"Count", Int(fCount)},
            {"Damage", Short(*(int16_t*)&fDamage)},
            {"Name", String(fName)},
            {"Slot", Byte(*(int8_t*)&fSlot)},
            {"WasPickedUp", Bool(fWasPickedUp)},
        };
        return tag;
    }

public:
    std::shared_ptr<mcfile::nbt::CompoundTag> fBlock;
    uint8_t fCount = 1;
    uint16_t fDamage = 0;
    std::string fName = "";
    uint8_t fSlot = 0;
    bool fWasPickedUp = false;
};

class Chest {
public:
    Chest(int x, int y, int z, mcfile::Block const& b) : fId("Chest"), fX(x), fY(y), fZ(z) {
        assert(b.fName == "minecraft:chest" || b.fName == "minecraft:trapped_chest");
        auto type = b.property("type", "single");
        auto facing = b.property("facing", "north");
        if (type == "left" && facing == "north") {
            fPair.emplace(x + 1, z);
        } else if (type == "right" && facing == "south") {
            fPair.emplace(x + 1, z);
        } else if (type == "right" && facing == "west") {
            fPair.emplace(x, z + 1);
        } else if (type == "left" && facing == "east") {
            fPair.emplace(x, z + 1);
        }
    }

    class Pair {
    public:
        Pair(int x, int z) : fX(x), fZ(z)
        {}

    public:
        int const fX;
        int const fZ;
    };

    std::shared_ptr<mcfile::nbt::CompoundTag> toCompoundTag() const {
        using namespace props;
        using namespace mcfile::nbt;
        auto tag = std::make_shared<CompoundTag>();
        auto items = std::make_shared<ListTag>();
        items->fType = Tag::TAG_Compound;
        for (auto const& item : fItems) {
            items->fValue.push_back(item.toCompoundTag());
        }
        tag->fValue = {
            {"Items", items},
            {"Findable", Bool(fFindable)},
            {"id", String(fId)},
            {"isMovable", Bool(fIsMovable)},
            {"x", Int(fX)},
            {"y", Int(fY)},
            {"z", Int(fZ)},
        };
        if (fPair) {
            tag->fValue.emplace("pairlead", Bool(true));
            tag->fValue.emplace("pairx", Int(fPair->fX));
            tag->fValue.emplace("pairz", Int(fPair->fZ));
        }
        return tag;
    }

public:
    std::vector<Item> fItems;
    bool fFindable = false;
    std::string const fId;
    bool fIsMovable = true;
    std::optional<Pair> fPair;
    int32_t const fX;
    int32_t const fY;
    int32_t const fZ;
};

}
