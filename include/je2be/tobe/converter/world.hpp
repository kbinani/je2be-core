#pragma once

namespace je2be::tobe {

class World {
  World() = delete;

public:
  [[nodiscard]] static bool Convert(mcfile::je::World const &w, mcfile::Dimension dim, DbInterface &db, WorldData &wd, unsigned int concurrency, Progress *progress, uint32_t &done, double const numTotalChunks) {
    using namespace std;
    using namespace mcfile;
    using namespace mcfile::je;

    auto queue = make_unique<hwm::task_queue>(concurrency);
    deque<future<Chunk::Result>> futures;

    bool completed = w.eachRegions([dim, &db, &queue, &futures, concurrency, &wd, &done, progress, numTotalChunks](shared_ptr<Region> const &region) {
      JavaEditionMap const &mapInfo = wd.fJavaEditionMap;
      for (int cx = region->minChunkX(); cx <= region->maxChunkX(); cx++) {
        for (int cz = region->minChunkZ(); cz <= region->maxChunkZ(); cz++) {
          vector<future<Chunk::Result>> drain;
          FutureSupport::Drain<Chunk::Result>(concurrency + 1, futures, drain);
          for (auto &f : drain) {
            Chunk::Result result = f.get();
            done++;
            if (!result.fData) {
              continue;
            }
            result.fData->drain(wd);
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
      result.fData->drain(wd);
    }

    return completed;
  }
};

} // namespace je2be::tobe
