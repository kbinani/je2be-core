#pragma once

#include "terraform/_block-accessor.hpp"

namespace je2be::terraform::java {

class BlockAccessorJava : public BlockAccessor<mcfile::je::Block> {
public:
  virtual ~BlockAccessorJava() {}
  virtual std::shared_ptr<mcfile::je::Chunk> at(int cx, int cz) const = 0;
};

} // namespace je2be::terraform::java
