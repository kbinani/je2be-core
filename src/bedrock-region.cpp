#include "bedrock/_region.hpp"

#include <defer.hpp>

#include "_parallel.hpp"
#include "_pos2i-set.hpp"
#include "bedrock/_chunk.hpp"
#include "bedrock/_context.hpp"
#include "terraform/_leaves.hpp"
#include "terraform/java/_block-accessor-java-mca.hpp"

using namespace std;
namespace fs = std::filesystem;

namespace je2be::bedrock {

class Region::Impl {
public:
  static Status Convert(mcfile::Dimension d,
                        Pos2iSet chunks,
                        Pos2i region,
                        unsigned int concurrency,
                        mcfile::be::DbInterface *db,
                        std::filesystem::path destination,
                        Context const &parentContext,
                        std::function<bool(void)> progress,
                        std::atomic_uint64_t &numConvertedChunks,
                        std::filesystem::path terrainTempDir,
                        std::shared_ptr<Context> &out) {
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
      return JE2BE_ERROR;
    }

    auto entities = mcfile::je::McaEditor::Open(entitiesMcaPath);
    if (!entities) {
      return JE2BE_ERROR;
    }

    bool ok = true;
    for (int cz = rz * 32; ok && cz < rz * 32 + 32; cz++) {
      auto cache = make_unique<terraform::bedrock::BlockAccessorBedrock<3, 3>>(d, rx * 32 - 1, cz - 1, db, ctx->fEndian);
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
          ok = ok && progress();
        };

        auto b = mcfile::be::Chunk::Load(cx, cz, d, *db, ctx->fEndian);
        if (!b) {
          continue;
        }
        cache->set(cx, cz, b);

        shared_ptr<mcfile::je::WritableChunk> j;
        if (auto st = Chunk::Convert(d, cx, cz, *b, *cache, *ctx, j); !st.ok()) {
          return JE2BE_ERROR_PUSH(st);
        }

        int localX = cx - rx * 32;
        int localZ = cz - rz * 32;
        auto terrainTag = j->toCompoundTag(d);
        if (!terrainTag) {
          return JE2BE_ERROR;
        }
        if (!terrain->insert(localX, localZ, *terrainTag)) {
          return JE2BE_ERROR;
        }
        auto entitiesTag = j->toEntitiesCompoundTag();
        if (!entitiesTag) {
          return JE2BE_ERROR;
        }
        if (!entities->insert(localX, localZ, *entitiesTag)) {
          return JE2BE_ERROR;
        }
        numConvertedChunks.fetch_add(1);
      }
    }

    if (!ok) {
      return JE2BE_ERROR;
    }

    if (!terrain->write(terrainMcaPath)) {
      return JE2BE_ERROR;
    }
    terrain.reset();

    if (!entities->write(entitiesMcaPath)) {
      return JE2BE_ERROR;
    }
    entities.reset();

    out.swap(ctx);
    return Status::Ok();
  }
};

Status Region::Convert(mcfile::Dimension d,
                       Pos2iSet chunks,
                       Pos2i region,
                       unsigned int concurrency,
                       mcfile::be::DbInterface *db,
                       std::filesystem::path destination,
                       Context const &parentContext,
                       std::function<bool(void)> progress,
                       std::atomic_uint64_t &numConvertedChunks,
                       std::filesystem::path terrainTempDir,
                       std::shared_ptr<Context> &out) {
  return Impl::Convert(d, chunks, region, concurrency, db, destination, parentContext, progress, numConvertedChunks, terrainTempDir, out);
}

} // namespace je2be::bedrock
