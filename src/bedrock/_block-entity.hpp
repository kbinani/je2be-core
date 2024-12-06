#pragma once

#include "bedrock/_block-entity-convert-result.hpp"

namespace je2be::bedrock {

class Context;

class BlockEntity {
  BlockEntity() = delete;
  class Impl;

  using Result = BlockEntityConvertResult;

public:
  static std::optional<Result> FromBlockAndBlockEntity(Pos3i const &pos, mcfile::be::Block const &block, CompoundTag const &tag, mcfile::je::Block const &blockJ, Context &ctx, int dataVersion, bool item);
  static ListTagPtr ContainerItems(CompoundTag const &parent, std::u8string const &key, Context &ctx, int outputDataVersion, bool item);
};

} // namespace je2be::bedrock
