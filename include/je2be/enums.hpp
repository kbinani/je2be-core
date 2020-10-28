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

}
