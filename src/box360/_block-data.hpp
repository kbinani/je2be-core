#pragma once

#include <je2be/integers.hpp>

#include <minecraft-file.hpp>

namespace je2be::box360 {

struct BlockData : public std::pair<u8, u8> {
private:
  class Impl;

public:
  explicit BlockData(u16 data) : std::pair<u8, u8>(0xff & (data >> 8), 0xff & data) {}
  BlockData() : std::pair<u8, u8>(0, 0) {}

  u16 rawBlockId() const {
    return this->first;
  }

  u8 rawData() const {
    return this->second;
  };

  u16 extendedBlockId() const {
    return (((u16)rawData() & 0x70) << 4) | rawBlockId();
  }

  u8 data() const {
    return rawData() & 0xf;
  }

  bool isWaterlogged() const {
    return (rawData() & 0x80) == 0x80;
  }

  std::shared_ptr<mcfile::je::Block const> toBlock() const {
    using namespace std;
    auto p = unsafeToBlock();
    if (p) {
      if (isWaterlogged()) {
        return p->applying({{"waterlogged", "true"}});
      } else {
        return p;
      }
    } else {
      return make_shared<mcfile::je::Block const>("minecraft:air");
    }
  }

  std::shared_ptr<mcfile::je::Block const> unsafeToBlock() const;
};

} // namespace je2be::box360
