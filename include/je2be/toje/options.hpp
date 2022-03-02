#pragma once

namespace je2be::toje {

class Options {
public:
  std::unordered_set<mcfile::Dimension> fDimensionFilter;
  std::unordered_set<Pos2i, Pos2iHasher> fChunkFilter;
  std::optional<Uuid> fLocalPlayer;
  std::optional<std::filesystem::path> fTempDirectory;
};

} // namespace je2be::toje
