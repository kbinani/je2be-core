#pragma once

#include "bedrock/_constants.hpp"

namespace je2be {

class Banner {
  Banner() = delete;

public:
  static CompoundTagPtr BannerPattern(std::u8string const &color, std::u8string const &pattern) {
    auto c = Compound();
    c->set(u8"color", String(color));
    c->set(u8"pattern", u8"minecraft:" + pattern);
    return c;
  }

  static CompoundTagPtr BannerPattern(i32 color, std::u8string const &pattern) {
    auto c = Compound();
    c->set(u8"Color", Int(color));
    c->set(u8"Pattern", pattern);
    return c;
  }

  static ListTagPtr OminousBannerPatterns(int dataVersion) {
    auto p = List<Tag::Type::Compound>();
    if (dataVersion >= bedrock::kDataVersionComponentIntroduced) {
      p->push_back(BannerPattern(u8"cyan", u8"rhombus"));
      p->push_back(BannerPattern(u8"light_gray", u8"stripe_bottom"));
      p->push_back(BannerPattern(u8"gray", u8"stripe_center"));
      p->push_back(BannerPattern(u8"light_gray", u8"border"));
      p->push_back(BannerPattern(u8"black", u8"stripe_middle"));
      p->push_back(BannerPattern(u8"light_gray", u8"half_horizontal"));
      p->push_back(BannerPattern(u8"light_gray", u8"circle"));
      p->push_back(BannerPattern(u8"black", u8"border"));
    } else {
      p->push_back(BannerPattern(9, u8"mr"));
      p->push_back(BannerPattern(8, u8"bs"));
      p->push_back(BannerPattern(7, u8"cs"));
      p->push_back(BannerPattern(8, u8"bo"));
      p->push_back(BannerPattern(15, u8"ms"));
      p->push_back(BannerPattern(8, u8"hh"));
      p->push_back(BannerPattern(8, u8"mc"));
      p->push_back(BannerPattern(15, u8"bo"));
    }
    return p;
  }
};

} // namespace je2be
