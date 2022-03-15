#pragma once

namespace je2be::toje {

class BlockAccessor {
public:
  virtual ~BlockAccessor() {}
  virtual std::shared_ptr<mcfile::je::Block const> blockAt(int x, int y, int z) = 0;
};

template <size_t width, size_t height>
class ChunkCacheWrapper : public BlockAccessor {
public:
  explicit ChunkCacheWrapper(ChunkCache<width, height> &base) : fBase(base) {}

  std::shared_ptr<mcfile::je::Block const> blockAt(int x, int y, int z) override {
    auto b = fBase.blockAt(x, y, z);
    if (!b) {
      return nullptr;
    }
    return BlockData::From(*b);
  }

private:
  ChunkCache<width, height> & fBase;
};

} // namespace je2be
