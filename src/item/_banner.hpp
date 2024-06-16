#pragma once

#include "_data-version.hpp"
#include "_reversible-map.hpp"
#include "bedrock/_constants.hpp"

namespace je2be {

class Banner {
  Banner() = delete;

  static ReversibleMap<std::u8string, std::u8string> const &PatternTable() {
    static std::unique_ptr<ReversibleMap<std::u8string, std::u8string> const> sTable(CreatePatternTable());
    return *sTable;
  }

  static ReversibleMap<std::u8string, std::u8string> *CreatePatternTable() {
    return new ReversibleMap<std::u8string, std::u8string>({
        // {java, bedrock(=legacy java prior to 1.20.5)}
        {u8"border", u8"bo"},
        {u8"circle", u8"mc"},
        {u8"cross", u8"cr"},
        {u8"diagonal_left", u8"ld"},
        {u8"diagonal_right", u8"rud"},
        {u8"diagonal_up_left", u8"lud"},
        {u8"diagonal_up_right", u8"rd"},
        {u8"half_vertical_right", u8"vhr"},
        {u8"half_vertical", u8"vh"},
        {u8"half_horizontal_bottom", u8"hhb"},
        {u8"half_horizontal", u8"hh"},
        {u8"gradient_up", u8"gru"},
        {u8"gradient", u8"gra"},
        {u8"small_stripes", u8"ss"},
        {u8"square_bottom_left", u8"bl"},
        {u8"square_bottom_right", u8"br"},
        {u8"square_top_left", u8"tl"},
        {u8"square_top_right", u8"tr"},
        {u8"straight_cross", u8"sc"},
        {u8"stripe_bottom", u8"bs"},
        {u8"stripe_top", u8"ts"},
        {u8"stripe_right", u8"rs"},
        {u8"stripe_middle", u8"ms"},
        {u8"stripe_left", u8"ls"},
        {u8"stripe_downright", u8"drs"},
        {u8"stripe_downleft", u8"dls"},
        {u8"stripe_center", u8"cs"},
        {u8"triangle_bottom", u8"bt"},
        {u8"triangle_top", u8"tt"},
        {u8"triangles_bottom", u8"bts"},
        {u8"triangles_top", u8"tts"},
        {u8"creeper", u8"cre"},
        {u8"skull", u8"sku"},
        {u8"flower", u8"flo"},
        {u8"mojang", u8"moj"},
        {u8"bricks", u8"bri"},
        {u8"curly_border", u8"cbo"},
        {u8"piglin", u8"pig"},
        {u8"globe", u8"glb"},
        {u8"rhombus", u8"mr"},
        {u8"guster", u8"gus"},
        {u8"flow", u8"flw"},
    });
  }

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
    if (dataVersion >= kJavaDataVersionComponentIntroduced) {
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

  static std::optional<std::u8string> JavaPatternFromBedrockOrLegacyJava(std::u8string const &p) {
    if (auto v = PatternTable().backward(Namespace::Remove(p)); v) {
      return Namespace::Add(*v);
    } else {
      return Namespace::Add(p);
    }
  }

  static std::optional<std::u8string> BedrockOrLegacyJavaPatternFromJava(std::u8string const &p) {
    return PatternTable().forward(Namespace::Remove(p));
  }
};

} // namespace je2be
