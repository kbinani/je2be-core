#pragma once

#include <je2be/nbt.hpp>
#include <je2be/status.hpp>

#include <map>

namespace je2be::lce {

class Behavior {
public:
  virtual ~Behavior() {}
  virtual Status decompressChunk(std::vector<uint8_t> &buffer) const = 0;
  virtual Status loadPlayers(std::filesystem::path const &inputDirectory, std::map<std::filesystem::path, CompoundTagPtr> &outBuffer) const = 0;
};

} // namespace je2be::lce
