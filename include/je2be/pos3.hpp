#pragma once

namespace je2be {
using Pos3i = mcfile::Pos3i;
using Pos3iHasher = mcfile::Pos3iHasher;

class Pos3d;

class Pos3f : public mcfile::detail::Pos3<float> {
public:
  Pos3f(float x, float y, float z) : Pos3<float>(x, y, z) {}

  std::shared_ptr<ListTag> toListTag() const {
    auto tag = List<Tag::Type::Float>();
    tag->push_back(std::make_shared<FloatTag>(fX));
    tag->push_back(std::make_shared<FloatTag>(fY));
    tag->push_back(std::make_shared<FloatTag>(fZ));
    return tag;
  }

  inline Pos3d toD() const;
};

class Pos3d : public mcfile::detail::Pos3<double> {
public:
  Pos3d(double x, double y, double z) : Pos3<double>(x, y, z) {}

  std::shared_ptr<ListTag> toListTag() const {
    auto tag = List<Tag::Type::Double>();
    tag->push_back(std::make_shared<DoubleTag>(fX));
    tag->push_back(std::make_shared<DoubleTag>(fY));
    tag->push_back(std::make_shared<DoubleTag>(fZ));
    return tag;
  }

  Pos3f toF() const {
    return Pos3f(fX, fY, fZ);
  }
};

inline Pos3d Pos3f::toD() const {
  return Pos3d(fX, fY, fZ);
}

} // namespace je2be
