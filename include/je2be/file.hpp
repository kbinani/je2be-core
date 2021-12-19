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

inline std::optional<std::string> GetContents(std::filesystem::path p) {
  using namespace std;
  using namespace mcfile;
  if (!Fs::Exists(p)) {
    return nullopt;
  }
  auto size = Fs::FileSize(p);
  if (!size) {
    return nullopt;
  }
  ScopedFile fp(mcfile::File::Open(p, mcfile::File::Mode::Read));
  if (!fp) {
    return nullopt;
  }
  string content(*size, (char)0);
  if (!File::Fread(content.data(), *size, 1, fp)) {
    return nullopt;
  }
  return content;
}

} // namespace je2be::file
