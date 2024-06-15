#pragma once

#include <je2be/integers.hpp>

namespace je2be {

class MapType {
  MapType() = delete;

  struct JavaInfo {
    std::u8string fTranslationKey;
    std::optional<i32> fColor;
  };

  static std::unordered_map<i16, JavaInfo> *CreateTable() {
    auto ret = new std::unordered_map<i16, JavaInfo>();
    (*ret)[5] = {u8"filled_map.buried_treasure", std::nullopt};
    (*ret)[4] = {u8"filled_map.mansion", 5393476};
    (*ret)[3] = {u8"filled_map.monument", 3830373};
    (*ret)[12] = {u8"filled_map.explorer_jungle", 10066329};
    (*ret)[10] = {u8"filled_map.village_savanna", 10066329};
    (*ret)[8] = {u8"filled_map.village_taiga", 10066329};
    (*ret)[13] = {u8"filled_map.explorer_swamp", 10066329};
    (*ret)[11] = {u8"filled_map.village_desert", 10066329};
    (*ret)[9] = {u8"filled_map.village_plains", 10066329};
    (*ret)[7] = {u8"filled_map.village_snowy", 10066329};
    (*ret)[14] = {u8"filled_map.trial_chambers", 12741452};
    return ret;
  }

  static std::unordered_map<i16, JavaInfo> const &JavaTable() {
    static std::unique_ptr<std::unordered_map<i16, JavaInfo> const> const sTable(CreateTable());
    return *sTable;
  }

  static std::unordered_map<std::u8string, i16> *CreateBedrockTable() {
    auto ret = new std::unordered_map<std::u8string, i16>();
    auto const &table = JavaTable();
    for (auto const &it : table) {
      (*ret)[it.second.fTranslationKey] = it.first;
    }
    return ret;
  }

  static std::unordered_map<std::u8string, i16> const &BedrockTable() {
    static std::unique_ptr<std::unordered_map<std::u8string, i16> const> const sTable(CreateBedrockTable());
    return *sTable;
  }

public:
  static std::optional<i16> BedrockDamageFromJavaTranslationKey(std::u8string const &key) {
    auto const &table = BedrockTable();
    auto found = table.find(key);
    if (found == table.end()) {
      return std::nullopt;
    }
    return found->second;
  }

  static std::optional<std::u8string> JavaTranslationKeyFromBedrockDamage(i16 damage) {
    auto const &table = JavaTable();
    auto found = table.find(damage);
    if (found == table.end()) {
      return std::nullopt;
    }
    return found->second.fTranslationKey;
  }

  static std::optional<i32> JavaMapColorFromBedrockDamage(i16 damage) {
    auto const &table = JavaTable();
    auto found = table.find(damage);
    if (found == table.end()) {
      return std::nullopt;
    }
    return found->second.fColor;
  }
};

} // namespace je2be
