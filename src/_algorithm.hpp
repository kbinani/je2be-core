#pragma once

#include <optional>

namespace je2be {

template <class T>
static inline std::optional<std::pair<T, T>> Intersection(std::pair<T, T> const &a, std::pair<T, T> const &b) {
  assert(a.first <= a.second);
  assert(b.first <= b.second);
  T minend = std::min<T>(a.second, b.second);
  if (b.first <= a.first && a.first <= b.second) {
    return std::make_pair(a.first, minend);
  } else if (a.first <= b.first && b.first <= a.second) {
    return std::make_pair(b.first, minend);
  } else {
    return std::nullopt;
  }
}

} // namespace je2be
