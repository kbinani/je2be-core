#pragma once

namespace je2be::tobe {

class BlockPalette {
  using CompoundTag = mcfile::nbt::CompoundTag;

public:
  BlockPalette() : fPaletteKeys({"minecraft:air"}), fPalette({BlockData::Air()}) {}

  void push(std::string const &blockState, std::shared_ptr<CompoundTag> const &tag) {
    fPaletteKeys.push_back(blockState);
    fPalette.push_back(tag);
  }

  uint16_t add(std::string const &blockState, std::shared_ptr<CompoundTag> const &tag) {
    auto i = findByBlockState(blockState);
    if (i) {
      return *i;
    }
    uint16_t index = (uint16_t)size();
    fPalette.push_back(tag);
    fPaletteKeys.push_back(blockState);
    return index;
  }

  size_t size() const { return fPalette.size(); }

  std::shared_ptr<CompoundTag> operator[](size_t index) const { return fPalette[index]; }

private:
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
  std::vector<std::shared_ptr<CompoundTag>> fPalette;
};

} // namespace je2be::tobe
