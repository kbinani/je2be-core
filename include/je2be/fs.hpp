#pragma once

namespace je2be {

class Fs {
public:
  static bool Exists(std::filesystem::path path) {
    std::error_code ec;
    bool exists = std::filesystem::exists(path, ec);
    if (ec) {
      return false;
    } else {
      return exists;
    }
  }

  static std::optional<std::filesystem::directory_iterator> DirectoryIterator(std::filesystem::path p) {
    std::error_code ec;
    std::filesystem::directory_iterator itr(p, ec);
    if (ec) {
      return std::nullopt;
    } else {
      return itr;
    }
  }

  static bool CreateDirectories(std::filesystem::path p) {
    if (Exists(p)) {
      return true;
    }
    std::error_code ec;
    bool ok = std::filesystem::create_directories(p, ec);
    if (ec) {
      return false;
    } else {
      return ok;
    }
  }

  static bool CopyFile(std::filesystem::path from, std::filesystem::path to) {
    std::error_code ec;
    bool ok = std::filesystem::copy_file(from, to, ec);
    if (ec) {
      return false;
    } else {
      return ok;
    }
  }

  static bool Delete(std::filesystem::path p) {
    std::error_code ec;
    bool ok = std::filesystem::remove(p, ec);
    if (ec) {
      return false;
    } else {
      return ok;
    }
  }

  static std::optional<uintmax_t> FileSize(std::filesystem::path p) {
    std::error_code ec;
    uintmax_t size = std::filesystem::file_size(p, ec);
    if (ec) {
      return std::nullopt;
    }
    return size;
  }

private:
  Fs() = delete;
};

} // namespace je2be
