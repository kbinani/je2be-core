#include <je2be/zip-file.hpp>

#include <je2be/defer.hpp>
#include <je2be/fs.hpp>

#include <mz.h>
#include <mz_os.h>
#include <mz_strm.h>
#include <mz_strm_os.h>
#include <mz_zip.h>

namespace je2be {

ZipFile::ZipFile(std::filesystem::path const &zipFilePath) : fHandle(nullptr), fStream(nullptr) {
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

ZipFile::~ZipFile() {
  close();
}

Status ZipFile::store(std::vector<u8> const &buffer, std::string const &filename, int compressionLevel0To9) {
  mz_zip_file s = {0};
  s.version_madeby = MZ_VERSION_MADEBY;
  s.compression_method = MZ_COMPRESS_METHOD_DEFLATE;
  s.filename = filename.c_str();
  u8 raw = 0;
  if (MZ_OK != mz_zip_entry_write_open(fHandle, &s, compressionLevel0To9, raw, nullptr)) {
    return JE2BE_ERROR;
  }
  i32 totalWritten = 0;
  i32 err = MZ_OK;
  i32 remaining = (i32)buffer.size();
  do {
    i32 code = mz_zip_entry_write(fHandle, buffer.data() + totalWritten, remaining);
    if (code < 0) {
      err = code;
    } else {
      totalWritten += code;
      remaining -= code;
    }
  } while (err == MZ_OK && remaining > 0);

  if (mz_zip_entry_close(fHandle) != MZ_OK) {
    return JE2BE_ERROR;
  }
  if (err != MZ_OK) {
    return JE2BE_ERROR;
  }
  return Status::Ok();
}

Status ZipFile::store(mcfile::stream::InputStream &stream, std::string const &filename, int compressionLevel0To9) {
  mz_zip_file s = {0};
  s.version_madeby = MZ_VERSION_MADEBY;
  s.compression_method = MZ_COMPRESS_METHOD_DEFLATE;
  s.filename = filename.c_str();
  u8 raw = 0;
  if (MZ_OK != mz_zip_entry_write_open(fHandle, &s, compressionLevel0To9, raw, nullptr)) {
    return JE2BE_ERROR;
  }
  std::vector<u8> buffer(512);
  i32 err = MZ_OK;
  do {
    size_t read = stream.read(buffer.data(), buffer.size());
    if (read == 0) {
      break;
    }
    i32 code = mz_zip_entry_write(fHandle, buffer.data(), (i32)read);
    if (code < 0) {
      err = code;
    } else if (read < buffer.size()) {
      break;
    }
  } while (err == MZ_OK);

  if (mz_zip_entry_close(fHandle) != MZ_OK) {
    return JE2BE_ERROR;
  }
  if (err != MZ_OK) {
    return JE2BE_ERROR;
  }
  return Status::Ok();
}

Status ZipFile::close() {
  Status st = Status::Ok();
  if (fHandle) {
    if (mz_zip_close(fHandle) != MZ_OK) {
      st = JE2BE_ERROR;
    }
    mz_zip_delete(&fHandle);
    fHandle = nullptr;
  } else {
    st = JE2BE_ERROR;
  }
  if (fStream) {
    if (mz_stream_close(fStream) != MZ_OK) {
      if (st.ok()) {
        st = JE2BE_ERROR;
      }
    }
    mz_stream_os_delete(&fStream);
    fStream = nullptr;
  } else {
    if (st.ok()) {
      st = JE2BE_ERROR;
    }
  }
  return st;
}

Status ZipFile::Unzip(
    std::filesystem::path const &input,
    std::filesystem::path const &output,
    std::function<bool(u64 done, u64 total)> progress) {
  using namespace std;
  namespace fs = std::filesystem;

  void *handle = nullptr;
  void *stream = nullptr;
  u64 numEntries = 0;
  u64 done = 0;

  handle = mz_zip_create(nullptr);
  if (!handle) {
    return JE2BE_ERROR;
  }
  defer {
    mz_zip_delete(&handle);
  };
  stream = mz_stream_os_create(nullptr);
  if (!stream) {
    return JE2BE_ERROR;
  }
  defer {
    mz_stream_os_delete(&stream);
  };
  if (mz_stream_os_open(stream, (char const *)input.u8string().c_str(), MZ_OPEN_MODE_READ) != MZ_OK) {
    return JE2BE_ERROR;
  }
  if (mz_zip_open(handle, stream, MZ_OPEN_MODE_READ) != MZ_OK) {
    return JE2BE_ERROR;
  }
  if (mz_zip_get_number_entry(handle, &numEntries) != MZ_OK) {
    return JE2BE_ERROR;
  }

  if (mz_zip_goto_first_entry(handle) != MZ_OK) {
    // no entry in file
    return Status::Ok();
  }
  if (!progress(0, numEntries)) {
    return JE2BE_ERROR;
  }
  do {
    mz_zip_file *fileInfo = nullptr;
    if (mz_zip_entry_get_info(handle, &fileInfo) != MZ_OK) {
      return JE2BE_ERROR;
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
      return JE2BE_ERROR;
    }
    defer {
      mz_zip_entry_close(handle);
    };
    i64 remaining = fileInfo->uncompressed_size;
    fs::path parentDir = fullFilePath.parent_path();
    Fs::CreateDirectories(parentDir);
    mcfile::ScopedFile fp(mcfile::File::Open(fullFilePath, mcfile::File::Mode::Write));
    if (!fp) {
      return JE2BE_ERROR;
    }
    vector<u8> chunk(1024);
    while (remaining > 0) {
      i32 amount = (i32)(std::min)(remaining, (i64)chunk.size());
      i32 read = mz_zip_entry_read(handle, chunk.data(), amount);
      if (read < 0) {
        return JE2BE_ERROR;
      }
      if (fwrite(chunk.data(), 1, read, fp.get()) != read) {
        return JE2BE_ERROR;
      }
      remaining -= amount;
    }
    done++;
    if (!progress(done, numEntries)) {
      return JE2BE_ERROR;
    }
  } while (mz_zip_goto_next_entry(handle) == MZ_OK);

  return Status::Ok();
}

Status ZipFile::Zip(
    std::filesystem::path const &inputDirectory,
    std::filesystem::path const &outputZipFile,
    std::function<bool(int done, int total)> progress) {
  namespace fs = std::filesystem;

  std::error_code ec;
  int total = 0;
  for (auto it : fs::recursive_directory_iterator(inputDirectory, ec)) {
    if (fs::is_regular_file(it.path())) {
      total++;
    }
  }
  if (ec) {
    return JE2BE_ERROR;
  }
  if (!progress(0, total)) {
    return JE2BE_ERROR;
  }

  ZipFile file(outputZipFile);
  int done = 0;
  for (auto it : fs::recursive_directory_iterator(inputDirectory, ec)) {
    auto path = it.path();
    if (!fs::is_regular_file(path)) {
      continue;
    }
    fs::path rel = fs::relative(path, inputDirectory, ec);
    if (ec) {
      return JE2BE_ERROR;
    }
    auto stream = std::make_shared<mcfile::stream::FileInputStream>(path);
    int compressionLevel = 9;
    if (auto st = file.store(*stream, rel.string(), compressionLevel); !st.ok()) {
      return st;
    }
    done++;
    if (!progress(done, total)) {
      return JE2BE_ERROR;
    }
  }
  if (ec) {
    return JE2BE_ERROR;
  }
  return file.close();
}

} // namespace je2be
