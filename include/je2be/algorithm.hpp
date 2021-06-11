#pragma once

namespace j2b {
template <class T>
inline T Clamp(T v, T minimum, T maximum) {
  return (std::min)((std::max)(v, minimum), maximum);
}
} // namespace j2b
