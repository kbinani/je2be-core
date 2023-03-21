#pragma once

#include <je2be/integers.hpp>

namespace je2be {

class System {
  System() = delete;

public:
  static u64 GetInstalledMemory();
  static u64 GetAvailableMemory();
};

} // namespace je2be
