#pragma once

namespace je2be::toje {

struct BlockEntityConvertResult {
  CompoundTagPtr fTileEntity;
  std::shared_ptr<mcfile::je::Block const> fBlock;
  std::optional<Pos3i> fTakeItemsFrom;
};

} // namespace je2be::toje
