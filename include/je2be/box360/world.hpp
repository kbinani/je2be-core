#pragma once

namespace je2be::box360 {

class World {
  World() = delete;

public:
  static bool Convert(std::filesystem::path const &levelRootDirectory, std::filesystem::path const &outputDirectory, mcfile::Dimension dimension, Options options) {
    namespace fs = std::filesystem;
    fs::path pathToRegion;
    switch (dimension) {
    case mcfile::Dimension::Nether:
      pathToRegion = fs::path("DIM-1") / "region";
      break;
    case mcfile::Dimension::End:
      pathToRegion = fs::path("DIM1") / "region";
      break;
    case mcfile::Dimension::Overworld:
    default:
      pathToRegion = fs::path("region");
      break;
    }

    fs::path tempRoot = options.fTempDirectory ? *options.fTempDirectory : fs::temp_directory_path();
    auto temp = mcfile::File::CreateTempDir(tempRoot);
    if (!temp) {
      return false;
    }
    if (!Fs::CreateDirectories(*temp)) {
      return false;
    }
    defer {
      Fs::Delete(*temp);
    };
    for (int rz = -1; rz <= 0; rz++) {
      for (int rx = -1; rx <= 0; rx++) {
        auto mcr = levelRootDirectory / pathToRegion / ("r." + std::to_string(rx) + "." + std::to_string(rz) + ".mcr");
        if (!Fs::Exists(mcr)) {
          continue;
        }
        if (!Region::Convert(dimension, mcr, rx, rz, *temp)) {
          return false;
        }
      }
    }

    if (!Terraform::Do(*temp)) {
      return false;
    }

    if (!Fs::CreateDirectories(outputDirectory / pathToRegion)) {
      return false;
    }
    for (int rz = -1; rz <= 0; rz++) {
      for (int rx = -1; rx <= 0; rx++) {
        auto mca = outputDirectory / pathToRegion / mcfile::je::Region::GetDefaultRegionFileName(rx, rz);
        if (!mcfile::je::Region::ConcatCompressedNbt(rx, rz, *temp, mca)) {
          return false;
        }
      }
    }
    return true;
  }
};

} // namespace je2be::box360
