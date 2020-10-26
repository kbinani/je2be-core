#pragma once

namespace j2b {

class TileEntity {
private:
    using Converter = std::function<std::shared_ptr<mcfile::nbt::CompoundTag>(Pos const&, mcfile::Block const&, mcfile::nbt::CompoundTag const&)>;

public:
    static bool IsTileEntity(std::string const& name) {
        auto const& table = Table();
        auto found = table.find(name);
        return found != table.end();
    }

    static std::shared_ptr<mcfile::nbt::CompoundTag> From(Pos const& pos, mcfile::Block const& block, std::shared_ptr<mcfile::nbt::CompoundTag> const& tag) {
        using namespace std;
        using namespace mcfile;
        using namespace mcfile::nbt;

        string const& name = block.fName;
        auto const& table = Table();
        return table.at(name)(pos, block, *tag);
    }

private:
    TileEntity() = delete;

    static std::unordered_map<std::string, Converter> const& Table() {
        using namespace std;
        static unique_ptr<unordered_map<string, Converter> const> const table(CreateTable());
        return *table;
    }

    static std::unordered_map<std::string, Converter>* CreateTable() {
        using namespace std;
        auto table = new unordered_map<string, Converter>();
#define E(__name, __func) table->insert(make_pair("minecraft:" __name, __func))
        E("chest", Chest);
        E("trapped_chest", Chest);
        E("oak_sign", Sign);
        E("spruce_sign", Sign);
        E("birch_sign", Sign);
        E("jungle_sign", Sign);
        E("acacia_sign", Sign);
        E("dark_oak_sign", Sign);
        E("crimson_sign", Sign);
        E("warped_sign", Sign);
        E("oak_wall_sign", Sign);
        E("spruce_wall_sign", Sign);
        E("birch_wall_sign", Sign);
        E("jungle_wall_sign", Sign);
        E("acacia_wall_sign", Sign);
        E("dark_oak_wall_sign", Sign);
        E("crimson_wall_sign", Sign);
        E("warped_wall_sign", Sign);
#undef E
        return table;
    }

    static std::string ColorCode(std::string const& color) {
        if (color == "black") {
            return "§0";
        } else if (color == "red") {
            return "§4";
        } else if (color == "green") {
            return "§2";
        } else if (color == "brown") {
            return ""; // no matching color for brown
        } else if (color == "blue") {
            return "§1";
        } else if (color == "purple") {
            return "§5";
        } else if (color == "cyan") {
            return "§3";
        } else if (color == "light_gray") {
            return "§7";
        } else if (color == "gray") {
            return "§8";
        } else if (color == "pink") {
            return "§d"; // not best match. same as magenta
        } else if (color == "lime") {
            return "§a";
        } else if (color == "yellow") {
            return "§e";
        } else if (color == "light_blue") {
            return "§b";
        } else if (color == "magenta") {
            return "§d";
        } else if (color == "orange") {
            return "§6";
        } else if (color == "white") {
            return "§f";
        }
        return "";
    }

    static std::string GetAsString(nlohmann::json const& obj, std::string const& key) {
        auto found = obj.find(key);
        if (found == obj.end()) return "";
        return found->get<std::string>();
    }

    static std::shared_ptr<mcfile::nbt::CompoundTag> Sign(Pos const& pos, mcfile::Block const& b, mcfile::nbt::CompoundTag const& c) {
        using namespace props;
        using namespace mcfile::nbt;
        using namespace std;

        auto color = GetString(c, "Color");
        auto text1 = GetJson(c, "Text1");
        auto text2 = GetJson(c, "Text2");
        auto text3 = GetJson(c, "Text3");
        auto text4 = GetJson(c, "Text4");
        if (!color || !text1 || !text2 || !text3 || !text4) return nullptr;
        string text = "";
        if (*color != "black") {
            text += ColorCode(*color);
        }
        text += GetAsString(*text1, "text") + "\x0a" + GetAsString(*text2, "text") + "\x0a" + GetAsString(*text3, "text") + "\x0a" + GetAsString(*text4, "text");
        auto tag = make_shared<CompoundTag>();
        tag->fValue = {
            {"id", String("Sign")},
            {"isMovable", Bool(true)},
            {"Text", String(text)},
            {"TextOwner", String("")},
        };
        Attach(pos, *tag);
        return tag;
    }

    static void Attach(Pos const& pos, mcfile::nbt::CompoundTag & tag) {
        tag.fValue.insert(std::make_pair("x", props::Int(pos.fX)));
        tag.fValue.insert(std::make_pair("y", props::Int(pos.fY)));
        tag.fValue.insert(std::make_pair("z", props::Int(pos.fZ)));
    }

    static std::shared_ptr<mcfile::nbt::CompoundTag> Chest(Pos const& pos, mcfile::Block const& b, mcfile::nbt::CompoundTag const& comp) {
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
        };
        if (pair) {
            tag->fValue.emplace("pairlead", Bool(true));
            tag->fValue.emplace("pairx", Int(pair->first));
            tag->fValue.emplace("pairz", Int(pair->second));
        }
        //TODO(kbinani): Items
        Attach(pos, *tag);
        return tag;
    }
};

}
