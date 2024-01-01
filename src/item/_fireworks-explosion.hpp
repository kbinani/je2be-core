#pragma once

#include "color/_lab.hpp"

namespace je2be {

class FireworksExplosion {
public:
  static FireworksExplosion FromJava(CompoundTag const &tag) {
    FireworksExplosion e;
    e.fTrail = tag.boolean(u8"Trail");
    e.fFlicker = tag.boolean(u8"Flicker");
    auto colors = tag.query(u8"Colors")->asIntArray();
    if (colors) {
      for (auto v : colors->value()) {
        u8 r = 0xff & ((*(u32 *)&v) >> 16);
        u8 g = 0xff & ((*(u32 *)&v) >> 8);
        u8 b = 0xff & (*(u32 *)&v);
        e.fColor.emplace_back(r, g, b);
      }
    }
    auto fadeColors = tag.query(u8"FadeColors")->asIntArray();
    if (fadeColors) {
      for (auto v : fadeColors->value()) {
        u8 r = 0xff & ((*(u32 *)&v) >> 16);
        u8 g = 0xff & ((*(u32 *)&v) >> 8);
        u8 b = 0xff & (*(u32 *)&v);
        e.fFadeColor.emplace_back(r, g, b);
      }
    }
    e.fType = tag.byte(u8"Type");
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
    e.fFlicker = tag.boolean(u8"FireworkFlicker");
    e.fTrail = tag.boolean(u8"FireworkTrail");
    auto colorB = tag.byteArrayTag(u8"FireworkColor");
    auto const *table = GetTable();
    if (colorB) {
      for (i8 code : colorB->value()) {
        auto found = table->find(code);
        if (found != table->end()) {
          e.fColor.push_back(found->second);
        }
      }
    }
    auto fade = tag.byteArrayTag(u8"FireworkFade");
    if (fade) {
      for (i8 code : fade->value()) {
        auto found = table->find(code);
        if (found != table->end()) {
          e.fFadeColor.push_back(found->second);
        }
      }
    }
    e.fType = tag.byte(u8"FireworkType");
    return e;
  }

  CompoundTagPtr toBedrockCompoundTag() const {
    auto ret = Compound();
    if (fFlicker) {
      ret->set(u8"FireworkFlicker", Bool(*fFlicker));
    }
    if (fTrail) {
      ret->set(u8"FireworkTrail", Bool(*fTrail));
    }
    if (fType) {
      ret->set(u8"FireworkType", Byte(*fType));
    }
    if (!fColor.empty()) {
      std::vector<u8> colors;
      for (Rgba c : fColor) {
        colors.push_back(GetBedrockColorCode(c));
      }
      ret->set(u8"FireworkColor", std::make_shared<ByteArrayTag>(colors));
    }
    std::vector<u8> fade;
    for (Rgba f : fFadeColor) {
      fade.push_back(GetBedrockColorCode(f));
    }
    ret->set(u8"FireworkFade", std::make_shared<ByteArrayTag>(fade));
    return ret;
  }

  CompoundTagPtr toJavaCompoundTag() const {
    using namespace std;
    auto ret = Compound();
    if (fTrail) {
      ret->set(u8"Trail", Bool(*fTrail));
    }
    if (fFlicker) {
      ret->set(u8"Flicker", Bool(*fFlicker));
    }
    if (fType) {
      ret->set(u8"Type", Byte(*fType));
    }
    if (!fColor.empty()) {
      vector<i32> colors;
      for (auto const &it : fColor) {
        colors.push_back(it.toRGB());
      }
      ret->set(u8"Colors", make_shared<IntArrayTag>(colors));
    }
    if (!fFadeColor.empty()) {
      vector<i32> fade;
      for (auto const &it : fFadeColor) {
        fade.push_back(it.toRGB());
      }
      ret->set(u8"FadeColors", make_shared<IntArrayTag>(fade));
    }
    return ret;
  }

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

  static std::optional<i32> BedrockCustomColorFromColorCode(i8 code) {
    auto const &table = BedrockCustomColorTable();
    if (auto found = table.find(code); found != table.end()) {
      u32 color = u32(0xff000000) | (u32)found->second;
      return *(i32 *)&color;
    } else {
      return std::nullopt;
    }
  }

private:
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

  static std::unordered_map<i8, i32> *CreateBedrockCustomColorTable() {
    return new std::unordered_map<i8, i32>({
        {0, 0x1d1d21},  // black
        {1, 0xb02e26},  // red
        {2, 0x5e7c16},  // green
        {3, 0x835432},  // brown
        {4, 0x3c44aa},  // blue
        {5, 0x8932b8},  // purple
        {6, 0x169c9c},  // cyan
        {7, 0x9d9d97},  // light gray
        {8, 0x474f52},  // gray
        {9, 0xf38baa},  // pink
        {10, 0x80c71f}, // lime
        {11, 0xfed83d}, // yellow
        {12, 0x3ab3da}, // light blue
        {13, 0xc74ebd}, // magenta
        {14, 0xf9801d}, // orange
        {15, 0xf0f0f0}, // white
    });
  }

  static std::unordered_map<i8, i32> const &BedrockCustomColorTable() {
    static std::unique_ptr<std::unordered_map<i8, i32> const> const sTable(CreateBedrockCustomColorTable());
    return *sTable;
  }

  static i8 MostSimilarColor(Rgba rgb) {
    struct Color {
      Rgba fColor;
      i8 fCode;
    };
    std::vector<Color> colors;
    auto const *table = GetTable();
    for (auto const &it : *table) {
      colors.emplace_back(it.second, it.first);
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
