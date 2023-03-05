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

  void next() {
    if (fItr) {
      std::error_code ec;
      fItr->increment(ec);
      if (ec) {
        fStatus = JE2BE_ERROR;
        fItr.reset();
      }
    }
  }

  std::filesystem::directory_entry const *operator->() const {
    static std::filesystem::directory_entry const sEmpty;
    if (!fItr) {
      return &sEmpty;
    }
    if (std::filesystem::end(*fItr) == *fItr) {
      return &sEmpty;
    }
    auto entry = fItr->operator->();
    if (entry) {
      return entry;
    } else {
      return &sEmpty;
    }
  }

  bool valid() const {
    if (!fItr) {
      return false;
    }
    return std::filesystem::end(*fItr) != *fItr;
  }

  Status status() const {
    return fStatus;
  }
};

} // namespace je2be
