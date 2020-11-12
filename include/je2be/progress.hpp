#pragma once

namespace j2b {

class Progress {
public:
  enum class Phase {
    Convert,
    LevelDbCompaction,
  };

  virtual ~Progress() {}
  virtual bool report(Phase phase, double progress, double total) = 0;
};

} // namespace j2b
