#pragma once

#include "terraform/java/_block-accessor-java.hpp"

namespace je2be::terraform::java {

template <size_t Width, size_t Height>
class BlockAccessorJavaDirectory : public BlockAccessorJava {
public:
  BlockAccessorJavaDirectory(int cx, int cz, std::filesystem::path const &directory)
      : fChunkX(cx), fChunkZ(cz), fCache(Width * Height), fCacheLoaded(Width * Height, false), fDir(directory) {
  }

  std::shared_ptr<mcfile::je::Chunk> at(int cx, int cz) const override {
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

  void loadAllWith(mcfile::je::McaEditor &editor, int rx, int rz) {
    for (int z = 0; z < Height; z++) {
      for (int x = 0; x < Width; x++) {
        int cx = fChunkX + x;
        int cz = fChunkZ + z;
        if (at(cx, cz)) {
          continue;
        }
        if (rx != mcfile::Coordinate::RegionFromChunk(cx) || rz != mcfile::Coordinate::RegionFromChunk(cz)) {
          continue;
        }
        int lx = cx - rx * 32;
        int lz = cz - rz * 32;
        if (auto tag = editor.get(lx, lz); tag) {
          auto chunk = mcfile::je::Chunk::MakeChunk(cx, cz, tag);
          set(cx, cz, chunk);
        }
      }
    }
  }

  std::shared_ptr<mcfile::je::Chunk> ensureLoadedAt(int cx, int cz) {
    auto index = this->index(cx, cz);
    if (!index) {
      return nullptr;
    }
    if (!fCacheLoaded[*index]) {
      int rx = mcfile::Coordinate::RegionFromChunk(cx);
      int rz = mcfile::Coordinate::RegionFromChunk(cz);
      auto file = fDir / mcfile::je::Region::GetDefaultRegionFileName(rx, rz);
      if (auto region = mcfile::je::Region::MakeRegion(file, rx, rz); region) {
        fCache[*index] = region->chunkAt(cx, cz);
      }
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

  void set(int cx, int cz, std::shared_ptr<mcfile::je::Chunk> const &chunk) {
    auto index = this->index(cx, cz);
    if (!index) {
      return;
    }
    fCache[*index] = chunk;
    fCacheLoaded[*index] = true;
  }

  BlockAccessorJavaDirectory<Width, Height> *makeRelocated(int cx, int cz) const {
    std::unique_ptr<BlockAccessorJavaDirectory<Width, Height>> ret(new BlockAccessorJavaDirectory<Width, Height>(cx, cz, fDir));
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

public:
  int const fChunkX;
  int const fChunkZ;

private:
  std::vector<std::shared_ptr<mcfile::je::Chunk>> fCache;
  std::vector<bool> fCacheLoaded;
  std::filesystem::path fDir;
};

} // namespace je2be::terraform::java
