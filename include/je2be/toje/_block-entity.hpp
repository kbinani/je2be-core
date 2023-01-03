#pragma once

#include <je2be/toje/_block-entity-convert-result.hpp>

namespace je2be::toje {

class Context;

class BlockEntity {
  BlockEntity() = delete;
  class Impl;

  using Result = BlockEntityConvertResult;

  using Converter = std::function<std::optional<Result>(Pos3i const &, mcfile::be::Block const &, CompoundTag const &, mcfile::je::Block const &, Context &ctx)>;

public:
  static std::optional<Result> FromBlockAndBlockEntity(Pos3i const &pos, mcfile::be::Block const &block, CompoundTag const &tag, mcfile::je::Block const &blockJ, Context &ctx);
  static ListTagPtr ContainerItems(CompoundTag const &parent, std::string const &key, Context &ctx);
};

} // namespace je2be::toje
