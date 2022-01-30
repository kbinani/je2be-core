#pragma once

namespace je2be {

class SignColor {
private:
  SignColor() = delete;

public:
  static Rgba constexpr kWhite = Rgba(240, 240, 240);
  static Rgba constexpr kOrange = Rgba(249, 128, 29);
  static Rgba constexpr kMagenta = Rgba(199, 78, 189);
  static Rgba constexpr kLightBlue = Rgba(58, 179, 218);
  static Rgba constexpr kYellow = Rgba(254, 216, 61);
  static Rgba constexpr kLime = Rgba(128, 199, 31);
  static Rgba constexpr kPink = Rgba(243, 139, 170);
  static Rgba constexpr kGray = Rgba(71, 79, 82);
  static Rgba constexpr kLightGray = Rgba(157, 157, 151);
  static Rgba constexpr kCyan = Rgba(22, 156, 156);
  static Rgba constexpr kPurple = Rgba(137, 50, 184);
  static Rgba constexpr kBlue = Rgba(60, 68, 170);
  static Rgba constexpr kBrown = Rgba(131, 84, 50);
  static Rgba constexpr kGreen = Rgba(94, 124, 22);
  static Rgba constexpr kRed = Rgba(176, 46, 38);
  static Rgba constexpr kBlack = Rgba(0, 0, 0);

  static Rgba BedrockTexteColorFromJavaColorCode(ColorCodeJava code) {
    switch (code) {
    case ColorCodeJava::White:
      return kWhite;
    case ColorCodeJava::Orange:
      return kOrange;
    case ColorCodeJava::Magenta:
      return kMagenta;
    case ColorCodeJava::LightBlue:
      return kLightBlue;
    case ColorCodeJava::Yellow:
      return kYellow;
    case ColorCodeJava::Lime:
      return kLime;
    case ColorCodeJava::Pink:
      return kPink;
    case ColorCodeJava::Gray:
      return kGray;
    case ColorCodeJava::LightGray:
      return kLightGray;
    case ColorCodeJava::Cyan:
      return kCyan;
    case ColorCodeJava::Purple:
      return kPurple;
    case ColorCodeJava::Blue:
      return kBlue;
    case ColorCodeJava::Brown:
      return kBrown;
    case ColorCodeJava::Green:
      return kGreen;
    case ColorCodeJava::Red:
      return kRed;
    case ColorCodeJava::Black:
    default:
      return kBlack;
    }
  }

  static ColorCodeJava MostSimilarColor(Rgba c) {
    using namespace std;
    vector<pair<ColorCodeJava, Rgba>> colors{
        {ColorCodeJava::White, kWhite},
        {ColorCodeJava::Orange, kOrange},
        {ColorCodeJava::Magenta, kMagenta},
        {ColorCodeJava::LightBlue, kLightBlue},
        {ColorCodeJava::Yellow, kYellow},
        {ColorCodeJava::Lime, kLime},
        {ColorCodeJava::Pink, kPink},
        {ColorCodeJava::Gray, kGray},
        {ColorCodeJava::LightGray, kLightGray},
        {ColorCodeJava::Cyan, kCyan},
        {ColorCodeJava::Purple, kPurple},
        {ColorCodeJava::Blue, kBlue},
        {ColorCodeJava::Brown, kBrown},
        {ColorCodeJava::Green, kGreen},
        {ColorCodeJava::Red, kRed},
        {ColorCodeJava::Black, kBlack}};
    sort(colors.begin(), colors.end(), [c](auto const &lhs, auto const &rhs) {
      return Lab::CompareBySimirality(c)(lhs.second, rhs.second);
    });
    return colors[0].first;
  }
};

} // namespace je2be
