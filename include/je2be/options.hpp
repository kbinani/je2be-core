#pragma once

namespace j2b {

class InputOption {
public:
  LevelDirectoryStructure fLevelDirectoryStructure = LevelDirectoryStructure::Vanilla;

  std::filesystem::path getWorldDirectory(std::filesystem::path const &root, Dimension dim) const {
    namespace fs = std::filesystem;
    switch (fLevelDirectoryStructure) {
    case LevelDirectoryStructure::Vanilla: {
      switch (dim) {
      case Dimension::Overworld:
        return root;
      case Dimension::Nether:
        return root / "DIM-1";
      case Dimension::End:
        return root / "DIM1";
      }
      break;
    }
    case LevelDirectoryStructure::Paper: {
      switch (dim) {
      case Dimension::Overworld:
        return root / "world";
      case Dimension::Nether:
        return root / "world_nether" / "DIM-1";
      case Dimension::End:
        return root / "world_the_end" / "DIM1";
      }
      break;
    }
    }
  }

  std::filesystem::path getDataDirectory(std::filesystem::path const &root) const {
    switch (fLevelDirectoryStructure) {
    case LevelDirectoryStructure::Vanilla:
      return root / "data";
    case LevelDirectoryStructure::Paper:
      return root / "world" / "data";
    }
  }

  std::filesystem::path getLevelDatFilePath(std::filesystem::path const &root) const {
    switch (fLevelDirectoryStructure) {
    case LevelDirectoryStructure::Vanilla:
      return root / "level.dat";
    case LevelDirectoryStructure::Paper:
      return root / "world" / "level.dat";
    }
  }
};

class OutputOption {
public:
};

} // namespace j2b
