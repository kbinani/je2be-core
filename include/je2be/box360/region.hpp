#pragma once

namespace je2be::box360 {

class Region {
  Region() = delete;

public:
  static bool Convert(std::filesystem::path const &mcr, int rx, int rz, std::filesystem::path const &outputMca, std::filesystem::path const &temp) {
    using namespace std;
    auto regionTempDir = mcfile::File::CreateTempDir(temp);
    if (!regionTempDir) {
      return false;
    }
    if (!Fs::CreateDirectories(*regionTempDir)) {
      return false;
    }
    defer {
      Fs::Delete(*regionTempDir);
    };
    for (int cz = 0; cz < 32; cz++) {
      for (int cx = 0; cx < 32; cx++) {
        shared_ptr<mcfile::je::WritableChunk> chunk;
        if (!Chunk::Convert(mcr, rx * 32 + cx, rz * 32 + cz, chunk)) {
          continue;
        }
        if (!chunk) {
          continue;
        }

        auto nbtz = *regionTempDir / mcfile::je::Region::GetDefaultCompressedChunkNbtFileName(rx * 32 + cx, rz * 32 + cz);
        auto stream = make_shared<mcfile::stream::FileOutputStream>(nbtz);
        if (!chunk->write(*stream)) {
          stream.reset();
          Fs::Delete(nbtz);
          continue;
        }
      }
    }
    return mcfile::je::Region::ConcatCompressedNbt(rx, rz, *regionTempDir, outputMca);
  }
};

} // namespace je2be::box360
