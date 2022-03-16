#pragma once

namespace je2be {

class BlockAccessor {
public:
  virtual ~BlockAccessor() {}
  virtual std::shared_ptr<mcfile::je::Block const> blockAt(int x, int y, int z) = 0;
};

} // namespace je2be
