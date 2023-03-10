#pragma once

#include "_pos3.hpp"
#include "terraform/java/_block-accessor-java.hpp"

namespace je2be::terraform::java {

template <size_t Width, size_t Height>
class BlockAccessorJavaMca : public BlockAccessorJava {
public:
  BlockAccessorJavaMca(int cx, int cz, mcfile::je::Region const &region) : fChunkX(cx), fChunkZ(cz), fCache(Width * Height), fCacheLoaded(Width * Height, false), fRegion(region) {
  }

  std::shared_ptr<mcfile::je::Block const> blockAt(int bx, int by, int bz) override {
    int cx = mcfile::Coordinate::ChunkFromBlock(bx);
    int cz = mcfile::Coordinate::ChunkFromBlock(bz);
    auto const &chunk = chunkAt(cx, cz);
    if (!chunk) {
      return nullptr;
    }
    return chunk->blockAt(bx, by, bz);
  }

  std::shared_ptr<CompoundTag const> tileEntityAt(Pos3i const &pos) {
    int cx = mcfile::Coordinate::ChunkFromBlock(pos.fX);
    int cz = mcfile::Coordinate::ChunkFromBlock(pos.fZ);
    auto const &chunk = chunkAt(cx, cz);
    if (!chunk) {
      return nullptr;
    }
    return chunk->tileEntityAt(pos);
  }

  std::shared_ptr<mcfile::je::Chunk> chunkAt(int cx, int cz) override {
    auto index = getIndex(cx, cz);
    if (!index) {
      return nullptr;
    }
    if (!fCacheLoaded[*index]) {
      fCache[*index] = fRegion.chunkAt(cx, cz);
      fCacheLoaded[*index] = true;
    }
    return fCache[*index];
  }

  std::optional<int> getIndex(int cx, int cz) const {
    int x = cx - fChunkX;
    int z = cz - fChunkZ;
    if (0 <= x && x < Width && 0 <= z && z < Height) {
      return z * (int)Width + x;
    } else {
      return std::nullopt;
    }
  }

  void set(std::shared_ptr<mcfile::je::Chunk> const &chunk) {
    if (!chunk) {
      return;
    }
    auto index = getIndex(chunk->fChunkX, chunk->fChunkZ);
    if (!index) {
      return;
    }
    fCache[*index] = chunk;
    fCacheLoaded[*index] = true;
  }

private:
  int const fChunkX;
  int const fChunkZ;
  std::vector<std::shared_ptr<mcfile::je::Chunk>> fCache;
  std::vector<bool> fCacheLoaded;
  mcfile::je::Region fRegion;
};

} // namespace je2be::terraform::java
