#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <system_error>

namespace je2be {

class Status {
public:
  struct Where {
    std::string fFile;
    int fLine;

    Where(char const *file, int line) {
      namespace fs = std::filesystem;
      static fs::path const sProjectRoot(fs::path(__FILE__).parent_path());
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
    Where fWhere;
    std::string fWhat;

    explicit ErrorData(Where where, std::string what = {}) : fWhere(where), fWhat(what) {}
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

  static Status Ok() {
    return Status();
  }

  static Status Error(char const *file, int line, std::string const &what) {
    return Status(ErrorData(Where(file, line), what));
  }

private:
  std::optional<ErrorData> fError;
};

} // namespace je2be

#define JE2BE_ERROR_HELPER(file, line, what) je2be::Status::Error((file), (line), (what))

#define JE2BE_ERROR JE2BE_ERROR_HELPER(__FILE__, __LINE__, std::string())
#define JE2BE_ERROR_WHAT(what) JE2BE_ERROR_HELPER(__FILE__, __LINE__, (what))
