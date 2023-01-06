#pragma once

namespace je2be::toje {

class Progress {
public:
  virtual ~Progress() {}
  virtual bool report(double progress, uint64_t numConvertedChunks) = 0;
};

} // namespace je2be::toje
