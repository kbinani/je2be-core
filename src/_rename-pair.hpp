#pragma once

namespace je2be {

template <class T>
class RenamePair {
public:
  std::u8string fBefore;
  std::u8string fAfter;
  std::optional<T> fDefault;

  RenamePair(std::u8string const &before, std::u8string const &after) : fBefore(before), fAfter(after), fDefault(std::nullopt) {
  }

  RenamePair(std::u8string const &before, std::u8string const &after, T def) : fBefore(before), fAfter(after), fDefault(def) {
  }

  RenamePair(std::u8string const &same) : fBefore(same), fAfter(same), fDefault(std::nullopt) {}
};

} // namespace je2be
