#include "lce/_terraform.hpp"

#include <je2be/lce/progress.hpp>

#include "_nullable.hpp"
#include "_poi-blocks.hpp"
#include "_queue2d.hpp"
#include "lce/_chunk.hpp"
#include "lce/_world.hpp"
#include "terraform/_chorus-plant.hpp"
#include "terraform/_door.hpp"
#include "terraform/_fence-connectable.hpp"
#include "terraform/_leaves.hpp"
#include "terraform/_note-block.hpp"
#include "terraform/_redstone-wire.hpp"
#include "terraform/_shape-of-stairs.hpp"
#include "terraform/_snowy.hpp"
#include "terraform/_wall-connectable.hpp"
#include "terraform/xbox360/_attached-stem.hpp"
#include "terraform/xbox360/_bed.hpp"
#include "terraform/xbox360/_block-accessor-box360.hpp"
#include "terraform/xbox360/_chest.hpp"
#include "terraform/xbox360/_kelp.hpp"

#include <latch>
#include <mutex>
#include <thread>

namespace je2be::lce {

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
  static Status Do(mcfile::Dimension dim, std::filesystem::path const &poiDirectory, std::filesystem::path const &directory, unsigned int concurrency, Progress *progress, u64 progressChunksOffset) {
    PoiBlocks poi;
    Status st = MultiThread(poi, directory, concurrency, progress, progressChunksOffset, dim);
    if (!st.ok()) {
      return JE2BE_ERROR_PUSH(st);
    }
    if (poi.write(poiDirectory, Chunk::kTargetDataVersion)) {
      return Status::Ok();
    } else {
      return JE2BE_ERROR;
    }
  }

private:
  static Status MultiThread(PoiBlocks &poi, std::filesystem::path const &directory, unsigned int concurrency, Progress *progress, u64 progressChunksOffset, mcfile::Dimension dim) {
    using namespace std;
    using namespace std::placeholders;

    atomic_bool ok(true);
    atomic_uint64_t count(0);
    unique_ptr<std::latch> latch;
    if (concurrency > 0) {
      latch.reset(new std::latch(concurrency));
    }
    std::latch *latchPtr = latch.get();
    mutex joinMut;
    using Queue = Queue2d<1, false, vector>;
    Queue queue({-32, -32}, 64, 64);
    mutex queueMut;

    auto action = [&queue, &queueMut, &joinMut, latchPtr, directory, &poi, &ok, progress, &count, progressChunksOffset, dim]() {
      while (ok) {
        optional<variant<Queue::Dequeue, Queue::Busy>> next;
        {
          lock_guard<mutex> lock(queueMut);
          next = queue.next();
        }

        if (!next) {
          break;
        }
        if (holds_alternative<Queue::Busy>(*next)) {
          this_thread::sleep_for(chrono::milliseconds(10));
          continue;
        }
        auto q = get<Queue::Dequeue>(*next);
        auto result = DoChunk(q.fRegion.fX, q.fRegion.fZ, directory, dim);
        if (result) {
          lock_guard<mutex> lock(joinMut);
          result->fPoi.mergeInto(poi);
        } else {
          ok = false;
          break;
        }
        {
          lock_guard<mutex> lock(queueMut);
          queue.unlockAround(result->fChunk);
        }
        u64 p = count.fetch_add(1) + 1;
        if (progress && !progress->report({p + progressChunksOffset, World::kProgressWeightTotal})) {
          ok = false;
          break;
        }
      }
      if (latchPtr) {
        latchPtr->count_down();
      }
    };

    vector<thread> threads;
    for (int i = 0; i < (int)concurrency - 1; i++) {
      threads.emplace_back(action);
    }

    action();
    if (latch) {
      latch->wait();
    }

    for (auto &th : threads) {
      th.join();
    }

    if (!ok) {
      return JE2BE_ERROR;
    }
    return Status::Ok();
  }

  struct Result {
    Pos2i fChunk;
    PoiBlocks fPoi;
  };

  static Nullable<Result> DoChunk(int cx, int cz, std::filesystem::path const &directory, mcfile::Dimension d) {
    using namespace std;
    using namespace je2be::terraform;
    using namespace je2be::terraform::box360;

    auto cache = make_shared<terraform::box360::BlockAccessorBox360<3, 3>>(cx - 1, cz - 1, directory);
    auto file = directory / mcfile::je::Region::GetDefaultCompressedChunkNbtFileName(cx, cz);
    auto input = make_shared<mcfile::stream::FileInputStream>(file);
    auto root = CompoundTag::ReadDeflateCompressed(*input, mcfile::Encoding::Java);
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
    Door::Do(*chunk, accessor);
    Bed::Do(*chunk, accessor);

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
    if (!chunk->write(*output, d)) {
      return JE2BE_NULLABLE_NULL;
    }

    return ret;
  }
};

Status Terraform::Do(mcfile::Dimension dim, std::filesystem::path const &poiDirectory, std::filesystem::path const &directory, unsigned int concurrency, Progress *progress, u64 progressChunksOffset) {
  return Impl::Do(dim, poiDirectory, directory, concurrency, progress, progressChunksOffset);
}

} // namespace je2be::lce
