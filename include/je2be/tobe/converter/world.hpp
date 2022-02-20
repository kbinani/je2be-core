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
    namespace fs = std::filesystem;

    auto temp = mcfile::File::CreateTempDir(fs::temp_directory_path());
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
          auto result = Chunk::Convert(dim, db, *region, cx, cz, mapInfo, entitiesDir);
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
    return PutEntities(dim, db, tempDir);
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
    namespace fs = std::filesystem;

    auto queue = make_unique<hwm::task_queue>(concurrency);
    deque<future<Chunk::Result>> futures;

    auto temp = mcfile::File::CreateTempDir(fs::temp_directory_path());
    if (!temp) {
      return false;
    }
    auto tempDir = *temp;

    bool completed = w.eachRegions([dim, &db, &queue, &futures, concurrency, &ld, &done, progress, numTotalChunks, &options, tempDir](shared_ptr<Region> const &region) {
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
          futures.push_back(move(queue->enqueue(Chunk::Convert, dim, std::ref(db), *region, cx, cz, mapInfo, entitiesDir)));
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
    return PutEntities(dim, db, tempDir);
  }

  static bool PutEntities(mcfile::Dimension d, DbInterface &db, std::filesystem::path temp) {
    using namespace std;
    using namespace mcfile::stream;
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
    for (auto const &i : files) {
      Pos2i chunk = i.first;
      vector<shared_ptr<CompoundTag>> entities;
      for (auto const &file : i.second) {
        auto s = make_shared<FileInputStream>(file);
        InputStreamReader r(s, {.fLittleEndian = true});
        CompoundTag::ReadUntilEos(r, [&entities](auto const &c) { entities.push_back(c); });
      }
      auto buffer = make_shared<ByteStream>();
      OutputStreamWriter writer(buffer, {.fLittleEndian = true});
      for (auto const &tag : entities) {
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
