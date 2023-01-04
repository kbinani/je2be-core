#pragma once

#include <minecraft-file.hpp>

namespace je2be {

class ZipFile {
public:
  explicit ZipFile(std::filesystem::path const &zipFilePath);
  ~ZipFile();

  bool store(std::vector<uint8_t> const &buffer, std::string const &filename);
  bool store(mcfile::stream::InputStream &stream, std::string const &filename);
  bool close();

  static bool Unzip(
      std::filesystem::path const &input,
      std::filesystem::path const &output,
      std::function<bool(uint64_t done, uint64_t total)> progress = [](uint64_t, uint64_t) { return true; });

  static bool Zip(
      std::filesystem::path const &inputDirectory,
      std::filesystem::path const &outputZipFile,
      std::function<bool(int done, int total)> progress = [](int, int) { return true; });

private:
  void *fHandle;
  void *fStream;
};

} // namespace je2be
