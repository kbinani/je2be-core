#pragma once

#include "_static-reversible-map.hpp"

namespace je2be {

class Effect : StaticReversibleMap<i8, std::u8string, Effect> {
  Effect() = delete;

public:
  enum Type : i8 {
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

  static i16 BedrockSuspiciousStewFromJavaEffect(i8 effect) {
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

  static SuspiciousStewEffect JavaEffectFromBedrockSuspiciousStew(i16 damage) {
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

  static std::optional<std::u8string> NamespacedIdFromLegacyId(Type type) {
    return Forward(type);
  }

  static std::optional<Type> LegacyIdFromNamespacedId(std::u8string const &id) {
    if (auto r = Backward(id); r) {
      return static_cast<Type>(*r);
    } else {
      return std::nullopt;
    }
  }

  static ReversibleMap<i8, std::u8string> *CreateTable() {
    return new ReversibleMap<i8, std::u8string>({
        {Speed, u8"minecraft:speed"},
        {Slowness, u8"minecraft:slowness"},
        {Haste, u8"minecraft:haste"},
        {MiningFatigue, u8"minecraft:mining_fatigue"},
        {Strength, u8"minecraft:strength"},
        {InstantHealth, u8"minecraft:instant_health"},
        {InstantDamage, u8"minecraft:instant_damage"},
        {JumpBoost, u8"minecraft:jump_boost"},
        {Nausea, u8"minecraft:nausea"},
        {Regeneration, u8"minecraft:regeneration"},
        {Resistance, u8"minecraft:resistance"},
        {FireResistance, u8"minecraft:fire_resistance"},
        {WaterBreathing, u8"minecraft:water_breathing"},
        {Invisibility, u8"minecraft:invisibility"},
        {Blindness, u8"minecraft:blindness"},
        {NightVision, u8"minecraft:night_vision"},
        {Hunger, u8"minecraft:hunger"},
        {Weakness, u8"minecraft:weakness"},
        {Poison, u8"minecraft:poison"},
        {Wither, u8"minecraft:poison"},
        {HealthBoost, u8"minecraft:health_boost"},
        {Absorption, u8"minecraft:absorption"},
        {Saturation, u8"minecraft:saturation"},
        {Glowing, u8"minecraft:glowing"},
        {Levitation, u8"minecraft:levitation"},
        {Luck, u8"minecraft:luck"},
        {Unluck, u8"minecraft:unluck"},
        {SlowFalling, u8"minecraft:slow_falling"},
        {ConduitPower, u8"minecraft:conduit_power"},
        {DolphinsGrace, u8"minecraft:dolphins_grace"},
        {BadOmen, u8"minecraft:bad_omen"},
        {HeroOfTheVillage, u8"minecraft:hero_of_the_village"},
    });
  }
};

} // namespace je2be
