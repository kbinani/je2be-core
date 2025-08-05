#pragma once

#include "color/_lab.hpp"

namespace je2be {

class FireworksExplosion {
public:
  enum class Shape {
    SmallBall, // default
    Star,      // gold nuget
    Burst,     // feather
    LargeBall, // firecharge
    Creeper,   // skull
  };

  static FireworksExplosion FromJava(CompoundTag const &tag) {
    FireworksExplosion e;
    e.fHasTrail = FallbackValue<bool>(tag, {u8"has_trail", u8"Trail"});
    e.fHasTwinkle = FallbackValue<bool>(tag, {u8"has_twinkle", u8"Flicker"});
    auto colorsArray = FallbackPtr<IntArrayTag>(tag, {u8"colors", u8"Colors"});
    if (colorsArray) {
      for (auto v : colorsArray->value()) {
        u8 r = 0xff & ((*(u32 *)&v) >> 16);
        u8 g = 0xff & ((*(u32 *)&v) >> 8);
        u8 b = 0xff & (*(u32 *)&v);
        e.fColors.emplace_back(r, g, b);
      }
    } else if (auto colorsList = tag.listTag(u8"colors"); colorsList) {
      // 1.20.4 tag IntArray
      // 1.20.5 component IntArray
      // 1.21.4 component IntArray
      // 25w02a component IntArray
      // 25w03a component IntArray
      // 25w04a component List
      // 25w07a component List
      // 1.21.5 component List
      // 1.21.6 component List
      // 1.21.8 component List
      for (auto v : colorsList->fValue) {
        if (auto i = v->asInt(); i) {
          int32_t iv = i->fValue;
          u8 r = 0xff & ((*(u32 *)&iv) >> 16);
          u8 g = 0xff & ((*(u32 *)&iv) >> 8);
          u8 b = 0xff & (*(u32 *)&iv);
          e.fColors.emplace_back(r, g, b);
        } else {
          e.fColors.clear();
          break;
        }
      }
    }
    auto fadeColors = FallbackPtr<IntArrayTag>(tag, {u8"fade_colors", u8"FadeColors"});
    if (fadeColors) {
      for (auto v : fadeColors->value()) {
        u8 r = 0xff & ((*(u32 *)&v) >> 16);
        u8 g = 0xff & ((*(u32 *)&v) >> 8);
        u8 b = 0xff & (*(u32 *)&v);
        e.fFadeColors.emplace_back(r, g, b);
      }
    }
    auto const &table = ShapeTable();
    if (auto shape = tag.string(u8"shape"); shape) {
      for (auto const &it : table) {
        if (it.second.first == *shape) {
          e.fShape = it.first;
          break;
        }
      }
    } else if (auto legacyType = tag.byte(u8"Type"); legacyType) {
      for (auto const &it : table) {
        if (it.second.second == *legacyType) {
          e.fShape = it.first;
          break;
        }
      }
    }
    return e;
  }

  static FireworksExplosion FromBedrock(CompoundTag const &tag) {
    FireworksExplosion e;
    e.fHasTwinkle = tag.boolean(u8"FireworkFlicker");
    e.fHasTrail = tag.boolean(u8"FireworkTrail");
    auto colorB = tag.byteArrayTag(u8"FireworkColor");
    auto const *table = GetTable();
    if (colorB) {
      for (i8 code : colorB->value()) {
        auto found = table->find(code);
        if (found != table->end()) {
          e.fColors.push_back(found->second);
        }
      }
    }
    auto fade = tag.byteArrayTag(u8"FireworkFade");
    if (fade) {
      for (i8 code : fade->value()) {
        auto found = table->find(code);
        if (found != table->end()) {
          e.fFadeColors.push_back(found->second);
        }
      }
    }
    auto fireworkTypeB = tag.byte(u8"FireworkType");
    for (auto const &it : ShapeTable()) {
      if (it.second.second == *fireworkTypeB) {
        e.fShape = it.first;
        break;
      }
    }
    return e;
  }

  CompoundTagPtr toBedrockCompoundTag() const {
    auto ret = Compound();
    if (fHasTwinkle) {
      ret->set(u8"FireworkFlicker", Bool(*fHasTwinkle));
    }
    if (fHasTrail) {
      ret->set(u8"FireworkTrail", Bool(*fHasTrail));
    }
    if (fShape) {
      auto const &table = ShapeTable();
      if (auto found = table.find(*fShape); found != table.end()) {
        ret->set(u8"FireworkType", Byte(found->second.second));
      }
    }
    if (!fColors.empty()) {
      std::vector<u8> colors;
      for (Rgba c : fColors) {
        colors.push_back(GetBedrockColorCode(c));
      }
      ret->set(u8"FireworkColor", std::make_shared<ByteArrayTag>(colors));
    }
    std::vector<u8> fade;
    for (Rgba f : fFadeColors) {
      fade.push_back(GetBedrockColorCode(f));
    }
    ret->set(u8"FireworkFade", std::make_shared<ByteArrayTag>(fade));
    return ret;
  }

  CompoundTagPtr toJavaCompoundTag(int targetDataVersion) const {
    using namespace std;
    auto ret = Compound();
    if (fHasTrail) {
      ret->set(u8"has_trail", Bool(*fHasTrail)); // this was "Trail"
    }
    if (fHasTwinkle) {
      ret->set(u8"has_twinkle", Bool(*fHasTwinkle)); // this was "Flicker"
    }
    if (fShape) {
      auto const &table = ShapeTable();
      if (auto found = table.find(*fShape); found != table.end()) {
        ret->set(u8"shape", String(found->second.first));
      }
    }
    if (!fColors.empty()) {
      if (targetDataVersion >= 4308) {
        // 25w04a
        auto colorsList = List<Tag::Type::Int>();
        for (auto const &it : fColors) {
          colorsList->push_back(Int(it.toRGB()));
        }
        ret->set(u8"colors", colorsList);
      } else {
        vector<i32> colors;
        for (auto const &it : fColors) {
          colors.push_back(it.toRGB());
        }
        ret->set(u8"colors", make_shared<IntArrayTag>(colors));
      }
    }
    if (!fFadeColors.empty()) {
      vector<i32> fade;
      for (auto const &it : fFadeColors) {
        fade.push_back(it.toRGB());
      }
      ret->set(u8"fade_colors", make_shared<IntArrayTag>(fade));
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
      Color(Rgba color, i8 code) : fColor(color), fCode(code) {}
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

  static std::unordered_map<Shape, std::pair<std::u8string, i8>> const *CreateShapeTable() {
    return new std::unordered_map<Shape, std::pair<std::u8string, i8>>({
        {Shape::SmallBall, {u8"small_ball", 0}},
        {Shape::LargeBall, {u8"large_ball", 1}},
        {Shape::Star, {u8"star", 2}},
        {Shape::Creeper, {u8"creeper", 3}},
        {Shape::Burst, {u8"burst", 4}},
    });
  }

  static std::unordered_map<Shape, std::pair<std::u8string, i8>> const &ShapeTable() {
    static std::unique_ptr<std::unordered_map<Shape, std::pair<std::u8string, i8>> const> sTable(CreateShapeTable());
    return *sTable;
  }

public:
  std::optional<bool> fHasTwinkle; // glowstone dust
  std::optional<bool> fHasTrail;   // diamond
  std::optional<Shape> fShape;
  std::vector<Rgba> fColors;
  std::vector<Rgba> fFadeColors;
};

} // namespace je2be
