#pragma once

#include <minecraft-file.hpp>

namespace je2be::box360 {

class Savegame {
  class Impl;
  Savegame() = delete;

public:
  static bool DecompressSavegame(std::vector<uint8_t> &buffer);

  static bool DecompressRawChunk(std::vector<uint8_t> &buffer);

  static void DecodeDecompressedChunk(std::vector<uint8_t> &buffer);

  static bool ExtractRawChunkFromRegionFile(mcfile::stream::InputStream &stream, int x, int z, std::vector<uint8_t> &buffer);

  static bool ExtractFilesFromDecompressedSavegame(std::vector<uint8_t> const &savegame, std::filesystem::path const &outputDirectory);

  static bool ExtractFile(std::vector<uint8_t> const &buffer, uint32_t indexPosition, std::filesystem::path outputDirectory);

  struct SavegameInfo {
    std::optional<std::string> fThumbnailImage;
    std::optional<std::chrono::system_clock::time_point> fCreatedTime;
  };

  static std::optional<SavegameInfo> ExtractSavagameFromSaveBin(std::filesystem::path const &saveBinFile, std::vector<uint8_t> &buffer);

  static std::optional<std::chrono::system_clock::time_point> TimePointFromFatTimestamp(uint32_t fat);
};

} // namespace je2be::box360
