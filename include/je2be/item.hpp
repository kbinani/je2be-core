#pragma once

namespace j2b {

class Item {
private:
    using ItemData = std::shared_ptr<mcfile::nbt::CompoundTag>;
    using Converter = std::function<ItemData(std::string const&, mcfile::nbt::CompoundTag const&)>;
    using CompoundTag = mcfile::nbt::CompoundTag;

public:
    static std::shared_ptr<CompoundTag> From(CompoundTag const& item) {
        using namespace props;
        using namespace std;
        using namespace mcfile::nbt;

        static unique_ptr<unordered_map<string, Converter> const> const mapping(CreateTable());

        auto id = GetString(item, "id");
        if (!id) return nullptr;
        string const& name = *id;

        auto found = mapping->find(name);
        if (found == mapping->end()) {
            return Default(name, item);
        } else {
            return found->second(name, item);
        }
    }

private:
    static std::unordered_map<std::string, Converter>* CreateTable() {
        using namespace std;
        using namespace mcfile::nbt;
        auto table = new unordered_map<string, Converter>();
#define E(__name, __func) table->insert(make_pair("minecraft:" __name, __func))
        E("brown_mushroom_block", MushroomBlock);
        E("red_mushroom_block", MushroomBlock);
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

        E("skeleton_skull", Skull);
        E("wither_skeleton_skull", Skull);
        E("player_head", Skull);
        E("zombie_head", Skull);
        E("greeper_head", Skull);
        E("dragon_head", Skull);

        E("item_frame", Rename("frame"));
        E("repeater", NoBlock);
        E("comparator", NoBlock);

        E("iron_door", NoBlock);
        E("oak_door", Rename("wooden_door"));
        E("spruce_door", NoBlock);
        E("birch_door", NoBlock);
        E("jungle_door", NoBlock);
        E("acacia_door", NoBlock);
        E("dark_oak_door", NoBlock);
        E("crimson_door", NoBlock);
        E("warped_door", NoBlock);
#undef E
        return table;
    }

    static Converter Rename(std::string const& newName) {
        return [=](std::string const& name, CompoundTag const& item) {
            auto tag = New(newName);
            return Post(tag, item);
        };
    }

    static ItemData Skull(std::string const& name, CompoundTag const& item) {
        int8_t type = TileEntity::GetSkullTypeFromBlockName(name);
        auto tag = New("skull");
        tag->fValue["Damage"] = props::Short(type);
        return Post(tag, item);
    }

    static ItemData Banner(std::string const& name, CompoundTag const& item) {
        auto colorName = strings::RTrim(strings::LTrim(name, "minecraft:"), "_banner");
        BannerColorCodeBedrock color = BannerColorCodeFromName(colorName);
        int16_t damage = (int16_t)color;
        auto tag = New("banner");
        tag->fValue["Damage"] = props::Short(damage);
        return Post(tag, item);
    }

    static ItemData Bed(std::string const& name, CompoundTag const& item) {
        using namespace std;
        string colorName = strings::RTrim(strings::LTrim(name, "minecraft:"), "_bed");
        ColorCodeJava color = ColorCodeJavaFromName(colorName);
        int16_t damage = (int16_t)color;
        auto tag = New("bed");
        tag->fValue["Damage"] = props::Short(damage);
        return Post(tag, item);
    }

    static ItemData MushroomBlock(std::string const& name, CompoundTag const& item) {
        using namespace std;
        using namespace props;

        auto count = GetByteOrDefault(item, "Count", 1);

        map<string, string> empty;
        auto block = make_shared<mcfile::Block>(name, empty);
        auto blockData = BlockData::From(block);

        auto states = make_shared<CompoundTag>();
        states->fValue = {
            {"huge_mushroom_bits", Int(14)},
        };
        blockData->fValue["states"] = states;

        auto tag = New(name, true);
        tag->fValue["Count"] = Byte(count);
        tag->fValue["Damage"] = Short(0);
        tag->fValue["Block"] = blockData;
        return Post(tag, item);
    }

    static ItemData New(std::string const& name, bool fullname = false) {
        using namespace props;
        std::string n;
        if (fullname) {
            n = name;
        } else {
            n = "minecraft:" + name;
        }
        auto tag = std::make_shared<CompoundTag>();
        tag->fValue = {
            {"Name", String(name)},
            {"WasPickedUp", Bool(false)},
            {"Count", Byte(1)},
            {"Damage", Short(0)},
        };
        return tag;
    }

    Item() = delete;

    static ItemData Default(std::string const& id, CompoundTag const& item) {
        using namespace std;
        using namespace props;
        using namespace mcfile::nbt;

        auto count = GetByteOrDefault(item, "Count", 1);

        map<string, string> p;
        auto block = make_shared<mcfile::Block>(id, p);
        auto blockData = BlockData::From(block);
        assert(blockData);

        auto name = GetString(*blockData, "name");
        if (!name) return nullptr;

        auto tag = make_shared<CompoundTag>();
        tag->fValue = {
            {"Name", String(ItemNameFromBlockName(*name))},
            {"Count", Byte(count)},
            {"WasPickedUp", Bool(false)},
            {"Damage", Short(0)},
        };
        tag->fValue["Block"] = blockData;
        return Post(tag, item);
    }

    static ItemData NoBlock(std::string const& name, CompoundTag const& item) {
        using namespace props;

        auto count = GetByteOrDefault(item, "Count", 1);
        auto tag = std::make_shared<CompoundTag>();
        tag->fValue = {
            {"Name", String(name)},
            {"Count", Byte(count)},
            {"WasPickedUp", Bool(false)},
            {"Damage", Short(0)},
        };
        return Post(tag, item);
    }

    static ItemData Post(ItemData const& tag, CompoundTag const& item) {
        //TODO: custom name, enchant, etc
        return tag;
    }

    // converts block name (bedrock) to item name (bedrock)
    static std::string ItemNameFromBlockName(std::string const& name) {
        if (name == "minecraft:concretePowder") {
            return "minecraft:concrete_powder";
        }
        return name;
    }
};

}
