#pragma once

namespace je2be::toje {

struct BlockEntityConvertResult {
  CompoundTagPtr fTileEntity;
  std::shared_ptr<mcfile::je::Block const> fBlock;
};

} // namespace je2be::toje
