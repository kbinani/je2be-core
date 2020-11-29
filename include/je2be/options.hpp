#pragma once

namespace j2b {

class InputOption {
public:
  LevelDirectoryStructure fLevelDirectoryStructure =
      LevelDirectoryStructure::Vanilla;

  j2b::filesystem::path getWorldDirectory(j2b::filesystem::path const &root,
                                          Dimension dim) const {
    namespace fs = j2b::filesystem;
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

  j2b::filesystem::path
  getDataDirectory(j2b::filesystem::path const &root) const {
    switch (fLevelDirectoryStructure) {
    case LevelDirectoryStructure::Vanilla:
      return root / "data";
    case LevelDirectoryStructure::Paper:
      return root / "world" / "data";
    }
  }

  j2b::filesystem::path
  getLevelDatFilePath(j2b::filesystem::path const &root) const {
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
