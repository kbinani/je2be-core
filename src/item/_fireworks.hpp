#pragma once

#include "item/_fireworks-explosion.hpp"

namespace je2be {

class FireworksData {
public:
  static FireworksData FromJava(CompoundTag const &fireworks) {
    FireworksData es;

    es.fFlight = FallbackValue<i8>(fireworks, {u8"flight_duration", u8"Flight"});

    auto explosions = FallbackPtr<ListTag>(fireworks, {u8"explosions", u8"Explosions"});
    if (explosions) {
      for (auto const &it : *explosions) {
        auto c = it->asCompound();
        if (!c) {
          continue;
        }
        FireworksExplosion e = FireworksExplosion::FromJava(*c);
        es.fExplosions.push_back(e);
      }
    }

    return es;
  }

  static FireworksData FromBedrock(CompoundTag const &tagB) {
    FireworksData fd;
    fd.fFlight = tagB.byte(u8"Flight");
    auto explosions = tagB.listTag(u8"Explosions");
    if (explosions) {
      for (auto const &it : *explosions) {
        auto c = it->asCompound();
        if (!c) {
          continue;
        }
        FireworksExplosion e = FireworksExplosion::FromBedrock(*c);
        fd.fExplosions.push_back(e);
      }
    }
    return fd;
  }

  CompoundTagPtr toBedrockCompoundTag() const {
    auto ret = Compound();
    auto explosions = List<Tag::Type::Compound>();
    for (auto const &it : fExplosions) {
      explosions->push_back(it.toBedrockCompoundTag());
    }
    ret->set(u8"Explosions", explosions);
    if (fFlight) {
      ret->set(u8"Flight", Byte(*fFlight));
    }
    return ret;
  }

  CompoundTagPtr toJavaCompoundTag() const {
    auto ret = Compound();
    auto explosions = List<Tag::Type::Compound>();
    for (auto const &it : fExplosions) {
      explosions->push_back(it.toJavaCompoundTag());
    }
    ret->set(u8"explosions", explosions);
    if (fFlight) {
      ret->set(u8"flight_duration", Byte(*fFlight));
    }
    return ret;
  }

public:
  std::vector<FireworksExplosion> fExplosions;
  std::optional<i8> fFlight;
};

} // namespace je2be
