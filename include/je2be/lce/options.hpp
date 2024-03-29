#pragma once

#include <je2be/pos2.hpp>
#include <je2be/uuid.hpp>

namespace je2be::lce {

class Options {
public:
  std::filesystem::path getTempDirectory() const {
    return fTempDirectory ? *fTempDirectory : std::filesystem::temp_directory_path();
  }

public:
  std::optional<std::filesystem::path> fTempDirectory;
  std::unordered_set<mcfile::Dimension> fDimensionFilter;
  std::unordered_set<Pos2i, Pos2iHasher> fChunkFilter;
  std::optional<Uuid> fLocalPlayer;
  std::optional<std::chrono::system_clock::time_point> fLastPlayed;
};

} // namespace je2be::lce
