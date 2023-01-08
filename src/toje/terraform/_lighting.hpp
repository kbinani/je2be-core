#pragma once

#include "terraform/_block-accessor.hpp"

namespace je2be::toje {

class Lighting {
public:
  static void Do(mcfile::je::Chunk &out, terraform::BlockAccessor<mcfile::je::Block> &blockAccessor, terraform::BlockPropertyAccessor const &propertyAccessor) {
    int x = out.minBlockX();
    int y = -64;
    int z = out.minBlockZ();

    //TODO:
  }
};

} // namespace je2be::toje
