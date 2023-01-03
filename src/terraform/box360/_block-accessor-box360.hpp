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
    auto index = this->index(cx, cz);
    if (!index) {
      return nullptr;
    }
    if (!fCacheLoaded[*index]) {
      auto file = fDir / mcfile::je::Region::GetDefaultCompressedChunkNbtFileName(cx, cz);
      fCache[*index] = mcfile::je::Chunk::LoadFromCompressedChunkNbtFile(file, cx, cz);
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

  BlockAccessorBox360<Width, Height> *makeRelocated(int cx, int cz) const {
    std::unique_ptr<BlockAccessorBox360<Width, Height>> ret(new BlockAccessorBox360<Width, Height>(cx, cz, fDir));
    for (int x = 0; x < Width; x++) {
      for (int z = 0; z < Height; z++) {
        auto index = this->index(fChunkX + x, fChunkZ + z);
        if (fCacheLoaded[*index]) {
          auto const &chunk = fCache[*index];
          ret->set(fChunkX + x, fChunkZ + z, chunk);
        }
      }
    }
    return ret.release();
  }

  void set(int cx, int cz, std::shared_ptr<mcfile::je::Chunk> const &chunk) {
    auto index = this->index(cx, cz);
    if (!index) {
      return;
    }
    fCache[*index] = chunk;
    fCacheLoaded[*index] = true;
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
