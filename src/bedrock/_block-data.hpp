#pragma once

#include "enums/_color-code-java.hpp"
#include "enums/_facing4.hpp"
#include "enums/_facing6.hpp"
#include "enums/_red-flower.hpp"

namespace je2be::bedrock {

class BlockData {
  BlockData() = delete;
  class Impl;

public:
  static std::shared_ptr<mcfile::je::Block const> From(mcfile::be::Block const &b, int dataVersion);
  static std::shared_ptr<mcfile::je::Block const> Identity(mcfile::be::Block const &b, int dataVersion);
};

} // namespace je2be::bedrock
