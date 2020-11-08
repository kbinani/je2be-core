#pragma once

namespace j2b {

inline int32_t Wrap(std::optional<int32_t> v, int32_t fallback) {
  return v ? *v : fallback;
}

} // namespace j2b
