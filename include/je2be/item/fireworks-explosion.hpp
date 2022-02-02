#pragma once

namespace je2be {

class FireworksExplosion {
public:
  static FireworksExplosion BedrockFromJava(mcfile::nbt::CompoundTag const &tag) {
    FireworksExplosion e;
    e.fTrail = tag.boolean("Trail", false);
    e.fFlicker = tag.boolean("Flicker", false);
    auto colors = tag.query("Colors")->asIntArray();
    if (colors) {
      for (auto v : colors->value()) {
        uint8_t r = 0xff & ((*(uint32_t *)&v) >> 16);
        uint8_t g = 0xff & ((*(uint32_t *)&v) >> 8);
        uint8_t b = 0xff & (*(uint32_t *)&v);
        e.fColor.push_back(Rgba(r, g, b));
      }
    }
    auto fadeColors = tag.query("FadeColors")->asIntArray();
    if (fadeColors) {
      for (auto v : fadeColors->value()) {
        uint8_t r = 0xff & ((*(uint32_t *)&v) >> 16);
        uint8_t g = 0xff & ((*(uint32_t *)&v) >> 8);
        uint8_t b = 0xff & (*(uint32_t *)&v);
        e.fFadeColor.push_back(Rgba(r, g, b));
      }
    }
    e.fType = tag.byte("Type", 0);
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

  std::shared_ptr<mcfile::nbt::CompoundTag> toBedrockCompoundTag() const {
    using namespace props;
    using namespace mcfile::nbt;
    auto ret = std::make_shared<CompoundTag>();
    std::vector<uint8_t> colors;
    for (Rgba c : fColor) {
      colors.push_back(GetBedrockColorCode(c));
    }
    std::vector<uint8_t> fade;
    for (Rgba f : fFadeColor) {
      fade.push_back(GetBedrockColorCode(f));
    }
    ret->insert({
        {"FireworkFlicker", Bool(fFlicker)},
        {"FireworkTrail", Bool(fTrail)},
        {"FireworkType", Byte(fType)},
        {"FireworkColor", std::make_shared<ByteArrayTag>(colors)},
        {"FireworkFade", std::make_shared<ByteArrayTag>(fade)},
    });
    return ret;
  }

private:
  static int8_t GetBedrockColorCode(Rgba rgb) {
    switch (rgb.toRGB()) {
    case 15790320:
      return 15; // white
    case 15435844:
      return 14; // orange
    case 12801229:
      return 13; // magenta
    case 6719955:
      return 12; // light blue
    case 14602026:
      return 11; // yellow
    case 4312372:
      return 10; // lime
    case 14188952:
      return 9; // pink
    case 4408131:
      return 8; // gray
    case 11250603:
      return 7; // light gray
    case 2651799:
      return 6; // cyan
    case 8073150:
      return 5; // purple
    case 2437522:
      return 4; // blue
    case 5320730:
      return 3; // brown
    case 3887386:
      return 2; // green
    case 11743532:
      return 1; // red
    case 1973019:
      return 0; // black
    }
    return MostSimilarColor(rgb);
  }

  static int8_t MostSimilarColor(Rgba rgb) {
    struct Color {
      Rgba fColor;
      int8_t fCode;
    };
    std::vector<Color> colors = {
        {Rgba::FromRGB(1973019), 0},   // black
        {Rgba::FromRGB(11743532), 1},  // red
        {Rgba::FromRGB(3887386), 2},   // green
        {Rgba::FromRGB(5320730), 3},   // brown
        {Rgba::FromRGB(2437522), 4},   // blue
        {Rgba::FromRGB(8073150), 5},   // purple
        {Rgba::FromRGB(2651799), 6},   // cyan
        {Rgba::FromRGB(11250603), 7},  // light gray
        {Rgba::FromRGB(4408131), 8},   // gray
        {Rgba::FromRGB(14188952), 9},  // pink
        {Rgba::FromRGB(4312372), 10},  // lime
        {Rgba::FromRGB(14602026), 11}, // yellow
        {Rgba::FromRGB(6719955), 12},  // light blue
        {Rgba::FromRGB(12801229), 13}, // magenta
        {Rgba::FromRGB(15435844), 14}, // orange
        {Rgba::FromRGB(15790320), 15}, // white
    };
    std::sort(colors.begin(), colors.end(), [rgb](Color const &lhs, Color const &rhs) { return Lab::CompareBySimirality(rgb)(lhs.fColor, rhs.fColor); });
    auto const &ret = colors[0];
    return ret.fCode;
  }

public:
  bool fFlicker = false;
  bool fTrail = false;
  int8_t fType;
  std::vector<Rgba> fColor;
  std::vector<Rgba> fFadeColor;
};

} // namespace je2be
