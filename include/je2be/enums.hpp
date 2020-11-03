#pragma once

namespace j2b {

enum class Dimension : uint8_t {
    Overworld = 0,
    Nether = 1,
    End = 2,
};

enum class LevelDirectoryStructure {
    Vanilla,
    Paper,
};

enum class BannerColorCodeBedrock : int32_t {
    Red = 1,
    Black = 0,
    Green = 2,
    Brown = 3,
    Blue = 4,
    Purple = 5,
    Cyan = 6,
    LightGray = 7,
    Gray = 8,
    Pink = 9,
    Lime = 10,
    Yellow = 11,
    LightBlue = 12,
    Magenta = 13,
    Orange = 14,
    White = 15,
};

inline BannerColorCodeBedrock BannerColorCodeFromName(std::string const& color) {
    static std::unordered_map<std::string, BannerColorCodeBedrock> const mapping = {
        {"white", BannerColorCodeBedrock::White},
        {"orange", BannerColorCodeBedrock::Orange},
        {"magenta", BannerColorCodeBedrock::Magenta},
        {"light_blue", BannerColorCodeBedrock::LightBlue},
        {"yellow", BannerColorCodeBedrock::Yellow},
        {"lime", BannerColorCodeBedrock::Lime},
        {"pink", BannerColorCodeBedrock::Pink},
        {"gray", BannerColorCodeBedrock::Gray},
        {"light_gray", BannerColorCodeBedrock::LightGray},
        {"cyan", BannerColorCodeBedrock::Cyan},
        {"purple", BannerColorCodeBedrock::Purple},
        {"blue", BannerColorCodeBedrock::Blue},
        {"brown", BannerColorCodeBedrock::Brown},
        {"green", BannerColorCodeBedrock::Green},
        {"red", BannerColorCodeBedrock::Red},
        {"black", BannerColorCodeBedrock::Black},
    };
    auto found = mapping.find(color);
    if (found == mapping.end()) {
        return BannerColorCodeBedrock::White;
    } else {
        return found->second;
    }
}

enum class ColorCodeJava : int32_t {
    White = 0,
    Orange = 1,
    Magenta = 2,
    LightBlue = 3,
    Yellow = 4,
    Lime = 5,
    Pink = 6,
    Gray = 7,
    LightGray = 8,
    Cyan = 9,
    Purple = 10,
    Blue = 11,
    Brown = 12,
    Green = 13,
    Red = 14,
    Black = 15,
};

inline ColorCodeJava ColorCodeJavaFromName(std::string const& color) {
    static std::unordered_map<std::string, ColorCodeJava> const mapping = {
        {"white", ColorCodeJava::White},
        {"orange", ColorCodeJava::Orange},
        {"magenta", ColorCodeJava::Magenta},
        {"light_blue", ColorCodeJava::LightBlue},
        {"yellow", ColorCodeJava::Yellow},
        {"lime", ColorCodeJava::Lime},
        {"pink", ColorCodeJava::Pink},
        {"gray", ColorCodeJava::Gray},
        {"light_gray", ColorCodeJava::LightGray},
        {"cyan", ColorCodeJava::Cyan},
        {"purple", ColorCodeJava::Purple},
        {"blue", ColorCodeJava::Blue},
        {"brown", ColorCodeJava::Brown},
        {"green", ColorCodeJava::Green},
        {"red", ColorCodeJava::Red},
        {"black", ColorCodeJava::Black},
    };
    auto found = mapping.find(color);
    if (found == mapping.end()) {
        return ColorCodeJava::White;
    } else {
        return found->second;
    }
}

inline std::string BedrockNameFromColorCodeJava(ColorCodeJava code) {
    switch (code) {
    case ColorCodeJava::Black:
        return "black";
    case ColorCodeJava::Blue:
        return "blue";
    case ColorCodeJava::Brown:
        return "brown";
    case ColorCodeJava::Cyan:
        return "cyan";
    case ColorCodeJava::Gray:
        return "gray";
    case ColorCodeJava::Green:
        return "green";
    case ColorCodeJava::LightBlue:
        return "light_blue";
    case ColorCodeJava::LightGray:
        return "silver";
    case ColorCodeJava::Lime:
        return "lime";
    case ColorCodeJava::Magenta:
        return "magenta";
    case ColorCodeJava::Orange:
        return "orange";
    case ColorCodeJava::Pink:
        return "pink";
    case ColorCodeJava::Purple:
        return "purple";
    case ColorCodeJava::Red:
        return "red";
    case ColorCodeJava::White:
        return "white";
    case ColorCodeJava::Yellow:
        return "yellow";
    }
}

}
