#pragma once

#include <je2be/integers.hpp>
#include <je2be/rational.hpp>

namespace je2be::toje {

class Progress {
public:
  virtual ~Progress() {}
  virtual bool reportConvert(Rational<u64> const &progress, u64 numConvertedChunks) = 0;
  virtual bool reportTerraform(Rational<u64> const &progress, u64 numProcessedChunks) = 0;
};

} // namespace je2be::toje
