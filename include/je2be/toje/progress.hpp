#pragma once

namespace je2be::toje {

class Progress {
public:
  enum class Phase {
    Dimension1,
    Dimension2,
    Dimension3,
  };

  virtual ~Progress() {}
  virtual bool report(Phase phase, double progress, double total) = 0;
};

} // namespace je2be::toje
