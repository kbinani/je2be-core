#pragma once

#include <je2be/status.hpp>

namespace je2be {
class DirectoryIterator {
  Status fStatus;
  std::unique_ptr<std::filesystem::directory_iterator> fItr;

  DirectoryIterator() {}

public:
  explicit DirectoryIterator(std::filesystem::path const &path) {
    std::error_code ec;
    fItr.reset(new std::filesystem::directory_iterator(path, ec));
    if (ec) {
      fStatus = JE2BE_ERROR;
      fItr.reset();
    }
  }

  DirectoryIterator &operator++() {
    if (fItr) {
      std::error_code ec;
      fItr->increment(ec);
      if (ec) {
        fStatus = JE2BE_ERROR;
        fItr.reset();
      }
    }
    return *this;
  }

  std::filesystem::directory_entry const *operator->() const {
    if (fItr) {
      return fItr->operator->();
    } else {
      static std::filesystem::directory_entry const sEmpty;
      return &sEmpty;
    }
  }

  bool valid() const {
    return fItr.get() != nullptr && fItr->operator->() != nullptr;
  }

  Status status() const {
    return fStatus;
  }
};

} // namespace je2be
