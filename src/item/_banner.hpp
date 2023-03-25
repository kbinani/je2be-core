#pragma once

namespace je2be {

class Banner {
  Banner() = delete;

public:
  static CompoundTagPtr BannerPattern(i32 color, std::u8string const &pattern) {
    auto c = Compound();
    c->set(u8"Color", Int(color));
    c->set(u8"Pattern", String(pattern));
    return c;
  }

  static ListTagPtr OminousBannerPatterns() {
    auto p = List<Tag::Type::Compound>();
    p->push_back(BannerPattern(9, u8"mr"));
    p->push_back(BannerPattern(8, u8"bs"));
    p->push_back(BannerPattern(7, u8"cs"));
    p->push_back(BannerPattern(8, u8"bo"));
    p->push_back(BannerPattern(15, u8"ms"));
    p->push_back(BannerPattern(8, u8"hh"));
    p->push_back(BannerPattern(8, u8"mc"));
    p->push_back(BannerPattern(15, u8"bo"));
    return p;
  }
};

} // namespace je2be
