#pragma once

namespace j2b {

class BlockPalette {
  struct Key {
    Key(std::string const &blockState, uintptr_t ptr)
        : fBlockState(blockState), fBlockPtr(ptr) {}

    std::string const fBlockState;
    uintptr_t const fBlockPtr;
  };

  using CompoundTag = mcfile::nbt::CompoundTag;

public:
  BlockPalette()
      : fBlockStateLut({{"minecraft:air", 0}}), fPalette({BlockData::Air()}) {}

  uint16_t add(std::shared_ptr<mcfile::Block const> const &block) {
    uintptr_t ptr = (uintptr_t)block.get();
    auto i = findByBlockPtr(ptr);
    if (i) {
      return *i;
    }
    auto const &blockState = block->toString();
    i = findByBlockState(blockState);
    if (i) {
      return *i;
    }
    uint16_t index = (uint16_t)size();

    auto tag = BlockData::From(block);
    fPalette.push_back(tag);
    fBlockPtrLut[ptr] = index;
    fBlockStateLut[blockState] = index;

    return index;
  }

  uint16_t add(std::string const &blockState,
               std::shared_ptr<CompoundTag> const &tag) {
    auto i = findByBlockState(blockState);
    if (i) {
      return *i;
    }
    uint16_t index = (uint16_t)size();
    fPalette.push_back(tag);
    fBlockStateLut[blockState] = index;
    return index;
  }

  size_t size() const { return fPalette.size(); }

  std::shared_ptr<CompoundTag> operator[](size_t index) const {
    return fPalette[index];
  }

private:
  std::optional<uint16_t> findByBlockPtr(uintptr_t ptr) const {
    auto found = fBlockPtrLut.find(ptr);
    if (found == fBlockPtrLut.end()) {
      return std::nullopt;
    } else {
      return found->second;
    }
  }

  std::optional<uint16_t>
  findByBlockState(std::string const &blockState) const {
    auto found = fBlockStateLut.find(blockState);
    if (found == fBlockStateLut.end()) {
      return std::nullopt;
    } else {
      return found->second;
    }
  }

private:
  std::unordered_map<uintptr_t, uint16_t> fBlockPtrLut;
  std::unordered_map<std::string, uint16_t> fBlockStateLut;
  std::vector<std::shared_ptr<CompoundTag>> fPalette;
};

} // namespace j2b
