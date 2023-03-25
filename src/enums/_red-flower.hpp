#pragma once

namespace je2be {

enum class RedFlower : u8 {
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

static inline std::u8string BedrockNameFromRedFlower(RedFlower f) {
  switch (f) {
  case RedFlower::BlueOrchid:
    return u8"orchid";
  case RedFlower::AzureBluet:
    return u8"houstonia";
  case RedFlower::RedTulip:
    return u8"tulip_red";
  case RedFlower::OrangeTulip:
    return u8"tulip_orange";
  case RedFlower::WhiteTulip:
    return u8"tulip_white";
  case RedFlower::PinkTulip:
    return u8"tulip_pink";
  case RedFlower::OxeyeDaisy:
    return u8"oxeye";
  case RedFlower::Poppy:
    return u8"poppy";
  case RedFlower::Allium:
    return u8"allium";
  case RedFlower::Cornflower:
    return u8"cornflower";
  case RedFlower::LilyOfTheValley:
    return u8"lily_of_the_valley";
  default:
    assert(false);
    return u8"";
  }
}

static inline std::u8string JavaNameFromRedFlower(RedFlower f) {
  switch (f) {
  case RedFlower::BlueOrchid:
    return u8"blue_orchid";
  case RedFlower::AzureBluet:
    return u8"azure_bluet";
  case RedFlower::RedTulip:
    return u8"red_tulip";
  case RedFlower::OrangeTulip:
    return u8"orange_tulip";
  case RedFlower::WhiteTulip:
    return u8"white_tulip";
  case RedFlower::PinkTulip:
    return u8"pink_tulip";
  case RedFlower::OxeyeDaisy:
    return u8"oxeye_daisy";
  case RedFlower::Poppy:
    return u8"poppy";
  case RedFlower::Allium:
    return u8"allium";
  case RedFlower::Cornflower:
    return u8"cornflower";
  case RedFlower::LilyOfTheValley:
    return u8"lily_of_the_valley";
  default:
    assert(false);
    return u8"";
  }
}

static inline std::optional<RedFlower> RedFlowerFromBedrockName(std::u8string const &name) {
  for (u8 f = 1; f < static_cast<u8>(RedFlower::RedFlowerLast); f++) {
    RedFlower rf = static_cast<RedFlower>(f);
    if (BedrockNameFromRedFlower(rf) == name) {
      return rf;
    }
  }
  return std::nullopt;
}

static inline std::optional<RedFlower> RedFlowerFromJavaName(std::u8string const &name) {
  for (u8 f = 1; f < static_cast<u8>(RedFlower::RedFlowerLast); f++) {
    RedFlower rf = static_cast<RedFlower>(f);
    if (JavaNameFromRedFlower(rf) == name) {
      return rf;
    }
  }
  return std::nullopt;
}

} // namespace je2be
