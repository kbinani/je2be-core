#pragma once

namespace j2b {

class FireworksData {
public:
  static FireworksData From(mcfile::nbt::CompoundTag const &fireworks) {
    FireworksData es;

    es.fFlight = fireworks.byte("Flight", 1);

    auto explosions = fireworks.listTag("Explosions");
    if (explosions) {
      for (auto const &it : *explosions) {
        auto c = it->asCompound();
        if (!c) {
          continue;
        }
        FireworksExplosion e = FireworksExplosion::From(*c);
        es.fExplosions.push_back(e);
      }
    }

    return es;
  }

  std::shared_ptr<mcfile::nbt::CompoundTag> toCompoundTag() const {
    using namespace mcfile::nbt;
    auto ret = std::make_shared<CompoundTag>();
    auto explosions = std::make_shared<ListTag>(Tag::Type::Compound);
    for (auto const &it : fExplosions) {
      explosions->push_back(it.toCompoundTag());
    }
    ret->set("Explosions", explosions);
    ret->set("Flight", props::Byte(fFlight));
    return ret;
  }

public:
  std::vector<FireworksExplosion> fExplosions;
  int8_t fFlight = 1;
};

} // namespace j2b
