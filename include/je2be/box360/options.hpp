#pragma once

#include <je2be/pos2.hpp>
#include <je2be/uuid.hpp>

namespace je2be::box360 {

class Options {
public:
  std::optional<std::filesystem::path> fTempDirectory;
  std::unordered_set<mcfile::Dimension> fDimensionFilter;
  std::unordered_set<Pos2i, Pos2iHasher> fChunkFilter;
  std::optional<Uuid> fLocalPlayer;
};

} // namespace je2be::box360
