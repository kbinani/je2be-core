#pragma once

namespace je2be::box360 {

class Terraform {
  Terraform() = delete;

public:
  enum class Category : uint32_t {
    Stairs = 1,
  };

  static bool Do(std::filesystem::path const &directory) {
    using namespace std;
    int cx0 = -32;
    int cz0 = -32;
    auto cache = make_shared<terraform::box360::BlockAccessorBox360<3, 3>>(cx0 - 1, cz0 - 1, directory);
    for (int cz = cz0; cz < 32; cz++) {
      for (int cx = cx0; cx < 32; cx++) {
        cache.reset(cache->makeRelocated(cx - 1, cz - 1));
        if (!DoChunk(cx, cz, directory, *cache)) {
          return false;
        }
      }
    }
    return true;
  }

  static bool DoChunk(int cx, int cz, std::filesystem::path const &directory, terraform::box360::BlockAccessorBox360<3, 3> &cache) {
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
    Snowy::Do(*chunk, cache, accessor);
    AttachedStem::Do(*chunk, cache, accessor);
    Leaves::Do(*chunk, cache, accessor);

    auto output = make_shared<mcfile::stream::FileOutputStream>(file);
    if (!chunk->write(*output)) {
      return false;
    }

    return true;
  }
};

} // namespace je2be::box360
