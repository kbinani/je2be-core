#pragma once

namespace je2be::box360 {

class Converter {
  Converter() = delete;

public:
  static bool Run(std::filesystem::path const &inputSaveBin, std::filesystem::path const &outputDirectory, unsigned int concurrency, Options const &options) {
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
    if (!CopyMapFiles(*temp, outputDirectory)) {
      return false;
    }
    vector<uint8_t>().swap(buffer);
    for (auto dimension : {mcfile::Dimension::Overworld, mcfile::Dimension::Nether, mcfile::Dimension::End}) {
      if (!options.fDimensionFilter.empty()) {
        if (options.fDimensionFilter.find(dimension) == options.fDimensionFilter.end()) {
          continue;
        }
      }
      if (!World::Convert(*temp, outputDirectory, dimension, concurrency, options)) {
        return false;
      }
    }
    return true;
  }

private:
  static bool CopyMapFiles(std::filesystem::path const &inputDirectory, std::filesystem::path const &outputDirectory) {
    namespace fs = std::filesystem;

    auto dataFrom = inputDirectory / "data";
    auto dataTo = outputDirectory / "data";
    if (!Fs::Exists(dataFrom)) {
      return true;
    }
    if (!Fs::CreateDirectories(dataTo)) {
      return false;
    }
    std::error_code ec;
    for (auto it : fs::directory_iterator(dataFrom, ec)) {
      if (!it.is_regular_file()) {
        continue;
      }
      auto fileName = it.path().filename();
      auto fileNameString = fileName.string();
      if (!fileNameString.starts_with("map_") || !fileNameString.ends_with(".dat")) {
        continue;
      }
      auto numberString = strings::RTrim(strings::LTrim(fileNameString, "map_"), ".dat");
      auto number = strings::Toi(numberString);
      if (!number) {
        continue;
      }
      std::error_code ec1;
      fs::copy_options o = fs::copy_options::overwrite_existing;
      fs::copy_file(dataFrom / fileNameString, dataTo / fileNameString, o, ec1);
      if (ec1) {
        return false;
      }
    }
    return true;
  }
};

} // namespace je2be::box360
