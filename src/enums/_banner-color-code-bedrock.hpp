#pragma once

#include <je2be/integers.hpp>

#include <cstdint>
#include <string>
#include <unordered_map>

namespace je2be {

enum class BannerColorCodeBedrock : i32 {
  Red = 1,
  Black = 0,
  Green = 2,
  Brown = 3,
  Blue = 4,
  Purple = 5,
  Cyan = 6,
  LightGray = 7,
  Gray = 8,
  Pink = 9,
  Lime = 10,
  Yellow = 11,
  LightBlue = 12,
  Magenta = 13,
  Orange = 14,
  White = 15,
};

static inline BannerColorCodeBedrock BannerColorCodeFromName(std::u8string const &color) {
  static std::unordered_map<std::u8string, BannerColorCodeBedrock> const mapping = {
      {u8"white", BannerColorCodeBedrock::White},
      {u8"orange", BannerColorCodeBedrock::Orange},
      {u8"magenta", BannerColorCodeBedrock::Magenta},
      {u8"light_blue", BannerColorCodeBedrock::LightBlue},
      {u8"yellow", BannerColorCodeBedrock::Yellow},
      {u8"lime", BannerColorCodeBedrock::Lime},
      {u8"pink", BannerColorCodeBedrock::Pink},
      {u8"gray", BannerColorCodeBedrock::Gray},
      {u8"light_gray", BannerColorCodeBedrock::LightGray},
      {u8"cyan", BannerColorCodeBedrock::Cyan},
      {u8"purple", BannerColorCodeBedrock::Purple},
      {u8"blue", BannerColorCodeBedrock::Blue},
      {u8"brown", BannerColorCodeBedrock::Brown},
      {u8"green", BannerColorCodeBedrock::Green},
      {u8"red", BannerColorCodeBedrock::Red},
      {u8"black", BannerColorCodeBedrock::Black},
  };
  auto found = mapping.find(color);
  if (found == mapping.end()) {
    return BannerColorCodeBedrock::White;
  } else {
    return found->second;
  }
}

} // namespace je2be
