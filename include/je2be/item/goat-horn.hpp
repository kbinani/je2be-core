#pragma once

#include <je2be/static-reversible-map.hpp>

namespace je2be {

class GoatHorn : StaticReversibleMap<std::string, int16_t, GoatHorn> {
public:
  static int16_t BedrockDamageFromJavaInstrument(std::string const &instrument) {
    return Forward(instrument, 0);
  }

  static std::string JavaInstrumentFromBedrockDamage(int16_t damage) {
    return Backward(damage, "minecraft:ponder_goat_horn");
  }

  static ReversibleMap<std::string, int16_t> const *CreateTable() {
    return new ReversibleMap<std::string, int16_t>({
        {"minecraft:ponder_goat_horn", 0},
        {"minecraft:sing_goat_horn", 1},
        {"minecraft:seek_goat_horn", 2},
        {"minecraft:feel_goat_horn", 3},
        {"minecraft:admire_goat_horn", 4},
        {"minecraft:call_goat_horn", 5},
        {"minecraft:yearn_goat_horn", 6},
        {"minecraft:dream_goat_horn", 7},
    });
  }
};

} // namespace je2be
