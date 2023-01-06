#pragma once

namespace je2be::tobe {

class Progress {
public:
  virtual ~Progress() {}
  virtual bool reportConvert(double progress, uint64_t numConvertedChunks) = 0;
  virtual bool reportCompaction(double progress) = 0;
};

} // namespace je2be::tobe
