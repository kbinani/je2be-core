#pragma once

namespace je2be {

template <class ContainerIn, class ContainerOut>
static inline void Invert(ContainerIn const &input, ContainerOut &output) {
  output.clear();
  for (auto const &it : input) {
    output[it.second] = it.first;
  }
}

} // namespace je2be
