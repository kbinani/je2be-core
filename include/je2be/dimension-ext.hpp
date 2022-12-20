#pragma once

#include <minecraft-file.hpp>

namespace je2be {

static inline std::optional<mcfile::Dimension> DimensionFromJavaString(std::string const &d) {
  if (d == "minecraft:overworld") {
    return mcfile::Dimension::Overworld;
  } else if (d == "minecraft:the_nether") {
    return mcfile::Dimension::Nether;
  } else if (d == "minecraft:the_end") {
    return mcfile::Dimension::End;
  }
  return std::nullopt;
}

static inline std::string JavaStringFromDimension(mcfile::Dimension d) {
  using namespace mcfile;
  switch (d) {
  case Dimension::Nether:
    return "minecraft:the_nether";
  case Dimension::End:
    return "minecraft:the_end";
  case Dimension::Overworld:
  default:
    return "minecraft:overworld";
  }
}

static inline int BedrockDimensionFromDimension(mcfile::Dimension d) {
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

static inline std::optional<mcfile::Dimension> DimensionFromBedrockDimension(int32_t d) {
  switch (d) {
  case 0:
    return mcfile::Dimension::Overworld;
  case 1:
    return mcfile::Dimension::Nether;
  case 2:
    return mcfile::Dimension::End;
  }
  return std::nullopt;
}

static inline std::optional<mcfile::Dimension> DimensionFromXbox360Dimension(int32_t d) {
  switch (d) {
  case 0:
    return mcfile::Dimension::Overworld;
  case 1:
    return mcfile::Dimension::End;
  case -1:
    return mcfile::Dimension::Nether;
  }
  return std::nullopt;
}

} // namespace je2be
