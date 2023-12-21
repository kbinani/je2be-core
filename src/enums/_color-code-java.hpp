#pragma once

#include "enums/_banner-color-code-bedrock.hpp"

#include <minecraft-file.hpp>

namespace je2be {

enum class ColorCodeJava : i32 {
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

static inline ColorCodeJava ColorCodeJavaFromJavaName(std::u8string const &color) {
  static std::unordered_map<std::u8string, ColorCodeJava> const mapping = {
      {u8"white", ColorCodeJava::White},
      {u8"orange", ColorCodeJava::Orange},
      {u8"magenta", ColorCodeJava::Magenta},
      {u8"light_blue", ColorCodeJava::LightBlue},
      {u8"yellow", ColorCodeJava::Yellow},
      {u8"lime", ColorCodeJava::Lime},
      {u8"pink", ColorCodeJava::Pink},
      {u8"gray", ColorCodeJava::Gray},
      {u8"light_gray", ColorCodeJava::LightGray},
      {u8"cyan", ColorCodeJava::Cyan},
      {u8"purple", ColorCodeJava::Purple},
      {u8"blue", ColorCodeJava::Blue},
      {u8"brown", ColorCodeJava::Brown},
      {u8"green", ColorCodeJava::Green},
      {u8"red", ColorCodeJava::Red},
      {u8"black", ColorCodeJava::Black},
  };
  auto found = mapping.find(color);
  if (found == mapping.end()) {
    return ColorCodeJava::White;
  } else {
    return found->second;
  }
}

static inline ColorCodeJava ColorCodeJavaFromBedrockName(std::u8string const &color) {
  static std::unordered_map<std::u8string, ColorCodeJava> const mapping = {
      {u8"white", ColorCodeJava::White},
      {u8"orange", ColorCodeJava::Orange},
      {u8"magenta", ColorCodeJava::Magenta},
      {u8"light_blue", ColorCodeJava::LightBlue},
      {u8"yellow", ColorCodeJava::Yellow},
      {u8"lime", ColorCodeJava::Lime},
      {u8"pink", ColorCodeJava::Pink},
      {u8"gray", ColorCodeJava::Gray},
      {u8"silver", ColorCodeJava::LightGray},
      {u8"cyan", ColorCodeJava::Cyan},
      {u8"purple", ColorCodeJava::Purple},
      {u8"blue", ColorCodeJava::Blue},
      {u8"brown", ColorCodeJava::Brown},
      {u8"green", ColorCodeJava::Green},
      {u8"red", ColorCodeJava::Red},
      {u8"black", ColorCodeJava::Black},
  };
  auto found = mapping.find(color);
  if (found == mapping.end()) {
    return ColorCodeJava::White;
  } else {
    return found->second;
  }
}

static inline std::u8string JavaNameFromColorCodeJava(ColorCodeJava code) {
  switch (code) {
  case ColorCodeJava::Black:
    return u8"black";
  case ColorCodeJava::Blue:
    return u8"blue";
  case ColorCodeJava::Brown:
    return u8"brown";
  case ColorCodeJava::Cyan:
    return u8"cyan";
  case ColorCodeJava::Gray:
    return u8"gray";
  case ColorCodeJava::Green:
    return u8"green";
  case ColorCodeJava::LightBlue:
    return u8"light_blue";
  case ColorCodeJava::LightGray:
    return u8"light_gray";
  case ColorCodeJava::Lime:
    return u8"lime";
  case ColorCodeJava::Magenta:
    return u8"magenta";
  case ColorCodeJava::Orange:
    return u8"orange";
  case ColorCodeJava::Pink:
    return u8"pink";
  case ColorCodeJava::Purple:
    return u8"purple";
  case ColorCodeJava::Red:
    return u8"red";
  case ColorCodeJava::Yellow:
    return u8"yellow";
  case ColorCodeJava::White:
  default:
    return u8"white";
  }
}

static inline std::u8string BedrockNameFromColorCodeJava(ColorCodeJava code) {
  switch (code) {
  case ColorCodeJava::Black:
    return u8"black";
  case ColorCodeJava::Blue:
    return u8"blue";
  case ColorCodeJava::Brown:
    return u8"brown";
  case ColorCodeJava::Cyan:
    return u8"cyan";
  case ColorCodeJava::Gray:
    return u8"gray";
  case ColorCodeJava::Green:
    return u8"green";
  case ColorCodeJava::LightBlue:
    return u8"light_blue";
  case ColorCodeJava::LightGray:
    return u8"silver";
  case ColorCodeJava::Lime:
    return u8"lime";
  case ColorCodeJava::Magenta:
    return u8"magenta";
  case ColorCodeJava::Orange:
    return u8"orange";
  case ColorCodeJava::Pink:
    return u8"pink";
  case ColorCodeJava::Purple:
    return u8"purple";
  case ColorCodeJava::Red:
    return u8"red";
  case ColorCodeJava::Yellow:
    return u8"yellow";
  case ColorCodeJava::White:
  default:
    return u8"white";
  }
}

static inline BannerColorCodeBedrock BannerColorCodeFromJava(ColorCodeJava ccj) {
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

static inline mcfile::blocks::BlockId BedBlockIdFromColorCodeJava(ColorCodeJava code) {
  using namespace mcfile::blocks::minecraft;
  switch (code) {
  case ColorCodeJava::Black:
    return black_bed;
  case ColorCodeJava::Blue:
    return blue_bed;
  case ColorCodeJava::Brown:
    return brown_bed;
  case ColorCodeJava::Cyan:
    return cyan_bed;
  case ColorCodeJava::Gray:
    return gray_bed;
  case ColorCodeJava::Green:
    return green_bed;
  case ColorCodeJava::LightBlue:
    return light_blue_bed;
  case ColorCodeJava::LightGray:
    return light_gray_bed;
  case ColorCodeJava::Lime:
    return lime_bed;
  case ColorCodeJava::Magenta:
    return magenta_bed;
  case ColorCodeJava::Orange:
    return orange_bed;
  case ColorCodeJava::Pink:
    return pink_bed;
  case ColorCodeJava::Purple:
    return purple_bed;
  case ColorCodeJava::Red:
    return red_bed;
  case ColorCodeJava::White:
    return white_bed;
  case ColorCodeJava::Yellow:
    return yellow_bed;
  }
}

static inline mcfile::blocks::BlockId BannerBlockIdFromColorCodeJava(ColorCodeJava code) {
  using namespace mcfile::blocks::minecraft;
  switch (code) {
  case ColorCodeJava::Black:
    return black_banner;
  case ColorCodeJava::Blue:
    return blue_banner;
  case ColorCodeJava::Brown:
    return brown_banner;
  case ColorCodeJava::Cyan:
    return cyan_banner;
  case ColorCodeJava::Gray:
    return gray_banner;
  case ColorCodeJava::Green:
    return green_banner;
  case ColorCodeJava::LightBlue:
    return light_blue_banner;
  case ColorCodeJava::LightGray:
    return light_gray_banner;
  case ColorCodeJava::Lime:
    return lime_banner;
  case ColorCodeJava::Magenta:
    return magenta_banner;
  case ColorCodeJava::Orange:
    return orange_banner;
  case ColorCodeJava::Pink:
    return pink_banner;
  case ColorCodeJava::Purple:
    return purple_banner;
  case ColorCodeJava::Red:
    return red_banner;
  case ColorCodeJava::White:
    return white_banner;
  case ColorCodeJava::Yellow:
    return yellow_banner;
  }
}

static inline mcfile::blocks::BlockId WallBannerBlockIdFromColorCodeJava(ColorCodeJava code) {
  using namespace mcfile::blocks::minecraft;
  switch (code) {
  case ColorCodeJava::Black:
    return black_wall_banner;
  case ColorCodeJava::Blue:
    return blue_wall_banner;
  case ColorCodeJava::Brown:
    return brown_wall_banner;
  case ColorCodeJava::Cyan:
    return cyan_wall_banner;
  case ColorCodeJava::Gray:
    return gray_wall_banner;
  case ColorCodeJava::Green:
    return green_wall_banner;
  case ColorCodeJava::LightBlue:
    return light_blue_wall_banner;
  case ColorCodeJava::LightGray:
    return light_gray_wall_banner;
  case ColorCodeJava::Lime:
    return lime_wall_banner;
  case ColorCodeJava::Magenta:
    return magenta_wall_banner;
  case ColorCodeJava::Orange:
    return orange_wall_banner;
  case ColorCodeJava::Pink:
    return pink_wall_banner;
  case ColorCodeJava::Purple:
    return purple_wall_banner;
  case ColorCodeJava::Red:
    return red_wall_banner;
  case ColorCodeJava::White:
    return white_wall_banner;
  case ColorCodeJava::Yellow:
    return yellow_wall_banner;
  }
}

} // namespace je2be
