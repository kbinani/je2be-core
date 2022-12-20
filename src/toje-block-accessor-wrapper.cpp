#include <je2be/toje/block-accessor-wrapper.hpp>

#include <je2be/toje/block-data.hpp>

namespace je2be::toje {

template <>
std::shared_ptr<mcfile::je::Block const> BlockAccessorWrapper<3, 3>::Convert(mcfile::be::Block const &b) {
  return BlockData::From(b);
}

} // namespace je2be::toje
