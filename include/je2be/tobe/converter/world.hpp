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
      Options const &options) {
    using namespace std;
    using namespace mcfile;
    using namespace mcfile::je;
    namespace fs = std::filesystem;

    auto temp = mcfile::File::CreateTempDir(options.getTempDirectory());
    if (!temp) {
      return false;
    }
    auto tempDir = *temp;

    bool ok = w.eachRegions([dim, &db, &ld, &done, progress, numTotalChunks, &options, tempDir](shared_ptr<Region> const &region) {
      JavaEditionMap const &mapInfo = ld.fJavaEditionMap;
      for (int cx = region->minChunkX(); cx <= region->maxChunkX(); cx++) {
        for (int cz = region->minChunkZ(); cz <= region->maxChunkZ(); cz++) {
          done++;
          if (!options.fChunkFilter.empty()) [[unlikely]] {
            if (options.fChunkFilter.find(Pos2i(cx, cz)) == options.fChunkFilter.end()) {
              continue;
            }
          }
          fs::path entitiesDir = tempDir / ("c." + to_string(cx) + "." + to_string(cz));
          optional<Chunk::PlayerAttachedEntities> playerAttachedEntities;
          if (ld.fPlayerAttachedEntities && ld.fPlayerAttachedEntities->fDim == dim && ld.fPlayerAttachedEntities->fVehicle && ld.fPlayerAttachedEntities->fVehicle->fChunk == Pos2i(cx, cz)) {
            Chunk::PlayerAttachedEntities pae;
            pae.fLocalPlayerUid = ld.fPlayerAttachedEntities->fLocalPlayerUid;
            pae.fVehicle = ld.fPlayerAttachedEntities->fVehicle->fVehicle;
            pae.fPassengers.swap(ld.fPlayerAttachedEntities->fVehicle->fPassengers);
            playerAttachedEntities = pae;
            ld.fPlayerAttachedEntities->fVehicle = nullopt;
          }
          auto result = Chunk::Convert(dim, db, *region, cx, cz, mapInfo, entitiesDir, playerAttachedEntities, ld.fGameTick, ld.fDifficultyBedrock, ld.fAllowCommand, ld.fGameType);
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
          if (!result.fOk) {
            return false;
          }
        }
      }
      return true;
    });
    if (!ok) {
      return false;
    }
    return PutWorldEntities(dim, db, tempDir, 0);
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
            result.fData->drain(ld);
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
      Chunk::Result const &result = f.get();
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
    using namespace mcfile::stream;
    vector<shared_ptr<CompoundTag>> entities;
    for (auto const &file : files) {
      auto s = make_shared<FileInputStream>(file);
      InputStreamReader r(s, mcfile::Endian::Little);
      CompoundTag::ReadUntilEos(r, [&entities](auto const &c) { entities.push_back(c); });
    }
    auto buffer = make_shared<ByteStream>();
    OutputStreamWriter writer(buffer, mcfile::Endian::Little);
    for (auto const &tag : entities) {
      if (!CompoundTag::Write(*tag, writer)) {
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
    return true;
  }
};

} // namespace je2be::tobe
