#pragma once

#include "_mcfile-fwd.hpp"
#include "terraform/_block-accessor.hpp"

namespace je2be::terraform {

class BlockPropertyAccessor;

class NoteBlock {
  class Impl;
  NoteBlock() = delete;

public:
  static void Do(mcfile::je::Chunk &out, BlockAccessor<mcfile::je::Block> &cache, terraform::BlockPropertyAccessor const &accessor);
};

} // namespace je2be::terraform
