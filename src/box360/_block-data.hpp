#pragma once

#include <minecraft-file.hpp>

namespace je2be::box360 {

struct BlockData : public std::pair<uint8_t, uint8_t> {
private:
  class Impl;

public:
  explicit BlockData(uint16_t data) : std::pair<uint8_t, uint8_t>(0xff & (data >> 8), 0xff & data) {}
  BlockData() : std::pair<uint8_t, uint8_t>(0, 0) {}

  uint16_t rawBlockId() const {
    return this->first;
  }

  uint8_t rawData() const {
    return this->second;
  };

  uint16_t extendedBlockId() const {
    return (((uint16_t)rawData() & 0x70) << 4) | rawBlockId();
  }

  uint8_t data() const {
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
