#include "lce/_savegame.hpp"

// clang-format off

#include <je2be/fs.hpp>

#include <fstream>

#include "_mem.hpp"
// clang-format on

namespace je2be::lce {

class Savegame::Impl {
  Impl() = delete;

public:
  static void DecodeDecompressedChunk(std::vector<u8> &buffer) {
    // This is a port of ExpandX function from https://sourceforge.net/projects/xboxtopcminecraftconverter/
    std::vector<u8> out;
    int i = 0;
    while (i < buffer.size()) {
      u8 b = buffer[i];
      if (b != 0xff) {
        out.push_back(b);
        i++;
        continue;
      }
      if (i + 1 >= buffer.size()) {
        out.push_back(b);
        break;
      }
      u8 count = buffer[i + 1];
      if (count >= 3) {
        if (i + 2 >= buffer.size()) {
          out.push_back(count);
          break;
        }
        u8 repeat = buffer[i + 2];
        for (int j = 0; j <= count; j++) {
          out.push_back(repeat);
        }
        i += 3;
      } else {
        for (int j = 0; j <= count; j++) {
          out.push_back(0xff);
        }
        i += 2;
      }
    }
    buffer.swap(out);
  }

  static bool ExtractRawChunkFromRegionFile(mcfile::stream::InputStream &stream, int x, int z, std::vector<u8> &buffer) {
    if (x < 0 || 32 <= x || z < 0 || 32 <= z) {
      return false;
    }
    buffer.clear();
    int idx = x + 32 * z;
    u64 pos = idx * 4;
    if (!stream.seek(pos)) {
      return false;
    }
    u32 location = 0;
    if (!stream.read(&location, sizeof(location))) {
      return true;
    }
    location = mcfile::U32FromBE(location);
    u32 offset = location >> 8;
    u8 sectorCount = 0xff & location;
    if (offset == 0 || sectorCount == 0) {
      return true;
    }
    if (!stream.seek(offset * 4096)) {
      return false;
    }
    u32 size = 0;
    if (!stream.read(&size, sizeof(size))) {
      return false;
    }
    size = 0xffffff & mcfile::U32FromBE(size);
    if (size < 4) {
      return false;
    }
    buffer.resize(size + 4);
    return stream.read(buffer.data(), size + 4);
  }

  static bool ExtractFilesFromDecompressedSavegame(std::vector<u8> const &savegame, std::filesystem::path const &outputDirectory) {
    if (savegame.size() < 8) {
      return false;
    }
    u32 const indexOffset = mcfile::U32FromBE(Mem::Read<u32>(savegame, 0));
    u32 const fileCount = mcfile::U32FromBE(Mem::Read<u32>(savegame, 4));
    for (u32 i = 0; i < fileCount; i++) {
      u32 pos = indexOffset + i * kIndexBytesPerFile;
      if (!ExtractFile(savegame, pos, outputDirectory)) {
        return false;
      }
    }
    return true;
  }

  static constexpr u32 kIndexBytesPerFile = 144;
  static constexpr u32 kFileNameLength = 56;

  static bool ExtractFile(std::vector<u8> const &buffer, u32 indexPosition, std::filesystem::path outputDirectory) {
    using namespace std;
    namespace fs = std::filesystem;
    if (indexPosition + kIndexBytesPerFile > buffer.size()) {
      return false;
    }
    u32 size = mcfile::U32FromBE(Mem::Read<u32>(buffer, indexPosition + 0x80));
    u32 offset = mcfile::U32FromBE(Mem::Read<u32>(buffer, indexPosition + 0x84));
    if (offset + size > buffer.size()) {
      return false;
    }
    u16string name;
    for (u32 i = 0; i < kFileNameLength; i++) {
      char16_t c = mcfile::U16FromBE(Mem::Read<u16>(buffer, indexPosition + i * 2));
      if (c == 0) {
        break;
      }
      name.push_back(c);
    }
    fs::path file;
    if (name.starts_with(u"DIM-1")) {
      file = outputDirectory / "DIM-1" / "region" / name.substr(5);
    } else if (name.starts_with(u"DIM1/")) {
      file = outputDirectory / "DIM1" / "region" / name.substr(5);
    } else if (name.ends_with(u".mcr")) {
      file = outputDirectory / "region" / name;
    } else {
      file = outputDirectory / name;
    }
    auto parent = file.parent_path();
    if (!Fs::CreateDirectories(parent)) {
      return false;
    }
    auto stream = make_shared<mcfile::stream::FileOutputStream>(file);
    return stream->write(buffer.data() + offset, size);
  }
};

void Savegame::DecodeDecompressedChunk(std::vector<u8> &buffer) {
  return Impl::DecodeDecompressedChunk(buffer);
}

bool Savegame::ExtractRawChunkFromRegionFile(mcfile::stream::InputStream &stream, int x, int z, std::vector<u8> &buffer) {
  return Impl::ExtractRawChunkFromRegionFile(stream, x, z, buffer);
}

bool Savegame::ExtractFilesFromDecompressedSavegame(std::vector<u8> const &savegame, std::filesystem::path const &outputDirectory) {
  return Impl::ExtractFilesFromDecompressedSavegame(savegame, outputDirectory);
}

} // namespace je2be::lce
