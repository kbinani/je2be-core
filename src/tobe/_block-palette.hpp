#pragma once

namespace je2be::tobe {

class BlockPalette {
public:
  BlockPalette() : fIndices(4096, 0) {}

  void append(CompoundTagPtr const &tag) {
    fPalette.push_back(tag);
  }

  void set(size_t idx, CompoundTagPtr const &tag) {
    u16 current = fIndices[idx];
    if (std::count(fIndices.begin(), fIndices.end(), current) == 1) {
      fPalette[current] = tag;
    } else {
      int found = -1;
      for (int i = 0; i < fPalette.size(); i++) {
        if (fPalette[i]->equals(*tag)) {
          found = i;
          break;
        }
      }
      if (found < 0) {
        fIndices[idx] = fPalette.size();
        fPalette.push_back(tag);
      } else {
        fIndices[idx] = found;
      }
    }
  }
  size_t size() const { return fPalette.size(); }

  CompoundTagPtr &operator[](size_t index) { return fPalette[index]; }
  CompoundTagPtr const &operator[](size_t index) const { return fPalette[index]; }

  std::vector<CompoundTagPtr> fPalette;
  std::vector<u16> fIndices;
};

} // namespace je2be::tobe
