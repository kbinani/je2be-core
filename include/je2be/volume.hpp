#pragma once

namespace j2b {

class Volume {
public:
  Volume(Pos3i start, Pos3i end) : fStart(start), fEnd(end) {}

  static std::optional<Volume> Intersection(Volume const &a, Volume const &b) {
    auto x = Intersection(a.fStart.fX, a.fEnd.fX, b.fStart.fX, b.fEnd.fX);
    auto y = Intersection(a.fStart.fY, a.fEnd.fY, b.fStart.fY, b.fEnd.fY);
    auto z = Intersection(a.fStart.fZ, a.fEnd.fZ, b.fStart.fZ, b.fEnd.fZ);
    if (!x || !y || !z) {
      return std::nullopt;
    }
    auto [x0, x1] = *x;
    auto [y0, y1] = *y;
    auto [z0, z1] = *z;
    Pos3i start(x0, y0, z0);
    Pos3i end(x1, y1, z1);
    return Volume(start, end);
  }

  static bool Equals(Volume const &a, Volume const &b) { return a.fStart == b.fStart && a.fEnd == b.fEnd; }

private:
  static std::optional<std::tuple<int32_t, int32_t>> Intersection(int32_t a0, int32_t a1, int32_t b0, int32_t b1) {
    using namespace std;
    int32_t maxLowerBound = std::max(a0, b0);
    int32_t minUpperBound = std::min(a1, b1);
    if (maxLowerBound < minUpperBound) {
      return make_tuple(maxLowerBound, minUpperBound);
    } else {
      return nullopt;
    }
  }

public:
  Pos3i fStart;
  Pos3i fEnd;
};

} // namespace j2b
