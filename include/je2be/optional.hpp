#pragma once

namespace j2b {

template <class T> inline T Wrap(std::optional<T> v, T fallback) { return v ? *v : fallback; }

} // namespace j2b
