#include "java/_region.hpp"

#include <je2be/java/progress.hpp>

#include "java/_chunk.hpp"
#include "java/_level-data.hpp"
#include "java/_world-data.hpp"

namespace je2be::java {

class Region::Impl {
  Impl() = delete;

public:
  static std::shared_ptr<WorldData> Convert(
      mcfile::Dimension dim,
      std::shared_ptr<mcfile::je::Region> const &region,
      Options const &options,
      std::shared_ptr<EntityStore> const &entityStore,
      LevelData const &levelData,
      DbInterface &db,
      Progress *progress,
      std::atomic_uint32_t &done,
      u64 const numTotalChunks,
      std::atomic_bool &abortSignal,
      std::atomic_uint64_t &numConvertedChunks) {
    using namespace std;
    namespace fs = std::filesystem;

    auto sum = make_shared<WorldData>(dim);
    auto terrain = mcfile::je::McaEditor::Open(region->fFilePath);
    if (!terrain) {
      return sum;
    }
    auto entities = mcfile::je::McaEditor::Open(region->entitiesRegionFilePath());

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
        auto ret = Chunk::Convert(
            dim,
            db,
            *terrain,
            entities.get(),
            cx, cz,
            levelData.fJavaEditionMap,
            entityStore,
            pae,
            levelData.fGameTick,
            levelData.fDifficultyBedrock,
            levelData.fAllowCommand,
            levelData.fGameType);
        if (!ret.fOk) {
          abortSignal.store(true);
          return {};
        }
        u64 t;
        if (ret.fData) {
          ret.fData->drain(*sum);
          t = numConvertedChunks.fetch_add(1) + 1;
        } else {
          t = numConvertedChunks.load();
        }
        u64 p = done.fetch_add(1) + 1;
        if (progress) {
          bool cont = progress->reportConvert({p, numTotalChunks}, t);
          if (!cont) {
            abortSignal.store(true);
          }
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
    std::shared_ptr<EntityStore> const &entityStore,
    LevelData const &levelData,
    DbInterface &db,
    Progress *progress,
    std::atomic_uint32_t &done,
    u64 const numTotalChunks,
    std::atomic_bool &abortSignal,
    std::atomic_uint64_t &numConvertedChunks) {
  return Impl::Convert(dim, region, options, entityStore, levelData, db, progress, done, numTotalChunks, abortSignal, numConvertedChunks);
}

} // namespace je2be::java
