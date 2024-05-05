#pragma once

#include "_static-reversible-map.hpp"

namespace je2be {

class TrialSpawner : StaticReversibleMap<i32, std::u8string, TrialSpawner> {
public:
  static i32 BedrockTrialSpawnerStateFromJava(std::u8string const &v) {
    return Backward(v, 0);
  }

  static std::u8string JavaTrialSpawnerStateFromBedrock(i32 v) {
    return Forward(v, u8"inactive");
  }

  static ReversibleMap<i32, std::u8string> *CreateTable() {
    return new ReversibleMap<i32, std::u8string>({
        {0, u8"inactive"},
        {1, u8"waiting_for_players"},
        {2, u8"active"},
        {3, u8"waiting_for_reward_ejection"},
        {4, u8"ejecting_reward"},
        {5, u8"cooldown"},
    });
  }
};

} // namespace je2be
