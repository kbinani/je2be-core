#pragma once

namespace je2be::toje {

class BlockData {
public:
  static std::shared_ptr<mcfile::je::Block const> From(mcfile::nbt::CompoundTag const &tag) {
    //TODO:
    return nullptr;
  }

private:
  BlockData() = delete;
};

} // namespace je2be::toje
