#pragma once

#include <unordered_set>

#include <je2be/pos2.hpp>
#include <je2be/uuid.hpp>

namespace je2be::toje {

class Options {
public:
  std::filesystem::path getTempDirectory() const {
    return fTempDirectory ? *fTempDirectory : std::filesystem::temp_directory_path();
  }

public:
  std::unordered_set<mcfile::Dimension> fDimensionFilter;
  std::unordered_set<Pos2i, Pos2iHasher> fChunkFilter;
  std::shared_ptr<Uuid const> fLocalPlayer;
  std::optional<std::filesystem::path> fTempDirectory;
};

} // namespace je2be::toje
