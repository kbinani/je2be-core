#pragma once

#include <je2be/nbt.hpp>

#include "_pos3.hpp"

namespace je2be {

struct DataVersion;

namespace java {

struct Context;

class TileEntity {
  class Impl;

  TileEntity() = delete;

public:
  static bool IsTileEntity(mcfile::blocks::BlockId id);
  static CompoundTagPtr FromBlockAndTileEntity(Pos3i const &pos,
                                               mcfile::je::Block const &block,
                                               CompoundTagPtr const &tag,
                                               Context &ctx,
                                               DataVersion const &dataVersion);
  static CompoundTagPtr FromBlock(Pos3i const &pos,
                                  mcfile::je::Block const &block,
                                  Context &ctx,
                                  DataVersion const &dataVersion);
  static bool IsStandaloneTileEntity(CompoundTagPtr const &tag);
  static std::optional<std::tuple<CompoundTagPtr, std::u8string>> StandaloneTileEntityBlockdData(Pos3i pos, CompoundTagPtr const &tag);
  static CompoundTagPtr StandaloneTileEntityData(CompoundTagPtr const &tag);
};

} // namespace java
} // namespace je2be
