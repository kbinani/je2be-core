#pragma once

namespace je2be::terraform {

template <class Block>
class BlockAccessor {
public:
  virtual ~BlockAccessor() {}
  virtual std::shared_ptr<Block const> blockAt(int x, int y, int z) = 0;
};

} // namespace je2be::terraform
