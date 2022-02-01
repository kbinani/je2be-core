#pragma once

namespace je2be {

template <class T1, class T2>
class ReversibleMap {
  std::unordered_map<T1, T2> fNormal;
  std::unordered_map<T2, T1> fReverse;

public:
  explicit ReversibleMap(std::initializer_list<std::pair<T1, T2>> list) {
    for (auto it : list) {
      fNormal[it.first] = it.second;
      fReverse[it.second] = it.first;
    }
  }

  explicit ReversibleMap(std::initializer_list<std::pair<T2, T1>> list) {
    for (auto it : list) {
      fReverse[it.first] = it.second;
      fNormal[it.second] = it.first;
    }
  }

  std::optional<T1> find(T2 const &v) const {
    auto found = fReverse.find(v);
    if (found == fReverse.end()) {
      return std::nullopt;
    }
    return found->second;
  }

  std::optional<T2> find(T1 const &v) const {
    auto found = fNormal.find(v);
    if (found == fNormal.end()) {
      return std::nullopt;
    }
    return found->second;
  }
};

} // namespace je2be
