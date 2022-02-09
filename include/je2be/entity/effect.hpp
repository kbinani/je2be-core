#pragma once

namespace je2be {

class Effect {
  Effect() = delete;

public:
  enum Type : int8_t {
    Speed = 1,
    Slowness = 2,
    Haste = 3,
    MiningFatigue = 4,
    Strength = 5,
    InstantHealth = 6,
    InstantDamage = 7,
    JumpBoost = 8,
    Nausea = 9,
    Regeneration = 10,
    Resistance = 11,
    FireResistance = 12,
    WaterBreathing = 13,
    Invisibility = 14,
    Blindness = 15,
    NightVision = 16,
    Hunger = 17,
    Weakness = 18,
    Poison = 19,
    Wither = 20,
    HealthBoost = 21,
    Absorption = 22,
    Saturation = 23,
    Glowing = 24,
    Levitation = 25,
    Luck = 26,
    Unluck = 27,
    SlowFalling = 28,
    ConduitPower = 29,
    DolphinsGrace = 30,
    BadOmen = 31,
    HeroOfTheVillage = 32,
  };

  static int16_t BedrockSuspiciousStewFromJavaEffect(int8_t effect) {
    switch (effect) {
    case JumpBoost:
      return 1;
    case Weakness:
      return 2;
    case Blindness:
      return 3;
    case Poison:
      return 4;
    case Saturation:
      return 6;
    case FireResistance:
      return 7;
    case Regeneration:
      return 8;
    case Wither:
      return 9;
    case NightVision:
    default:
      return 0;
    }
  }

  struct SuspiciousStewEffect {
    Type fEffectId;
    int fDuration;

    SuspiciousStewEffect(Type id, int duration) : fEffectId(id), fDuration(duration) {}
  };

  static SuspiciousStewEffect JavaEffectFromBedrockSuspiciousStew(int16_t damage) {
    switch (damage) {
    case 1:
      return {JumpBoost, 120};
    case 2:
      return {Weakness, 180};
    case 3:
      return {Blindness, 160};
    case 4:
      return {Poison, 240};
    case 6:
      return {Saturation, 7};
    case 7:
      return {FireResistance, 80};
    case 8:
      return {Regeneration, 160};
    case 9:
      return {Wither, 160};
    case 0:
    default:
      return {NightVision, 100};
    }
  }
};

} // namespace je2be
