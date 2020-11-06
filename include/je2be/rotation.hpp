#pragma once

namespace j2b {

class Rotation {
public:
  Rotation(float yaw, float pitch) : fYaw(yaw), fPitch(pitch) {}

  std::shared_ptr<mcfile::nbt::ListTag> toListTag() const {
    using namespace mcfile::nbt;
    auto tag = std::make_shared<ListTag>();
    tag->fType = Tag::TAG_Float;
    tag->fValue = {std::make_shared<FloatTag>(fYaw),
                   std::make_shared<FloatTag>(fPitch)};
    return tag;
  }

public:
  float fYaw;
  float fPitch;
};

} // namespace j2b
