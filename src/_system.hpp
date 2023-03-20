#pragma once

#include <je2be/integers.hpp>

namespace je2be {

class System {
  System() = delete;

public:
  static u64 GetInstalledMemory();
};

} // namespace je2be
