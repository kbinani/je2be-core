#pragma once

namespace je2be::tobe {

class Progress {
public:
  enum class Phase {
    Convert,
    LevelDbCompaction,
  };

  virtual ~Progress() {}
  virtual bool report(Phase phase, double progress, double total) = 0;
};

} // namespace je2be::tobe
