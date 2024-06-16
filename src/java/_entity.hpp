#pragma once

#include <je2be/nbt.hpp>
#include <je2be/pos2.hpp>

#include "_pos3.hpp"
#include "_rotation.hpp"

#include <unordered_map>
#include <vector>

namespace je2be {
struct DataVersion;
namespace java {

struct Context;

class Entity {
  class Impl;

public:
  struct Result {
    CompoundTagPtr fEntity;
    std::vector<CompoundTagPtr> fPassengers;
    std::unordered_map<Pos2i, std::vector<CompoundTagPtr>, Pos2iHasher> fLeashKnots;
  };

  enum class Flag : uint32_t {
    ShoulderRider = uint32_t(1) << 0,
  };

  static Result From(CompoundTag const &tag, Context &ctx, DataVersion const &dataVersion, std::set<Flag> flags);

  static bool RotAlmostEquals(Rotation const &rot, float yaw, float pitch) { return Rotation::DegAlmostEquals(rot.fYaw, yaw) && Rotation::DegAlmostEquals(rot.fPitch, pitch); }

  static std::optional<std::pair<Pos3i, CompoundTagPtr>> ToTileEntityBlock(CompoundTag const &c);

  static CompoundTagPtr ToTileEntityData(CompoundTag const &c, Context &ctx, DataVersion const &dataVersion);

  static CompoundTagPtr ToItemFrameTileEntityData(CompoundTag const &c, Context &ctx, std::u8string const &name, DataVersion const &dataVersion);

  static bool IsTileEntity(CompoundTag const &tag);

  struct LocalPlayerResult {
    CompoundTagPtr fEntity;
    i64 fUid;
    mcfile::Dimension fDimension;
    Pos2i fChunk;
  };
  static std::optional<LocalPlayerResult> LocalPlayer(CompoundTag const &tag, Context &ctx, DataVersion const &dataVersion, std::set<Flag> flags);

  static CompoundTagPtr TropicalFishProperties(i32 variant);

  struct Rep {
    explicit Rep(i64 uid) : fMotion(0, 0, 0), fPos(0, 0, 0), fRotation(0, 0), fUniqueId(uid) {}

    CompoundTagPtr toCompoundTag() const {
      using namespace std;
      auto tag = Compound();
      auto tags = List<Tag::Type::Compound>();
      auto definitions = List<Tag::Type::String>();
      for (auto const &d : fDefinitions) {
        definitions->push_back(String(d));
      }
      tag->insert({
          {u8"definitions", definitions},
          {u8"Motion", fMotion.toListTag()},
          {u8"Pos", fPos.toListTag()},
          {u8"Rotation", fRotation.toListTag()},
          {u8"Tags", tags},
          {u8"Chested", Bool(fChested)},
          {u8"Color2", Byte(fColor2)},
          {u8"Color", Byte(fColor)},
          {u8"Dir", Byte(fDir)},
          {u8"FallDistance", Float(fFallDistance)},
          {u8"Fire", Short(std::max((i16)0, fFire))},
          {u8"identifier", String(fIdentifier)},
          {u8"Invulnerable", Bool(fInvulnerable)},
          {u8"IsAngry", Bool(fIsAngry)},
          {u8"IsAutonomous", Bool(fIsAutonomous)},
          {u8"IsBaby", Bool(fIsBaby)},
          {u8"IsEating", Bool(fIsEating)},
          {u8"IsGliding", Bool(fIsGliding)},
          {u8"IsGlobal", Bool(fIsGlobal)},
          {u8"IsIllagerCaptain", Bool(fIsIllagerCaptain)},
          {u8"IsOrphaned", Bool(fIsOrphaned)},
          {u8"IsRoaring", Bool(fIsRoaring)},
          {u8"IsScared", Bool(fIsScared)},
          {u8"IsStunned", Bool(fIsStunned)},
          {u8"IsSwimming", Bool(fIsSwimming)},
          {u8"IsTamed", Bool(fIsTamed)},
          {u8"IsTrusting", Bool(fIsTrusting)},
          {u8"LastDimensionId", Int(fLastDimensionId)},
          {u8"LootDropped", Bool(fLootDropped)},
          {u8"MarkVariant", Int(fMarkVariant)},
          {u8"OnGround", Bool(fOnGround)},
          {u8"OwnerNew", Long(fOwnerNew)},
          {u8"PortalCooldown", Int(fPortalCooldown)},
          {u8"Saddled", Bool(fSaddled)},
          {u8"Sheared", Bool(fSheared)},
          {u8"ShowBottom", Bool(fShowBottom)},
          {u8"Sitting", Bool(fSitting)},
          {u8"SkinID", Int(fSkinId)},
          {u8"Strength", Int(fStrength)},
          {u8"StrengthMax", Int(fStrengthMax)},
          {u8"UniqueID", Long(fUniqueId)},
          {u8"Variant", Int(fVariant)},
      });
      if (fCustomName) {
        tag->set(u8"CustomName", *fCustomName);
        tag->set(u8"CustomNameVisible", Bool(fCustomNameVisible));
      }
      return tag;
    }

    std::vector<std::u8string> fDefinitions;
    Pos3f fMotion;
    Pos3f fPos;
    Rotation fRotation;
    std::vector<CompoundTagPtr> fTags;
    bool fChested = false;
    i8 fColor2 = 0;
    i8 fColor = 0;
    i8 fDir = 0;
    float fFallDistance = 0;
    i16 fFire = 0;
    std::u8string fIdentifier;
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
    i32 fLastDimensionId = 0;
    bool fLootDropped = false;
    i32 fMarkVariant = 0;
    bool fOnGround = true;
    i64 fOwnerNew = -1;
    i32 fPortalCooldown = 0;
    bool fSaddled = false;
    bool fSheared = false;
    bool fShowBottom = false;
    bool fSitting = false;
    i32 fSkinId = 0;
    i32 fStrength = 0;
    i32 fStrengthMax = 0;
    i64 const fUniqueId;
    i32 fVariant = 0;
    std::optional<std::u8string> fCustomName;
    bool fCustomNameVisible = false;
  };
};

} // namespace java
} // namespace je2be
