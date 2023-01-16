#pragma once

namespace je2be::tobe {
class SessionLock {
public:
  explicit SessionLock(std::filesystem::path directory) : fDirectory(directory), fFile(nullptr) {}

  ~SessionLock() {
    if (fFile) {
      LockOrUnlock(fFile, false);
      fclose(fFile);
      fFile = nullptr;
    }
  }

  bool lock() {
    using namespace mcfile;

    auto path = fDirectory / "session.lock";
    FILE *fp = File::Open(path, File::Mode::Write);
    if (!fp) {
      return false;
    }
    if (!LockOrUnlock(fp, true)) {
      fclose(fp);
      return false;
    }
    std::string snowman = "\xe2\x98\x83"; // "☃"
    if (!File::Fwrite(snowman.c_str(), snowman.size(), 1, fp)) {
      fclose(fp);
      return false;
    }
    fflush(fp);
    fFile = fp;
    return true;
  }

private:
  static bool LockOrUnlock(FILE *fp, bool lock) {
    if (!fp) {
      return false;
    }
#if __has_include(<windows.h>)
    int fd = _fileno(fp);
    if (fd == -1) {
      return false;
    }
    HANDLE handle = (HANDLE)_get_osfhandle(fd);
    if (handle == INVALID_HANDLE_VALUE) {
      return false;
    }
    if (lock) {
      return ::LockFile(handle, 0, 0, MAXDWORD, MAXDWORD);
    } else {
      return ::UnlockFile(handle, 0, 0, MAXDWORD, MAXDWORD);
    }
#else
    int fd = fileno(fp);
    if (fd == -1) {
      return false;
    }
    struct flock lk = {};
    if (lock) {
      lk.l_type = F_WRLCK;
    } else {
      lk.l_type = F_UNLCK;
    }
    lk.l_whence = SEEK_SET;
    lk.l_start = 0;
    lk.l_len = 0;
    return fcntl(fd, F_SETLKW, &lk) != -1;
#endif
  }

private:
  std::filesystem::path const fDirectory;
  FILE *fFile;
};
} // namespace je2be::tobe
