#pragma once

namespace je2be {

enum class BannerColorCodeBedrock : int32_t {
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

static inline BannerColorCodeBedrock BannerColorCodeFromName(std::string const &color) {
  static std::unordered_map<std::string, BannerColorCodeBedrock> const mapping = {
      {"white", BannerColorCodeBedrock::White},
      {"orange", BannerColorCodeBedrock::Orange},
      {"magenta", BannerColorCodeBedrock::Magenta},
      {"light_blue", BannerColorCodeBedrock::LightBlue},
      {"yellow", BannerColorCodeBedrock::Yellow},
      {"lime", BannerColorCodeBedrock::Lime},
      {"pink", BannerColorCodeBedrock::Pink},
      {"gray", BannerColorCodeBedrock::Gray},
      {"light_gray", BannerColorCodeBedrock::LightGray},
      {"cyan", BannerColorCodeBedrock::Cyan},
      {"purple", BannerColorCodeBedrock::Purple},
      {"blue", BannerColorCodeBedrock::Blue},
      {"brown", BannerColorCodeBedrock::Brown},
      {"green", BannerColorCodeBedrock::Green},
      {"red", BannerColorCodeBedrock::Red},
      {"black", BannerColorCodeBedrock::Black},
  };
  auto found = mapping.find(color);
  if (found == mapping.end()) {
    return BannerColorCodeBedrock::White;
  } else {
    return found->second;
  }
}

} // namespace je2be
