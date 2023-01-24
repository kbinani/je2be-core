#pragma once

#include <optional>

namespace je2be {

template <class T>
class ClosedRange {
public:
  ClosedRange(T minimum, T maximum) : fMin(minimum), fMax(maximum) {
    assert(minimum <= maximum);
  }

  static std::optional<ClosedRange<T>> Intersection(ClosedRange<T> const &a, ClosedRange<T> const &b) {
    T minend = std::min<T>(a.fMax, b.fMax);
    if (b.fMin <= a.fMin && a.fMin <= b.fMax) {
      return ClosedRange<T>(a.fMin, minend);
    } else if (a.fMin <= b.fMin && b.fMin <= a.fMax) {
      return ClosedRange<T>(b.fMin, minend);
    } else {
      return std::nullopt;
    }
  }

  T fMin;
  T fMax;
};

} // namespace je2be
