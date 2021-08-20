#pragma once

namespace j2b {

class Vec {
public:
  Vec(float x, float y, float z) : fX(x), fY(y), fZ(z) {}

  std::shared_ptr<mcfile::nbt::ListTag> toListTag() const {
    using namespace mcfile::nbt;
    auto tag = std::make_shared<ListTag>(Tag::Type::Float);
    tag->push_back(std::make_shared<FloatTag>(fX));
    tag->push_back(std::make_shared<FloatTag>(fY));
    tag->push_back(std::make_shared<FloatTag>(fZ));
    return tag;
  }

public:
  float fX;
  float fY;
  float fZ;
};

} // namespace j2b
