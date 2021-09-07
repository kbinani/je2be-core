#pragma once

namespace je2be {
class ScopedFile {
public:
  explicit ScopedFile(FILE *file) : fFile(file) {
  }

  ScopedFile() : fFile(nullptr) {}

  ~ScopedFile() {
    close();
  }

  void close() {
    if (fFile) {
      fclose(fFile);
      fFile = nullptr;
    }
  }

  operator FILE *() const {
    return fFile;
  }

  operator bool() const {
    return fFile != nullptr;
  }

private:
  FILE *fFile;
};
} // namespace je2be
