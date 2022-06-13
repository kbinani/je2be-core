#pragma once

namespace je2be::tobe {

struct Context {
  Context(JavaEditionMap const &mapInfo,
          WorldData &wd,
          int64_t gameTick,
          int difficultyBedrock,
          bool allowCommand,
          GameMode gameType,
          std::function<CompoundTagPtr(Pos3i const &, mcfile::je::Block const &, CompoundTagPtr const &, Context const &)> fromBlockAndTileEntity) : fMapInfo(mapInfo), fWorldData(wd), fGameTick(gameTick), fDifficultyBedrock(difficultyBedrock), fAllowCommand(allowCommand), fGameType(gameType), fFromBlockAndTileEntity(fromBlockAndTileEntity) {}

  JavaEditionMap const &fMapInfo;
  WorldData &fWorldData;
  int64_t const fGameTick;
  int const fDifficultyBedrock;
  bool const fAllowCommand;
  GameMode const fGameType;

  // NOTE: This std::function must be TileEntity::FromBlockAndTileEntity. By doing this, the "Item" class can use the function (the "Item" class is #includ'ed before "TileEntity")
  std::function<CompoundTagPtr(Pos3i const &, mcfile::je::Block const &, CompoundTagPtr const &, Context const &)> const fFromBlockAndTileEntity;
};

} // namespace je2be::tobe
