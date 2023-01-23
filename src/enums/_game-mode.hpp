#pragma once

#include <je2be/integers.hpp>

#include <cstdint>
#include <optional>

namespace je2be {

enum class GameMode {
  Survival,
  Creative,
  Adventure,
  Spectator,
};

static inline std::optional<GameMode> GameModeFromJava(i32 v) {
  switch (v) {
  case 0:
    return GameMode::Survival;
  case 1:
    return GameMode::Creative;
  case 2:
    return GameMode::Adventure;
  case 3:
    return GameMode::Spectator;
  default:
    return std::nullopt;
  }
}

static inline std::optional<GameMode> GameModeFromBedrock(i32 v) {
  switch (v) {
  case 0:
    return GameMode::Survival;
  case 1:
    return GameMode::Creative;
  case 2:
    return GameMode::Adventure;
  case 6:
    return GameMode::Spectator;
  default:
    // 5 means "same as world game mode"
    return std::nullopt;
  }
}

static inline i32 JavaFromGameMode(GameMode m) {
  switch (m) {
  case GameMode::Creative:
    return 1;
  case GameMode::Adventure:
    return 2;
  case GameMode::Spectator:
    return 3;
  case GameMode::Survival:
  default:
    return 0;
  }
}

static inline i32 BedrockFromGameMode(GameMode m) {
  switch (m) {
  case GameMode::Creative:
    return 1;
  case GameMode::Adventure:
    return 2;
  case GameMode::Spectator:
    return 6;
  case GameMode::Survival:
  default:
    return 0;
  }
}

} // namespace je2be
