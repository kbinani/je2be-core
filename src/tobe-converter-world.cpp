#include <je2be/tobe/converter/world.hpp>

#include <je2be/tobe/converter/chunk.hpp>
#include <je2be/tobe/converter/java-edition-map.hpp>
#include <je2be/tobe/level-data.hpp>
#include <je2be/tobe/options.hpp>
#include <je2be/tobe/progress.hpp>
#include <je2be/tobe/world-data.hpp>

#include <oneapi/tbb/concurrent_vector.h>
#include <oneapi/tbb/parallel_for_each.h>

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

    auto temp = mcfile::File::CreateTempDir(options.getTempDirectory());
    if (!temp) {
      return false;
    }
    auto tempDir = *temp;

    struct WorkItem {
      WorkItem(shared_ptr<mcfile::je::Region> region, int cx, int cz, optional<Chunk::PlayerAttachedEntities> pae) : fRegion(region), fChunkX(cx), fChunkZ(cz), fPlayerAttachedEntities(pae) {}

      shared_ptr<mcfile::je::Region> fRegion;
      int fChunkX;
      int fChunkZ;
      optional<Chunk::PlayerAttachedEntities> fPlayerAttachedEntities;
    };

    vector<WorkItem> regions;
    w.eachRegions([&](auto region) {
      for (int cx = region->minChunkX(); cx <= region->maxChunkX(); cx++) {
        for (int cz = region->minChunkZ(); cz <= region->maxChunkZ(); cz++) {
          Pos2i chunkPos(cx, cz);
          if (!options.fChunkFilter.empty()) [[unlikely]] {
            if (options.fChunkFilter.find(chunkPos) == options.fChunkFilter.end()) {
              done++;
              continue;
            }
          }
          auto playerAttachedEntities = ErasePlayerAttachedEntity(dim, ld, tempDir, cx, cz);
          regions.push_back({region, cx, cz, playerAttachedEntities});
        }
      }
      return true;
    });

    oneapi::tbb::concurrent_vector<Chunk::Result> results;
    JavaEditionMap const &mapInfo = ld.fJavaEditionMap;

    auto gameTick = ld.fGameTick;
    auto difficultyB = ld.fDifficultyBedrock;
    auto allowCommand = ld.fAllowCommand;
    auto gameType = ld.fGameType;
    oneapi::tbb::parallel_for_each(regions.begin(), regions.end(), [dim, &db, mapInfo, tempDir, gameTick, difficultyB, allowCommand, gameType, &results](WorkItem const &item) {
      int cx = item.fChunkX;
      int cz = item.fChunkZ;
      fs::path entitiesDir = tempDir / ("c." + to_string(cx) + "." + to_string(cz));
      auto ret = Chunk::Convert(dim, db, *item.fRegion, cx, cz, mapInfo, entitiesDir, item.fPlayerAttachedEntities, gameTick, difficultyB, allowCommand, gameType);
      results.push_back(ret);
    });
    for (auto result : results) {
      if (result.fOk && result.fData) {
        result.fData->drain(ld);
      }
    }
    return PutWorldEntities(dim, db, tempDir, concurrency);
  }

  static std::optional<Chunk::PlayerAttachedEntities> ErasePlayerAttachedEntity(mcfile::Dimension dim, LevelData &ld, std::filesystem::path tempDir, int cx, int cz) {
    using namespace std;
    namespace fs = std::filesystem;

    fs::path entitiesDir = tempDir / ("c." + to_string(cx) + "." + to_string(cz));
    Pos2i chunkPos(cx, cz);

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
        return pae;
      }
    }
    return nullopt;
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
    atomic_bool ok = true;
    oneapi::tbb::parallel_for_each(files.begin(), files.end(), [&](auto it) {
      if (!ok.load()) {
        return;
      }
      Pos2i chunk = it.first;
      vector<fs::path> const &files = it.second;
      if (!PutChunkEntities(d, chunk, files, db)) {
        ok.store(false);
      }
    });
    return ok.load();
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
