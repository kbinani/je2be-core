#pragma once

#include <je2be/nbt.hpp>
#include <je2be/pos2.hpp>

#include "enums/_chunk-conversion-mode.hpp"

namespace je2be {
struct DataVersion;
namespace java {

class ChunkData;
struct Context;

class ChunkDataPackage {
public:
  explicit ChunkDataPackage(ChunkConversionMode mode);
  ~ChunkDataPackage();

  void build(mcfile::je::Chunk const &chunk, Context &ctx, DataVersion const &dataVersion, std::unordered_map<Pos2i, std::vector<CompoundTagPtr>, Pos2iHasher> &entities);
  [[nodiscard]] bool serialize(ChunkData &cd);
  void updateAltitude(int x, int y, int z);
  void addTileBlock(int x, int y, int z, std::shared_ptr<mcfile::je::Block const> const &block);
  void addLiquidTick(int order, CompoundTagPtr const &pendingTick);
  void addTileTick(int order, CompoundTagPtr const &pendingTick);

private:
  class Impl;
  std::unique_ptr<Impl> fImpl;
};
} // namespace java
} // namespace je2be
