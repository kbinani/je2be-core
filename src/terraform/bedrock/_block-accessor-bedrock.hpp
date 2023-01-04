#pragma once

#include <minecraft-file.hpp>

#include <je2be/nbt.hpp>

#include "_pos3.hpp"
#include "terraform/_block-accessor.hpp"

namespace je2be::terraform::bedrock {

template <size_t Width, size_t Height>
class BlockAccessorBedrock : public BlockAccessor<mcfile::be::Block> {
public:
  BlockAccessorBedrock(mcfile::Dimension d, int cx, int cz, mcfile::be::DbInterface *db, mcfile::Endian endian)
      : fDim(d), fChunkX(cx), fChunkZ(cz), fCache(Width * Height), fCacheLoaded(Width * Height, false), fDb(db), fEndian(endian) {
  }

  std::shared_ptr<mcfile::be::Chunk> at(int cx, int cz) const {
    auto index = this->index(cx, cz);
    if (!index) {
      return nullptr;
    }
    return fCache[*index];
  }

  std::shared_ptr<mcfile::be::Block const> blockAt(int bx, int by, int bz) override {
    int cx = mcfile::Coordinate::ChunkFromBlock(bx);
    int cz = mcfile::Coordinate::ChunkFromBlock(bz);
    auto const &chunk = ensureLoadedAt(cx, cz);
    if (!chunk) {
      return nullptr;
    }
    return chunk->blockAt(bx, by, bz);
  }

  std::shared_ptr<CompoundTag const> blockEntityAt(Pos3i const &pos) {
    int cx = mcfile::Coordinate::ChunkFromBlock(pos.fX);
    int cz = mcfile::Coordinate::ChunkFromBlock(pos.fZ);
    auto const &chunk = ensureLoadedAt(cx, cz);
    if (!chunk) {
      return nullptr;
    }
    return chunk->blockEntityAt(pos);
  }

  std::shared_ptr<mcfile::be::Chunk> ensureLoadedAt(int cx, int cz) {
    auto index = this->index(cx, cz);
    if (!index) {
      return nullptr;
    }
    if (!fCacheLoaded[*index]) {
      mcfile::be::Chunk::LoadWhat what;
      what.fBiomes = false;
      what.fBlockEntities = false;
      what.fEntities = false;
      what.fPendingTicks = false;
      fCache[*index] = mcfile::be::Chunk::Load(cx, cz, fDim, *fDb, fEndian, what);
      fCacheLoaded[*index] = true;
    }
    return fCache[*index];
  }

  std::optional<int> index(int cx, int cz) const {
    int x = cx - fChunkX;
    int z = cz - fChunkZ;
    if (0 <= x && x < Width && 0 <= z && z < Height) {
      return z * (int)Width + x;
    } else {
      return std::nullopt;
    }
  }

  BlockAccessorBedrock<Width, Height> *makeRelocated(int chunkX, int chunkZ) const {
    auto ret = new BlockAccessorBedrock<Width, Height>(fDim, chunkX, chunkZ, fDb, fEndian);
    for (int cx = fChunkX; cx < fChunkX + Width; cx++) {
      for (int cz = fChunkZ; cz < fChunkZ + Height; cz++) {
        auto index = this->index(cx, cz);
        if (fCacheLoaded[*index]) {
          auto const &chunk = fCache[*index];
          ret->set(cx, cz, chunk);
        }
      }
    }
    return ret;
  }

  void set(int cx, int cz, std::shared_ptr<mcfile::be::Chunk> const &chunk) {
    auto index = this->index(cx, cz);
    if (!index) {
      return;
    }
    fCache[*index] = chunk;
    fCacheLoaded[*index] = true;
  }

public:
  mcfile::Dimension const fDim;
  int const fChunkX;
  int const fChunkZ;

private:
  std::vector<std::shared_ptr<mcfile::be::Chunk>> fCache;
  std::vector<bool> fCacheLoaded;
  mcfile::be::DbInterface *const fDb;
  mcfile::Endian const fEndian;
};

} // namespace je2be::terraform::bedrock
