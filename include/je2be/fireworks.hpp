#pragma once

namespace j2b {

class FireworksData {
public:
  static FireworksData From(mcfile::nbt::CompoundTag const &fireworks) {
    FireworksData es;

    es.fFlight = fireworks.byte("Flight", 1);

    auto explosions = fireworks.query("Explosions")->asList();
    if (explosions) {
      for (auto const &it : explosions->fValue) {
        auto c = it->asCompound();
        if (!c)
          continue;
        FireworksExplosion e = FireworksExplosion::From(*c);
        es.fExplosions.push_back(e);
      }
    }

    return es;
  }

  std::shared_ptr<mcfile::nbt::CompoundTag> toCompoundTag() const {
    using namespace mcfile::nbt;
    auto ret = std::make_shared<CompoundTag>();
    auto explosions = std::make_shared<ListTag>();
    explosions->fType = Tag::TAG_Compound;
    for (auto const &it : fExplosions) {
      explosions->fValue.push_back(it.toCompoundTag());
    }
    ret->fValue["Explosions"] = explosions;
    ret->fValue["Flight"] = props::Byte(fFlight);
    return ret;
  }

public:
  std::vector<FireworksExplosion> fExplosions;
  int8_t fFlight = 1;
};

} // namespace j2b
