#pragma once

#include "_closed-range.hpp"
#include "_pos3.hpp"

namespace je2be {

class Volume {
public:
  Volume(Pos3i start, Pos3i end) : fStart(start), fEnd(end) {}

  bool contains(Pos3i const &p) const {
    return fStart.fX <= p.fX && p.fX <= fEnd.fX && fStart.fY <= p.fY && p.fY <= fEnd.fY && fStart.fZ <= p.fZ && p.fZ <= fEnd.fZ;
  }

  static Volume Union(Volume const &a, Volume const &b) {
    Pos3i start((std::min)(a.fStart.fX, b.fStart.fX), (std::min)(a.fStart.fY, b.fStart.fY), (std::min)(a.fStart.fZ, b.fStart.fZ));
    Pos3i end((std::max)(a.fEnd.fX, b.fEnd.fX), (std::max)(a.fEnd.fY, b.fEnd.fY), (std::max)(a.fEnd.fZ, b.fEnd.fZ));
    return Volume(start, end);
  }

  static std::optional<Volume> Intersection(Volume const &a, Volume const &b) {
    using namespace std;
    auto x = ClosedRange<int>::Intersection({a.fStart.fX, a.fEnd.fX}, {b.fStart.fX, b.fEnd.fX});
    auto y = ClosedRange<int>::Intersection({a.fStart.fY, a.fEnd.fY}, {b.fStart.fY, b.fEnd.fY});
    auto z = ClosedRange<int>::Intersection({a.fStart.fZ, a.fEnd.fZ}, {b.fStart.fZ, b.fEnd.fZ});
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

  static void Connect(std::vector<Volume> &volumes) {
    using namespace std;
    while (true) {
      int before = volumes.size();
      CollectAndConnect<0, 1, 2>(volumes);
      CollectAndConnect<0, 2, 1>(volumes);
      CollectAndConnect<1, 2, 0>(volumes);
      if (before == volumes.size()) {
        break;
      }
    }
  }

  static void ConnectGreed(std::vector<Volume> &volumes, int maxSizeX, int maxSizeY, int maxSizeZ) {
    while (true && volumes.size() > 1) {
      int before = volumes.size();
      std::stable_sort(volumes.begin(), volumes.end(), [](Volume const &a, Volume const &b) {
        return a.volume() > b.volume();
      });
      std::vector<Volume> tmp;
      for (int i = 0; i < volumes.size(); i++) {
        Volume a = volumes[i];
        int match = -1;
        for (int j = i + 1; j < volumes.size(); j++) {
          Volume b = volumes[j];
          int x0 = std::min(a.fStart.fX, b.fStart.fX);
          int y0 = std::min(a.fStart.fY, b.fStart.fY);
          int z0 = std::min(a.fStart.fZ, b.fStart.fZ);
          int x1 = std::max(a.fEnd.fX, b.fEnd.fX);
          int y1 = std::max(a.fEnd.fY, b.fEnd.fY);
          int z1 = std::max(a.fEnd.fZ, b.fEnd.fZ);
          if (x1 - x0 <= maxSizeX && y1 - y0 <= maxSizeY && z1 - z0 <= maxSizeZ) {
            tmp.push_back(Volume({x0, y0, z0}, {x1, y1, z1}));
            match = j;
            break;
          }
        }
        if (match >= 0) {
          for (int j = match + 1; j < volumes.size(); j++) {
            tmp.push_back(volumes[j]);
          }
          break;
        } else {
          tmp.push_back(a);
        }
      }
      tmp.swap(volumes);
      if (before == volumes.size()) {
        break;
      }
    }
  }

  template <size_t dimension> // = 0 for X, = 1 for Y, = 2 for Z
  i32 size() const {
    return end<dimension>() - start<dimension>() + 1;
  }

  template <size_t dimension>
  i32 start() const {
    static_assert(dimension < 3);
    switch (dimension) {
    case 0:
      return fStart.fX;
    case 1:
      return fStart.fY;
    case 2:
      return fStart.fZ;
    default:
      return 0;
    }
  }

  template <size_t dimension>
  i32 end() const {
    static_assert(dimension < 3);
    switch (dimension) {
    case 0:
      return fEnd.fX;
    case 1:
      return fEnd.fY;
    case 2:
      return fEnd.fZ;
    default:
      return 0;
    }
  }

  u64 volume() const {
    u64 dx = size<0>();
    u64 dy = size<1>();
    u64 dz = size<2>();
    return dx * dy * dz;
  }

private:
  template <size_t d1, size_t d2, size_t d3>
  static void CollectAndConnect(std::vector<Volume> &volumes) {
    static_assert(d1 < 3 && d2 < 3 && d3 < 3 && d1 != d2 && d2 != d3 && d1 != d3);
    using namespace std;
    vector<Volume> tmp;
    vector<vector<Volume>> buffer;
    CollectAlignedCuboids<d1, d2>(volumes, buffer);
    for (auto &collected : buffer) {
      ConnectAdjacentCuboids<d3>(collected);
      copy(collected.begin(), collected.end(), back_inserter(tmp));
    }
    tmp.swap(volumes);
  }

  template <size_t d1, size_t d2>
  static void CollectAlignedCuboids(std::vector<Volume> const &input, std::vector<std::vector<Volume>> &output) {
    static_assert(d1 < 3 && d2 < 3 && d1 != d2);
    using namespace std;
    map<tuple<i32, i32, i32, i32>, vector<Volume>> categorized;
    for (Volume const &v : input) {
      i32 s1 = v.start<d1>();
      i32 e1 = v.end<d1>();
      i32 s2 = v.start<d2>();
      i32 e2 = v.end<d2>();
      tuple<i32, i32, i32, i32> key(s1, e1, s2, e2);
      categorized[key].push_back(v);
    }
    output.clear();
    for (auto const &it : categorized) {
      output.push_back(it.second);
    }
  }

  template <size_t d3>
  static void ConnectAdjacentCuboids(std::vector<Volume> &inout) {
    static_assert(d3 < 3);
    using namespace std;
    while (true) {
      vector<Volume> buffer;
      vector<bool> used(inout.size(), false);
      unordered_set<int> ignore;
      for (int i = 0; i < inout.size() - 1; i++) {
        if (used[i]) {
          continue;
        }
        Volume v1 = inout[i];
        i32 s1 = v1.start<d3>();
        i32 e1 = v1.end<d3>();
        for (int j = i + 1; j < inout.size(); j++) {
          if (used[j]) {
            continue;
          }
          Volume v2 = inout[j];
          i32 s2 = v2.start<d3>();
          i32 e2 = v2.end<d3>();
          if (e1 + 1 == s2 || e2 + 1 == s1) {
            Pos3i start((min)(v1.start<0>(), v2.start<0>()), (min)(v1.start<1>(), v2.start<1>()), (min)(v1.start<2>(), v2.start<2>()));
            Pos3i end((max)(v1.end<0>(), v2.end<0>()), (max)(v1.end<1>(), v2.end<1>()), (max)(v1.end<2>(), v2.end<2>()));
            Volume connected(start, end);
            buffer.push_back(connected);
            used[i] = true;
            used[j] = true;
            break;
          }
        }
      }
      for (int i = 0; i < inout.size(); i++) {
        if (!used[i]) {
          buffer.push_back(inout[i]);
        }
      }
      if (buffer.size() == inout.size()) {
        break;
      }
      buffer.swap(inout);
    }
  }

public:
  Pos3i fStart;
  Pos3i fEnd;
};

} // namespace je2be
