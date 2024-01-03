#pragma once

#include <je2be/nbt.hpp>
#include <je2be/uuid.hpp>

#include "_pos3.hpp"
#include "lce/_tile-entity-convert-result.hpp"

namespace je2be::box360 {

class Context {
public:
  using TileEntityConverter = std::function<std::optional<TileEntityConvertResult>(CompoundTag const &in, std::shared_ptr<mcfile::je::Block const> const &block, Pos3i const &pos, Context const &ctx)>;
  using EntityNameMigrator = std::function<std::u8string(std::u8string const &)>;

  Context(TileEntityConverter tileEntityConverter, EntityNameMigrator entityNameMigrator)
      : fTileEntityConverter(tileEntityConverter), fEntityNameMigrator(entityNameMigrator), fNewSeaLevel(false) {}

  TileEntityConverter const fTileEntityConverter;
  EntityNameMigrator const fEntityNameMigrator;
  std::unordered_map<std::u8string, Uuid> fPlayers;
  bool fNewSeaLevel;
};

} // namespace je2be::box360
