#include <je2be/tobe/converter/world.hpp>

#include <je2be/parallel.hpp>
#include <je2be/tobe/converter/chunk.hpp>
#include <je2be/tobe/converter/java-edition-map.hpp>
#include <je2be/tobe/level-data.hpp>
#include <je2be/tobe/options.hpp>
#include <je2be/tobe/progress.hpp>
#include <je2be/tobe/world-data.hpp>

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

    struct Work {
      Pos2i fChunk;
      optional<Chunk::PlayerAttachedEntities> fPlayerAttachedEntities;
      shared_ptr<Region const> fRegion;
    };
    vector<Work> works;
    w.eachRegions([dim, &ld, &options, &works, &done](shared_ptr<Region> const &region) {
      for (int cx = region->minChunkX(); cx <= region->maxChunkX(); cx++) {
        for (int cz = region->minChunkZ(); cz <= region->maxChunkZ(); cz++) {
          Pos2i chunkPos(cx, cz);
          if (!options.fChunkFilter.empty()) [[unlikely]] {
            if (options.fChunkFilter.find(chunkPos) == options.fChunkFilter.end()) {
              done++;
              continue;
            }
          }
          Work work;
          work.fChunk = chunkPos;
          work.fPlayerAttachedEntities = FindPlayerAttachedEntities(ld, dim, chunkPos);
          work.fRegion = region;
          works.push_back(work);
        }
      }
      return true;
    });

    LevelData const *ldPtr = &ld;
    DbInterface *dbPtr = &db;
    atomic_uint32_t atomicDone(done);
    atomic_bool abortSignal(false);
    Chunk::Result zero;
    zero.fOk = true;
    Chunk::Result result = Parallel::Reduce<Work, Chunk::Result>(
        works,
        zero,
        [dim, dbPtr, ldPtr, tempDir, progress, &atomicDone, numTotalChunks, &abortSignal](Work const &work) -> Chunk::Result {
          if (abortSignal.load()) {
            return {};
          }
          int cx = work.fChunk.fX;
          int cz = work.fChunk.fZ;
          fs::path entitiesDir = tempDir / ("c." + to_string(cx) + "." + to_string(cz));
          auto ret = Chunk::Convert(
              dim,
              *dbPtr,
              *work.fRegion,
              cx, cz,
              ldPtr->fJavaEditionMap,
              entitiesDir,
              work.fPlayerAttachedEntities,
              ldPtr->fGameTick,
              ldPtr->fDifficultyBedrock,
              ldPtr->fAllowCommand,
              ldPtr->fGameType);
          auto p = atomicDone.fetch_add(1) + 1;
          if (progress) {
            bool cont = progress->report(Progress::Phase::Convert, p, numTotalChunks);
            if (!cont) {
              abortSignal.store(true);
            }
          }
          return ret;
        },
        [](Chunk::Result const &from, Chunk::Result &to) -> void {
          if (!from.fOk || !to.fOk) {
            return;
          }
          if (from.fData && to.fData) {
            from.fData->drain(*to.fData);
          } else if (from.fData) {
            to.fData = from.fData;
          }
        });
    if (!result.fOk || abortSignal.load()) {
      return false;
    }
    if (result.fData) {
      result.fData->drain(ld);
    }
    done = atomicDone.load();

    return PutWorldEntities(dim, db, tempDir, concurrency);
  }

  static std::optional<Chunk::PlayerAttachedEntities> FindPlayerAttachedEntities(LevelData &ld, mcfile::Dimension dim, Pos2i chunkPos) {
    using namespace std;
    if (!ld.fPlayerAttachedEntities) {
      return nullopt;
    }
    if (ld.fPlayerAttachedEntities->fDim != dim) {
      return nullopt;
    }
    if (ld.fPlayerAttachedEntities->fVehicle || !ld.fPlayerAttachedEntities->fShoulderRiders.empty()) {
      Chunk::PlayerAttachedEntities pae;
      if (ld.fPlayerAttachedEntities->fVehicle && ld.fPlayerAttachedEntities->fVehicle->fChunk == chunkPos) {
        pae.fLocalPlayerUid = ld.fPlayerAttachedEntities->fLocalPlayerUid;
        pae.fVehicle = ld.fPlayerAttachedEntities->fVehicle->fVehicle;
        pae.fPassengers.swap(ld.fPlayerAttachedEntities->fVehicle->fPassengers);
        ld.fPlayerAttachedEntities->fVehicle = std::nullopt;
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
    return nullopt;
  }

  static bool PutWorldEntities(mcfile::Dimension d, DbInterface &db, std::filesystem::path temp, unsigned int concurrency) {
    using namespace std;
    using namespace std::placeholders;
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

    vector<pair<Pos2i, vector<fs::path>>> works;
    for (auto const &it : files) {
      works.push_back(it);
    }
    bool ok = Parallel::Reduce<pair<Pos2i, vector<fs::path>>, bool>(
        works,
        true,
        bind(PutChunkEntities, d, _1, &db),
        Parallel::Merge);
    return ok;
  }

  static bool PutChunkEntities(mcfile::Dimension d, std::pair<Pos2i, std::vector<std::filesystem::path>> item, DbInterface *db) {
    using namespace std;
    using namespace mcfile;
    using namespace mcfile::stream;
    using namespace mcfile::be;
    Pos2i chunk = item.first;
    string digp;
    for (auto const &file : item.second) {
      auto s = make_shared<FileInputStream>(file);
      if (!s->valid()) {
        return false;
      }
      InputStreamReader r(s, mcfile::Endian::Little);
      CompoundTag::ReadUntilEos(r, [db, &digp](auto const &c) {
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
        db->put(key, leveldb::Slice(*value));
      });
    }
    auto key = mcfile::be::DbKey::Digp(chunk.fX, chunk.fZ, d);
    if (digp.empty()) {
      db->del(key);
    } else {
      db->put(key, leveldb::Slice(digp));
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
