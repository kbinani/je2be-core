#pragma once

namespace je2be::box360 {

class Converter {
public:
  Converter(std::filesystem::path const &inputSaveBin, std::filesystem::path const &outputDirectory, Options options) : fInputSaveBin(inputSaveBin), fOutputDirectory(outputDirectory), fOptions(options) {
  }

  bool run() {
    using namespace std;
    namespace fs = std::filesystem;

    auto tempRoot = fOptions.fTempDirectory ? *fOptions.fTempDirectory : fs::temp_directory_path();
    auto temp = mcfile::File::CreateTempDir(tempRoot);
    if (!temp) {
      return false;
    }
    fs::path savegame = *temp / "savegame.dat";

    if (!Savegame::ExtractSavagameFromSaveBin(fInputSaveBin, savegame)) {
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
      if (!fOptions.fDimensionFilter.empty()) {
        if (fOptions.fDimensionFilter.find(dimension) == fOptions.fDimensionFilter.end()) {
          continue;
        }
      }
      if (!World::Convert(*temp, fOutputDirectory, dimension, fOptions)) {
        return false;
      }
    }
    return true;
  }

public:
  std::filesystem::path const fInputSaveBin;
  std::filesystem::path const fOutputDirectory;
  Options const fOptions;
};

} // namespace je2be::box360
