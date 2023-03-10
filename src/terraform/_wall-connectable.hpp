#pragma once

#include "_mcfile-fwd.hpp"
#include "terraform/_block-accessor.hpp"

namespace je2be::terraform {

class BlockPropertyAccessor;

class WallConnectable {
  class Impl;
  WallConnectable() = delete;

public:
  static void Do(mcfile::je::Chunk &out, BlockAccessor<mcfile::je::Block> &blockAccessor, BlockPropertyAccessor const &accessor);
};

} // namespace je2be::terraform
