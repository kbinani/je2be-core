#pragma once

namespace je2be::file {

inline std::optional<std::filesystem::path> CreateTempDir(std::filesystem::path const &tempDir) {
  namespace fs = std::filesystem;
  auto tmp = fs::temp_directory_path();
#if defined(_MSC_VER)
  wchar_t *dir = _wtempnam(tmp.native().c_str(), L"j2b-tmp-");
  if (dir) {
    fs::path ret(dir);
    fs::create_directory(ret);
    free(dir);
    return ret;
  } else {
    return std::nullopt;
  }
#else
  using namespace std;
  string tmpl("j2b-tmp-XXXXXX");
  vector<char> buffer;
  copy(tmpl.begin(), tmpl.end(), back_inserter(buffer));
  buffer.push_back(0);
  char *dir = mkdtemp(buffer.data());
  if (dir) {
    return string(dir, strlen(dir));
  } else {
    return nullopt;
  }
#endif
}

class ScopedFile {
public:
  explicit ScopedFile(FILE *file) : fFile(file) {
  }

  ~ScopedFile() {
    if (fFile) {
      fclose(fFile);
    }
  }

  operator FILE *() const {
    return fFile;
  }

  operator bool() const {
    return fFile != nullptr;
  }

private:
  FILE *const fFile;
};

} // namespace je2be::file
