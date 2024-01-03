#pragma once

#include <je2be/integers.hpp>

#include <chrono>
#include <filesystem>
#include <optional>
#include <string>

namespace je2be::box360 {

class SaveBin {
public:
  struct SavegameInfo {
    std::optional<std::string> fThumbnailImage;
    std::optional<std::chrono::system_clock::time_point> fCreatedTime;
  };

  static std::optional<SavegameInfo> ExtractSavagame(std::filesystem::path const &saveBinFile, std::vector<u8> &buffer);
  static bool DecompressSavegame(std::vector<u8> &buffer);
};

} // namespace je2be::box360
