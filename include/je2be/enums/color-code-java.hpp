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
  case ColorCodeJava::White:
    return "white";
  case ColorCodeJava::Yellow:
    return "yellow";
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
  case ColorCodeJava::White:
    return "white";
  case ColorCodeJava::Yellow:
    return "yellow";
  }
}

inline int32_t BannerColorCodeFromJava(int32_t java) {
  ColorCodeJava ccj = (ColorCodeJava)java;
  switch (ccj) {
  case ColorCodeJava::Red:
    return (int32_t)BannerColorCodeBedrock::Red;
  case ColorCodeJava::Black:
    return (int32_t)BannerColorCodeBedrock::Black;
  case ColorCodeJava::Blue:
    return (int32_t)BannerColorCodeBedrock::Blue;
  case ColorCodeJava::Brown:
    return (int32_t)BannerColorCodeBedrock::Brown;
  case ColorCodeJava::Cyan:
    return (int32_t)BannerColorCodeBedrock::Cyan;
  case ColorCodeJava::Gray:
    return (int32_t)BannerColorCodeBedrock::Gray;
  case ColorCodeJava::Green:
    return (int32_t)BannerColorCodeBedrock::Green;
  case ColorCodeJava::LightBlue:
    return (int32_t)BannerColorCodeBedrock::LightBlue;
  case ColorCodeJava::LightGray:
    return (int32_t)BannerColorCodeBedrock::LightGray;
  case ColorCodeJava::Lime:
    return (int32_t)BannerColorCodeBedrock::Lime;
  case ColorCodeJava::Magenta:
    return (int32_t)BannerColorCodeBedrock::Magenta;
  case ColorCodeJava::Orange:
    return (int32_t)BannerColorCodeBedrock::Orange;
  case ColorCodeJava::Pink:
    return (int32_t)BannerColorCodeBedrock::Pink;
  case ColorCodeJava::Purple:
    return (int32_t)BannerColorCodeBedrock::Purple;
  case ColorCodeJava::White:
    return (int32_t)BannerColorCodeBedrock::White;
  case ColorCodeJava::Yellow:
    return (int32_t)BannerColorCodeBedrock::Yellow;
  }
}

} // namespace je2be
