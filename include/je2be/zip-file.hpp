#pragma once

#include <je2be/integers.hpp>
#include <je2be/status.hpp>

#include <minecraft-file.hpp>

namespace je2be {

class ZipFile {
public:
  explicit ZipFile(std::filesystem::path const &zipFilePath);
  ~ZipFile();

  struct ZipResult {
    Status fStatus = Status::Ok();
    bool fZip64Used = false;
  };

  Status store(std::vector<u8> const &buffer, std::string const &filename, int compressionLevel0To9 = 9);
  Status store(mcfile::stream::InputStream &stream, std::string const &filename, int compressionLevel0To9 = 9);
  ZipResult close();

  static Status Unzip(
      std::filesystem::path const &input,
      std::filesystem::path const &output,
      std::function<bool(u64 done, u64 total)> progress = [](u64, u64) { return true; });

  static ZipResult Zip(
      std::filesystem::path const &inputDirectory,
      std::filesystem::path const &outputZipFile,
      std::function<bool(int done, int total)> progress = [](int, int) { return true; });

private:
  void *fHandle;
  void *fStream;
  bool fZip64Used = false;
};

} // namespace je2be
