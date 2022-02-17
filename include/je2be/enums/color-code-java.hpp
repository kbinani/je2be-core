#pragma once

namespace je2be {

enum class ColorCodeJava : int32_t {
  White = 0,
  Orange = 1,
  Magenta = 2,
  LightBlue = 3,
  Yellow = 4,
  Lime = 5,
  Pink = 6,
  Gray = 7,
  LightGray = 8,
  Cyan = 9,
  Purple = 10,
  Blue = 11,
  Brown = 12,
  Green = 13,
  Red = 14,
  Black = 15,
};

inline ColorCodeJava ColorCodeJavaFromJavaName(std::string const &color) {
  static std::unordered_map<std::string, ColorCodeJava> const mapping = {
      {"white", ColorCodeJava::White},
      {"orange", ColorCodeJava::Orange},
      {"magenta", ColorCodeJava::Magenta},
      {"light_blue", ColorCodeJava::LightBlue},
      {"yellow", ColorCodeJava::Yellow},
      {"lime", ColorCodeJava::Lime},
      {"pink", ColorCodeJava::Pink},
      {"gray", ColorCodeJava::Gray},
      {"light_gray", ColorCodeJava::LightGray},
      {"cyan", ColorCodeJava::Cyan},
      {"purple", ColorCodeJava::Purple},
      {"blue", ColorCodeJava::Blue},
      {"brown", ColorCodeJava::Brown},
      {"green", ColorCodeJava::Green},
      {"red", ColorCodeJava::Red},
      {"black", ColorCodeJava::Black},
  };
  auto found = mapping.find(color);
  if (found == mapping.end()) {
    return ColorCodeJava::White;
  } else {
    return found->second;
  }
}

inline ColorCodeJava ColorCodeJavaFromBedrockName(std::string const &color) {
  static std::unordered_map<std::string, ColorCodeJava> const mapping = {
      {"white", ColorCodeJava::White},
      {"orange", ColorCodeJava::Orange},
      {"magenta", ColorCodeJava::Magenta},
      {"light_blue", ColorCodeJava::LightBlue},
      {"yellow", ColorCodeJava::Yellow},
      {"lime", ColorCodeJava::Lime},
      {"pink", ColorCodeJava::Pink},
      {"gray", ColorCodeJava::Gray},
      {"silver", ColorCodeJava::LightGray},
      {"cyan", ColorCodeJava::Cyan},
      {"purple", ColorCodeJava::Purple},
      {"blue", ColorCodeJava::Blue},
      {"brown", ColorCodeJava::Brown},
      {"green", ColorCodeJava::Green},
      {"red", ColorCodeJava::Red},
      {"black", ColorCodeJava::Black},
  };
  auto found = mapping.find(color);
  if (found == mapping.end()) {
    return ColorCodeJava::White;
  } else {
    return found->second;
  }
}

inline std::string JavaNameFromColorCodeJava(ColorCodeJava code) {
  switch (code) {
  case ColorCodeJava::Black:
    return "black";
  case ColorCodeJava::Blue:
    return "blue";
  case ColorCodeJava::Brown:
    return "brown";
  case ColorCodeJava::Cyan:
    return "cyan";
  case ColorCodeJava::Gray:
    return "gray";
  case ColorCodeJava::Green:
    return "green";
  case ColorCodeJava::LightBlue:
    return "light_blue";
  case ColorCodeJava::LightGray:
    return "light_gray";
  case ColorCodeJava::Lime:
    return "lime";
  case ColorCodeJava::Magenta:
    return "magenta";
  case ColorCodeJava::Orange:
    return "orange";
  case ColorCodeJava::Pink:
    return "pink";
  case ColorCodeJava::Purple:
    return "purple";
  case ColorCodeJava::Red:
    return "red";
  case ColorCodeJava::Yellow:
    return "yellow";
  case ColorCodeJava::White:
  default:
    return "white";
  }
}

inline std::string BedrockNameFromColorCodeJava(ColorCodeJava code) {
  switch (code) {
  case ColorCodeJava::Black:
    return "black";
  case ColorCodeJava::Blue:
    return "blue";
  case ColorCodeJava::Brown:
    return "brown";
  case ColorCodeJava::Cyan:
    return "cyan";
  case ColorCodeJava::Gray:
    return "gray";
  case ColorCodeJava::Green:
    return "green";
  case ColorCodeJava::LightBlue:
    return "light_blue";
  case ColorCodeJava::LightGray:
    return "silver";
  case ColorCodeJava::Lime:
    return "lime";
  case ColorCodeJava::Magenta:
    return "magenta";
  case ColorCodeJava::Orange:
    return "orange";
  case ColorCodeJava::Pink:
    return "pink";
  case ColorCodeJava::Purple:
    return "purple";
  case ColorCodeJava::Red:
    return "red";
  case ColorCodeJava::Yellow:
    return "yellow";
  case ColorCodeJava::White:
  default:
    return "white";
  }
}

inline BannerColorCodeBedrock BannerColorCodeFromJava(ColorCodeJava ccj) {
  switch (ccj) {
  case ColorCodeJava::Red:
    return BannerColorCodeBedrock::Red;
  case ColorCodeJava::Black:
    return BannerColorCodeBedrock::Black;
  case ColorCodeJava::Blue:
    return BannerColorCodeBedrock::Blue;
  case ColorCodeJava::Brown:
    return BannerColorCodeBedrock::Brown;
  case ColorCodeJava::Cyan:
    return BannerColorCodeBedrock::Cyan;
  case ColorCodeJava::Gray:
    return BannerColorCodeBedrock::Gray;
  case ColorCodeJava::Green:
    return BannerColorCodeBedrock::Green;
  case ColorCodeJava::LightBlue:
    return BannerColorCodeBedrock::LightBlue;
  case ColorCodeJava::LightGray:
    return BannerColorCodeBedrock::LightGray;
  case ColorCodeJava::Lime:
    return BannerColorCodeBedrock::Lime;
  case ColorCodeJava::Magenta:
    return BannerColorCodeBedrock::Magenta;
  case ColorCodeJava::Orange:
    return BannerColorCodeBedrock::Orange;
  case ColorCodeJava::Pink:
    return BannerColorCodeBedrock::Pink;
  case ColorCodeJava::Purple:
    return BannerColorCodeBedrock::Purple;
  case ColorCodeJava::Yellow:
    return BannerColorCodeBedrock::Yellow;
  case ColorCodeJava::White:
  default:
    return BannerColorCodeBedrock::White;
  }
}

static inline ColorCodeJava ColorCodeJavaFromBannerColorCodeBedrock(BannerColorCodeBedrock bccb) {
  switch (bccb) {
  case BannerColorCodeBedrock::Red:
    return ColorCodeJava::Red;
  case BannerColorCodeBedrock::Black:
    return ColorCodeJava::Black;
  case BannerColorCodeBedrock::Green:
    return ColorCodeJava::Green;
  case BannerColorCodeBedrock::Brown:
    return ColorCodeJava::Brown;
  case BannerColorCodeBedrock::Blue:
    return ColorCodeJava::Blue;
  case BannerColorCodeBedrock::Purple:
    return ColorCodeJava::Purple;
  case BannerColorCodeBedrock::Cyan:
    return ColorCodeJava::Cyan;
  case BannerColorCodeBedrock::LightGray:
    return ColorCodeJava::LightGray;
  case BannerColorCodeBedrock::Gray:
    return ColorCodeJava::Gray;
  case BannerColorCodeBedrock::Pink:
    return ColorCodeJava::Pink;
  case BannerColorCodeBedrock::Lime:
    return ColorCodeJava::Lime;
  case BannerColorCodeBedrock::Yellow:
    return ColorCodeJava::Yellow;
  case BannerColorCodeBedrock::LightBlue:
    return ColorCodeJava::LightBlue;
  case BannerColorCodeBedrock::Magenta:
    return ColorCodeJava::Magenta;
  case BannerColorCodeBedrock::Orange:
    return ColorCodeJava::Orange;
  case BannerColorCodeBedrock::White:
  default:
    return ColorCodeJava::White;
  }
}

} // namespace je2be
