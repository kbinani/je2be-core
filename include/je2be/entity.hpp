#pragma once

namespace j2b {

class Vec {
public:
    Vec(float x, float y, float z) : fX(x), fY(y), fZ(z) {}

    std::shared_ptr<mcfile::nbt::ListTag> toListTag() const {
        using namespace mcfile::nbt;
        using namespace props;
        auto tag = std::make_shared<ListTag>();
        tag->fType = Tag::TAG_Float;
        tag->fValue = {Float(fX), Float(fY), Float(fZ)};
        return tag;
    }

public:
    float const fX;
    float const fY;
    float const fZ;
};

class Rotation {
public:
    Rotation(float yaw, float pitch) : fYaw(yaw), fPitch(pitch) {}

    std::shared_ptr<mcfile::nbt::ListTag> toListTag() const {
        using namespace mcfile::nbt;
        using namespace props;
        auto tag = std::make_shared<ListTag>();
        tag->fType = Tag::TAG_Float;
        tag->fValue = {Float(fYaw), Float(fPitch)};
        return tag;
    }

public:
    float const fYaw;
    float const fPitch;
};

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

    static std::optional<int64_t> GetUUID(mcfile::nbt::CompoundTag const& tag, std::string const& name) {
        using namespace std;
        using namespace mcfile::nbt;
        auto found = tag.fValue.find(name);
        if (found == tag.fValue.end()) {
            return nullopt;
        }
        IntArrayTag const* list = found->second->asIntArray();
        if (!list) {
            return nullopt;
        }
        vector<int32_t> const& value = list->value();
        if (value.size() != 4) {
            return nullopt;
        }

        int32_t a = value[0];
        int32_t b = value[1];
        int32_t c = value[2];
        int32_t d = value[3];

        XXH64_state_t* state = XXH64_createState();
        XXH64_hash_t seed = 0;
        XXH64_reset(state, seed);
        XXH64_update(state, &a, sizeof(a));
        XXH64_update(state, &b, sizeof(b));
        XXH64_update(state, &c, sizeof(c));
        XXH64_update(state, &d, sizeof(d));
        XXH64_hash_t hash = XXH64_digest(state);
        XXH64_freeState(state);
        return *(int64_t*)&hash;
    }


    static std::optional<Vec> GetVec(mcfile::nbt::CompoundTag const& tag, std::string const& name) {
        using namespace std;
        using namespace mcfile::nbt;
        auto found = tag.fValue.find(name);
        if (found == tag.fValue.end()) {
            return nullopt;
        }
        auto list = found->second->asList();
        if (!list) {
            return nullopt;
        }
        if (list->fType != Tag::TAG_Double || list->fValue.size() != 3) {
            return nullopt;
        }
        double x = list->fValue[0]->asDouble()->fValue;
        double y = list->fValue[1]->asDouble()->fValue;
        double z = list->fValue[2]->asDouble()->fValue;
        return Vec((float)x, (float)y, (float)z);
    }

    static std::optional<Rotation> GetRotation(mcfile::nbt::CompoundTag const& tag, std::string const& name) {
        using namespace std;
        using namespace mcfile::nbt;
        auto found = tag.fValue.find(name);
        if (found == tag.fValue.end()) {
            return nullopt;
        }
        auto list = found->second->asList();
        if (!list) {
            return nullopt;
        }
        if (list->fType != Tag::TAG_Float || list->fValue.size() != 2) {
            return nullopt;
        }
        double yaw = list->fValue[0]->asFloat()->fValue;
        double pitch = list->fValue[1]->asFloat()->fValue;
        return Rotation(yaw, pitch);
    }
};

}
