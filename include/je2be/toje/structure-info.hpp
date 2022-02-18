#pragma once

namespace je2be::toje {

class StructureInfo {
public:
  struct Structure {
    StructureType fType;
    Volume fBounds;
  };

  std::unordered_map<mcfile::Dimension, std::unordered_map<Pos2i, std::vector<Structure>, Pos2iHasher>> fStructures;

public:
  void add(mcfile::Dimension d, Pos2i chunk, Structure s) {
    fStructures[d][chunk].push_back(s);
  }

  void structures(mcfile::Dimension d, Pos2i chunk, std::vector<Structure> &out) const {
    out.clear();
    auto found = fStructures.find(d);
    if (found == fStructures.end()) {
      return;
    }
    if (auto f = found->second.find(chunk); f != found->second.end()) {
      std::copy(f->second.begin(), f->second.end(), std::back_inserter(out));
    }
  }
};

} // namespace je2be::toje
