#pragma once

#include <je2be/pos2.hpp>

#include <je2be/_mcfile-fwd.hpp>
#include <je2be/terraform/_block-accessor.hpp>

namespace je2be::terraform {

class BlockPropertyAccessor;

class ShapeOfStairs {
  class Impl;
  ShapeOfStairs() = delete;

public:
  static void Do(mcfile::je::Chunk &out, BlockAccessor<mcfile::je::Block> &dataAccessor, BlockPropertyAccessor const &accessor);

  static Pos2i VecFromWeirdoDirection(mcfile::blocks::data::Directional::BlockFace d);
};

} // namespace je2be::terraform
