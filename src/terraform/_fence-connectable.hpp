#pragma once

#include "_mcfile-fwd.hpp"
#include "terraform/_block-accessor.hpp"

namespace je2be::terraform {

class BlockPropertyAccessor;

class FenceConnectable {
  class Impl;
  FenceConnectable() = delete;

public:
  static void Do(mcfile::je::Chunk &out, BlockAccessor<mcfile::je::Block> &cache, BlockPropertyAccessor const &accessor);
};

} // namespace je2be::terraform
