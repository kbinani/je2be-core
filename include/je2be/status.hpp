#pragma once

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

  explicit Status(std::optional<Where> error) : fError(error) {
  }

  Status() : fError(std::nullopt) {}

  bool ok() const {
    return !fError;
  }

  std::optional<Where> error() const {
    return fError;
  }

  static Status Ok() {
    return Status();
  }

  static Status Error(char const *file, int line) {
    return Status(Where(file, line));
  }

private:
  std::optional<Where> fError;
};

} // namespace je2be

#define JE2BE_ERROR_HELPER(file, line) je2be::Status::Error(file, line)
#define JE2BE_ERROR JE2BE_ERROR_HELPER(__FILE__, __LINE__)
