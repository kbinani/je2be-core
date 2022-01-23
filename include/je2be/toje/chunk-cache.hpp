#pragma once

namespace je2be::toje {

template <size_t width, size_t height>
class ChunkCache {
public:
  ChunkCache(mcfile::Dimension d, int cx, int cz) : fDim(d), fCache(width * height), fCacheLoaded(width * height, false), fChunkX(cx), fChunkZ(cz) {
  }

  std::shared_ptr<mcfile::be::Chunk> at(int cx, int cz) const {
    int index = this->index(cx, cz);
    return fCache[index];
  }

  std::shared_ptr<mcfile::be::Block const> blockAt(int bx, int by, int bz) const {
    int cx = mcfile::Coordinate::ChunkFromBlock(bx);
    int cz = mcfile::Coordinate::ChunkFromBlock(bz);
    int index = this->index(cx, cz);
    auto const &chunk = fCache[index];
    if (!chunk) {
      return nullptr;
    }
    return chunk->blockAt(bx, by, bz);
  }

  void load(int cx, int cz, leveldb::DB &db) {
    int index = this->index(cx, cz);
    if (fCacheLoaded[index]) {
      return;
    }
    fCache[index] = mcfile::be::Chunk::Load(cx, cz, fDim, &db);
    fCacheLoaded[index] = true;
  }

  int index(int cx, int cz) const {
    int x = cx - fChunkX;
    int z = cz - fChunkZ;
    if (0 <= x && x < width && 0 <= z && z < height) {
      return z * width + x;
    } else {
      assert(false);
      return 0;
    }
  }

  void purge(int cx, int cz) {
    int index = this->index(cx, cz);
    fCache[index] = nullptr;
  }

private:
  mcfile::Dimension const fDim;
  std::vector<std::shared_ptr<mcfile::be::Chunk>> fCache;
  std::vector<bool> fCacheLoaded;
  int const fChunkX;
  int const fChunkZ;
};

} // namespace je2be::toje
