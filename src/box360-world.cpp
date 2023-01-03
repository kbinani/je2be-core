#include <je2be/box360/_world.hpp>

#include <je2be/box360/options.hpp>
#include <je2be/box360/progress.hpp>
#include <je2be/defer.hpp>
#include <je2be/pos2.hpp>

#include <je2be/_fs.hpp>
#include <je2be/box360/_chunk.hpp>
#include <je2be/box360/_context.hpp>
#include <je2be/box360/_terraform.hpp>

#include <je2be/_future-support.hpp>

#include <BS_thread_pool.hpp>

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
                        int progressChunksOffset) {
    using namespace std;
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

    fs::path tempRoot = options.fTempDirectory ? *options.fTempDirectory : fs::temp_directory_path();

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

    unique_ptr<BS::thread_pool> queue(new BS::thread_pool(concurrency));
    deque<future<Status>> futures;
    optional<Status> error;
    bool ok = true;
    int progressChunks = progressChunksOffset;
    for (int rz = -1; rz <= 0; rz++) {
      for (int rx = -1; rx <= 0; rx++) {
        auto mcr = levelRootDirectory / worldDir / "region" / ("r." + std::to_string(rx) + "." + std::to_string(rz) + ".mcr");
        if (!Fs::Exists(mcr)) {
          progressChunks += 2048;
          continue;
        }
        for (int z = 0; z < 32; z++) {
          for (int x = 0; x < 32; x++) {
            int cx = rx * 32 + x;
            int cz = rz * 32 + z;
            if (!options.fChunkFilter.empty()) [[unlikely]] {
              if (options.fChunkFilter.find(Pos2i(cx, cz)) == options.fChunkFilter.end()) {
                progressChunks++;
                continue;
              }
            }
            vector<future<Status>> drain;
            FutureSupport::Drain(concurrency + 1, futures, drain);
            for (auto &f : drain) {
              if (auto st = f.get(); !st.ok()) {
                ok = false;
                if (!error) {
                  error = st;
                }
              }
              progressChunks++;
            }
            if (!ok) {
              break;
            }
            if (progress && !progress->report(progressChunks, 8192 * 3)) {
              ok = false;
              break;
            }
            futures.push_back(queue->submit(ProcessChunk, dimension, mcr, cx, cz, *chunkTempDir, *entityTempDir, ctx, options));
          }
        }
      }
    }

    for (auto &f : futures) {
      if (auto st = f.get(); !st.ok()) {
        ok = false;
        if (!error) {
          error = st;
        }
      }
      progressChunks++;
    }
    futures.clear();
    queue.reset();
    if (!ok) {
      return error ? *error : JE2BE_ERROR;
    }

    auto poiDirectory = outputDirectory / worldDir / "poi";
    if (auto st = Terraform::Do(dimension, poiDirectory, *chunkTempDir, concurrency, progress, progressChunks); !st.ok()) {
      return st;
    }
    progressChunks += 4096;

    if (!Fs::CreateDirectories(outputDirectory / worldDir / "region")) {
      return JE2BE_ERROR;
    }
    if (!Fs::CreateDirectories(outputDirectory / worldDir / "entities")) {
      return JE2BE_ERROR;
    }

    queue.reset(new BS::thread_pool(concurrency));

    mcfile::je::Region::ConcatOptions o;
    o.fDeleteInput = true;
    for (int rz = -1; rz <= 0; rz++) {
      for (int rx = -1; rx <= 0; rx++) {
        futures.push_back(queue->submit(ConcatCompressedNbt, rx, rz, outputDirectory / worldDir, *chunkTempDir, *entityTempDir, o));
      }
    }
    for (auto &f : futures) {
      if (auto st = f.get(); !st.ok()) {
        ok = false;
        if (!error) {
          error = st;
        }
      }
    }

    if (progress) {
      progress->report(progressChunks, 8192 * 3);
    }

    if (ok) {
      return Status::Ok();
    } else {
      return error ? *error : JE2BE_ERROR;
    }
  }

private:
  static Status ConcatCompressedNbt(int rx,
                                    int rz,
                                    std::filesystem::path outputDirectory,
                                    std::filesystem::path chunkTempDir,
                                    std::filesystem::path entityTempDir,
                                    mcfile::je::Region::ConcatOptions options) {
    auto chunkMca = outputDirectory / "region" / mcfile::je::Region::GetDefaultRegionFileName(rx, rz);
    auto entityMca = outputDirectory / "entities" / mcfile::je::Region::GetDefaultRegionFileName(rx, rz);
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
                      int progressChunksOffset) {
  return Impl::Convert(levelRootDirectory, outputDirectory, dimension, concurrency, ctx, options, progress, progressChunksOffset);
}

} // namespace je2be::box360
