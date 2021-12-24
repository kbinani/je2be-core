#pragma once

namespace je2be::tobe {
inline std::optional<mcfile::Dimension> DimensionFromString(std::string const &d) {
  if (d == "minecraft:overworld") {
    return mcfile::Dimension::Overworld;
  } else if (d == "minecraft:the_nether") {
    return mcfile::Dimension::Nether;
  } else if (d == "minecraft:the_end") {
    return mcfile::Dimension::End;
  }
  return std::nullopt;
}

inline int IntFromDimension(mcfile::Dimension d) {
  using namespace mcfile;
  switch (d) {
  case Dimension::Nether:
    return 1;
  case Dimension::End:
    return 2;
  case Dimension::Overworld:
  default:
    return 0;
  }
}
} // namespace je2be::tobe
