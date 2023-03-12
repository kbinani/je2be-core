#pragma once

namespace je2be {

template <std::integral T>
struct Rational {
  T fNum;
  T fDen;

  Rational(T num, T den) : fNum(num), fDen(den) {}

  double toD() const {
    return double(fNum) / double(fDen);
  }

  float toF() const {
    return float(fNum) / float(fDen);
  }
};

} // namespace je2be
