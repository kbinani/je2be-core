#pragma once

namespace je2be {

class Status {
  Status(bool ok, std::string const &file, int line) : fOk(ok), fFile(file), fLine(line) {
  }

public:
  bool ok() const {
    return fOk;
  }

  static Status Ok() {
    return Status(true, {}, -1);
  }

  static Status Error(char const *file, int line) {
    namespace fs = std::filesystem;
    static fs::path const sProjectRoot(fs::path(__FILE__).parent_path());
    std::error_code ec;
    fs::path path(file ? file : "(unknown)");
    fs::path p = fs::relative(path, sProjectRoot, ec);
    if (ec) {
      return Status(false, path.filename().string(), line);
    } else {
      return Status(false, p.string(), line);
    }
  }

  bool const fOk;
  std::string const fFile;
  int const fLine;
};

} // namespace je2be
