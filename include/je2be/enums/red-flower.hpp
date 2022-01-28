#pragma once

namespace je2be {

enum class RedFlower : uint8_t {
  BlueOrchid = 1,
  AzureBluet,
  RedTulip,
  OrangeTulip,
  WhiteTulip,
  PinkTulip,
  OxeyeDaisy,
  Poppy,
  Allium,
  Cornflower,
  LilyOfTheValley,

  RedFlowerLast
};

static inline std::string BedrockNameFromRedFlower(RedFlower f) {
  switch (f) {
  case RedFlower::BlueOrchid:
    return "orchid";
  case RedFlower::AzureBluet:
    return "houstonia";
  case RedFlower::RedTulip:
    return "tulip_red";
  case RedFlower::OrangeTulip:
    return "tulip_orange";
  case RedFlower::WhiteTulip:
    return "tulip_white";
  case RedFlower::PinkTulip:
    return "tulip_pink";
  case RedFlower::OxeyeDaisy:
    return "oxeye";
  case RedFlower::Poppy:
    return "poppy";
  case RedFlower::Allium:
    return "allium";
  case RedFlower::Cornflower:
    return "cornflower";
  case RedFlower::LilyOfTheValley:
    return "lily_of_the_valley";
  default:
    assert(false);
    return "";
  }
}

static inline std::string JavaNameFromRedFlower(RedFlower f) {
  switch (f) {
  case RedFlower::BlueOrchid:
    return "blue_orchid";
  case RedFlower::AzureBluet:
    return "azure_bluet";
  case RedFlower::RedTulip:
    return "red_tulip";
  case RedFlower::OrangeTulip:
    return "orange_tulip";
  case RedFlower::WhiteTulip:
    return "white_tulip";
  case RedFlower::PinkTulip:
    return "pink_tulip";
  case RedFlower::OxeyeDaisy:
    return "oxeye_daisy";
  case RedFlower::Poppy:
    return "poppy";
  case RedFlower::Allium:
    return "allium";
  case RedFlower::Cornflower:
    return "cornflower";
  case RedFlower::LilyOfTheValley:
    return "lily_of_the_valley";
  default:
    assert(false);
    return "";
  }
}

static std::optional<RedFlower> RedFlowerFromBedrockName(std::string const &name) {
  for (uint8_t f = 1; f < static_cast<uint8_t>(RedFlower::RedFlowerLast); f++) {
    RedFlower rf = static_cast<RedFlower>(f);
    if (BedrockNameFromRedFlower(rf) == name) {
      return rf;
    }
  }
  return std::nullopt;
}

static std::optional<RedFlower> RedFlowerFromJavaName(std::string const &name) {
  for (uint8_t f = 1; f < static_cast<uint8_t>(RedFlower::RedFlowerLast); f++) {
    RedFlower rf = static_cast<RedFlower>(f);
    if (JavaNameFromRedFlower(rf) == name) {
      return rf;
    }
  }
  return std::nullopt;
}

} // namespace je2be
