#pragma once

namespace je2be::box360 {

class Options {
public:
  std::optional<std::filesystem::path> fTempDirectory;
  std::unordered_set<mcfile::Dimension> fDimensionFilter;
  std::unordered_set<Pos2i, Pos2iHasher> fChunkFilter;
  std::unordered_map<std::string, Uuid> fPlayers;
};

} // namespace je2be::box360
