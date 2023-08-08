#pragma once

namespace je2be {

template <class Key, class Value>
class DefaultMap {
public:
  explicit DefaultMap(Value def) : fDefault(def) {}

  DefaultMap(std::initializer_list<typename std::map<Key, Value>::value_type> initializer, Value def) : fStorage(initializer), fDefault(def) {}

  void insert(Key const &k, Value const &v) {
    fStorage[k] = v;
  }

  void erase(Key const &k) {
    fStorage.erase(k);
  }

  Value at(Key const &k) const {
    if (auto found = fStorage.find(k); found != fStorage.end()) {
      return found->second;
    } else {
      return fDefault;
    }
  }

private:
  std::map<Key, Value> fStorage;
  Value fDefault;
};

} // namespace je2be
