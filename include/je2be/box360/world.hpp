#pragma once

namespace je2be::box360 {

class World {
  World() = delete;

public:
  static bool Convert(std::filesystem::path const &levelRootDirectory, std::filesystem::path const &outputDirectory, mcfile::Dimension dimension, unsigned int concurrency, Options options) {
    using namespace std;
    namespace fs = std::filesystem;
    fs::path pathToRegion;
    switch (dimension) {
    case mcfile::Dimension::Nether:
      pathToRegion = fs::path("DIM-1") / "region";
      break;
    case mcfile::Dimension::End:
      pathToRegion = fs::path("DIM1") / "region";
      break;
    case mcfile::Dimension::Overworld:
    default:
      pathToRegion = fs::path("region");
      break;
    }

    fs::path tempRoot = options.fTempDirectory ? *options.fTempDirectory : fs::temp_directory_path();
    auto temp = mcfile::File::CreateTempDir(tempRoot);
    if (!temp) {
      return false;
    }
    if (!Fs::CreateDirectories(*temp)) {
      return false;
    }
    defer {
      Fs::DeleteAll(*temp);
    };
    unique_ptr<hwm::task_queue> queue;
    if (concurrency > 0) {
      queue.reset(new hwm::task_queue(concurrency));
    }
    deque<future<bool>> futures;
    bool ok = true;
    for (int rz = -1; rz <= 0; rz++) {
      for (int rx = -1; rx <= 0; rx++) {
        auto mcr = levelRootDirectory / pathToRegion / ("r." + std::to_string(rx) + "." + std::to_string(rz) + ".mcr");
        if (!Fs::Exists(mcr)) {
          continue;
        }
        for (int z = 0; z < 32; z++) {
          for (int x = 0; x < 32; x++) {
            int cx = rx * 32 + x;
            int cz = rz * 32 + z;
            if (!options.fChunkFilter.empty()) [[unlikely]] {
              if (options.fChunkFilter.find(Pos2i(cx, cz)) == options.fChunkFilter.end()) {
                continue;
              }
            }
            if (queue) {
              vector<future<bool>> drain;
              FutureSupport::Drain(concurrency + 1, futures, drain);
              for (auto &f : drain) {
                ok = ok && f.get();
              }
              if (!ok) {
                break;
              }
              futures.push_back(move(queue->enqueue(ProcessChunk, dimension, mcr, cx, cz, *temp)));
            } else {
              if (ProcessChunk(dimension, mcr, cx, cz, *temp)) {
                return false;
              }
            }
          }
        }
      }
    }

    if (queue) {
      for (auto &f : futures) {
        ok = ok && f.get();
      }
      futures.clear();
    }
    if (!ok) {
      return false;
    }

    if (!Terraform::Do(*temp, concurrency)) {
      return false;
    }

    if (!Fs::CreateDirectories(outputDirectory / pathToRegion)) {
      return false;
    }

    mcfile::je::Region::ConcatOptions o;
    o.fDeleteInput = true;
    for (int rz = -1; rz <= 0; rz++) {
      for (int rx = -1; rx <= 0; rx++) {
        auto mca = outputDirectory / pathToRegion / mcfile::je::Region::GetDefaultRegionFileName(rx, rz);
        if (queue) {
          futures.push_back(move(queue->enqueue(ConcatCompressedNbt, rx, rz, *temp, mca, o)));
        } else {
          if (!ConcatCompressedNbt(rx, rz, *temp, mca, o)) {
            return false;
          }
        }
      }
    }
    for (auto &f : futures) {
      ok = ok && f.get();
    }
    return ok;
  }

private:
  static bool ConcatCompressedNbt(int rx, int rz, std::filesystem::path directory, std::filesystem::path mca, mcfile::je::Region::ConcatOptions options) {
    return mcfile::je::Region::ConcatCompressedNbt(rx, rz, directory, mca, options);
  }

  static bool ProcessChunk(mcfile::Dimension dimension, std::filesystem::path mcr, int cx, int cz, std::filesystem::path regionTempDir) {
    using namespace std;
    shared_ptr<mcfile::je::WritableChunk> chunk;
    if (!Chunk::Convert(dimension, mcr, cx, cz, chunk)) {
      return true;
    }
    if (!chunk) {
      return true;
    }

    auto nbtz = regionTempDir / mcfile::je::Region::GetDefaultCompressedChunkNbtFileName(cx, cz);
    auto stream = make_shared<mcfile::stream::FileOutputStream>(nbtz);
    if (!chunk->write(*stream)) {
      stream.reset();
      Fs::Delete(nbtz);
      return false;
    }
    return true;
  }
};

} // namespace je2be::box360
