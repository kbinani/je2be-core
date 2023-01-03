#include <je2be/tobe/_region.hpp>

#include <je2be/tobe/progress.hpp>

#include <je2be/tobe/_chunk.hpp>
#include <je2be/tobe/_level-data.hpp>
#include <je2be/tobe/_world-data.hpp>

namespace je2be::tobe {

class Region::Impl {
  Impl() = delete;

public:
  static std::shared_ptr<WorldData> Convert(
      mcfile::Dimension dim,
      std::shared_ptr<mcfile::je::Region> const &region,
      Options const &options,
      std::filesystem::path const &worldTempDir,
      LevelData const &levelData,
      DbInterface &db,
      Progress *progress,
      std::atomic_uint32_t &done,
      double const numTotalChunks,
      std::atomic_bool &abortSignal) {
    using namespace std;
    namespace fs = std::filesystem;

    auto sum = make_shared<WorldData>(dim);
    for (int cx = region->minChunkX(); cx <= region->maxChunkX(); cx++) {
      for (int cz = region->minChunkZ(); cz <= region->maxChunkZ(); cz++) {
        if (abortSignal) {
          return {};
        }
        Pos2i chunkPos(cx, cz);
        if (!options.fChunkFilter.empty()) [[unlikely]] {
          if (options.fChunkFilter.find(chunkPos) == options.fChunkFilter.end()) {
            done.fetch_add(1);
            continue;
          }
        }
        auto pae = FindPlayerAttachedEntities(levelData, dim, chunkPos);
        fs::path entitiesDir = worldTempDir / ("c." + to_string(cx) + "." + to_string(cz));
        auto ret = Chunk::Convert(
            dim,
            db,
            *region,
            cx, cz,
            levelData.fJavaEditionMap,
            entitiesDir,
            pae,
            levelData.fGameTick,
            levelData.fDifficultyBedrock,
            levelData.fAllowCommand,
            levelData.fGameType);
        auto p = done.fetch_add(1) + 1;
        if (progress) {
          bool cont = progress->report(Progress::Phase::Convert, p, numTotalChunks);
          if (!cont) {
            abortSignal.store(true);
          }
        }
        if (!ret.fOk) {
          abortSignal.store(true);
          return {};
        }
        if (ret.fData) {
          ret.fData->drain(*sum);
        }
      }
    }
    return sum;
  }

private:
  static std::optional<Chunk::PlayerAttachedEntities> FindPlayerAttachedEntities(LevelData const &ld, mcfile::Dimension dim, Pos2i chunkPos) {
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
        pae.fPassengers = ld.fPlayerAttachedEntities->fVehicle->fPassengers;
      }
      for (int i = 0; i < ld.fPlayerAttachedEntities->fShoulderRiders.size(); i++) {
        auto const &rider = ld.fPlayerAttachedEntities->fShoulderRiders[i];
        if (rider.first == chunkPos) {
          pae.fShoulderRiders.push_back(rider.second);
        }
      }
      return pae;
    }
    return nullopt;
  }
};

std::shared_ptr<WorldData> Region::Convert(
    mcfile::Dimension dim,
    std::shared_ptr<mcfile::je::Region> const &region,
    Options const &options,
    std::filesystem::path const &worldTempDir,
    LevelData const &levelData,
    DbInterface &db,
    Progress *progress,
    std::atomic_uint32_t &done,
    double const numTotalChunks,
    std::atomic_bool &abortSignal) {
  return Impl::Convert(dim, region, options, worldTempDir, levelData, db, progress, done, numTotalChunks, abortSignal);
}

} // namespace je2be::tobe
