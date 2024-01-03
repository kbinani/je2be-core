#pragma once

#include <je2be/integers.hpp>

#include <minecraft-file.hpp>

namespace je2be::lce {

class Savegame {
  class Impl;
  Savegame() = delete;

public:
  static bool DecompressRawChunk(std::vector<u8> &buffer);

  static void DecodeDecompressedChunk(std::vector<u8> &buffer);

  static bool ExtractRawChunkFromRegionFile(mcfile::stream::InputStream &stream, int x, int z, std::vector<u8> &buffer);

  static bool ExtractFilesFromDecompressedSavegame(std::vector<u8> const &savegame, std::filesystem::path const &outputDirectory);
};

} // namespace je2be::lce
