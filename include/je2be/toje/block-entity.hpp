#pragma once

namespace je2be::toje {

class BlockEntity {
  BlockEntity() = delete;

public:
  static std::shared_ptr<mcfile::nbt::CompoundTag> FromBlockAndBlockEntity(Pos3i const &pos, mcfile::be::Block const &block, mcfile::nbt::CompoundTag const &tag) {
    //TODO:
    return nullptr;
  }
};

} // namespace je2be::toje
