#pragma once

namespace je2be::box360 {

class Context {
public:
  using TileEntityConverter = std::function<std::optional<TileEntityConvertResult>(CompoundTag const &in, mcfile::je::Block const &block, Pos3i const &pos, Context const &ctx)>;

  explicit Context(TileEntityConverter tileEntityConverter) : fTileEntityConverter(tileEntityConverter) {}

  TileEntityConverter fTileEntityConverter;
};

} // namespace je2be::box360
