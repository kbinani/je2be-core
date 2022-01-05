#pragma once

namespace je2be {

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

  std::shared_ptr<mcfile::nbt::Tag> toNbt() const {
    using namespace std;
    using namespace mcfile;
    using namespace mcfile::nbt;
    auto tag = make_shared<CompoundTag>();
    (*tag)["start"] = Pos3iToNbt(fStart);
    (*tag)["end"] = Pos3iToNbt(fEnd);
    return tag;
  }

  static std::optional<Volume> FromNbt(mcfile::nbt::Tag const &tag) {
    using namespace std;
    using namespace mcfile;
    using namespace mcfile::nbt;
    auto c = tag.asCompound();
    if (!c) {
      return nullopt;
    }
    auto startTag = c->compoundTag("start");
    auto endTag = c->compoundTag("end");
    if (!startTag || !endTag) {
      return nullopt;
    }
    auto start = Pos3iFromNbt(*startTag);
    auto end = Pos3iFromNbt(*endTag);
    if (!start || !end) {
      return nullopt;
    }
    return Volume(*start, *end);
  }

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

} // namespace je2be
