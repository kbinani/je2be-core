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
                                          std::atomic_uint64_t &numConvertedChunks) {
    using namespace mcfile;
    using namespace mcfile::stream;

    auto ctx = parentContext.make();
    int rx = region.fX;
    int rz = region.fZ;

    auto name = mcfile::je::Region::GetDefaultRegionFileName(rx, rz);
    auto mcaPath = ctx->fTempDirectory / Uuid::Gen().toString();

    auto terrain = make_shared<mcfile::je::McaEditor>(mcaPath);
    auto entities = make_shared<mcfile::je::McaEditor>(destination / "entities" / name);

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

    if (!terrain->close()) {
      return nullptr;
    }
    terrain.reset();

    if (!entities->close()) {
      return nullptr;
    }
    entities.reset();

    if (!TerraformExceptRegionBoundary(region, mcaPath, destination / "region" / name, concurrency)) {
      return nullptr;
    }

    return ctx;
  }

private:
  static bool TerraformExceptRegionBoundary(Pos2i const &region, fs::path const &from, fs::path const &to, unsigned int concurrency) {
    if (!Fs::CopyFile(from, to)) {
      return false;
    }

    mcfile::je::McaEditor edit(to);

    for (int x = 1; x < 31; x++) {
      for (int z = 1; z < 31; z++) {
        auto r = mcfile::je::Region::MakeRegion(from, region.fX, region.fZ);
        if (!r) {
          return false;
        }
        Pos2i chunk(x + region.fX * 32, z + region.fZ * 32);

        terraform::java::BlockAccessorJavaMca<3, 3> loader(chunk.fX - 1, chunk.fZ - 1, *r);
        auto original = edit.extract(x, z);
        if (!original) {
          continue;
        }
        auto copy = original->copy();
        auto readonlyChunk = mcfile::je::Chunk::MakeChunk(chunk.fX, chunk.fZ, original);
        if (!readonlyChunk) {
          return false;
        }
        loader.set(chunk.fX, chunk.fZ, readonlyChunk);

        auto writableChunk = mcfile::je::WritableChunk::MakeChunk(chunk.fX, chunk.fZ, copy);
        if (!writableChunk) {
          return false;
        }

        terraform::BlockPropertyAccessorJava accessor(*readonlyChunk);
        terraform::Leaves::Do(*writableChunk, loader, accessor);

        if (auto compound = writableChunk->toCompoundTag(); compound) {
          if (!edit.insert(x, z, *compound)) {
            return false;
          }
        } else {
          return false;
        }
      }
    }
    if (!edit.close()) {
      return false;
    }
    return Fs::Delete(from);
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
                                         std::atomic_uint64_t &numConvertedChunks) {
  return Impl::Convert(d, chunks, region, concurrency, db, destination, parentContext, progress, numConvertedChunks);
}

} // namespace je2be::toje
