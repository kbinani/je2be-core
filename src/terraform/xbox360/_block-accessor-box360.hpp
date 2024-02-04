#pragma once

#include "terraform/_block-accessor.hpp"

namespace je2be::terraform::box360 {

template <size_t Width, size_t Height>
class BlockAccessorBox360 : public BlockAccessor<mcfile::je::Block> {
public:
  BlockAccessorBox360(int cx, int cz, std::filesystem::path const &directory) : fChunkX(cx), fChunkZ(cz), fCache(Width * Height), fCacheLoaded(Width * Height, false), fDir(directory) {
  }

  std::shared_ptr<mcfile::je::Chunk> at(int cx, int cz) const {
    auto index = this->index(cx, cz);
    if (!index) {
      return nullptr;
    }
    return fCache[*index];
  }

  std::shared_ptr<mcfile::je::Block const> blockAt(int bx, int by, int bz) override {
    int cx = mcfile::Coordinate::ChunkFromBlock(bx);
    int cz = mcfile::Coordinate::ChunkFromBlock(bz);
    auto const &chunk = ensureLoadedAt(cx, cz);
    if (!chunk) {
      return nullptr;
    }
    return chunk->blockAt(bx, by, bz);
  }

  std::shared_ptr<CompoundTag const> tileEntityAt(Pos3i const &pos) {
    int cx = mcfile::Coordinate::ChunkFromBlock(pos.fX);
    int cz = mcfile::Coordinate::ChunkFromBlock(pos.fZ);
    auto const &chunk = ensureLoadedAt(cx, cz);
    if (!chunk) {
      return nullptr;
    }
    return chunk->tileEntityAt(pos);
  }

  std::shared_ptr<mcfile::je::Chunk> ensureLoadedAt(int cx, int cz) {
    auto idx = getIndex(cx, cz);
    if (!idx) {
      return nullptr;
    }
    if (!fCacheLoaded[*idx]) {
      auto file = fDir / mcfile::je::Region::GetDefaultCompressedChunkNbtFileName(cx, cz);
      fCache[*idx] = mcfile::je::Chunk::LoadFromDeflateCompressedChunkNbtFile(file, cx, cz);
      fCacheLoaded[*idx] = true;
    }
    return fCache[*idx];
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

  BlockAccessorBox360<Width, Height> *makeRelocated(int cx, int cz) const {
    auto ret = std::make_unique<BlockAccessorBox360<Width, Height>>(cx, cz, fDir);
    for (int x = 0; x < Width; x++) {
      for (int z = 0; z < Height; z++) {
        auto idx = getIndex(fChunkX + x, fChunkZ + z);
        if (idx && fCacheLoaded[*idx]) {
          auto const &chunk = fCache[*idx];
          ret->set(fChunkX + x, fChunkZ + z, chunk);
        }
      }
    }
    return ret.release();
  }

  void set(int cx, int cz, std::shared_ptr<mcfile::je::Chunk> const &chunk) {
    auto idx = getIndex(cx, cz);
    if (!idx) {
      return;
    }
    fCache[*idx] = chunk;
    fCacheLoaded[*idx] = true;
  }

public:
  int const fChunkX;
  int const fChunkZ;

private:
  std::vector<std::shared_ptr<mcfile::je::Chunk>> fCache;
  std::vector<bool> fCacheLoaded;
  std::filesystem::path fDir;
};

} // namespace je2be::terraform::box360
