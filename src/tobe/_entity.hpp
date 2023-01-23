#pragma once

#include <je2be/nbt.hpp>
#include <je2be/pos2.hpp>

#include "_pos3.hpp"
#include "_rotation.hpp"

#include <unordered_map>
#include <vector>

namespace je2be::tobe {

struct Context;

class Entity {
  class Impl;

public:
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

  struct LocalPlayerResult {
    CompoundTagPtr fEntity;
    i64 fUid;
    mcfile::Dimension fDimension;
    Pos2i fChunk;
  };
  static std::optional<LocalPlayerResult> LocalPlayer(CompoundTag const &tag, Context const &ctx);
};

} // namespace je2be::tobe
