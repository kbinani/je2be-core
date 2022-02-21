#pragma once

namespace je2be::toje {

class InputOption {
public:
  std::unordered_set<mcfile::Dimension> fDimensionFilter;
  std::unordered_set<Pos2i, Pos2iHasher> fChunkFilter;
  std::optional<Uuid> fLocalPlayer;
};

class OutputOption {
public:
};

} // namespace je2be::toje
