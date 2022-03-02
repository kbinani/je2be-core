#pragma once

namespace je2be {

class Rotation {
public:
  Rotation(float yaw, float pitch) : fYaw(yaw), fPitch(pitch) {}

  ListTagPtr toListTag() const {
    auto tag = List<Tag::Type::Float>();
    tag->push_back(std::make_shared<FloatTag>(fYaw));
    tag->push_back(std::make_shared<FloatTag>(fPitch));
    return tag;
  }

  static bool DegAlmostEquals(float a, float b) {
    a = ClampDegreesBetween0And360(a);
    b = ClampDegreesBetween0And360(b);
    float diff = ClampDegreesBetween0And360(a - b);
    assert(0 <= diff && diff < 360);
    float const tolerance = 1;
    if (0 <= diff && diff < tolerance) {
      return true;
    } else if (360 - tolerance < diff) {
      return true;
    } else {
      return false;
    }
  }

  static float ClampDegreesBetween0And360(float v) {
    return std::fmod(std::fmod(v, 360.0f) + 360.0f, 360.0f);
  }

  static float ClampDegreesBetweenMinus180And180(float v) {
    return ClampDegreesBetween0And360(v + 180) - 180;
  }

  static float DiffDegrees(float a, float b) {
    return std::fabs(std::atan2(std::sin((a - b) * std::numbers::pi / 180.0), std::cos((a - b) * std::numbers::pi / 180.0)));
  }

public:
  float fYaw;
  float fPitch;
};

} // namespace je2be
