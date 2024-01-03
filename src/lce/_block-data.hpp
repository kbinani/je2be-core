#pragma once

#include <je2be/integers.hpp>

#include <minecraft-file.hpp>

namespace je2be::box360 {

struct BlockData {
private:
  class Impl;

public:
  BlockData() : fRawId(0), fRawData(0) {}
  BlockData(u8 rawId, u8 rawData) : fRawId(rawId), fRawData(rawData) {}

  u16 id() const {
    return (((u16)fRawData & 0x70) << 4) | fRawId;
  }

  u8 data() const {
    return fRawData & 0xf;
  }

  bool isWaterlogged() const {
    return (fRawData & 0x80) == 0x80;
  }

  std::shared_ptr<mcfile::je::Block const> toBlock() const;

public:
  u8 fRawId;
  u8 fRawData;

private:
  std::shared_ptr<mcfile::je::Block const> unsafeToBlock() const;
};

} // namespace je2be::box360
