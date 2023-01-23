#pragma once

#include <je2be/integers.hpp>

namespace je2be::tobe {

class Progress {
public:
  virtual ~Progress() {}
  virtual bool reportConvert(double progress, u64 numConvertedChunks) = 0;
  virtual bool reportCompaction(double progress) = 0;
};

} // namespace je2be::tobe
