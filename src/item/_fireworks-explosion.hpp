#pragma once

#include "color/_lab.hpp"

namespace je2be {

class FireworksExplosion {
public:
  static FireworksExplosion FromJava(CompoundTag const &tag) {
    FireworksExplosion e;
    e.fTrail = tag.boolean("Trail");
    e.fFlicker = tag.boolean("Flicker");
    auto colors = tag.query("Colors")->asIntArray();
    if (colors) {
      for (auto v : colors->value()) {
        u8 r = 0xff & ((*(u32 *)&v) >> 16);
        u8 g = 0xff & ((*(u32 *)&v) >> 8);
        u8 b = 0xff & (*(u32 *)&v);
        e.fColor.push_back(Rgba(r, g, b));
      }
    }
    auto fadeColors = tag.query("FadeColors")->asIntArray();
    if (fadeColors) {
      for (auto v : fadeColors->value()) {
        u8 r = 0xff & ((*(u32 *)&v) >> 16);
        u8 g = 0xff & ((*(u32 *)&v) >> 8);
        u8 b = 0xff & (*(u32 *)&v);
        e.fFadeColor.push_back(Rgba(r, g, b));
      }
    }
    e.fType = tag.byte("Type");
    return e;

    // Java
    // Fireworks
    //   Explosions
    //     Type
    //       1: large (firecharge)
    //       2: star (gold nugget)
    //       3: creeper (skull)
    //       4: explosion (feather)
    //    Flicker: true (glowstone dust)
    //    Trail: true (diamond)
    //    Colors: IntArray (RGB)
    //    FadeColors: IntArray (RGB)
  }

  static FireworksExplosion FromBedrock(CompoundTag const &tag) {
    FireworksExplosion e;
    e.fFlicker = tag.boolean("FireworkFlicker");
    e.fTrail = tag.boolean("FireworkTrail");
    auto colorB = tag.byteArrayTag("FireworkColor");
    auto const *table = GetTable();
    if (colorB) {
      for (i8 code : colorB->value()) {
        auto found = table->find(code);
        if (found != table->end()) {
          e.fColor.push_back(found->second);
        }
      }
    }
    auto fade = tag.byteArrayTag("FireworkFade");
    if (fade) {
      for (i8 code : fade->value()) {
        auto found = table->find(code);
        if (found != table->end()) {
          e.fFadeColor.push_back(found->second);
        }
      }
    }
    e.fType = tag.byte("FireworkType");
    return e;
  }

  CompoundTagPtr toBedrockCompoundTag() const {
    auto ret = Compound();
    if (fFlicker) {
      ret->set("FireworkFlicker", Bool(*fFlicker));
    }
    if (fTrail) {
      ret->set("FireworkTrail", Bool(*fTrail));
    }
    if (fType) {
      ret->set("FireworkType", Byte(*fType));
    }
    if (!fColor.empty()) {
      std::vector<u8> colors;
      for (Rgba c : fColor) {
        colors.push_back(GetBedrockColorCode(c));
      }
      ret->set("FireworkColor", std::make_shared<ByteArrayTag>(colors));
    }
    if (!fFadeColor.empty()) {
      std::vector<u8> fade;
      for (Rgba f : fFadeColor) {
        fade.push_back(GetBedrockColorCode(f));
      }
      ret->set("FireworkFade", std::make_shared<ByteArrayTag>(fade));
    }
    return ret;
  }

  CompoundTagPtr toJavaCompoundTag() const {
    using namespace std;
    auto ret = Compound();
    if (fTrail) {
      ret->set("Trail", Bool(*fTrail));
    }
    if (fFlicker) {
      ret->set("Flicker", Bool(*fFlicker));
    }
    if (fType) {
      ret->set("Type", Byte(*fType));
    }
    if (!fColor.empty()) {
      vector<i32> colors;
      for (auto const &it : fColor) {
        colors.push_back(it.toRGB());
      }
      ret->set("Colors", make_shared<IntArrayTag>(colors));
    }
    if (!fFadeColor.empty()) {
      vector<i32> fade;
      for (auto const &it : fFadeColor) {
        fade.push_back(it.toRGB());
      }
      ret->set("FadeColors", make_shared<IntArrayTag>(fade));
    }
    return ret;
  }

private:
  static i8 GetBedrockColorCode(Rgba rgb) {
    auto const *table = GetTable();
    i32 code = rgb.toRGB();
    for (auto const &it : *table) {
      if (it.second.toRGB() == code) {
        return it.first;
      }
    }
    return MostSimilarColor(rgb);
  }

  static std::unordered_map<i8, Rgba> const *GetTable() {
    static std::unique_ptr<std::unordered_map<i8, Rgba> const> const sTable(CreateTable());
    return sTable.get();
  }

  static std::unordered_map<i8, Rgba> *CreateTable() {
    return new std::unordered_map<i8, Rgba>({
        {0, Rgba::FromRGB(1973019)},   // black
        {1, Rgba::FromRGB(11743532)},  // red
        {2, Rgba::FromRGB(3887386)},   // green
        {3, Rgba::FromRGB(5320730)},   // brown
        {4, Rgba::FromRGB(2437522)},   // blue
        {5, Rgba::FromRGB(8073150)},   // purple
        {6, Rgba::FromRGB(2651799)},   // cyan
        {7, Rgba::FromRGB(11250603)},  // light gray
        {8, Rgba::FromRGB(4408131)},   // gray
        {9, Rgba::FromRGB(14188952)},  // pink
        {10, Rgba::FromRGB(4312372)},  // lime
        {11, Rgba::FromRGB(14602026)}, // yellow
        {12, Rgba::FromRGB(6719955)},  // light blue
        {13, Rgba::FromRGB(12801229)}, // magenta
        {14, Rgba::FromRGB(15435844)}, // orange
        {15, Rgba::FromRGB(15790320)}, // white
    });
  }

  static i8 MostSimilarColor(Rgba rgb) {
    struct Color {
      Rgba fColor;
      i8 fCode;
    };
    std::vector<Color> colors;
    auto const *table = GetTable();
    for (auto const &it : *table) {
      colors.push_back({it.second, it.first});
    }
    std::sort(colors.begin(), colors.end(), [rgb](Color const &lhs, Color const &rhs) { return Lab::CompareBySimirality(rgb)(lhs.fColor, rhs.fColor); });
    auto const &ret = colors[0];
    return ret.fCode;
  }

public:
  std::optional<bool> fFlicker;
  std::optional<bool> fTrail;
  std::optional<i8> fType;
  std::vector<Rgba> fColor;
  std::vector<Rgba> fFadeColor;
};

} // namespace je2be
