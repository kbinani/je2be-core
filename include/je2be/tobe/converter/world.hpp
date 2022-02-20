#pragma once

namespace je2be::tobe {

class World {
  World() = delete;

public:
  [[nodiscard]] static bool ConvertSingleThread(
      mcfile::je::World const &w,
      mcfile::Dimension dim,
      DbInterface &db,
      LevelData &ld,
      Progress *progress,
      uint32_t &done,
      double const numTotalChunks,
      InputOption const &options) {
    using namespace std;
    using namespace mcfile;
    using namespace mcfile::je;
    unordered_map<Pos2i, vector<shared_ptr<CompoundTag>>, Pos2iHasher> entities;
    bool ok = w.eachRegions([dim, &db, &ld, &done, progress, numTotalChunks, &options, &entities](shared_ptr<Region> const &region) {
      JavaEditionMap const &mapInfo = ld.fJavaEditionMap;
      for (int cx = region->minChunkX(); cx <= region->maxChunkX(); cx++) {
        for (int cz = region->minChunkZ(); cz <= region->maxChunkZ(); cz++) {
          done++;
          if (!options.fChunkFilter.empty()) [[unlikely]] {
            if (options.fChunkFilter.find(Pos2i(cx, cz)) == options.fChunkFilter.end()) {
              continue;
            }
          }
          auto result = Chunk::Convert(dim, db, *region, cx, cz, mapInfo);
          if (progress) {
            bool continue_ = progress->report(Progress::Phase::Convert, done, numTotalChunks);
            if (!continue_) {
              return false;
            }
          }

          if (!result.fData) {
            continue;
          }
          result.fData->drain(ld);
          result.fData->drainEntities(entities);
          if (!result.fOk) {
            return false;
          }
        }
      }
      return true;
    });
    if (ok) {
      ok = PutEntities(dim, db, entities);
    }
    return ok;
  }

  [[nodiscard]] static bool ConvertMultiThread(
      mcfile::je::World const &w,
      mcfile::Dimension dim,
      DbInterface &db,
      LevelData &ld,
      unsigned int concurrency,
      Progress *progress,
      uint32_t &done,
      double const numTotalChunks,
      InputOption const &options) {
    using namespace std;
    using namespace mcfile;
    using namespace mcfile::je;

    auto queue = make_unique<hwm::task_queue>(concurrency);
    deque<future<Chunk::Result>> futures;
    unordered_map<Pos2i, vector<shared_ptr<CompoundTag>>, Pos2iHasher> entities;

    bool completed = w.eachRegions([dim, &db, &queue, &futures, concurrency, &ld, &done, progress, numTotalChunks, &options, &entities](shared_ptr<Region> const &region) {
      JavaEditionMap const &mapInfo = ld.fJavaEditionMap;
      for (int cx = region->minChunkX(); cx <= region->maxChunkX(); cx++) {
        for (int cz = region->minChunkZ(); cz <= region->maxChunkZ(); cz++) {
          if (!options.fChunkFilter.empty()) [[unlikely]] {
            if (options.fChunkFilter.find(Pos2i(cx, cz)) == options.fChunkFilter.end()) {
              done++;
              continue;
            }
          }
          vector<future<Chunk::Result>> drain;
          FutureSupport::Drain<Chunk::Result>(concurrency + 1, futures, drain);
          for (auto &f : drain) {
            Chunk::Result result = f.get();
            done++;
            if (!result.fData) {
              continue;
            }
            result.fData->drain(ld);
            result.fData->drainEntities(entities);
            if (!result.fOk) {
              return false;
            }
          }

          if (progress) {
            bool continue_ = progress->report(Progress::Phase::Convert, done, numTotalChunks);
            if (!continue_) {
              return false;
            }
          }

          futures.push_back(move(queue->enqueue(Chunk::Convert, dim, std::ref(db), *region, cx, cz, mapInfo)));
        }
      }
      return true;
    });

    for (auto &f : futures) {
      Chunk::Result const &result = f.get();
      done++;
      if (progress) {
        progress->report(Progress::Phase::Convert, done, numTotalChunks);
      }
      if (!result.fData) {
        continue;
      }
      result.fData->drain(ld);
      result.fData->drainEntities(entities);
    }

    if (completed) {
      completed = PutEntities(dim, db, entities);
    }

    return completed;
  }

  static bool PutEntities(mcfile::Dimension d, DbInterface &db, std::unordered_map<Pos2i, std::vector<std::shared_ptr<CompoundTag>>, Pos2iHasher> const &entities) {
    using namespace std;
    using namespace mcfile::stream;
    for (auto const &it : entities) {
      Pos2i chunk = it.first;
      auto buffer = make_shared<ByteStream>();
      OutputStreamWriter writer(buffer, {.fLittleEndian = true});
      for (auto const &tag : it.second) {
        if (!tag->writeAsRoot(writer)) {
          return false;
        }
      }
      auto key = mcfile::be::DbKey::Entity(chunk.fX, chunk.fZ, d);
      string value;
      buffer->drain(value);
      if (value.empty()) {
        db.del(key);
      } else {
        db.put(key, leveldb::Slice(value));
      }
    }
    return true;
  }
};

} // namespace je2be::tobe
