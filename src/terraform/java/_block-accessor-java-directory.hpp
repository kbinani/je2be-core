#pragma once

#include "terraform/java/_block-accessor-java.hpp"

namespace je2be::terraform::java {

template <size_t Width, size_t Height>
class BlockAccessorJavaDirectory : public BlockAccessorJava {
public:
  BlockAccessorJavaDirectory(int cx, int cz, std::filesystem::path const &directory)
      : fChunkX(cx), fChunkZ(cz), fCache(Width * Height), fCacheLoaded(Width * Height, false), fDir(directory) {
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

  void loadAllWith(mcfile::je::McaEditor &editor, int rx, int rz) {
    for (int z = 0; z < Height; z++) {
      for (int x = 0; x < Width; x++) {
        int cx = fChunkX + x;
        int cz = fChunkZ + z;
        if (chunkAt(cx, cz)) {
          continue;
        }
        if (rx != mcfile::Coordinate::RegionFromChunk(cx) || rz != mcfile::Coordinate::RegionFromChunk(cz)) {
          continue;
        }
        int lx = cx - rx * 32;
        int lz = cz - rz * 32;
        if (auto tag = editor.get(lx, lz); tag) {
          auto chunk = mcfile::je::Chunk::MakeChunk(cx, cz, tag);
          set(chunk);
        }
      }
    }
  }

  std::shared_ptr<mcfile::je::Chunk> chunkAt(int cx, int cz) override {
    auto idx = getIndex(cx, cz);
    if (!idx) {
      return nullptr;
    }
    if (!fCacheLoaded[*idx]) {
      int rx = mcfile::Coordinate::RegionFromChunk(cx);
      int rz = mcfile::Coordinate::RegionFromChunk(cz);
      auto file = fDir / mcfile::je::Region::GetDefaultRegionFileName(rx, rz);
      if (auto region = mcfile::je::Region::MakeRegion(file, rx, rz); region) {
        fCache[*idx] = region->chunkAt(cx, cz);
      }
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

  void set(std::shared_ptr<mcfile::je::Chunk> const &chunk) {
    if (!chunk) {
      return;
    }
    auto idx = getIndex(chunk->fChunkX, chunk->fChunkZ);
    if (!idx) {
      return;
    }
    fCache[*idx] = chunk;
    fCacheLoaded[*idx] = true;
  }

  BlockAccessorJavaDirectory<Width, Height> *makeRelocated(int cx, int cz) const {
    auto ret = std::make_unique<BlockAccessorJavaDirectory<Width, Height>>(cx, cz, fDir);
    for (int x = 0; x < Width; x++) {
      for (int z = 0; z < Height; z++) {
        auto idx = getIndex(fChunkX + x, fChunkZ + z);
        if (idx && fCacheLoaded[*idx]) {
          auto const &chunk = fCache[*idx];
          ret->set(chunk);
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
