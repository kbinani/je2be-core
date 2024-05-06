#pragma once

#include "entity/_entity-attributes.hpp"
#include "java/_entity.hpp"

namespace je2be {

class TropicalFish {
public:
  static TropicalFish FromJavaVariant(i32 variant) {
    u32 u = *(u32 *)&variant;
    bool small = (0xf & u) == 0;
    i32 pattern = 0xf & (u >> 8);
    i32 bodyColor = 0xf & (u >> 16);
    i32 patternColor = 0xf & (u >> 24);
    TropicalFish tf(small, pattern, (i8)bodyColor, (i8)patternColor);
    return tf;
  }

  static TropicalFish FromBedrockBucketTag(CompoundTag const &tag) {
    bool small = tag.int32(u8"Variant", 0) == 0;
    i32 pattern = tag.int32(u8"MarkVariant", 0);
    i8 bodyColor = tag.byte(u8"Color", 0);
    i8 patternColor = tag.byte(u8"Color2", 0);
    return TropicalFish(small, pattern, bodyColor, patternColor);
  }

  i32 toJavaVariant() const {
    u32 u = 0;
    u |= fSmall ? 0 : 1;
    u |= (0xf & (u32)fPattern) << 8;
    u |= (0xf & (u32)fBodyColor) << 16;
    u |= (0xf & (u32)fPatternColor) << 24;
    return *(i32 *)&u;
  }

  CompoundTagPtr toBedrockBucketTag(i64 entityId, std::optional<float> health) const {
    using namespace std;
    java::Entity::Rep rep(entityId);
    auto ret = rep.toCompoundTag();
    auto attributes = EntityAttributes::Mob(u8"minecraft:tropical_fish", health);
    if (attributes) {
      ret->set(u8"Attributes", attributes->toBedrockListTag());
    }
    auto props = java::Entity::TropicalFishProperties(toJavaVariant());
    for (auto const &it : *props) {
      ret->set(it.first, it.second);
    }
    return ret;
  }

private:
  TropicalFish(bool small, i32 pattern, i8 bodyColor, i8 patternColor) : fSmall(small), fPattern(pattern), fBodyColor(bodyColor), fPatternColor(patternColor) {}

public:
  bool fSmall;
  i32 fPattern;
  i8 fBodyColor;
  i8 fPatternColor;
};

} // namespace je2be
