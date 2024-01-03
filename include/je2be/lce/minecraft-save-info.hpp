#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace je2be::lce {

class MinecraftSaveInfo {
  class Impl;
  MinecraftSaveInfo() = delete;

public:
  struct SaveBin {
    std::u16string fTitle;
    std::string fFileName;
    std::string fThumbnailData;
  };

  static bool Parse(std::filesystem::path const &saveInfoFilePath, std::vector<SaveBin> &bins);
};

} // namespace je2be::lce
