#pragma once

#include <je2be/nbt.hpp>
#include <je2be/pos2.hpp>

#include <je2be/enums/_chunk-conversion-mode.hpp>

namespace je2be::tobe {

class ChunkData;
struct Context;

class ChunkDataPackage {
public:
  explicit ChunkDataPackage(ChunkConversionMode mode);
  ~ChunkDataPackage();

  void build(mcfile::je::Chunk const &chunk, Context const &ctx, std::unordered_map<Pos2i, std::vector<CompoundTagPtr>, Pos2iHasher> &entities);
  [[nodiscard]] bool serialize(ChunkData &cd);
  void updateAltitude(int x, int y, int z);
  void addTileBlock(int x, int y, int z, std::shared_ptr<mcfile::je::Block const> const &block);
  void addLiquidTick(int order, CompoundTagPtr const &pendingTick);
  void addTileTick(int order, CompoundTagPtr const &pendingTick);

private:
  class Impl;
  std::unique_ptr<Impl> fImpl;
};

} // namespace je2be::tobe
