#pragma once

#include <je2be/enums/level-directory-structure.hpp>
#include <je2be/pos2.hpp>

namespace je2be::tobe {

class Options {
public:
  LevelDirectoryStructure fLevelDirectoryStructure = LevelDirectoryStructure::Vanilla;
  std::unordered_set<mcfile::Dimension> fDimensionFilter;
  std::unordered_set<Pos2i, Pos2iHasher> fChunkFilter;
  std::optional<std::filesystem::path> fTempDirectory;

  std::filesystem::path getWorldDirectory(std::filesystem::path const &root, mcfile::Dimension dim) const {
    using namespace mcfile;
    namespace fs = std::filesystem;
    switch (fLevelDirectoryStructure) {
    case LevelDirectoryStructure::Paper: {
      switch (dim) {
      case Dimension::Nether:
        return root / "world_nether" / "DIM-1";
      case Dimension::End:
        return root / "world_the_end" / "DIM1";
      case Dimension::Overworld:
      default:
        return root / "world";
      }
      break;
    }
    case LevelDirectoryStructure::Vanilla:
    default: {
      switch (dim) {
      case Dimension::Nether:
        return root / "DIM-1";
      case Dimension::End:
        return root / "DIM1";
      case Dimension::Overworld:
      default:
        return root;
      }
      break;
    }
    }
  }

  std::filesystem::path getDataDirectory(std::filesystem::path const &root) const {
    switch (fLevelDirectoryStructure) {
    case LevelDirectoryStructure::Paper:
      return root / "world" / "data";
    case LevelDirectoryStructure::Vanilla:
    default:
      return root / "data";
    }
  }

  std::filesystem::path getLevelDatFilePath(std::filesystem::path const &root) const {
    switch (fLevelDirectoryStructure) {
    case LevelDirectoryStructure::Paper:
      return root / "world" / "level.dat";
    case LevelDirectoryStructure::Vanilla:
    default:
      return root / "level.dat";
    }
  }

  std::filesystem::path getTempDirectory() const {
    return fTempDirectory ? *fTempDirectory : std::filesystem::temp_directory_path();
  }
};

} // namespace je2be::tobe
