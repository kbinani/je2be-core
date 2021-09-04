#pragma once

namespace je2be {
template <class T>
inline T Clamp(T v, T minimum, T maximum) {
  return (std::min)((std::max)(v, minimum), maximum);
}
} // namespace je2be
