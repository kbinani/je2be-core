#pragma once

namespace je2be::file {

[[nodiscard]] static inline bool GetContents(std::filesystem::path p, std::vector<uint8_t> &buffer) {
  using namespace std;
  using namespace mcfile;
  if (!Fs::Exists(p)) {
    return false;
  }
  auto size = Fs::FileSize(p);
  if (!size) {
    return false;
  }
  ScopedFile fp(mcfile::File::Open(p, mcfile::File::Mode::Read));
  if (!fp) {
    return false;
  }
  buffer.resize(*size);
  if (!File::Fread(buffer.data(), *size, 1, fp.get())) {
    return false;
  }
  return true;
}

} // namespace je2be::file
