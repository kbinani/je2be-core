#pragma once

namespace j2b {

class Sign {
private:
  Sign() = delete;

public:
  static Rgba BedrockTexteColorFromJavaColorCode(ColorCodeJava code) {
    switch (code) {
    case ColorCodeJava::White:
      return Rgba(240, 240, 240);
    case ColorCodeJava::Orange:
      return Rgba(249, 128, 29);
    case ColorCodeJava::Magenta:
      return Rgba(199, 78, 189);
    case ColorCodeJava::LightBlue:
      return Rgba(58, 179, 218);
    case ColorCodeJava::Yellow:
      return Rgba(254, 216, 61);
    case ColorCodeJava::Lime:
      return Rgba(128, 199, 31);
    case ColorCodeJava::Pink:
      return Rgba(243, 139, 170);
    case ColorCodeJava::Gray:
      return Rgba(71, 79, 82);
    case ColorCodeJava::LightGray:
      return Rgba(157, 157, 151);
    case ColorCodeJava::Cyan:
      return Rgba(22, 156, 156);
    case ColorCodeJava::Purple:
      return Rgba(137, 50, 184);
    case ColorCodeJava::Blue:
      return Rgba(60, 68, 170);
    case ColorCodeJava::Brown:
      return Rgba(131, 84, 50);
    case ColorCodeJava::Green:
      return Rgba(94, 124, 22);
    case ColorCodeJava::Red:
      return Rgba(176, 46, 38);
    case ColorCodeJava::Black:
    default:
      return Rgba(0, 0, 0);
    }
  }
};

} // namespace j2b
