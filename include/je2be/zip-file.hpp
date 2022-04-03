#pragma once

namespace je2be {

class ZipFile {
public:
  explicit ZipFile(std::filesystem::path const &zipFilePath) : fHandle(nullptr), fStream(nullptr) {
    if (!mz_stream_os_create(&fStream)) {
      return;
    }
    if (MZ_OK != mz_stream_os_open(fStream, (char const *)zipFilePath.u8string().c_str(), MZ_OPEN_MODE_CREATE)) {
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
    s.compression_method = MZ_COMPRESS_METHOD_DEFLATE;
    s.filename = filename.c_str();
    int16_t compressLevel = 9;
    uint8_t raw = 0;
    if (MZ_OK != mz_zip_entry_write_open(fHandle, &s, compressLevel, raw, nullptr)) {
      return false;
    }
    int32_t totalWritten = 0;
    int32_t err = MZ_OK;
    int32_t remaining = (int32_t)buffer.size();
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

  bool store(mcfile::stream::InputStream &stream, std::string const &filename) {
    mz_zip_file s = {0};
    s.version_madeby = MZ_VERSION_MADEBY;
    s.compression_method = MZ_COMPRESS_METHOD_DEFLATE;
    s.filename = filename.c_str();
    int16_t compressLevel = 9;
    uint8_t raw = 0;
    if (MZ_OK != mz_zip_entry_write_open(fHandle, &s, compressLevel, raw, nullptr)) {
      return false;
    }
    std::vector<uint8_t> buffer(512);
    int32_t err = MZ_OK;
    do {
      size_t read = stream.read(buffer.data(), buffer.size());
      if (read == 0) {
        break;
      }
      int32_t code = mz_zip_entry_write(fHandle, buffer.data(), (int32_t)read);
      if (code < 0) {
        err = code;
      } else if (read < buffer.size()) {
        break;
      }
    } while (err == MZ_OK);

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

  static bool Unzip(
      std::filesystem::path const &input,
      std::filesystem::path const &output,
      std::function<bool(uint64_t done, uint64_t total)> progress = [](uint64_t, uint64_t) { return true; }) {
    using namespace std;
    namespace fs = std::filesystem;

    void *handle = nullptr;
    void *stream = nullptr;
    uint64_t numEntries = 0;
    uint64_t done = 0;

    handle = mz_zip_create(nullptr);
    if (!handle) {
      return false;
    }
    defer {
      mz_zip_delete(&handle);
    };
    stream = mz_stream_os_create(nullptr);
    if (!stream) {
      return false;
    }
    defer {
      mz_stream_os_delete(&stream);
    };
    if (mz_stream_os_open(stream, (char const *)input.u8string().c_str(), MZ_OPEN_MODE_READ) != MZ_OK) {
      return false;
    }
    if (mz_zip_open(handle, stream, MZ_OPEN_MODE_READ) != MZ_OK) {
      return false;
    }
    if (mz_zip_get_number_entry(handle, &numEntries) != MZ_OK) {
      return false;
    }

    if (mz_zip_goto_first_entry(handle) != MZ_OK) {
      // no entry in file
      return true;
    }
    if (!progress(0, numEntries)) {
      return false;
    }
    do {
      mz_zip_file *fileInfo = nullptr;
      if (mz_zip_entry_get_info(handle, &fileInfo) != MZ_OK) {
        return false;
      }
      string relFilePath;
      relFilePath.assign(fileInfo->filename, fileInfo->filename_size);
      fs::path fullFilePath = output / relFilePath;
      if (mz_zip_entry_is_dir(handle) == MZ_OK) {
        Fs::CreateDirectories(fullFilePath);
        done++;
        if (progress(done, numEntries)) {
          continue;
        } else {
          break;
        }
      }
      if (mz_zip_entry_read_open(handle, 0, nullptr) != MZ_OK) {
        return false;
      }
      defer {
        mz_zip_entry_close(handle);
      };
      int64_t remaining = fileInfo->uncompressed_size;
      fs::path parentDir = fullFilePath.parent_path();
      Fs::CreateDirectories(parentDir);
      ScopedFile fp(mcfile::File::Open(fullFilePath, mcfile::File::Mode::Write));
      if (!fp) {
        return false;
      }
      vector<uint8_t> chunk(1024);
      while (remaining > 0) {
        int32_t amount = (int32_t)(std::min)(remaining, (int64_t)chunk.size());
        int32_t read = mz_zip_entry_read(handle, chunk.data(), amount);
        if (read < 0) {
          return false;
        }
        if (fwrite(chunk.data(), 1, read, fp) != read) {
          return false;
        }
        remaining -= amount;
      }
      done++;
      if (!progress(done, numEntries)) {
        return false;
      }
    } while (mz_zip_goto_next_entry(handle) == MZ_OK);

    return true;
  }

private:
  void *fHandle;
  void *fStream;
};

} // namespace je2be
