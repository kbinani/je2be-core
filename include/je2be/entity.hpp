#pragma once

namespace j2b {

class Entity {
private:
    using CompoundTag = mcfile::nbt::CompoundTag;
    using EntityDataType = std::shared_ptr<mcfile::nbt::CompoundTag>;
    using Converter = std::function<EntityDataType(CompoundTag const&)>;

public:
    Entity() : fMotion(0, 0, 0), fPos(0, 0, 0), fRotation(0, 0) {}

    static std::shared_ptr<mcfile::nbt::CompoundTag> From(mcfile::nbt::CompoundTag const& tag) {
        using namespace props;
        auto id = GetString(tag, "id");
        if (!id) return nullptr;
        static std::unique_ptr<std::unordered_map<std::string, Converter> const> const table(CreateEntityTable());
        auto found = table->find(*id);
        if (found == table->end()) {
            return nullptr;
        }
        return found->second(tag);
    }

    static bool DegAlmostEquals(float a, float b) {
        a = fmod(fmod(a, 360) + 360, 360);
        assert(0 <= a && a < 360);
        b = fmod(fmod(b, 360) + 360, 360);
        assert(0 <= b && b < 360);
        float diff = fmod(fmod(a - b, 360) + 360, 360);
        assert(0 <= diff && diff < 360);
        float const tolerance = 1;
        if (0 <= diff && diff < tolerance) {
            return true;
        } else if (360 - tolerance < diff) {
            return true;
        } else {
            return false;
        }
    }

    static bool RotAlmostEquals(Rotation const& rot, float yaw, float pitch) {
        return DegAlmostEquals(rot.fYaw, yaw) && DegAlmostEquals(rot.fPitch, pitch);
    }

    static std::tuple<Pos, std::shared_ptr<CompoundTag>, std::string> ToTileEntityBlock(CompoundTag const& c) {
        using namespace std;
        using namespace props;
        auto id = GetString(c, "id");
        assert(id);
        if (*id == "minecraft:item_frame") {
            auto tileX = GetInt(c, "TileX");
            auto tileY = GetInt(c, "TileY");
            auto tileZ = GetInt(c, "TileZ");

            auto rot = GetRotation(c, "Rotation");
            int32_t facing = 0;
            if (RotAlmostEquals(*rot, 0, -90)) {
                // up
                facing = 1;
            } else if (RotAlmostEquals(*rot, 180, 0)) {
                // north
                facing = 2;
            } else if (RotAlmostEquals(*rot, 270, 0)) {
                // east
                facing = 5;
            } else if (RotAlmostEquals(*rot, 0, 0)) {
                // south
                facing = 3;
            } else if (RotAlmostEquals(*rot, 90, 0)) {
                // west
                facing = 4;
            } else if (RotAlmostEquals(*rot, 0, 90)) {
                // down
                facing = 0;
            }

            bool map = false;
            auto itemId = c.query("Item/id");
            if (itemId) {
                auto itemIdString = itemId->asString();
                if (itemIdString) {
                    string itemName = itemIdString->fValue;
                    if (itemName == "minecraft:filled_map") {
                        map = true;
                    }
                }
            }

            auto b = make_shared<CompoundTag>();
            auto states = make_shared<CompoundTag>();
            states->fValue = {
                {"facing_direction", Int(facing)},
                {"item_frame_map_bit", Bool(map)},
            };
            string key = "minecraft:frame[facing_direction=" + to_string(facing) + ",item_frame_map_bit=" + (map ? "true" : "false") + "]";
            b->fValue = {
                {"name", String("minecraft:frame")},
                {"version", Int(BlockData::kBlockDataVersion)},
                {"states", states},
            };
            Pos pos(*tileX, *tileY, *tileZ);
            return make_tuple(pos, b, key);
        }
    }

    static TileEntity::TileEntityData ToTileEntityData(CompoundTag const& c, JavaEditionMap const& mapInfo, DimensionDataFragment &ddf) {
        using namespace props;
        auto id = GetString(c, "id");
        assert(id);
        if (*id == "minecraft:item_frame") {
            auto tag = std::make_shared<CompoundTag>();
            auto tileX = GetInt(c, "TileX");
            auto tileY = GetInt(c, "TileY");
            auto tileZ = GetInt(c, "TileZ");
            if (!tileX || !tileY || !tileZ) return nullptr;
            tag->fValue = {
                {"id", String("ItemFrame")},
                {"isMovable", Bool(true)},
                {"x", Int(*tileX)},
                {"y", Int(*tileY)},
                {"z", Int(*tileZ)},
            };
            auto itemRotation = GetByteOrDefault(c, "ItemRotation", 0);
            auto itemDropChange = GetFloatOrDefault(c, "ItemDropChange", 1);
            auto found = c.fValue.find("Item");
            if (found != c.fValue.end() && found->second->id() == mcfile::nbt::Tag::TAG_Compound) {
                auto item = std::dynamic_pointer_cast<CompoundTag>(found->second);
                auto m = Item::From(item, mapInfo, ddf);
                if (m) {
                    tag->fValue.insert(make_pair("Item", m));
                    tag->fValue.insert(make_pair("ItemRotation", Float(itemRotation)));
                    tag->fValue.insert(make_pair("ItemDropChange", Float(itemDropChange)));
                }
            }
            return tag;
        }
    }

    static bool IsTileEntity(CompoundTag const& tag) {
        auto id = props::GetString(tag, "id");
        if (!id) return false;
        return *id == "minecraft:item_frame";
    }

    std::shared_ptr<mcfile::nbt::CompoundTag> toCompoundTag() const {
        using namespace std;
        using namespace props;
        using namespace mcfile::nbt;
        auto tag = make_shared<CompoundTag>();
        auto tags = make_shared<ListTag>();
        tags->fType = Tag::TAG_Compound;
        auto definitions = make_shared<ListTag>();
        definitions->fType = Tag::TAG_String;
        for (auto const& d : fDefinitions) {
            definitions->fValue.push_back(String(d));
        }
        tag->fValue = {
            {"definitions", definitions},
            {"Motion", fMotion.toListTag()},
            {"Pos", fPos.toListTag()},
            {"Rotation", fRotation.toListTag()},
            {"Tags", tags},
            {"Chested", Bool(fChested)},
            {"Color2", Byte(fColor2)},
            {"Color", Byte(fColor)},
            {"Dir", Byte(fDir)},
            {"FallDistance", Float(fFallDistance)},
            {"Fire", Short(fFire)},
            {"identifier", String(fIdentifier)},
            {"Invulnerable", Bool(fInvulnerable)},
            {"IsAngry", Bool(fIsAngry)},
            {"IsAutonomous", Bool(fIsAutonomous)},
            {"IsBaby", Bool(fIsBaby)},
            {"IsEating", Bool(fIsEating)},
            {"IsGliding", Bool(fIsGliding)},
            {"IsGlobal", Bool(fIsGlobal)},
            {"IsIllagerCaptain", Bool(fIsIllagerCaptain)},
            {"IsOrphaned", Bool(fIsOrphaned)},
            {"IsRoaring", Bool(fIsRoaring)},
            {"IsScared", Bool(fIsScared)},
            {"IsStunned", Bool(fIsStunned)},
            {"IsSwimming", Bool(fIsSwimming)},
            {"IsTamed", Bool(fIsTamed)},
            {"IsTrusting", Bool(fIsTrusting)},
            {"LastDimensionId", Int(fLastDimensionId)},
            {"LootDropped", Bool(fLootDropped)},
            {"MarkVariant", Int(fMarkVariant)},
            {"OnGround", Bool(fOnGround)},
            {"OwnerNew", Int(fOwnerNew)},
            {"PortalCooldown", Int(fPortalCooldown)},
            {"Saddled", Bool(fSaddled)},
            {"Sheared", Bool(fSheared)},
            {"ShowBottom", Bool(fShowBottom)},
            {"Sitting", Bool(fSitting)},
            {"SkinID", Int(fSkinId)},
            {"Strength", Int(fStrength)},
            {"StrengthMax", Int(fStrengthMax)},
            {"UniqueID", Long(fUniqueId)},
            {"Variant", Int(fVariant)},
        };
        return tag;
    }

private:
    static std::unordered_map<std::string, Converter>* CreateEntityTable() {
        auto table = new std::unordered_map<std::string, Converter>();
#define E(__name, __func) table->insert(std::make_pair("minecraft:" __name, __func))
        E("painting", Painting);
        E("end_crystal", EndCrystal);
#undef E
        return table;
    }

    static void BaseProperties(CompoundTag const& tag, Entity& e) {
        using namespace props;
        using namespace mcfile::nbt;
        using namespace std;

        auto fallDistance = GetFloat(tag, "FallDistance");
        auto fire = GetShort(tag, "Fire");
        auto invulnerable = GetBool(tag, "Invulnerable");
        auto onGround = GetBool(tag, "OnGround");
        auto portalCooldown = GetInt(tag, "PortalCooldown");
        auto motion = GetVec(tag, "Motion");
        auto pos = GetVec(tag, "Pos");
        auto rotation = GetRotation(tag, "Rotation");
        auto uuid = GetUUID(tag, "UUID");

        if (motion) e.fMotion = *motion;
        if (pos) e.fPos = *pos;
        if (rotation) e.fRotation = *rotation;
        if (fallDistance) e.fFallDistance = *fallDistance;
        if (fire) e.fFire = *fire;
        if (invulnerable) e.fInvulnerable = *invulnerable;
        if (onGround) e.fOnGround = *onGround;
        if (portalCooldown) e.fPortalCooldown = *portalCooldown;
        if (uuid) e.fUniqueId = *uuid;
    }

    static EntityDataType EndCrystal(CompoundTag const& tag) {
        using namespace props;
        Entity e;
        BaseProperties(tag, e);
        e.fIdentifier = "minecraft:ender_crystal";
        auto c = e.toCompoundTag();
        c->fValue.emplace("ShowBottom", Bool(false));
        return c;
    }

    static std::shared_ptr<mcfile::nbt::CompoundTag> Painting(mcfile::nbt::CompoundTag const& tag) {
        using namespace props;
        using namespace mcfile::nbt;
        using namespace std;

        auto facing = GetByte(tag, "Facing");
        auto motive = GetString(tag, "Motive");
        auto beMotive = PaintingMotive(*motive);

        Entity e;
        BaseProperties(tag, e);
        e.fIdentifier = "minecraft:painting";
        auto c = e.toCompoundTag();
        c->fValue.emplace("Motive", String(*beMotive));
        c->fValue.emplace("Direction", Byte(*facing));
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

public:
    std::vector<std::string> fDefinitions;
    Vec fMotion;
    Vec fPos;
    Rotation fRotation;
    std::vector<std::shared_ptr<mcfile::nbt::CompoundTag>> fTags;
    bool fChested = false;
    int8_t fColor2 = 0;
    int8_t fColor = 0;
    int8_t fDir = 0;
    float fFallDistance = 0;
    int16_t fFire = 0;
    std::string fIdentifier;
    bool fInvulnerable = false;
    bool fIsAngry = false;
    bool fIsAutonomous = false;
    bool fIsBaby = false;
    bool fIsEating = false;
    bool fIsGliding = false;
    bool fIsGlobal = false;
    bool fIsIllagerCaptain = false;
    bool fIsOrphaned = false;
    bool fIsRoaring = false;
    bool fIsScared = false;
    bool fIsStunned = false;
    bool fIsSwimming = false;
    bool fIsTamed = false;
    bool fIsTrusting = false;
    int32_t fLastDimensionId = 0;
    bool fLootDropped = false;
    int32_t fMarkVariant = 0;
    bool fOnGround = true;
    int64_t fOwnerNew = -1;
    int32_t fPortalCooldown = 0;
    bool fSaddled = false;
    bool fSheared = false;
    bool fShowBottom = false;
    bool fSitting = false;
    int32_t fSkinId = 0;
    int32_t fStrength = 0;
    int32_t fStrengthMax = 0;
    int64_t fUniqueId;
    int32_t fVariant = 0;
};

}
