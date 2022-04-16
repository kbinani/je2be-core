#pragma once

namespace je2be {

class NullableNull final {
public:
  Status::Where fWhere;

  NullableNull(char const *file, int line) : fWhere(file, line) {}
};

template <class T>
class Nullable {
public:
  Nullable(T value) : fStorage(value) {}

  Nullable() : fStorage(Status::Where(__FILE__, __LINE__)) {}

  explicit Nullable(Status::Where reason) : fStorage(reason) {}

  Nullable(NullableNull null) : fStorage(null.fWhere) {}

  explicit operator bool() const {
    return fStorage.index() == 0;
  }

  T &operator*() {
    assert(fStorage.index() == 0);
    return std::get<T>(fStorage);
  }

  T *operator->() {
    assert(fStorage.index() == 0);
    return &std::get<T>(fStorage);
  }

  Status status() const {
    if (fStorage.index() == 0) {
      return Status::Ok();
    } else {
      Status::Where w = std::get<Status::Where>(fStorage);
      return Status(w);
    }
  }

private:
  std::variant<T, Status::Where> fStorage;
};

} // namespace je2be

#define JE2BE_NULLABLE_NULL_HELPER(file, line) je2be::NullableNull(file, line)
#define JE2BE_NULLABLE_NULL JE2BE_NULLABLE_NULL_HELPER(__FILE__, __LINE__)
