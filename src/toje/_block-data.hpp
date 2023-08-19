#pragma once

#include "enums/_color-code-java.hpp"
#include "enums/_facing4.hpp"
#include "enums/_facing6.hpp"
#include "enums/_red-flower.hpp"

namespace je2be::toje {

class BlockData {
  BlockData() = delete;
  class Impl;

  using String = std::string;
  using Props = std::map<std::string, std::string>;
  using Converter = std::function<String(String const &bName, CompoundTag const &s, Props &p)>;

public:
  static std::shared_ptr<mcfile::je::Block const> From(mcfile::be::Block const &b);
  static std::shared_ptr<mcfile::je::Block const> Identity(mcfile::be::Block const &b);
};

} // namespace je2be::toje
