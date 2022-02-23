#pragma once

namespace je2be::tobe {

struct Context {
  Context(JavaEditionMap const &mapInfo, WorldData &wd, int64_t gameTick, std::function<std::shared_ptr<CompoundTag>(Pos3i const &, mcfile::je::Block const &, std::shared_ptr<CompoundTag> const &, Context const &)> fromBlockAndTileEntity) : fMapInfo(mapInfo), fWorldData(wd), fGameTick(gameTick), fFromBlockAndTileEntity(fromBlockAndTileEntity) {}

  JavaEditionMap const &fMapInfo;
  WorldData &fWorldData;
  int64_t const fGameTick;

  // NOTE: This std::function must be TileEntity::FromBlockAndTileEntity. By doing this, the "Item" class can use the function (the "Item" class is #includ'ed before "TileEntity")
  std::function<std::shared_ptr<CompoundTag>(Pos3i const &, mcfile::je::Block const &, std::shared_ptr<CompoundTag> const &, Context const &)> const fFromBlockAndTileEntity;
};

} // namespace je2be::tobe
