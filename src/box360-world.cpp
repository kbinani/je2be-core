#include "box360/_world.hpp"

#include <je2be/box360/options.hpp>
#include <je2be/box360/progress.hpp>
#include <je2be/defer.hpp>
#include <je2be/fs.hpp>
#include <je2be/pos2.hpp>

#include "_parallel.hpp"
#include "box360/_chunk.hpp"
#include "box360/_context.hpp"
#include "box360/_terraform.hpp"
#include "terraform/box360/_nether-portal.hpp"
#include "terraform/java/_block-accessor-java-directory.hpp"
#include "terraform/lighting/_lighting.hpp"

namespace je2be::box360 {

class World::Impl {
  Impl() = delete;

public:
  static Status Convert(std::filesystem::path const &levelRootDirectory,
                        std::filesystem::path const &outputDirectory,
                        mcfile::Dimension dimension,
                        unsigned int concurrency,
                        Context const &ctx,
                        Options const &options,
                        Progress *progress,
                        u64 progressChunksOffset) {
    using namespace std;
    using namespace std::placeholders;

    namespace fs = std::filesystem;
    string worldDir;
    switch (dimension) {
    case mcfile::Dimension::Nether:
      worldDir = "DIM-1";
      break;
    case mcfile::Dimension::End:
      worldDir = "DIM1";
      break;
    case mcfile::Dimension::Overworld:
    default:
      worldDir = ".";
      break;
    }

    fs::path tempRoot = options.getTempDirectory();

    auto chunkTempDir = mcfile::File::CreateTempDir(tempRoot);
    if (!chunkTempDir) {
      return JE2BE_ERROR;
    }
    if (!Fs::CreateDirectories(*chunkTempDir)) {
      return JE2BE_ERROR;
    }
    defer {
      Fs::DeleteAll(*chunkTempDir);
    };

    auto entityTempDir = mcfile::File::CreateTempDir(tempRoot);
    if (!entityTempDir) {
      return JE2BE_ERROR;
    }
    if (!Fs::CreateDirectories(*entityTempDir)) {
      return JE2BE_ERROR;
    }
    defer {
      Fs::DeleteAll(*entityTempDir);
    };

    int skipChunks = 0;

    vector<Pos2i> works;
    for (int rz = -1; rz <= 0; rz++) {
      for (int rx = -1; rx <= 0; rx++) {
        auto mcr = levelRootDirectory / worldDir / "region" / ("r." + std::to_string(rx) + "." + std::to_string(rz) + ".mcr");
        if (!Fs::Exists(mcr)) {
          skipChunks += 1024;
          continue;
        }
        for (int z = 0; z < 32; z++) {
          for (int x = 0; x < 32; x++) {
            int cx = rx * 32 + x;
            int cz = rz * 32 + z;
            if (!options.fChunkFilter.empty()) [[unlikely]] {
              if (options.fChunkFilter.find(Pos2i(cx, cz)) == options.fChunkFilter.end()) {
                skipChunks += 1;
                continue;
              }
            }
            works.push_back({cx, cz});
          }
        }
      }
    }

    atomic_uint64_t progressChunks(progressChunksOffset + skipChunks);
    atomic_bool ok = true;
    Status st = Parallel::Reduce<Pos2i, Status>(
        works,
        concurrency,
        Status::Ok(),
        [levelRootDirectory, worldDir, chunkTempDir, entityTempDir, ctx, options, dimension, progress, &progressChunks, &ok](Pos2i const &chunk) -> Status {
          int rx = mcfile::Coordinate::RegionFromChunk(chunk.fX);
          int rz = mcfile::Coordinate::RegionFromChunk(chunk.fZ);
          auto mcr = levelRootDirectory / worldDir / "region" / ("r." + std::to_string(rx) + "." + std::to_string(rz) + ".mcr");
          auto st = ProcessChunk(dimension, mcr, chunk.fX, chunk.fZ, *chunkTempDir, *entityTempDir, ctx, options);
          u64 p = progressChunks.fetch_add(1) + 1;
          if (progress && !progress->report({p, kProgressWeightPerWorld * 3})) {
            ok = false;
          }
          if (!st.ok()) {
            ok = false;
          }
          return st;
        },
        Status::Merge);

    if (!st.ok()) {
      return st;
    }

    auto poiDirectory = outputDirectory / worldDir / "poi";
    if (st = Terraform::Do(dimension, poiDirectory, *chunkTempDir, concurrency, progress, progressChunksOffset + 4096); !st.ok()) {
      return st;
    }
    if (progress && !progress->report({progressChunksOffset + 8192, kProgressWeightPerWorld * 3})) {
      return JE2BE_ERROR;
    }

    if (!Fs::CreateDirectories(outputDirectory / worldDir / "region_")) {
      return JE2BE_ERROR;
    }
    if (!Fs::CreateDirectories(outputDirectory / worldDir / "entities")) {
      return JE2BE_ERROR;
    }

    mcfile::je::Region::ConcatOptions o;
    o.fDeleteInput = true;

    vector<Pos2i> regions;
    regions.push_back({-1, -1});
    regions.push_back({0, -1});
    regions.push_back({-1, 0});
    regions.push_back({0, 0});

    st = Parallel::Reduce<Pos2i, Status>(
        regions,
        4,
        Status::Ok(),
        bind(ConcatCompressedNbt, _1, outputDirectory / worldDir, *chunkTempDir, *entityTempDir, o),
        Status::Merge);
    if (!st.ok()) {
      return st;
    }
    if (progress && !progress->report({progressChunksOffset + 12288, kProgressWeightPerWorld * 3})) {
      return JE2BE_ERROR;
    }

    if (!Fs::CreateDirectories(outputDirectory / worldDir / "region")) {
      return JE2BE_ERROR;
    }
    progressChunks = progressChunksOffset + 12288;
    st = Parallel::Reduce<Pos2i, Status>(
        regions,
        4,
        Status::Ok(),
        bind(Lighting, _1, dimension, outputDirectory / worldDir / "region_", outputDirectory / worldDir / "region", &progressChunks, progress),
        Status::Merge);
    if (!st.ok()) {
      return st;
    }
    Fs::DeleteAll(outputDirectory / worldDir / "region_");

    if (progress && !progress->report({progressChunksOffset + kProgressWeightPerWorld, kProgressWeightPerWorld * 3})) {
      return JE2BE_ERROR;
    }

    return Status::Ok();
  }

private:
  static Status Lighting(Pos2i const &region, mcfile::Dimension dim, std::filesystem::path inputDirectory, std::filesystem::path outputDirectory, std::atomic_uint64_t *progressChunks, Progress *progress) {
    using namespace std;
    namespace fs = std::filesystem;

    int rx = region.fX;
    int rz = region.fZ;

    auto name = mcfile::je::Region::GetDefaultRegionFileName(rx, rz);
    fs::path in = inputDirectory / name;
    if (!Fs::Exists(in)) {
      return Status::Ok();
    }
    if (auto size = Fs::FileSize(in); size && *size == 0) {
      return Status::Ok();
    }

    auto editor = mcfile::je::McaEditor::Open(in);
    if (!editor) {
      return Status::Ok();
    }

    terraform::lighting::LightCache lightCache(rx, rz);
    auto blockAccessor = make_shared<terraform::java::BlockAccessorJavaDirectory<5, 5>>(rx * 32 - 1, rz * 32 - 1, inputDirectory);

    for (int z = 0; z < 32; z++) {
      for (int x = 0; x < 32; x++) {
        int cx = x + rx * 32;
        int cz = z + rz * 32;
        auto current = editor->extract(x, z);
        if (!current) {
          if (progress) {
            u64 p = progressChunks->fetch_add(1) + 1;
            if (!progress->report({p, kProgressWeightPerWorld * 3})) {
              return JE2BE_ERROR;
            }
          }
          continue;
        }

        if (!blockAccessor) {
          blockAccessor.reset(new terraform::java::BlockAccessorJavaDirectory<5, 5>(cx - 2, cz - 2, inputDirectory));
        }
        if (blockAccessor->fChunkX != cx - 2 || blockAccessor->fChunkZ != cz - 2) {
          auto next = blockAccessor->makeRelocated(cx - 2, cz - 2);
          blockAccessor.reset(next);
        }
        blockAccessor->loadAllWith(*editor, rx, rz);

        auto copy = current->copy();
        auto ch = mcfile::je::Chunk::MakeChunk(cx, cz, current);
        if (!ch) {
          return JE2BE_ERROR;
        }
        blockAccessor->set(ch);

        auto writable = mcfile::je::WritableChunk::MakeChunk(cx, cz, copy);
        if (!writable) {
          return JE2BE_ERROR;
        }

        terraform::BlockPropertyAccessorJava propertyAccessor(*ch);
        terraform::box360::NetherPortal::Do(*writable, *blockAccessor, propertyAccessor);
        terraform::lighting::Lighting::Do(dim, *writable, *blockAccessor, lightCache);

        lightCache.dispose(cx - 1, cz - 1);

        auto tag = writable->toCompoundTag();
        if (!tag) {
          return JE2BE_ERROR;
        }
        if (!editor->insert(x, z, *tag)) {
          return JE2BE_ERROR;
        }

        if (progress) {
          u64 p = progressChunks->fetch_add(1) + 1;
          if (!progress->report({p, kProgressWeightPerWorld * 3})) {
            return JE2BE_ERROR;
          }
        }
      }
    }

    if (!editor->write(outputDirectory / name)) {
      return JE2BE_ERROR;
    }

    return Status::Ok();
  }

  static Status ConcatCompressedNbt(Pos2i region,
                                    std::filesystem::path outputDirectory,
                                    std::filesystem::path chunkTempDir,
                                    std::filesystem::path entityTempDir,
                                    mcfile::je::Region::ConcatOptions options) {
    int rx = region.fX;
    int rz = region.fZ;
    auto name = mcfile::je::Region::GetDefaultRegionFileName(rx, rz);
    auto chunkMca = outputDirectory / "region_" / name;
    auto entityMca = outputDirectory / "entities" / name;
    if (!mcfile::je::Region::ConcatCompressedNbt(rx,
                                                 rz,
                                                 chunkTempDir,
                                                 chunkMca,
                                                 options)) {
      return JE2BE_ERROR;
    }
    if (!mcfile::je::Region::ConcatCompressedNbt(rx,
                                                 rz,
                                                 entityTempDir,
                                                 entityMca,
                                                 options)) {
      return JE2BE_ERROR;
    }
    return Status::Ok();
  }

  static Status ProcessChunk(mcfile::Dimension dimension,
                             std::filesystem::path mcr,
                             int cx,
                             int cz,
                             std::filesystem::path chunkTempDir,
                             std::filesystem::path entityTempDir,
                             Context ctx,
                             Options options) {
    using namespace std;
    shared_ptr<mcfile::je::WritableChunk> chunk;
    if (auto st = Chunk::Convert(dimension, mcr, cx, cz, chunk, ctx, options); !st.ok()) {
      return st;
    }
    if (!chunk) {
      return Status::Ok();
    }

    {
      auto nbtz = chunkTempDir / mcfile::je::Region::GetDefaultCompressedChunkNbtFileName(cx, cz);
      auto stream = make_shared<mcfile::stream::FileOutputStream>(nbtz);
      if (!chunk->write(*stream)) {
        stream.reset();
        Fs::Delete(nbtz);
        return JE2BE_ERROR;
      }
    }

    {
      auto nbtz = entityTempDir / mcfile::je::Region::GetDefaultCompressedChunkNbtFileName(cx, cz);
      auto stream = make_shared<mcfile::stream::FileOutputStream>(nbtz);
      if (!chunk->writeEntities(*stream)) {
        stream.reset();
        Fs::Delete(nbtz);
        return JE2BE_ERROR;
      }
    }

    return Status::Ok();
  }
};

Status World::Convert(std::filesystem::path const &levelRootDirectory,
                      std::filesystem::path const &outputDirectory,
                      mcfile::Dimension dimension,
                      unsigned int concurrency,
                      Context const &ctx,
                      Options const &options,
                      Progress *progress,
                      u64 progressChunksOffset) {
  return Impl::Convert(levelRootDirectory, outputDirectory, dimension, concurrency, ctx, options, progress, progressChunksOffset);
}

} // namespace je2be::box360
