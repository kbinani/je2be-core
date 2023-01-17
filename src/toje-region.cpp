#include "toje/_region.hpp"

#include <je2be/defer.hpp>

#include "_parallel.hpp"
#include "terraform/_leaves.hpp"
#include "terraform/java/_block-accessor-java-mca.hpp"
#include "toje/_chunk.hpp"
#include "toje/_context.hpp"

using namespace std;
namespace fs = std::filesystem;

namespace je2be::toje {

class Region::Impl {
public:
  static std::shared_ptr<Context> Convert(mcfile::Dimension d,
                                          std::unordered_set<Pos2i, Pos2iHasher> chunks,
                                          Pos2i region,
                                          unsigned int concurrency,
                                          mcfile::be::DbInterface *db,
                                          std::filesystem::path destination,
                                          Context const &parentContext,
                                          std::function<bool(void)> progress,
                                          std::atomic_uint64_t &numConvertedChunks,
                                          std::filesystem::path terrainTempDir) {
    using namespace mcfile;
    using namespace mcfile::stream;

    auto ctx = parentContext.make();
    int rx = region.fX;
    int rz = region.fZ;

    auto name = mcfile::je::Region::GetDefaultRegionFileName(rx, rz);
    auto terrainMcaPath = terrainTempDir / name;
    auto entitiesMcaPath = destination / "entities" / name;

    auto terrain = mcfile::je::McaEditor::Open(terrainMcaPath);
    if (!terrain) {
      return nullptr;
    }

    auto entities = mcfile::je::McaEditor::Open(entitiesMcaPath);
    if (!entities) {
      return nullptr;
    }

    bool ok = true;
    for (int cz = rz * 32; ok && cz < rz * 32 + 32; cz++) {
      unique_ptr<terraform::bedrock::BlockAccessorBedrock<3, 3>> cache(new terraform::bedrock::BlockAccessorBedrock<3, 3>(d, rx * 32 - 1, cz - 1, db, ctx->fEndian));
      for (int cx = rx * 32; ok && cx < rx * 32 + 32; cx++) {
        defer {
          unique_ptr<terraform::bedrock::BlockAccessorBedrock<3, 3>> next(cache->makeRelocated(cx, cz - 1));
          cache.swap(next);
        };

        assert(cache->fChunkX == cx - 1);
        assert(cache->fChunkZ == cz - 1);
        Pos2i p(cx, cz);
        auto found = chunks.find(p);
        if (found == chunks.end()) {
          continue;
        }

        defer {
          ok = progress();
        };

        auto b = mcfile::be::Chunk::Load(cx, cz, d, *db, ctx->fEndian);
        if (!b) {
          continue;
        }
        cache->set(cx, cz, b);

        auto j = Chunk::Convert(d, cx, cz, *b, *cache, *ctx);

        int localX = cx - rx * 32;
        int localZ = cz - rz * 32;
        auto terrainTag = j->toCompoundTag();
        if (!terrainTag) {
          return nullptr;
        }
        if (!terrain->insert(localX, localZ, *terrainTag)) {
          return nullptr;
        }
        auto entitiesTag = j->toEntitiesCompoundTag();
        if (!entitiesTag) {
          return nullptr;
        }
        if (!entities->insert(localX, localZ, *entitiesTag)) {
          return nullptr;
        }
        numConvertedChunks.fetch_add(1);
      }
    }

    if (!ok) {
      return nullptr;
    }

    if (!terrain->write(terrainMcaPath)) {
      return nullptr;
    }
    terrain.reset();

    if (!entities->write(entitiesMcaPath)) {
      return nullptr;
    }
    entities.reset();

    return ctx;
  }
};

std::shared_ptr<Context> Region::Convert(mcfile::Dimension d,
                                         std::unordered_set<Pos2i, Pos2iHasher> chunks,
                                         Pos2i region,
                                         unsigned int concurrency,
                                         mcfile::be::DbInterface *db,
                                         std::filesystem::path destination,
                                         Context const &parentContext,
                                         std::function<bool(void)> progress,
                                         std::atomic_uint64_t &numConvertedChunks,
                                         std::filesystem::path terrainTempDir) {
  return Impl::Convert(d, chunks, region, concurrency, db, destination, parentContext, progress, numConvertedChunks, terrainTempDir);
}

} // namespace je2be::toje
