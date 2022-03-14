#pragma once

namespace je2be::box360 {

class Region {
  Region() = delete;

public:
  static bool Convert(mcfile::Dimension dimension, std::filesystem::path const &mcr, int rx, int rz, std::filesystem::path const &outputMca, std::filesystem::path const &temp) {
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
    for (int z = 0; z < 32; z++) {
      for (int x = 0; x < 32; x++) {
        int cx = rx * 32 + x;
        int cz = rz * 32 + z;
        shared_ptr<mcfile::je::WritableChunk> chunk;
        if (!Chunk::Convert(dimension, mcr, cx, cz, chunk)) {
          continue;
        }
        if (!chunk) {
          continue;
        }

        auto nbtz = *regionTempDir / mcfile::je::Region::GetDefaultCompressedChunkNbtFileName(cx, cz);
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
