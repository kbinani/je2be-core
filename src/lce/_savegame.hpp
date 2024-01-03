#pragma once

#include <je2be/integers.hpp>

#include <minecraft-file.hpp>

namespace je2be::box360 {

class Savegame {
  class Impl;
  Savegame() = delete;

public:
  static bool DecompressSavegame(std::vector<u8> &buffer);

  static bool DecompressRawChunk(std::vector<u8> &buffer);

  static void DecodeDecompressedChunk(std::vector<u8> &buffer);

  static bool ExtractRawChunkFromRegionFile(mcfile::stream::InputStream &stream, int x, int z, std::vector<u8> &buffer);

  static bool ExtractFilesFromDecompressedSavegame(std::vector<u8> const &savegame, std::filesystem::path const &outputDirectory);

  static bool ExtractFile(std::vector<u8> const &buffer, u32 indexPosition, std::filesystem::path outputDirectory);

  struct SavegameInfo {
    std::optional<std::string> fThumbnailImage;
    std::optional<std::chrono::system_clock::time_point> fCreatedTime;
  };

  static std::optional<SavegameInfo> ExtractSavagameFromSaveBin(std::filesystem::path const &saveBinFile, std::vector<u8> &buffer);

  static std::optional<std::chrono::system_clock::time_point> TimePointFromFatTimestamp(u32 fat);
};

} // namespace je2be::box360
