#pragma once

#include <je2be/nbt.hpp>
#include <je2be/pos2.hpp>

#include "_pos3.hpp"
#include "_rotation.hpp"

#include <unordered_map>
#include <vector>

namespace je2be::java {

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

  static Result From(CompoundTag const &tag, Context &ctx, int dataVersion, std::set<Flag> flags);

  static bool RotAlmostEquals(Rotation const &rot, float yaw, float pitch) { return Rotation::DegAlmostEquals(rot.fYaw, yaw) && Rotation::DegAlmostEquals(rot.fPitch, pitch); }

  static std::optional<std::pair<Pos3i, CompoundTagPtr>> ToTileEntityBlock(CompoundTag const &c);

  static CompoundTagPtr ToTileEntityData(CompoundTag const &c, Context &ctx, int dataVersion);

  static CompoundTagPtr ToItemFrameTileEntityData(CompoundTag const &c, Context &ctx, std::u8string const &name, int dataVersion);

  static bool IsTileEntity(CompoundTag const &tag);

  struct LocalPlayerResult {
    CompoundTagPtr fEntity;
    i64 fUid;
    mcfile::Dimension fDimension;
    Pos2i fChunk;
  };
  static std::optional<LocalPlayerResult> LocalPlayer(CompoundTag const &tag, Context &ctx, int dataVersion, std::set<Flag> flags);
};

} // namespace je2be::java
