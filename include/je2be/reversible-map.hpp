#pragma once

namespace je2be {

template <class T1, class T2>
class ReversibleMap {
  std::unordered_map<T1, T2> fForward;
  std::unordered_map<T2, T1> fBackward;

public:
  explicit ReversibleMap(std::initializer_list<std::pair<T1, T2>> list) {
    for (auto it : list) {
      fForward[it.first] = it.second;
      fBackward[it.second] = it.first;
    }
  }

  std::optional<T1> forward(T2 const &v) const {
    auto found = fBackward.find(v);
    if (found == fBackward.end()) {
      return std::nullopt;
    }
    return found->second;
  }

  std::optional<T2> backward(T1 const &v) const {
    auto found = fForward.find(v);
    if (found == fForward.end()) {
      return std::nullopt;
    }
    return found->second;
  }
};

} // namespace je2be
