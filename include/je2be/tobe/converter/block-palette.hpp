#pragma once

namespace je2be::tobe {

class BlockPalette {
public:
  BlockPalette() : fPaletteKeys({"minecraft:air"}), fPalette({BlockData::Air()}) {}

  void push(std::string const &blockState, CompoundTagPtr const &tag) {
    fPaletteKeys.push_back(blockState);
    fPalette.push_back(tag);
  }

  uint16_t add(std::string const &blockState, CompoundTagPtr const &tag) {
    auto i = findByBlockState(blockState);
    if (i) {
      return *i;
    }
    uint16_t index = (uint16_t)size();
    fPalette.push_back(tag);
    fPaletteKeys.push_back(blockState);
    return index;
  }

  uint16_t add(CompoundTagPtr const &tag) {
    for (size_t i = 0; i < fPalette.size(); i++) {
      if (tag->equals(*fPalette[i])) {
        return static_cast<uint16_t>(i);
      }
    }
    uint16_t index = (uint16_t)size();
    fPalette.push_back(tag);
    fPaletteKeys.push_back("je2be:dummy_palette_key_" + std::to_string(index));
    return index;
  }

  size_t size() const { return fPalette.size(); }

  CompoundTagPtr operator[](size_t index) const { return fPalette[index]; }

  std::optional<uint16_t> findByBlockState(std::string const &blockState) const {
    auto found = std::find(fPaletteKeys.begin(), fPaletteKeys.end(), blockState);
    if (found == fPaletteKeys.end()) {
      return std::nullopt;
    } else {
      ptrdiff_t d = std::distance(fPaletteKeys.begin(), found);
      if (d < 0 || std::numeric_limits<uint16_t>::max() < d) {
        return std::nullopt;
      } else {
        return (uint16_t)d;
      }
    }
  }

private:
  std::vector<std::string> fPaletteKeys;
  std::vector<CompoundTagPtr> fPalette;
};

} // namespace je2be::tobe
