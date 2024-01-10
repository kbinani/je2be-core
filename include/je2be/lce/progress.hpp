#pragma once

#include <je2be/integers.hpp>
#include <je2be/rational.hpp>

namespace je2be::lce {

class Progress {
public:
  virtual ~Progress() {}
  virtual bool report(Rational<u64> const &progress) = 0;
};

} // namespace je2be::lce
