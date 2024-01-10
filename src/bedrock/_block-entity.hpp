#pragma once

#include "bedrock/_block-entity-convert-result.hpp"

namespace je2be::bedrock {

class Context;

class BlockEntity {
  BlockEntity() = delete;
  class Impl;

  using Result = BlockEntityConvertResult;

  using Converter = std::function<std::optional<Result>(Pos3i const &, mcfile::be::Block const &, CompoundTag const &, mcfile::je::Block const &, Context &ctx, int dataVersion)>;

public:
  static std::optional<Result> FromBlockAndBlockEntity(Pos3i const &pos, mcfile::be::Block const &block, CompoundTag const &tag, mcfile::je::Block const &blockJ, Context &ctx, int dataVersion);
  static ListTagPtr ContainerItems(CompoundTag const &parent, std::u8string const &key, Context &ctx, int dataVersion);
};

} // namespace je2be::bedrock
