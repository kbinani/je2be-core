#pragma once

namespace je2be::toje {

class Progress {
public:
  virtual ~Progress() {}
  virtual bool report(double progress, double total) = 0;
};

} // namespace je2be::toje
