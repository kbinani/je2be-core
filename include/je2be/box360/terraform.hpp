#pragma once

namespace je2be::box360 {

class Terraform {
  Terraform() = delete;

  enum {
    cx0 = -32,
    cz0 = -32,
    cx1 = 31,
    cz1 = 31,

    width = cx1 - cx0 + 1,
    height = cz1 - cz0 + 1,
  };

public:
  static bool Do(std::filesystem::path const &directory, unsigned int concurrency) {
    if (concurrency > 0) {
      return MultiThread(directory, concurrency);
    } else {
      return SingleThread(directory);
    }
  }

private:
  static bool MultiThread(std::filesystem::path const &directory, unsigned int concurrency) {
    using namespace std;
    bool running[width][height];
    bool done[width][height];
    for (int i = 0; i < width; i++) {
      fill_n(running[i], height, false);
      fill_n(done[i], height, false);
    }
    unique_ptr<hwm::task_queue> queue(new hwm::task_queue(concurrency));
    deque<future<optional<Pos2i>>> futures;
    bool ok = true;
    while (ok) {
      vector<future<optional<Pos2i>>> drain;
      FutureSupport::Drain(concurrency + 1, futures, drain);
      for (auto &f : drain) {
        auto result = f.get();
        if (result) {
          MarkFinished(result->fX, result->fZ, done, running);
        } else {
          ok = false;
        }
      }
      if (!ok || IsComplete(done)) {
        break;
      }
      auto next = NextQueue(done, running);
      if (next) {
        MarkRunning(next->fX, next->fZ, running);
        futures.push_back(move(queue->enqueue(DoChunk, next->fX, next->fZ, directory)));
      } else {
        assert(!futures.empty());
        auto result = futures.front().get();
        if (result) {
          MarkFinished(result->fX, result->fZ, done, running);
        } else {
          ok = false;
        }
        futures.pop_front();
        if (!ok || IsComplete(done)) {
          break;
        }
      }
    }
    for (auto &f : futures) {
      ok = ok && f.get();
    }
    return ok;
  }

  static bool SingleThread(std::filesystem::path const &directory) {
    for (int cx = cx0; cx <= cx1; cx++) {
      for (int cz = cz0; cz <= cz1; cz++) {
        if (!DoChunk(cx, cz, directory)) {
          return false;
        }
      }
    }
    return true;
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
    bool fOk;
  };

  static std::optional<Pos2i> DoChunk(int cx, int cz, std::filesystem::path const &directory) {
    using namespace std;
    using namespace je2be::terraform;
    using namespace je2be::terraform::box360;

    auto cache = make_shared<terraform::box360::BlockAccessorBox360<3, 3>>(cx - 1, cz - 1, directory);
    auto file = directory / mcfile::je::Region::GetDefaultCompressedChunkNbtFileName(cx, cz);
    auto input = make_shared<mcfile::stream::FileInputStream>(file);
    auto root = CompoundTag::ReadCompressed(*input, endian::big);
    if (!root) {
      return Pos2i(cx, cz);
    }
    input.reset();
    auto chunk = mcfile::je::WritableChunk::MakeChunk(cx, cz, root);
    if (!chunk) {
      return nullopt;
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

    auto output = make_shared<mcfile::stream::FileOutputStream>(file);
    if (!chunk->write(*output)) {
      return nullopt;
    }

    return Pos2i(cx, cz);
  }
};

} // namespace je2be::box360
