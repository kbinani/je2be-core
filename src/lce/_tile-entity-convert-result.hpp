#pragma once

#include <je2be/nbt.hpp>

namespace je2be::box360 {

struct TileEntityConvertResult {
  CompoundTagPtr fTileEntity;
  std::shared_ptr<mcfile::je::Block const> fBlock;
};

} // namespace je2be::box360
