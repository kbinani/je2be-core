#pragma once

namespace je2be {

class Banner {
  Banner() = delete;

public:
  static CompoundTagPtr BannerPattern(i32 color, std::string const &pattern) {
    auto c = Compound();
    c->set("Color", Int(color));
    c->set("Pattern", String(pattern));
    return c;
  }

  static ListTagPtr OminousBannerPatterns() {
    auto p = List<Tag::Type::Compound>();
    p->push_back(BannerPattern(9, "mr"));
    p->push_back(BannerPattern(8, "bs"));
    p->push_back(BannerPattern(7, "cs"));
    p->push_back(BannerPattern(8, "bo"));
    p->push_back(BannerPattern(15, "ms"));
    p->push_back(BannerPattern(8, "hh"));
    p->push_back(BannerPattern(8, "mc"));
    p->push_back(BannerPattern(15, "bo"));
    return p;
  }
};

} // namespace je2be
