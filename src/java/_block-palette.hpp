#pragma once

namespace je2be::java {

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
        size_t s = fPalette.size();
        if (std::numeric_limits<uint16_t>::max() < s) [[unlikely]] {
          return;
        }
        fIndices[idx] = (uint16_t)s;
        fPalette.push_back(tag);
      } else {
        fIndices[idx] = found;
      }
    }
  }

  size_t size() const { return fPalette.size(); }

  void resolveDuplication() {
    using namespace std;
    if (fPalette.size() < 2) {
      return;
    }
    vector<CompoundTagPtr> palette;
    vector<u16> paletteMap;
    vector<u16> indices;
    unordered_map<string, u16> lookup;
    for (size_t i = 0; i < fPalette.size(); i++) {
      auto block = fPalette[i];
      if (!block) [[unlikely]] {
        return;
      }
      auto serialized = CompoundTag::Write(*block, mcfile::Encoding::LittleEndian);
      if (!serialized) [[unlikely]] {
        return;
      }
      auto index = lookup.find(*serialized);
      if (index == lookup.end()) {
        auto idx = palette.size();
        if (idx > numeric_limits<u16>::max()) [[unlikely]] {
          return;
        }
        paletteMap.push_back((u16)idx);
        lookup[*serialized] = (u16)idx;
        palette.push_back(block);
      } else {
        paletteMap.push_back(index->second);
      }
    }
    indices.reserve(fIndices.size());
    for (size_t i = 0; i < fIndices.size(); i++) {
      u16 index = fIndices[i];
      u16 mapped = paletteMap[index];
      indices.push_back(mapped);
    }
    fPalette.swap(palette);
    fIndices.swap(indices);
  }

  CompoundTagPtr &operator[](size_t index) { return fPalette[index]; }
  CompoundTagPtr const &operator[](size_t index) const { return fPalette[index]; }

  std::vector<CompoundTagPtr> fPalette;
  std::vector<u16> fIndices;
};

} // namespace je2be::java
