#pragma once

#include "_mcfile-fwd.hpp"
#include "terraform/_block-accessor.hpp"
#include "terraform/_block-property-accessor.hpp"

namespace je2be::terraform {

class Leaves {
  Leaves() = delete;
  class Impl;

public:
  static void Do(mcfile::je::Chunk &out, BlockAccessor<mcfile::je::Block> &cache, BlockPropertyAccessor const &accessor);
  static void Do(mcfile::je::Chunk &out, BlockAccessor<mcfile::be::Block> &cache, BlockPropertyAccessor const &accessor);
};

} // namespace je2be::terraform
