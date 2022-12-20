#pragma once

#include <je2be/nbt.hpp>
#include <je2be/pos3.hpp>
#include <je2be/uuid.hpp>

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
};

} // namespace je2be::toje
