#pragma once

namespace je2be::toje {

class BlockData {
  using Input = mcfile::be::Block;
  using Return = std::shared_ptr<mcfile::je::Block const>;

public:
  static Return From(Input const &b) {
    return nullptr;
  }

  static Return Identity(Input const &b) {
    return std::make_shared<mcfile::je::Block const>(b.fName);
  }

private:
  BlockData() = delete;
};

} // namespace je2be::toje
