#pragma once

#include <minecraft-file.hpp>

#include "_namespace.hpp"

namespace je2be {

static inline std::optional<mcfile::Dimension> DimensionFromJavaString(std::u8string const &d) {
  auto dim = Namespace::Remove(d);
  if (dim == u8"overworld") {
    return mcfile::Dimension::Overworld;
  } else if (dim == u8"the_nether") {
    return mcfile::Dimension::Nether;
  } else if (dim == u8"the_end") {
    return mcfile::Dimension::End;
  }
  return std::nullopt;
}

static inline std::u8string JavaStringFromDimension(mcfile::Dimension d) {
  using namespace mcfile;
  switch (d) {
  case Dimension::Nether:
    return u8"minecraft:the_nether";
  case Dimension::End:
    return u8"minecraft:the_end";
  case Dimension::Overworld:
  default:
    return u8"minecraft:overworld";
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

static inline std::optional<mcfile::Dimension> DimensionFromBedrockDimension(i32 d) {
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

static inline std::optional<mcfile::Dimension> DimensionFromXbox360Dimension(i32 d) {
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
