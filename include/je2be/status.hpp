#pragma once

#include <je2be/errno.hpp>

#if __has_include(<leveldb/status.h>)
#include <leveldb/status.h>
#endif

#include <filesystem>
#include <optional>
#include <string>
#include <system_error>
#include <vector>

namespace je2be {

class Status {
public:
  struct Where {
    std::string fFile;
    int fLine;

    Where(char const *file, int line) {
      namespace fs = std::filesystem;
      static fs::path const sProjectRoot(fs::path(__FILE__).parent_path().parent_path().parent_path());
      std::error_code ec;
      fs::path path(file ? file : "(unknown)");
      fs::path p = fs::relative(path, sProjectRoot, ec);
      if (ec) {
        fFile = path.filename().string();
        fLine = line;
      } else {
        fFile = p.string();
        fLine = line;
      }
    }
  };

  struct ErrorData {
    std::vector<Where> fTrace;
    std::string fWhat;

    explicit ErrorData(Where where, std::string what = {}) : fTrace({where}), fWhat(what) {}
  };

  explicit Status(std::optional<ErrorData> error) : fError(error) {
  }

  Status() : fError(std::nullopt) {}

  bool ok() const {
    return !fError;
  }

  std::optional<ErrorData> error() const {
    return fError;
  }

  Status pushed(char const *file, int line) const {
    if (fError) {
      ErrorData copy = *fError;
      copy.fTrace.push_back(Where(file, line));
      return Status(copy);
    } else {
      return *this;
    }
  }

  static Status Ok() {
    return Status();
  }

  static Status Error(char const *file, int line, std::string const &what) {
    return Status(ErrorData(Where(file, line), what));
  }

  static void Merge(Status const &from, Status &to) {
    if (!to.ok()) {
      return;
    }
    if (!from.ok()) {
      to = from;
    } else {
      to = Status::Ok();
    }
  }

#if __has_include(<leveldb/status.h>)
  static Status FromLevelDBStatus(leveldb::Status st) {
    if (st.ok()) {
      return Ok();
    } else {
      return Status(ErrorData(Where(__FILE__, __LINE__), st.ToString()));
    }
  }
#endif

private:
  std::optional<ErrorData> fError;
};

} // namespace je2be

#define JE2BE_ERROR_HELPER(file, line, what) je2be::Status::Error((file), (line), (what))
#define JE2BE_ERROR JE2BE_ERROR_HELPER(__FILE__, __LINE__, std::string())
#define JE2BE_ERROR_WHAT(what) JE2BE_ERROR_HELPER(__FILE__, __LINE__, (what))

#define JE2BE_ERROR_ERRNO JE2BE_ERROR_HELPER(__FILE__, __LINE__, Errno::StringFromErrno(errno))

#define JE2BE_ERROR_PUSH_HELPER(base, file, line) (base).pushed((file), (line))
#define JE2BE_ERROR_PUSH(base) JE2BE_ERROR_PUSH_HELPER((base), __FILE__, __LINE__)
