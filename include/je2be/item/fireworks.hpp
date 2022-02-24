#pragma once

namespace je2be {

class FireworksData {
public:
  static FireworksData FromJava(CompoundTag const &fireworks) {
    FireworksData es;

    es.fFlight = fireworks.byte("Flight");

    auto explosions = fireworks.listTag("Explosions");
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
    fd.fFlight = tagB.byte("Flight");
    auto explosions = tagB.listTag("Explosions");
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

  std::shared_ptr<CompoundTag> toBedrockCompoundTag() const {
    auto ret = Compound();
    auto explosions = List<Tag::Type::Compound>();
    for (auto const &it : fExplosions) {
      explosions->push_back(it.toBedrockCompoundTag());
    }
    ret->set("Explosions", explosions);
    if (fFlight) {
      ret->set("Flight", Byte(*fFlight));
    }
    return ret;
  }

  std::shared_ptr<CompoundTag> toJavaCompoundTag() const {
    auto ret = Compound();
    auto explosions = List<Tag::Type::Compound>();
    for (auto const &it : fExplosions) {
      explosions->push_back(it.toJavaCompoundTag());
    }
    ret->set("Explosions", explosions);
    if (fFlight) {
      ret->set("Flight", Byte(*fFlight));
    }
    return ret;
  }

public:
  std::vector<FireworksExplosion> fExplosions;
  std::optional<int8_t> fFlight;
};

} // namespace je2be
