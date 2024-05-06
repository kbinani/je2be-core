#pragma once

#include <je2be/integers.hpp>

#include "_namespace.hpp"

namespace je2be {

class MapDecoration {
public:
  static std::optional<i32> BedrockTypeFromLegacyJava(i8 type) {
    auto const &table = Table();
    for (auto const &it : table) {
      if (it.second.fLegacyJava == type) {
        return it.second.fBedrock;
      }
    }
    return std::nullopt;
  }

  static std::optional<i8> LegacyJavaTypeFromJava(std::u8string const &type) {
    auto const &table = Table();
    if (auto found = table.find(Namespace::Remove(type)); found != table.end()) {
      return found->second.fLegacyJava;
    }
    return std::nullopt;
  }

  static std::optional<i32> BedrockTypeFromJava(std::u8string const &type) {
    auto const &table = Table();
    if (auto found = table.find(Namespace::Remove(type)); found != table.end()) {
      return found->second.fBedrock;
    }
    return std::nullopt;
  }

  static std::optional<std::u8string> JavaTypeFromBedrock(i32 type) {
    auto const &table = Table();
    for (auto const &it : table) {
      if (it.second.fBedrock == type) {
        return Namespace::Add(it.first);
      }
    }
    return std::nullopt;
  }

private:
  struct Element {
    std::optional<i8> fLegacyJava;
    std::optional<i32> fBedrock;
  };
  static std::unordered_map<std::u8string, Element> const &Table() {
    static std::unique_ptr<std::unordered_map<std::u8string, Element> const> const sTable(CreateTable());
    return *sTable;
  }

  static std::unordered_map<std::u8string, Element> const *CreateTable() {
    using namespace std;
    return new unordered_map<u8string, Element>({
        //  {java, {legacy_java, bedrock}}
        {u8"player", {0, nullopt}},
        {u8"frame", {1, nullopt}},
        {u8"red_marker", {2, nullopt}},
        {u8"blue_marker", {3, nullopt}},
        {u8"target_x", {4, nullopt}},
        {u8"target_point", {5, nullopt}},
        {u8"player_off_map", {6, nullopt}},
        {u8"player_off_limits", {7, nullopt}},
        {u8"mansion", {8, 14}},  // id = "+"
        {u8"monument", {9, 15}}, // id = "+"
        {u8"banner_white", {10, nullopt}},
        {u8"banner_orange", {11, nullopt}},
        {u8"banner_magenta", {12, nullopt}},
        {u8"banner_light_blue", {13, nullopt}},
        {u8"banner_yellow", {14, nullopt}},
        {u8"banner_lime", {15, nullopt}},
        {u8"banner_pink", {16, nullopt}},
        {u8"banner_gray", {17, nullopt}},
        {u8"banner_light_gray", {18, nullopt}},
        {u8"banner_cyan", {19, nullopt}},
        {u8"banner_purple", {20, nullopt}},
        {u8"banner_blue", {21, nullopt}},
        {u8"banner_brown", {22, nullopt}},
        {u8"banner_green", {23, nullopt}},
        {u8"banner_red", {24, nullopt}},
        {u8"banner_black", {25, nullopt}},
        {u8"red_x", {26, 4}}, // id = "+"
        {u8"village_desert", {27, 17}},
        {u8"village_plains", {28, 18}},
        {u8"village_savanna", {29, 19}},
        {u8"village_snowy", {30, 20}},
        {u8"village_taiga", {31, 21}},
        {u8"jungle_temple", {32, 22}},
        {u8"swamp_hut", {33, 23}},
    });
  }
};

} // namespace je2be
