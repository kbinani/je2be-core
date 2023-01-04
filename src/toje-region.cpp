#include "toje/_region.hpp"

#include <je2be/defer.hpp>

#include "toje/_chunk.hpp"
#include "toje/_context.hpp"

namespace je2be::toje {

class Region::Impl {
public:
  static std::shared_ptr<Context> Convert(mcfile::Dimension d,
                                          std::unordered_set<Pos2i, Pos2iHasher> chunks,
                                          int rx,
                                          int rz,
                                          leveldb::DB *db,
                                          std::filesystem::path destination,
                                          Context const &parentContext,
                                          std::function<bool(void)> progress) {
    using namespace std;
    using namespace mcfile;
    using namespace mcfile::stream;
    namespace fs = std::filesystem;

    auto ctx = parentContext.make();

    mcfile::je::McaBuilder terrain(destination / "region" / mcfile::je::Region::GetDefaultRegionFileName(rx, rz));
    mcfile::je::McaBuilder entities(destination / "entities" / mcfile::je::Region::GetDefaultRegionFileName(rx, rz));

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

        auto b = mcfile::be::Chunk::Load(cx, cz, d, db, ctx->fEndian);
        if (!b) {
          continue;
        }
        cache->set(cx, cz, b);

        auto j = Chunk::Convert(d, cx, cz, *b, *cache, *ctx);

        int localX = cx - rx * 32;
        int localZ = cz - rz * 32;
        if (!terrain.writeChunk(localX, localZ, [j](auto &out) { return j->write(out); })) {
          return nullptr;
        }
        if (!entities.writeChunk(localX, localZ, [j](auto &out) { return j->writeEntities(out); })) {
          return nullptr;
        }
      }
    }

    if (!ok) {
      return nullptr;
    }

    return ctx;
  }
};

std::shared_ptr<Context> Region::Convert(mcfile::Dimension d,
                                         std::unordered_set<Pos2i, Pos2iHasher> chunks,
                                         int rx,
                                         int rz,
                                         leveldb::DB *db,
                                         std::filesystem::path destination,
                                         Context const &parentContext,
                                         std::function<bool(void)> progress) {
  return Impl::Convert(d, chunks, rx, rz, db, destination, parentContext, progress);
}

} // namespace je2be::toje
