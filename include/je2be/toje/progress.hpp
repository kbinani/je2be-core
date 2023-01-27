#pragma once

namespace je2be::toje {

class Progress {
public:
  virtual ~Progress() {}
  virtual bool reportConvert(double progress, u64 numConvertedChunks) = 0;
  virtual bool reportTerraform(double progress, u64 numProcessedChunks) = 0;
};

} // namespace je2be::toje
