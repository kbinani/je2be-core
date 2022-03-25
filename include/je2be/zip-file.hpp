#pragma once

namespace je2be {

class ZipFile {
public:
  explicit ZipFile(std::filesystem::path const &zipFilePath) : fHandle(nullptr), fStream(nullptr) {
    if (!mz_stream_os_create(&fStream)) {
      return;
    }
    if (MZ_OK != mz_stream_os_open(fStream, zipFilePath.string().c_str(), MZ_OPEN_MODE_CREATE)) {
      return;
    }
    if (!mz_zip_create(&fHandle)) {
      return;
    }
    if (MZ_OK != mz_zip_open(fHandle, fStream, MZ_OPEN_MODE_WRITE)) {
      return;
    }
  }

  ~ZipFile() {
    close();
  }

  bool store(std::vector<uint8_t> const &buffer, std::string const &filename) {
    mz_zip_file s = {0};
    s.version_madeby = MZ_VERSION_MADEBY;
    s.compression_method = MZ_COMPRESS_METHOD_STORE;
    s.filename = filename.c_str();
    int16_t compressLevel = 0;
    uint8_t raw = 0;
    if (MZ_OK != mz_zip_entry_write_open(fHandle, &s, compressLevel, raw, nullptr)) {
      return false;
    }
    int32_t totalWritten = 0;
    int32_t err = MZ_OK;
    int32_t remaining = buffer.size();
    do {
      int32_t code = mz_zip_entry_write(fHandle, buffer.data() + totalWritten, remaining);
      if (code < 0) {
        err = code;
      } else {
        totalWritten += code;
        remaining -= code;
      }
    } while (err == MZ_OK && remaining > 0);

    return err == MZ_OK && mz_zip_entry_close(fHandle) == MZ_OK;
  }

  bool close() {
    bool ok = true;
    if (fHandle) {
      ok &= mz_zip_close(fHandle) == MZ_OK;
      mz_zip_delete(&fHandle);
      fHandle = nullptr;
    } else {
      ok = false;
    }
    if (fStream) {
      ok &= mz_stream_close(fStream) == MZ_OK;
      mz_stream_os_delete(&fStream);
      fStream = nullptr;
    } else {
      ok = false;
    }
    return ok;
  }

private:
  void *fHandle;
  void *fStream;
};

} // namespace je2be
