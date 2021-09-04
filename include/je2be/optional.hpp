#pragma once

namespace je2be {

template <class T>
inline T Wrap(std::optional<T> v, T fallback) { return v ? *v : fallback; }

} // namespace je2be
