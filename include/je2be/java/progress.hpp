#pragma once

#include <je2be/integers.hpp>
#include <je2be/rational.hpp>

namespace je2be::java {

class Progress {
public:
  virtual ~Progress() {}
  virtual bool reportConvert(Rational<u64> const &progress, u64 numConvertedChunks) = 0;
  virtual bool reportEntityPostProcess(Rational<u64> const &) = 0;
  virtual bool reportCompaction(Rational<u64> const &) = 0;
};

} // namespace je2be::java
