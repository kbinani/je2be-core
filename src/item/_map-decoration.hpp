#pragma once

#include <je2be/integers.hpp>

#include "_namespace.hpp"
#include "_static-reversible-map.hpp"

namespace je2be {

class MapDecoration {
public:
  static std::optional<i32> BedrockTypeFromLegacyJava(i8 type) {
    auto const &table = Table();
    if (auto found = table.find(type); found != table.end()) {
      return found->second.second;
    }
    return std::nullopt;
  }

  static std::optional<i8> LegacyJavaTypeFromJava(std::u8string const &type) {
    auto const &table = Table();
    for (auto const &it : table) {
      if (Namespace::Add(it.second.first) == type) {
        return it.first;
      }
    }
    return std::nullopt;
  }

  static std::optional<i32> BedrockTypeFromJava(std::u8string const &type) {
    auto const &table = Table();
    for (auto const &it : table) {
      if (Namespace::Add(it.second.first) == type) {
        return it.first;
      }
    }
    return std::nullopt;
  }

  static std::optional<std::u8string> JavaTypeFromBedrock(i32 type) {
    auto const &table = Table();
    if (auto found = table.find(type); found != table.end()) {
      return found->second.first;
    }
    return std::nullopt;
  }

private:
  static std::unordered_map<i32, std::pair<std::u8string, i8>> const &Table() {
    static std::unique_ptr<std::unordered_map<i32, std::pair<std::u8string, i8>> const> const sTable(CreateTable());
    return *sTable;
  }

  static std::unordered_map<i32, std::pair<std::u8string, i8>> const *CreateTable() {
    return new std::unordered_map<i32, std::pair<std::u8string, i8>>({
        //  {bedrock, {java, legacy_java}}
        {15, {u8"monument", 9}}, // id = "+"
        {14, {u8"mansion", 8}},  // id = "+"
        {4, {u8"red_x", 26}},    // id = "+"
        {17, {u8"village_desert", 27}},
        {18, {u8"village_plains", 28}},
        {19, {u8"village_savanna", 29}},
        {20, {u8"village_snowy", 30}},
        {21, {u8"village_taiga", 31}},
        {22, {u8"jungle_temple", 32}},
        {23, {u8"swamp_hut", 33}},
    });
  }
};

} // namespace je2be
