#pragma once

namespace je2be {

class Banner {
  Banner() = delete;

public:
  static std::shared_ptr<CompoundTag> BannerPattern(int32_t color, std::string const &pattern) {
    auto c = nbt::Compound();
    c->set("Color", nbt::Int(color));
    c->set("Pattern", nbt::String(pattern));
    return c;
  }

  static std::shared_ptr<ListTag> OminousBannerPatterns() {
    auto p = std::make_shared<ListTag>(Tag::Type::Compound);
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
