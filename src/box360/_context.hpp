#pragma once

#include <je2be/nbt.hpp>
#include <je2be/uuid.hpp>

#include "_pos3.hpp"
#include "box360/_tile-entity-convert-result.hpp"

namespace je2be::box360 {

class Context {
public:
  using TileEntityConverter = std::function<std::optional<TileEntityConvertResult>(CompoundTag const &in, std::shared_ptr<mcfile::je::Block const> const &block, Pos3i const &pos, Context const &ctx)>;
  using EntityNameMigrator = std::function<std::string(std::string const &)>;

  Context(TileEntityConverter tileEntityConverter, EntityNameMigrator entityNameMigrator)
      : fTileEntityConverter(tileEntityConverter), fEntityNameMigrator(entityNameMigrator) {}

  TileEntityConverter const fTileEntityConverter;
  EntityNameMigrator const fEntityNameMigrator;
  std::unordered_map<std::string, Uuid> fPlayers;
};

} // namespace je2be::box360
