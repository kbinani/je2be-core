#include <je2be/tobe/converter/world.hpp>

#include <je2be/future-support.hpp>
#include <je2be/tobe/converter/chunk.hpp>
#include <je2be/tobe/converter/java-edition-map.hpp>
#include <je2be/tobe/level-data.hpp>
#include <je2be/tobe/options.hpp>
#include <je2be/tobe/progress.hpp>
#include <je2be/tobe/world-data.hpp>

#include <hwm/task/task_queue.hpp>

namespace je2be::tobe {

class World::Impl {
  Impl() = delete;

public:
  [[nodiscard]] static bool Convert(
      mcfile::je::World const &w,
      mcfile::Dimension dim,
      DbInterface &db,
      LevelData &ld,
      unsigned int concurrency,
      Progress *progress,
      uint32_t &done,
      double const numTotalChunks,
      Options const &options) {
    using namespace std;
    using namespace mcfile;
    using namespace mcfile::je;
    namespace fs = std::filesystem;

    auto queue = make_unique<hwm::task_queue>(concurrency);
    deque<future<Chunk::Result>> futures;

    auto temp = mcfile::File::CreateTempDir(options.getTempDirectory());
    if (!temp) {
      return false;
    }
    auto tempDir = *temp;

    bool completed = w.eachRegions([dim, &db, &queue, &futures, concurrency, &ld, &done, progress, numTotalChunks, &options, tempDir](shared_ptr<Region> const &region) {
      JavaEditionMap const &mapInfo = ld.fJavaEditionMap;
      for (int cx = region->minChunkX(); cx <= region->maxChunkX(); cx++) {
        for (int cz = region->minChunkZ(); cz <= region->maxChunkZ(); cz++) {
          Pos2i chunkPos(cx, cz);
          if (!options.fChunkFilter.empty()) [[unlikely]] {
            if (options.fChunkFilter.find(chunkPos) == options.fChunkFilter.end()) {
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
            if (!result.fOk) {
              return false;
            }
            result.fData->drain(ld);
          }

          if (progress) {
            bool continue_ = progress->report(Progress::Phase::Convert, done, numTotalChunks);
            if (!continue_) {
              return false;
            }
          }

          fs::path entitiesDir = tempDir / ("c." + to_string(cx) + "." + to_string(cz));
          optional<Chunk::PlayerAttachedEntities> playerAttachedEntities;
          if (ld.fPlayerAttachedEntities && ld.fPlayerAttachedEntities->fDim == dim) {
            if (ld.fPlayerAttachedEntities->fVehicle || !ld.fPlayerAttachedEntities->fShoulderRiders.empty()) {
              Chunk::PlayerAttachedEntities pae;
              if (ld.fPlayerAttachedEntities->fVehicle && ld.fPlayerAttachedEntities->fVehicle->fChunk == chunkPos) {
                pae.fLocalPlayerUid = ld.fPlayerAttachedEntities->fLocalPlayerUid;
                pae.fVehicle = ld.fPlayerAttachedEntities->fVehicle->fVehicle;
                pae.fPassengers.swap(ld.fPlayerAttachedEntities->fVehicle->fPassengers);
                ld.fPlayerAttachedEntities->fVehicle = nullopt;
              }
              for (int i = 0; i < ld.fPlayerAttachedEntities->fShoulderRiders.size(); i++) {
                auto const &rider = ld.fPlayerAttachedEntities->fShoulderRiders[i];
                if (rider.first == chunkPos) {
                  pae.fShoulderRiders.push_back(rider.second);
                  ld.fPlayerAttachedEntities->fShoulderRiders.erase(ld.fPlayerAttachedEntities->fShoulderRiders.begin() + i);
                  i--;
                }
              }
              playerAttachedEntities = pae;
            }
          }
          futures.push_back(queue->enqueue(Chunk::Convert, dim, std::ref(db), *region, cx, cz, mapInfo, entitiesDir, playerAttachedEntities, ld.fGameTick, ld.fDifficultyBedrock, ld.fAllowCommand, ld.fGameType));
        }
      }
      return true;
    });

    for (auto &f : futures) {
      Chunk::Result result = f.get();
      done++;
      if (progress) {
        progress->report(Progress::Phase::Convert, done, numTotalChunks);
      }
      if (!result.fData) {
        continue;
      }
      completed = completed && result.fOk;
      if (!result.fOk) {
        continue;
      }
      result.fData->drain(ld);
    }

    if (!completed) {
      return false;
    }

    queue.reset();

    return PutWorldEntities(dim, db, tempDir, concurrency);
  }

  static bool PutWorldEntities(mcfile::Dimension d, DbInterface &db, std::filesystem::path temp, unsigned int concurrency) {
    using namespace std;
    namespace fs = std::filesystem;
    error_code ec;
    unordered_map<Pos2i, vector<fs::path>, Pos2iHasher> files;
    for (auto i : fs::directory_iterator(temp, ec)) {
      auto path = i.path();
      if (!fs::is_directory(path)) {
        continue;
      }
      error_code ec1;
      for (auto j : fs::directory_iterator(path, ec1)) {
        auto p = j.path();
        if (!fs::is_regular_file(p)) {
          continue;
        }
        auto name = p.filename().string();
        auto tokens = mcfile::String::Split(name, '.');
        if (tokens.size() != 4) {
          continue;
        }
        if (tokens[0] != "c" || tokens[3] != "nbt") {
          continue;
        }
        auto cx = strings::Toi(tokens[1]);
        auto cz = strings::Toi(tokens[2]);
        if (!cx || !cz) {
          continue;
        }
        files[{*cx, *cz}].push_back(p);
      }
    }
    deque<future<bool>> futures;
    unique_ptr<hwm::task_queue> pool;
    if (concurrency > 0) {
      pool.reset(new hwm::task_queue(concurrency));
    }
    bool ok = true;
    for (auto const &i : files) {
      if (pool) {
        vector<future<bool>> drained;
        FutureSupport::Drain(concurrency + 1, futures, drained);
        for (auto &f : drained) {
          ok = ok && f.get();
        }
        if (!ok) {
          break;
        }
        futures.push_back(pool->enqueue(PutChunkEntities, d, i.first, i.second, ref(db)));
      } else {
        if (!PutChunkEntities(d, i.first, i.second, db)) {
          return false;
        }
      }
    }
    for (auto &f : futures) {
      ok = ok && f.get();
    }
    return ok;
  }

  static bool PutChunkEntities(mcfile::Dimension d, Pos2i chunk, std::vector<std::filesystem::path> files, DbInterface &db) {
    using namespace std;
    using namespace mcfile;
    using namespace mcfile::stream;
    using namespace mcfile::be;
    string digp;
    for (auto const &file : files) {
      auto s = make_shared<FileInputStream>(file);
      if (!s->valid()) {
        return false;
      }
      InputStreamReader r(s, mcfile::Endian::Little);
      CompoundTag::ReadUntilEos(r, [&db, &digp](auto const &c) {
        auto id = c->int64("UniqueID");
        if (!id) {
          return;
        }
        int64_t v = *id;
        string prefix;
        prefix.assign((char const *)&v, sizeof(v));
        digp += prefix;

        auto key = DbKey::Actorprefix(prefix);
        auto value = CompoundTag::Write(*c, Endian::Little);
        if (!value) {
          return;
        }
        db.put(key, leveldb::Slice(*value));
      });
    }
    auto key = mcfile::be::DbKey::Digp(chunk.fX, chunk.fZ, d);
    if (digp.empty()) {
      db.del(key);
    } else {
      db.put(key, leveldb::Slice(digp));
    }
    return true;
  }
};

bool World::Convert(
    mcfile::je::World const &w,
    mcfile::Dimension dim,
    DbInterface &db,
    LevelData &ld,
    unsigned int concurrency,
    Progress *progress,
    uint32_t &done,
    double const numTotalChunks,
    Options const &options) {
  return Impl::Convert(w, dim, db, ld, concurrency, progress, done, numTotalChunks, options);
}

} // namespace je2be::tobe
