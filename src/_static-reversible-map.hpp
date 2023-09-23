#pragma once

#include "_reversible-map.hpp"

namespace je2be {

template <class T1, class T2, class TableCreator>
class StaticReversibleMap {
  StaticReversibleMap() {}

protected:
  static ReversibleMap<T1, T2> const *GetTable() {
    static std::unique_ptr<ReversibleMap<T1, T2> const> const sTable(TableCreator::CreateTable());
    return sTable.get();
  }

  static T2 Forward(T1 const &v, T2 defaultValue) {
    auto const *table = GetTable();
    auto ret = table->forward(v);
    if (ret) {
      return *ret;
    } else {
      return defaultValue;
    }
  }

  static std::optional<T2> Forward(T1 const &v) {
    auto const *table = GetTable();
    return table->forward(v);
  }

  static T1 Backward(T2 const &v, T1 defaultValue) {
    auto const *table = GetTable();
    auto ret = table->backward(v);
    if (ret) {
      return *ret;
    } else {
      return defaultValue;
    }
  }

  static std::optional<T1> Backward(T2 const &v) {
    auto const *table = GetTable();
    return table->backward(v);
  }
};

} // namespace je2be
