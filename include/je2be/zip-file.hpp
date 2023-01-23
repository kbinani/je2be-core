#pragma once

#include <je2be/integers.hpp>

#include <minecraft-file.hpp>

namespace je2be {

class ZipFile {
public:
  explicit ZipFile(std::filesystem::path const &zipFilePath);
  ~ZipFile();

  bool store(std::vector<u8> const &buffer, std::string const &filename);
  bool store(mcfile::stream::InputStream &stream, std::string const &filename);
  bool close();

  static bool Unzip(
      std::filesystem::path const &input,
      std::filesystem::path const &output,
      std::function<bool(u64 done, u64 total)> progress = [](u64, u64) { return true; });

  static bool Zip(
      std::filesystem::path const &inputDirectory,
      std::filesystem::path const &outputZipFile,
      std::function<bool(int done, int total)> progress = [](int, int) { return true; });

private:
  void *fHandle;
  void *fStream;
};

} // namespace je2be
