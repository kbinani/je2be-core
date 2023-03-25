#pragma once

#include "_static-reversible-map.hpp"

namespace je2be {

class GoatHorn : StaticReversibleMap<std::u8string, i16, GoatHorn> {
public:
  static i16 BedrockDamageFromJavaInstrument(std::u8string const &instrument) {
    return Forward(instrument, 0);
  }

  static std::u8string JavaInstrumentFromBedrockDamage(i16 damage) {
    return Backward(damage, u8"minecraft:ponder_goat_horn");
  }

  static ReversibleMap<std::u8string, i16> const *CreateTable() {
    return new ReversibleMap<std::u8string, i16>({
        {u8"minecraft:ponder_goat_horn", 0},
        {u8"minecraft:sing_goat_horn", 1},
        {u8"minecraft:seek_goat_horn", 2},
        {u8"minecraft:feel_goat_horn", 3},
        {u8"minecraft:admire_goat_horn", 4},
        {u8"minecraft:call_goat_horn", 5},
        {u8"minecraft:yearn_goat_horn", 6},
        {u8"minecraft:dream_goat_horn", 7},
    });
  }
};

} // namespace je2be
