#pragma once

#include "_pos3.hpp"
#include "lce/_tile-entity-convert-result.hpp"

namespace je2be::lce {

class Context;

class TileEntity {
  class Impl;
  TileEntity() = delete;

  using Result = TileEntityConvertResult;

public:
  static std::optional<Result> Convert(CompoundTag const &in, std::shared_ptr<mcfile::je::Block const> const &block, Pos3i const &pos, Context const &ctx);
};

} // namespace je2be::lce
