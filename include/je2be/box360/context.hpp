#pragma once

namespace je2be::box360 {

class Context {
public:
  using TileEntityConverter = std::function<std::optional<TileEntityConvertResult>(CompoundTag const &in, std::shared_ptr<mcfile::je::Block const> const &block, Pos3i const &pos, Context const &ctx)>;
  using EntityNameMigrator = std::function<std::string(std::string const &)>;

  Context(TileEntityConverter tileEntityConverter, EntityNameMigrator entityNameMigrator, Options o)
      : fTileEntityConverter(tileEntityConverter), fEntityNameMigrator(entityNameMigrator), fOptions(o) {}

  TileEntityConverter const fTileEntityConverter;
  EntityNameMigrator const fEntityNameMigrator;
  Options const fOptions;
};

} // namespace je2be::box360
