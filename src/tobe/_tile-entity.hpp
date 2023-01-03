#pragma once

#include <je2be/nbt.hpp>

#include "_pos3.hpp"

namespace je2be::tobe {

struct Context;

class TileEntity {
  class Impl;

  TileEntity() = delete;

public:
  static bool IsTileEntity(mcfile::blocks::BlockId id);
  static CompoundTagPtr FromBlockAndTileEntity(Pos3i const &pos,
                                               mcfile::je::Block const &block,
                                               CompoundTagPtr const &tag,
                                               Context const &ctx);
  static CompoundTagPtr FromBlock(Pos3i const &pos,
                                  mcfile::je::Block const &block,
                                  Context const &ctx);
  static bool IsStandaloneTileEntity(CompoundTagPtr const &tag);
  static std::optional<std::tuple<CompoundTagPtr, std::string>> StandaloneTileEntityBlockdData(Pos3i pos, CompoundTagPtr const &tag);
  static CompoundTagPtr StandaloneTileEntityData(CompoundTagPtr const &tag);
};

} // namespace je2be::tobe
