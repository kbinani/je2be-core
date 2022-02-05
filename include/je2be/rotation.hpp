#pragma once

namespace je2be {

class Rotation {
public:
  Rotation(float yaw, float pitch) : fYaw(yaw), fPitch(pitch) {}

  std::shared_ptr<ListTag> toListTag() const {
    auto tag = std::make_shared<ListTag>(Tag::Type::Float);
    tag->push_back(std::make_shared<FloatTag>(fYaw));
    tag->push_back(std::make_shared<FloatTag>(fPitch));
    return tag;
  }

  static float ClampAngleBetween0To360(float v) {
    return std::fmodf(std::fmodf(v, 360.0f) + 360.0f, 360.0f);
  }

  static float ClampAngleBetweenMinus180To180(float v) {
    return ClampAngleBetween0To360(v + 180) - 180;
  }

public:
  float fYaw;
  float fPitch;
};

} // namespace je2be
