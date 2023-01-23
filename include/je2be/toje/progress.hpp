#pragma once

namespace je2be::toje {

class Progress {
public:
  virtual ~Progress() {}
  virtual bool report(double progress, u64 numConvertedChunks) = 0;
};

} // namespace je2be::toje
