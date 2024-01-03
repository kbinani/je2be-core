#pragma once

namespace je2be::box360::detail {

class MemoryIO : public BaseIO {
public:
  void SetPosition(u64 position, std::ios_base::seekdir dir = std::ios_base::beg) override {
    u64 pos;
    switch (dir) {
    case std::ios_base::cur:
      pos = this->position + position;
      break;
    case std::ios_base::end:
      pos = buffer.size() + position;
      break;
    case std::ios_base::beg:
    default:
      pos = position;
      break;
    }
    if (this->position < pos) {
      buffer.resize(pos);
    }
    this->position = pos;
  }

  u64 GetPosition() override {
    return position;
  }

  u64 Length() override {
    return buffer.size();
  }

  void ReadBytes(u8 *outBuffer, u32 len) override {
    if (buffer.size() <= position + len) {
      throw std::string("MemoryIO: index out of range");
    }
    for (u32 i = 0; i < len; i++) {
      outBuffer[i] = buffer[position + i];
    }
    position += len;
  }

  void WriteBytes(u8 *buffer, u32 len) override {
    if (this->buffer.size() <= position + len) {
      this->buffer.resize(position + len);
    }
    for (u32 i = 0; i < len; i++) {
      this->buffer[position + i] = buffer[i];
    }
    position += len;
  }

  void Flush() override {}

  void Close() override {}

  void Drain(std::vector<u8> &buffer) {
    this->buffer.swap(buffer);
    std::vector<u8>().swap(this->buffer);
  }

private:
  std::vector<u8> buffer;
  u64 position = 0;
};

class FileIO2 : public BaseIO {
public:
  explicit FileIO2(std::filesystem::path const &path) : fStream(nullptr) {
    fStream = mcfile::File::Open(path, mcfile::File::Mode::Read);
  }

  void SetPosition(u64 position, std::ios_base::seekdir dir = std::ios_base::beg) override {
    if (!fStream) {
      throw std::string("FileIO2::SetPosition; stream is NULL");
    }
    switch (dir) {
    case std::ios_base::cur:
      if (!mcfile::File::Fseek(fStream, position, SEEK_CUR)) {
        throw std::string("FileIO2::SetPosition; fseek failed");
      }
      break;
    case std::ios_base::end:
      if (!mcfile::File::Fseek(fStream, position, SEEK_END)) {
        throw std::string("FileIO2::SetPosition; fseek failed");
      }
      break;
    case std::ios_base::beg:
    default:
      if (!mcfile::File::Fseek(fStream, position, SEEK_SET)) {
        throw std::string("FileIO2::SetPosition; fseek failed");
      }
      break;
    }
  }

  u64 GetPosition() override {
    if (!fStream) {
      throw std::string("FileIO2::GetPosition; stream is NULL");
    }
    auto p = mcfile::File::Ftell(fStream);
    if (!p) {
      throw std::string("FileIO2::GetPosition; ftell failed");
    }
    return *p;
  }

  u64 Length() override {
    if (!fStream) {
      throw std::string("FileIO2::Length; stream is NULL");
    }
    auto p = mcfile::File::Ftell(fStream);
    if (!p) {
      throw std::string("FileIO2::Legnth; ftell failed (1)");
    }
    if (!mcfile::File::Fseek(fStream, 0, SEEK_END)) {
      throw std::string("FileIO2::Legnth; fseek failed (1)");
    }
    auto result = mcfile::File::Ftell(fStream);
    if (!result) {
      throw std::string("FileIO2::Legnth; ftell failed (2)");
    }
    if (!mcfile::File::Fseek(fStream, *p, SEEK_SET)) {
      throw std::string("FileIO2::Legnth; fseek failed (2)");
    }
    return *result;
  }

  void ReadBytes(u8 *outBuffer, u32 len) override {
    if (!fStream) {
      throw std::string("FileIO2::ReadBytes; stream is NULL");
    }
    if (len == 0) {
      return;
    }
    if (!outBuffer) {
      throw std::string("FileIO2::ReadBytes; outBuffer is NULL");
    }
    if (fread(outBuffer, 1, len, fStream) != len) {
      throw std::string("FileIO2::ReadBytes; fread failed");
    }
  }

  void WriteBytes(u8 *buffer, u32 len) override {
    throw std::string("FileIO2::WriteBytes; unsupported operation");
  }

  void Flush() override {
    if (!fStream) {
      throw std::string("FileIO2::Flush; stream is NULL");
    }
    if (fflush(fStream) != 0) {
      throw std::string("FileIO2::Flush; fflush failed");
    }
  }

  void Close() override {
    if (!fStream) {
      throw std::string("FileIO2::Close; stream is NULL");
    }
    if (fclose(fStream) != 0) {
      throw std::string("FileIO2::Close; fclose failed");
    }
    fStream = nullptr;
  }

private:
  FILE *fStream;
};

} // namespace je2be::box360::detail
