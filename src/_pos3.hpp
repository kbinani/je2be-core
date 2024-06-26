#pragma once

#include <je2be/nbt.hpp>

namespace je2be {
using Pos3i = mcfile::Pos3i;
using Pos3iHasher = mcfile::Pos3iHasher;

class Pos3d;

class Pos3f : public mcfile::detail::Pos3<float> {
public:
  Pos3f(float x, float y, float z) : Pos3<float>(x, y, z) {}

  ListTagPtr toListTag() const {
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

  ListTagPtr toListTag() const {
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

inline IntArrayTagPtr IntArrayFromPos3i(Pos3i p) {
  std::vector<i32> v(3);
  v[0] = p.fX;
  v[1] = p.fY;
  v[2] = p.fZ;
  return std::make_shared<mcfile::nbt::IntArrayTag>(v);
}

inline ListTagPtr ListFromPos3i(Pos3i p) {
  auto ret = List<Tag::Type::Int>();
  ret->push_back(Int(p.fX));
  ret->push_back(Int(p.fY));
  ret->push_back(Int(p.fZ));
  return ret;
}

} // namespace je2be
