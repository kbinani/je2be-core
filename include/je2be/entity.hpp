#pragma once

namespace j2b {

class Entity {
public:
    static std::shared_ptr<mcfile::nbt::CompoundTag> From(mcfile::nbt::CompoundTag const& tag) {
        using namespace props;
        auto id = GetString(tag, "id");
        if (!id) return nullptr;
        if (*id == "minecraft:painting") {
            return Painting(tag);
        }
        return nullptr;
    }

private:
    static std::shared_ptr<mcfile::nbt::CompoundTag> Painting(mcfile::nbt::CompoundTag const& tag) {
        using namespace props;
        using namespace mcfile::nbt;
        using namespace std;

        auto facing = GetByte(tag, "Facing");
        auto fallDistance = GetFloat(tag, "FallDistance");
        auto fire = GetShort(tag, "Fire");
        auto invulnerable = GetBool(tag, "Invulnerable");
        auto motive = GetString(tag, "Motive");
        auto onGround = GetBool(tag, "OnGround");
        auto portalCooldown = GetInt(tag, "PortalCooldown");
        auto x = GetInt(tag, "TileX");
        auto y = GetInt(tag, "TileY");
        auto z = GetInt(tag, "TileZ");
        auto motion = GetVec(tag, "Motion");
        auto pos = GetVec(tag, "Pos");
        auto rotation = GetRotation(tag, "Rotation");
        auto uuid = GetUUID(tag, "UUID");
        auto beMotive = PaintingMotive(*motive);

        auto definitions = make_shared<ListTag>();
        definitions->fType = Tag::TAG_String;

        auto c = make_shared<CompoundTag>();
        c->fValue = {
            {"definitions", definitions},
            {"Motion", motion->toListTag()},
            {"Pos", pos->toListTag()},
            {"Rotation", rotation->toListTag()},
            //{"Tags", tags},
            {"Chested", Bool(false)},
            {"Color2", Byte(0)},
            {"Color", Byte(0)},
            {"Dir", Byte(0)},//TODO(kbinani)
            {"Direction", Byte(*facing)},
            {"FallDistance", Float(*fallDistance)},
            {"Fire", Short(*fire)},
            {"identifier", String("minecraft:painting")},
            {"Invulnerable", Bool(*invulnerable)},
            {"IsAngry", Bool(false)},
            {"IsAutonomous", Bool(false)},
            {"IsBaby", Bool(false)},
            {"IsEating", Bool(false)},
            {"IsGliding", Bool(false)},
            {"IsGlobal", Bool(false)},
            {"IsIllagerCaptain", Bool(false)},
            {"IsOrphaned", Bool(false)},
            {"IsRoaring", Bool(false)},
            {"IsScared", Bool(false)},
            {"IsStunned", Bool(false)},
            {"IsSwimming", Bool(false)},
            {"IsTamed", Bool(false)},
            {"IsTrusting", Bool(false)},
            {"LastDimensionId", Int(0)},
            {"LootDropped", Bool(false)},
            {"MarkVariant", Int(0)},
            {"Motive", String(*beMotive)},
            {"OnGround", Bool(*onGround)},
            {"OwnerNew", Long(-1)},
            {"PortalCooldown", Int(*portalCooldown)},
            {"Saddled", Bool(false)},
            {"Sheared", Bool(false)},
            {"ShowBottom", Bool(false)},
            {"Sitting", Bool(false)},
            {"SkinID", Int(0)},
            {"Strength", Int(0)},
            {"StrengthMax", Int(0)},
            {"UniqueID", Long(*uuid)},
            {"Variant", Int(0)},
        };
        return c;
    }

    static std::optional<std::string> PaintingMotive(std::string const& je) {
        using namespace std;
        static unordered_map<string, string> const mapping = {
            {"minecraft:bust", "Bust"},
            {"minecraft:pigscene", "Pigscene"},
            {"minecraft:burning_skull", "BurningSkull"},
            {"minecraft:pointer", "Pointer"},
            {"minecraft:skeleton", "Skeleton"},
            {"minecraft:donkey_kong", "DonkeyKong"},
            {"minecraft:fighters", "Fighters"},
            {"minecraft:skull_and_roses", "SkullAndRoses"},
            {"minecraft:match", "Match"},
            {"minecraft:bust", "Bust"},
            {"minecraft:stage", "Stage"},
            {"minecraft:void", "Void"},
            {"minecraft:wither", "Wither"},
            {"minecraft:sunset", "Sunset"},
            {"minecraft:courbet", "Courbet"},
            {"minecraft:creebet", "Creebet"},
            {"minecraft:sea", "Sea"},
            {"minecraft:wanderer", "Wanderer"},
            {"minecraft:graham", "Graham"},
            {"minecraft:aztec2", "Aztec2"},
            {"minecraft:alban", "Alban"},
            {"minecraft:bomb", "Bomb"},
            {"minecraft:kebab", "Kebab"},
            {"minecraft:wasteland", "Wasteland"},
            {"minecraft:aztec", "Aztec"},
            {"minecraft:plant", "Plant"},
            {"minecraft:pool", "Pool"},
        };
        auto found = mapping.find(je);
        if (found != mapping.end()) {
            return found->second;
        }
        return nullopt;
    }
};

}
