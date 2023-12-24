#pragma once

#include "terraform/_block-accessor.hpp"
#include "terraform/bedrock/_block-accessor-bedrock.hpp"
#include "toje/_block-data.hpp"

namespace je2be::toje {

template <size_t width, size_t height>
class BlockAccessorWrapper : public je2be::terraform::BlockAccessor<mcfile::je::Block> {
public:
  BlockAccessorWrapper(je2be::terraform::bedrock::BlockAccessorBedrock<width, height> &base, int targetDataVersion) : fBase(base), fTargetDataVersion(targetDataVersion) {}

  std::shared_ptr<mcfile::je::Block const> blockAt(int x, int y, int z) override {
    auto b = fBase.blockAt(x, y, z);
    if (!b) {
      return nullptr;
    }
    return BlockData::From(*b, fTargetDataVersion);
  }

private:
  je2be::terraform::bedrock::BlockAccessorBedrock<width, height> &fBase;
  int const fTargetDataVersion;
};

} // namespace je2be::toje
