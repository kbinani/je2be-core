#pragma once

#include <je2be/integers.hpp>

#include <enums/_effect.hpp>

#include "_static-reversible-map.hpp"

namespace je2be {

class Beacon {
public:
  static std::optional<std::u8string> JavaEffectFromLegacyJavaAndBedrock(i32 id) {
    if (id < 0 || std::numeric_limits<std::underlying_type<Effect::Type>::type>::max() < id) {
      return std::nullopt;
    }
    return Effect::NamespacedIdFromLegacyId(static_cast<Effect::Type>(id));
  }

  static i32 BedrockEffectFromJava(std::u8string const &id) {
    if (auto i = Effect::LegacyIdFromNamespacedId(id); i) {
      return *i;
    } else {
      return -1;
    }
  }
};

} // namespace je2be
