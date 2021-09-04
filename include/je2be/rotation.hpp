#pragma once

namespace je2be {

class Rotation {
public:
  Rotation(float yaw, float pitch) : fYaw(yaw), fPitch(pitch) {}

  std::shared_ptr<mcfile::nbt::ListTag> toListTag() const {
    using namespace mcfile::nbt;
    auto tag = std::make_shared<ListTag>(Tag::Type::Float);
    tag->push_back(std::make_shared<FloatTag>(fYaw));
    tag->push_back(std::make_shared<FloatTag>(fPitch));
    return tag;
  }

public:
  float fYaw;
  float fPitch;
};

} // namespace je2be
