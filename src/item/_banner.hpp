#pragma once

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

  static ListTagPtr OminousBannerPatterns() {
    auto p = List<Tag::Type::Compound>();
    p->push_back(BannerPattern(u8"cyan", u8"rhombus"));
    p->push_back(BannerPattern(u8"light_gray", u8"stripe_bottom"));
    p->push_back(BannerPattern(u8"gray", u8"stripe_center"));
    p->push_back(BannerPattern(u8"light_gray", u8"border"));
    p->push_back(BannerPattern(u8"black", u8"stripe_middle"));
    p->push_back(BannerPattern(u8"light_gray", u8"half_horizontal"));
    p->push_back(BannerPattern(u8"light_gray", u8"circle"));
    p->push_back(BannerPattern(u8"black", u8"border"));
    return p;
  }
};

} // namespace je2be
