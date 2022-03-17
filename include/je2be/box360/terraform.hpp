#pragma once

namespace je2be::box360 {

template <size_t width, size_t height>
class ChunkCache : public je2be::BlockAccessor {
public:
  ChunkCache(int cx, int cz, std::filesystem::path const &directory) : fCache(width * height), fCacheLoaded(width * height, false), fChunkX(cx), fChunkZ(cz), fDir(directory) {
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
    if (0 <= x && x < width && 0 <= z && z < height) {
      return z * width + x;
    } else {
      return std::nullopt;
    }
  }

  ChunkCache *makeRelocated(int cx, int cz) const {
    std::unique_ptr<ChunkCache<width, height>> ret(new ChunkCache(cx, cz, fDir));
    for (int x = 0; x < width; x++) {
      for (int z = 0; z < height; z++) {
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

class Terraform {
  Terraform() = delete;

public:
  enum class Category : uint32_t {
    Stairs = 1,
  };

  static bool Do(std::filesystem::path const &directory) {
    using namespace std;
    int cx0 = -27;
    int cz0 = -27;
    auto cache = make_shared<ChunkCache<3, 3>>(cx0 - 1, cz0 - 1, directory);
    for (int cz = cz0; cz < 27; cz++) {
      for (int cx = cx0; cx < 27; cx++) {
        cache.reset(cache->makeRelocated(cx - 1, cz - 1));
        if (!DoChunk(cx, cz, directory, *cache)) {
          return false;
        }
      }
    }
    return true;
  }

  static bool DoChunk(int cx, int cz, std::filesystem::path const &directory, ChunkCache<3, 3> &cache) {
    using namespace std;
    using namespace je2be::terraform;
    using namespace je2be::terraform::box360;

    auto file = directory / mcfile::je::Region::GetDefaultCompressedChunkNbtFileName(cx, cz);
    auto input = make_shared<mcfile::stream::FileInputStream>(file);
    auto root = CompoundTag::ReadCompressed(*input, endian::big);
    if (!root) {
      return true;
    }
    input.reset();
    auto chunk = mcfile::je::WritableChunk::MakeChunk(cx, cz, root);
    if (!chunk) {
      return false;
    }
    cache.set(cx, cz, chunk);

    BlockPropertyAccessorJava accessor(*chunk);

    ShapeOfStairs::Do(*chunk, cache, accessor);
    FenceConnectable::Do(*chunk, cache, accessor);
    RedstoneWire::Do(*chunk, cache, accessor);
    ChorusPlant::Do(*chunk, cache, accessor);
    WallConnectable::Do(*chunk, cache, accessor);
    Kelp::Do(*chunk, accessor);

    auto output = make_shared<mcfile::stream::FileOutputStream>(file);
    if (!chunk->write(*output)) {
      return false;
    }

    return true;
  }
};

} // namespace je2be::box360
