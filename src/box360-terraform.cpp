#include "box360/_terraform.hpp"

#include <je2be/box360/progress.hpp>

#include "_nullable.hpp"
#include "_poi-blocks.hpp"
#include "box360/_chunk.hpp"
#include "terraform/_chorus-plant.hpp"
#include "terraform/_fence-connectable.hpp"
#include "terraform/_leaves.hpp"
#include "terraform/_note-block.hpp"
#include "terraform/_redstone-wire.hpp"
#include "terraform/_shape-of-stairs.hpp"
#include "terraform/_snowy.hpp"
#include "terraform/_wall-connectable.hpp"
#include "terraform/box360/_attached-stem.hpp"
#include "terraform/box360/_block-accessor-box360.hpp"
#include "terraform/box360/_chest.hpp"
#include "terraform/box360/_kelp.hpp"

#include <latch>

namespace je2be::box360 {

class Terraform::Impl {
  Impl() = delete;

  enum {
    cx0 = -32,
    cz0 = -32,
    cx1 = 31,
    cz1 = 31,

    width = cx1 - cx0 + 1,
    height = cz1 - cz0 + 1,
  };

public:
  static Status Do(mcfile::Dimension dim, std::filesystem::path const &poiDirectory, std::filesystem::path const &directory, unsigned int concurrency, Progress *progress, int progressChunksOffset) {
    PoiBlocks poi;
    Status st = MultiThread(poi, directory, concurrency, progress, progressChunksOffset);
    if (!st.ok()) {
      return st;
    }
    if (poi.write(poiDirectory, Chunk::kTargetDataVersion)) {
      return Status::Ok();
    } else {
      return JE2BE_ERROR;
    }
  }

private:
  static Status MultiThread(PoiBlocks &poi, std::filesystem::path const &directory, unsigned int concurrency, Progress *progress, int progressChunksOffset) {
    using namespace std;
    using namespace std::placeholders;

    bool running[width][height];
    bool done[width][height];
    for (int i = 0; i < width; i++) {
      fill_n(running[i], height, false);
      fill_n(done[i], height, false);
    }
    atomic_bool ok(true);
    optional<Status> error;
    atomic_int count(0);
    mutex queueMut;
    std::latch latch(concurrency);
    mutex joinMut;

    auto action = [&queueMut, &joinMut, &done, &running, &latch, directory, &poi, &ok, progress, &count, progressChunksOffset]() {
      while (ok) {
        optional<Pos2i> next;
        {
          lock_guard<mutex> lock(queueMut);
          next = NextQueue(done, running);
          if (next) {
            MarkRunning(next->fX, next->fZ, running);
          }
        }
        if (!next) {
          break;
        }
        auto result = DoChunk(next->fX, next->fZ, directory);
        if (result) {
          lock_guard<mutex> lock(joinMut);
          result->fPoi.mergeInto(poi);
        } else {
          ok = false;
          break;
        }
        {
          lock_guard<mutex> lock(queueMut);
          MarkFinished(result->fChunk.fX, result->fChunk.fZ, done, running);
        }
        auto p = count.fetch_add(1) + 1;
        if (progress && !progress->report(p + progressChunksOffset, 8192 * 3)) {
          ok = false;
          break;
        }
      }
      latch.count_down();
    };

    vector<thread> threads;
    for (int i = 0; i < (int)concurrency - 1; i++) {
      threads.push_back(thread(action));
    }

    action();
    latch.wait();

    for (auto &th : threads) {
      th.join();
    }

    if (!ok) {
      return error ? *error : JE2BE_ERROR;
    }
    return Status::Ok();
  }

  static void MarkFinished(int cx, int cz, bool done[width][height], bool running[width][height]) {
    int x = cx - cx0;
    int z = cz - cz0;
    done[x][z] = true;

    for (int i = -1; i <= 1; i++) {
      int tx = x + i;
      if (tx < 0 || width <= tx) {
        continue;
      }
      for (int j = -1; j <= 1; j++) {
        int tz = z + j;
        if (tz < 0 || height <= tz) {
          continue;
        }
        running[tx][tz] = false;
      }
    }
  }

  static void MarkRunning(int cx, int cz, bool running[width][height]) {
    int x = cx - cx0;
    int z = cz - cz0;

    for (int i = -1; i <= 1; i++) {
      int tx = x + i;
      if (tx < 0 || width <= tx) {
        continue;
      }
      for (int j = -1; j <= 1; j++) {
        int tz = z + j;
        if (tz < 0 || height <= tz) {
          continue;
        }
        running[tx][tz] = true;
      }
    }
  }

  static bool IsComplete(bool done[width][height]) {
    for (int i = 0; i < width; i++) {
      for (int j = 0; j < height; j++) {
        if (!done[i][j]) {
          return false;
        }
      }
    }
    return true;
  }

  static std::optional<Pos2i> NextQueue(bool done[width][height], bool running[width][height]) {
    using namespace std;
    for (int x = 0; x < width; x++) {
      for (int z = 0; z < height; z++) {
        if (done[x][z]) {
          continue;
        }
        bool ok = true;
        for (int i = -1; i <= 1 && ok; i++) {
          int tx = x + i;
          if (tx < 0 || width <= tx) {
            continue;
          }
          for (int j = -1; j <= 1; j++) {
            int tz = z + j;
            if (tz < 0 || height <= tz) {
              continue;
            }
            if (running[tx][tz]) {
              ok = false;
              break;
            }
          }
        }
        if (ok) {
          return Pos2i(cx0 + x, cz0 + z);
        }
      }
    }
    return nullopt;
  }

  struct Result {
    Pos2i fChunk;
    PoiBlocks fPoi;
  };

  static Nullable<Result> DoChunk(int cx, int cz, std::filesystem::path const &directory) {
    using namespace std;
    using namespace je2be::terraform;
    using namespace je2be::terraform::box360;

    auto cache = make_shared<terraform::box360::BlockAccessorBox360<3, 3>>(cx - 1, cz - 1, directory);
    auto file = directory / mcfile::je::Region::GetDefaultCompressedChunkNbtFileName(cx, cz);
    auto input = make_shared<mcfile::stream::FileInputStream>(file);
    auto root = CompoundTag::ReadCompressed(*input, mcfile::Endian::Big);
    Result ret;
    ret.fChunk = Pos2i(cx, cz);
    if (!root) {
      return ret;
    }
    input.reset();
    auto chunk = mcfile::je::WritableChunk::MakeChunk(cx, cz, root);
    if (!chunk) {
      return JE2BE_NULLABLE_NULL;
    }
    cache->set(cx, cz, chunk);

    BlockPropertyAccessorJava accessor(*chunk);

    ShapeOfStairs::Do(*chunk, *cache, accessor);
    FenceConnectable::Do(*chunk, *cache, accessor);
    RedstoneWire::Do(*chunk, *cache, accessor);
    ChorusPlant::Do(*chunk, *cache, accessor);
    WallConnectable::Do(*chunk, *cache, accessor);
    Kelp::Do(*chunk, accessor);
    Snowy::Do(*chunk, *cache, accessor);
    AttachedStem::Do(*chunk, *cache, accessor);
    Leaves::Do(*chunk, *cache, accessor);
    Chest::Do(*chunk, *cache, accessor);
    NoteBlock::Do(*chunk, *cache, accessor);

    for (auto const &section : chunk->fSections) {
      if (!section) {
        continue;
      }
      bool hasPoiBlock = false;
      section->eachBlockPalette([&hasPoiBlock](shared_ptr<mcfile::je::Block const> const &block, size_t) {
        if (PoiBlocks::Interest(*block)) {
          hasPoiBlock = true;
          return false;
        }
        return true;
      });
      if (hasPoiBlock) {
        for (int y = 0; y < 16; y++) {
          for (int z = 0; z < 16; z++) {
            for (int x = 0; x < 16; x++) {
              auto block = section->blockAt(x, y, z);
              if (!block) {
                continue;
              }
              if (!PoiBlocks::Interest(*block)) {
                continue;
              }
              int bx = chunk->fChunkX * 16 + x;
              int by = section->y() * 16 + y;
              int bz = chunk->fChunkZ * 16 + z;
              ret.fPoi.add({bx, by, bz}, block->fId);
            }
          }
        }
      }
    }

    auto output = make_shared<mcfile::stream::FileOutputStream>(file);
    if (!chunk->write(*output)) {
      return JE2BE_NULLABLE_NULL;
    }

    return ret;
  }
};

Status Terraform::Do(mcfile::Dimension dim, std::filesystem::path const &poiDirectory, std::filesystem::path const &directory, unsigned int concurrency, Progress *progress, int progressChunksOffset) {
  return Impl::Do(dim, poiDirectory, directory, concurrency, progress, progressChunksOffset);
}

} // namespace je2be::box360
