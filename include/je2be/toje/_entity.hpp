#pragma once

#include <je2be/nbt.hpp>
#include <je2be/uuid.hpp>

#include <je2be/_pos3.hpp>

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
    std::optional<int64_t> fLeasherId;
  };

  static std::optional<Result> From(CompoundTag const &entityB, Context &ctx);

  struct LocalPlayerData {
    CompoundTagPtr fEntity;
    int64_t fEntityIdBedrock;
    Uuid fEntityIdJava;
    std::optional<int64_t> fShoulderEntityLeft;
    std::optional<int64_t> fShoulderEntityRight;
  };

  static std::optional<LocalPlayerData> LocalPlayer(CompoundTag const &b, Context &ctx, Uuid const *uuid);
};

} // namespace je2be::toje
