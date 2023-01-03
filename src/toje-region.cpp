#include <je2be/toje/_region.hpp>

#include <je2be/defer.hpp>

#include <je2be/toje/_chunk.hpp>
#include <je2be/toje/_context.hpp>

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

    auto regionDir = File::CreateTempDir(parentContext.fTempDirectory);
    if (!regionDir) {
      return nullptr;
    }
    auto entityDir = destination / "entities" / ("r." + to_string(rx) + "." + to_string(rz));
    if (!Fs::CreateDirectories(entityDir)) {
      return nullptr;
    }

    auto ctx = parentContext.make();

    defer {
      error_code ec1;
      fs::remove_all(*regionDir, ec1);
    };

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

        auto streamTerrain = make_shared<FileOutputStream>(*regionDir / mcfile::je::Region::GetDefaultCompressedChunkNbtFileName(cx, cz));
        if (!j->write(*streamTerrain)) {
          return nullptr;
        }

        auto streamEntities = make_shared<FileOutputStream>(entityDir / mcfile::je::Region::GetDefaultCompressedChunkNbtFileName(cx, cz));
        if (!j->writeEntities(*streamEntities)) {
          return nullptr;
        }
      }
    }

    if (!ok) {
      return nullptr;
    }

    auto terrainMca = destination / "region" / mcfile::je::Region::GetDefaultRegionFileName(rx, rz);
    if (!mcfile::je::Region::ConcatCompressedNbt(rx, rz, *regionDir, terrainMca)) {
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
