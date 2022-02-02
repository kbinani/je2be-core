#pragma once

namespace je2be {

template <class T>
class RenamePair {
public:
  std::string fBefore;
  std::string fAfter;
  std::optional<T> fDefault;

  RenamePair(std::string const &before, std::string const &after) : fBefore(before), fAfter(after), fDefault(std::nullopt) {
  }

  RenamePair(std::string const &before, std::string const &after, T def) : fBefore(before), fAfter(after), fDefault(def) {
  }

  RenamePair(std::string const &same) : fBefore(same), fAfter(same), fDefault(std::nullopt) {}
};

} // namespace je2be
