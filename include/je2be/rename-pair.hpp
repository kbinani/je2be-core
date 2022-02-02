#pragma once

namespace je2be {

class RenamePair {
public:
  std::string fBefore;
  std::string fAfter;

  RenamePair(std::string const &before, std::string const &after) : fBefore(before), fAfter(after) {
    assert(before != after);
  }

  RenamePair(std::string const &same) : fBefore(same), fAfter(same) {}
};

} // namespace je2be
