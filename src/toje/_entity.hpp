#pragma once

#include <je2be/nbt.hpp>
#include <je2be/uuid.hpp>

#include "_pos3.hpp"

namespace je2be::toje {

class Context;

class Entity {
  class Impl;

public:
  static CompoundTagPtr ItemFrameFromBedrock(mcfile::Dimension d, Pos3i pos, mcfile::be::Block const &blockJ, CompoundTag const &blockEntityB, Context &ctx);

  struct Result {
    Uuid fUuid;
    CompoundTagPtr fEntity;

    std::map<size_t, Uuid> fPassengers;
    std::optional<i64> fLeasherId;
  };

  static std::optional<Result> From(CompoundTag const &entityB, Context &ctx);

  struct LocalPlayerData {
    CompoundTagPtr fEntity;
    i64 fEntityIdBedrock;
    Uuid fEntityIdJava;
    std::optional<i64> fShoulderEntityLeft;
    std::optional<i64> fShoulderEntityRight;
  };

  static std::optional<LocalPlayerData> LocalPlayer(CompoundTag const &b, Context &ctx, Uuid const *uuid);
};

} // namespace je2be::toje
