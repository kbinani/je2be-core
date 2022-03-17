#pragma once

namespace je2be::box360 {

class Converter {
  Converter() = delete;

public:
  static bool Run(std::filesystem::path const &inputSaveBin, std::filesystem::path const &outputDirectory, Options const &options) {
    using namespace std;
    namespace fs = std::filesystem;

    auto tempRoot = options.fTempDirectory ? *options.fTempDirectory : fs::temp_directory_path();
    auto temp = mcfile::File::CreateTempDir(tempRoot);
    if (!temp) {
      return false;
    }
    defer {
      Fs::DeleteAll(*temp);
    };
    fs::path savegame = *temp / "savegame.dat";

    if (!Savegame::ExtractSavagameFromSaveBin(inputSaveBin, savegame)) {
      return false;
    }
    vector<uint8_t> buffer;
    if (!Savegame::DecompressSavegame(savegame, buffer)) {
      return false;
    }
    if (!Savegame::ExtractFilesFromDecompressedSavegame(buffer, *temp)) {
      return false;
    }
    vector<uint8_t>().swap(buffer);
    for (auto dimension : {mcfile::Dimension::Overworld, mcfile::Dimension::Nether, mcfile::Dimension::End}) {
      if (!options.fDimensionFilter.empty()) {
        if (options.fDimensionFilter.find(dimension) == options.fDimensionFilter.end()) {
          continue;
        }
      }
      if (!World::Convert(*temp, outputDirectory, dimension, options)) {
        return false;
      }
    }
    return true;
  }
};

} // namespace je2be::box360
