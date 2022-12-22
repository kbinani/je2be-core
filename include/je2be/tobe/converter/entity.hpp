#pragma once

#include <je2be/nbt.hpp>
#include <je2be/pos2.hpp>
#include <je2be/pos3.hpp>
#include <je2be/rotation.hpp>

#include <unordered_map>
#include <vector>

namespace je2be::tobe {

struct Context;

class Entity {
private:
  class Impl;
  struct ConverterContext {
    explicit ConverterContext(Context const &ctx) : fCtx(ctx) {}

    Context const &fCtx;
    std::vector<CompoundTagPtr> fPassengers;
    std::unordered_map<Pos2i, std::vector<CompoundTagPtr>, Pos2iHasher> fLeashKnots;
  };
  using Converter = std::function<CompoundTagPtr(CompoundTag const &, ConverterContext &)>;

  using Behavior = std::function<void(CompoundTag &, CompoundTag const &, ConverterContext &)>;

public:
  explicit Entity(int64_t uid) : fMotion(0, 0, 0), fPos(0, 0, 0), fRotation(0, 0), fUniqueId(uid) {}

  struct Result {
    CompoundTagPtr fEntity;
    std::vector<CompoundTagPtr> fPassengers;
    std::unordered_map<Pos2i, std::vector<CompoundTagPtr>, Pos2iHasher> fLeashKnots;
  };

  static Result From(CompoundTag const &tag, Context const &ctx);

  static bool RotAlmostEquals(Rotation const &rot, float yaw, float pitch) { return Rotation::DegAlmostEquals(rot.fYaw, yaw) && Rotation::DegAlmostEquals(rot.fPitch, pitch); }

  static std::optional<std::tuple<Pos3i, CompoundTagPtr, std::string>> ToTileEntityBlock(CompoundTag const &c);

  static std::tuple<Pos3i, CompoundTagPtr, std::string> ToItemFrameTileEntityBlock(CompoundTag const &c, std::string const &name);

  static CompoundTagPtr ToTileEntityData(CompoundTag const &c, Context const &ctx);

  static CompoundTagPtr ToItemFrameTileEntityData(CompoundTag const &c, Context const &ctx, std::string const &name);

  static bool IsTileEntity(CompoundTag const &tag);

  CompoundTagPtr toCompoundTag() const {
    using namespace std;
    auto tag = Compound();
    auto tags = List<Tag::Type::Compound>();
    auto definitions = List<Tag::Type::String>();
    for (auto const &d : fDefinitions) {
      definitions->push_back(String(d));
    }
    tag->insert({
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
        {"Fire", Short(std::max((int16_t)0, fFire))},
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
        {"OwnerNew", Long(fOwnerNew)},
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
    });
    if (fCustomName) {
      tag->set("CustomName", String(*fCustomName));
      tag->set("CustomNameVisible", Bool(fCustomNameVisible));
    }
    return tag;
  }

  struct LocalPlayerResult {
    CompoundTagPtr fEntity;
    int64_t fUid;
    mcfile::Dimension fDimension;
    Pos2i fChunk;
  };
  static std::optional<LocalPlayerResult> LocalPlayer(CompoundTag const &tag, Context const &ctx);

public:
  std::vector<std::string> fDefinitions;
  Pos3f fMotion;
  Pos3f fPos;
  Rotation fRotation;
  std::vector<CompoundTagPtr> fTags;
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
  int64_t const fUniqueId;
  int32_t fVariant = 0;
  std::optional<std::string> fCustomName;
  bool fCustomNameVisible = false;
};

} // namespace je2be::tobe
