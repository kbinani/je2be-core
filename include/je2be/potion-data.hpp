#pragma once

namespace j2b {

class PotionData {
    using CompoundTag = mcfile::nbt::CompoundTag;

public:
    static int16_t PotionType(std::string const& name) {
        static std::unordered_map<std::string, int16_t> const mapping = {
            {"minecraft:water", 0},
            {"minecraft:mundane", 1},
            {"minecraft:night_vision", 5},
            {"minecraft:long_night_vision", 6},
            {"minecraft:thick", 2},
            {"minecraft:awkward", 3},
            {"minecraft:invisibility", 7},
            {"minecraft:long_invisibility", 8},
            {"minecraft:leaping", 9},
            {"minecraft:long_leaping", 10},
            {"minecraft:strong_leaping", 11},
            {"minecraft:fire_resistance", 12},
            {"minecraft:long_fire_resistance", 13},
            {"minecraft:swiftness", 14},
            {"minecraft:long_swiftness", 15},
            {"minecraft:strong_swiftness", 16},
            {"minecraft:slowness", 17},
            {"minecraft:long_slowness", 18},
            {"minecraft:strong_slowness", 42},
            {"minecraft:water_breathing", 19},
            {"minecraft:long_water_breathing", 20},
            {"minecraft:healing", 21},
            {"minecraft:strong_healing", 22},
            {"minecraft:harming", 23},
            {"minecraft:strong_harming", 24},
            {"minecraft:poison", 25},
            {"minecraft:long_poison", 26},
            {"minecraft:strong_poison", 27},
            {"minecraft:regeneration", 28},
            {"minecraft:long_regeneration", 29},
            {"minecraft:strong_regeneration", 30},
            {"minecraft:strength", 31},
            {"minecraft:long_strength", 32},
            {"minecraft:strong_strength", 33},
            {"minecraft:weakness", 34},
            {"minecraft:long_weakness", 35},
            {"minecraft:turtle_master", 37},
            {"minecraft:long_turtle_master", 38},
            {"minecraft:strong_turtle_master", 39},
            {"minecraft:slow_falling", 40},
            {"minecraft:long_slow_falling", 41},
        };
        int16_t type = 0;
        auto found = mapping.find(name);
        if (found != mapping.end()) {
            type = found->second;
        }
        return type;
    }

    static int16_t TippedArrowPotionType(std::string const& potion) {
        int16_t type = 0;
        if (potion == "minecraft:night_vision") {
            type = 6;
        } else if (potion == "minecraft:long_night_vision") {
            type = 7;
        } else if (potion == "minecraft:fire_resistance") {
            type = 13;
        } else if (potion == "minecraft:long_fire_resistance") {
            type = 14;
        } else if (potion == "minecraft:invisibility") {
            type = 8;
        } else if (potion == "minecraft:long_invisibility") {
            type = 9;
        } else if (potion == "minecraft:leaping") {
            type = 10;
        } else if (potion == "minecraft:long_leaping") {
            type = 11;
        } else if (potion == "minecraft:strong_leaping") {
            type = 12;
        } else if (potion == "minecraft:swiftness") {
            type = 15;
        } else if (potion == "minecraft:long_swiftness") {
            type = 16;
        } else if (potion == "minecraft:strong_swiftness") {
            type = 17;
        } else if (potion == "minecraft:slowness") {
            type = 18;
        } else if (potion == "minecraft:long_slowness") {
            type = 19;
        } else if (potion == "minecraft:strong_slowness") {
            type = 43;
        } else if (potion == "minecraft:water_breathing") {
            type = 20;
        } else if (potion == "minecraft:long_water_breathing") {
            type = 21;
        } else if (potion == "minecraft:healing") {
            type = 22;
        } else if (potion == "minecraft:strong_healing") {
            type = 23;
        } else if (potion == "minecraft:harming") {
            type = 24;
        } else if (potion == "minecraft:strong_harming") {
            type = 25;
        } else if (potion == "minecraft:poison") {
            type = 26;
        } else if (potion == "minecraft:long_poison") {
            type = 27;
        } else if (potion == "minecraft:strong_poison") {
            type = 28;
        } else if (potion == "minecraft:regeneration") {
            type = 29;
        } else if (potion == "minecraft:long_regeneration") {
            type = 30;
        } else if (potion == "minecraft:strong_regeneration") {
            type = 31;
        } else if (potion == "minecraft:strength") {
            type = 32;
        } else if (potion == "minecraft:long_strength") {
            type = 33;
        } else if (potion == "minecraft:strong_strength") {
            type = 34;
        } else if (potion == "minecraft:weakness") {
            type = 35;
        } else if (potion == "minecraft:long_weakness") {
            type = 36;
        } else if (potion == "minecraft:turtle_master") {
            type = 38;
        } else if (potion == "minecraft:long_turtle_master") {
            type = 39;
        } else if (potion == "minecraft:strong_turtle_master") {
            type = 40;
        } else if (potion == "minecraft:slow_falling") {
            type = 41;
        } else if (potion == "minecraft:long_slow_falling") {
            type = 42;
        }
        return type;
    }

private:
    PotionData() = delete;
};

}
