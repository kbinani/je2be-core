#include <je2be/box360/savegame.hpp>

// clang-format off
#include <fstream>
#include <je2be/mem.hpp>

#include <je2be/box360/lzx-decoder.hpp>
#include <je2be/box360/stfs.hpp>
#include <je2be/box360/stfs-ext.hpp>
#include <je2be/fs.hpp>
// clang-format on

namespace je2be::box360 {

class Savegame::Impl {
  Impl() = delete;

public:
  static bool DecompressSavegame(std::vector<uint8_t> &buffer) {
    using namespace std;
    if (buffer.size() < 12) {
      return false;
    }
    uint32_t inputSize = mcfile::U32FromBE(Mem::Read<uint32_t>(buffer, 0));
    uint32_t outputSize = mcfile::U32FromBE(Mem::Read<uint32_t>(buffer, 8));
    for (int i = 0; i < 12; i++) {
      buffer.erase(buffer.begin());
    }
    if (LzxDecoder::Decode(buffer) != outputSize) {
      return false;
    }
    return true;
  }

  static bool DecompressRawChunk(std::vector<uint8_t> &buffer) {
    if (buffer.size() < 4) {
      return false;
    }
    uint32_t decompressedSize = mcfile::U32FromBE(Mem::Read<uint32_t>(buffer, 0));
    for (int j = 0; j < 4; j++) {
      buffer.erase(buffer.begin());
    }
    size_t decodedSize = LzxDecoder::Decode(buffer);
    if (decodedSize == 0) {
      return false;
    } else {
      return true;
    }
  }

  static void DecodeDecompressedChunk(std::vector<uint8_t> &buffer) {
    // This is a port of ExpandX function from https://sourceforge.net/projects/xboxtopcminecraftconverter/
    std::vector<uint8_t> out;
    int i = 0;
    while (i < buffer.size()) {
      uint8_t b = buffer[i];
      if (b != 0xff) {
        out.push_back(b);
        i++;
        continue;
      }
      if (i + 1 >= buffer.size()) {
        out.push_back(b);
        break;
      }
      uint8_t count = buffer[i + 1];
      if (count >= 3) {
        if (i + 2 >= buffer.size()) {
          out.push_back(count);
          break;
        }
        uint8_t repeat = buffer[i + 2];
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

  static bool ExtractRawChunkFromRegionFile(mcfile::stream::InputStream &stream, int x, int z, std::vector<uint8_t> &buffer) {
    if (x < 0 || 32 <= x || z < 0 || 32 <= z) {
      return false;
    }
    buffer.clear();
    int idx = x + 32 * z;
    uint64_t pos = idx * 4;
    if (!stream.seek(pos)) {
      return false;
    }
    uint32_t location = 0;
    if (!stream.read(&location, sizeof(location))) {
      return true;
    }
    location = mcfile::U32FromBE(location);
    uint32_t offset = location >> 8;
    uint8_t sectorCount = 0xff & location;
    if (offset == 0 || sectorCount == 0) {
      return true;
    }
    if (!stream.seek(offset * 4096)) {
      return false;
    }
    uint32_t size = 0;
    if (!stream.read(&size, sizeof(size))) {
      return false;
    }
    size = 0xffffff & mcfile::U32FromBE(size);
    if (size < 4) {
      return false;
    }
    buffer.resize(size);
    return stream.read(buffer.data(), size);
  }

  static bool ExtractFilesFromDecompressedSavegame(std::vector<uint8_t> const &savegame, std::filesystem::path const &outputDirectory) {
    if (savegame.size() < 8) {
      return false;
    }
    uint32_t const indexOffset = mcfile::U32FromBE(Mem::Read<uint32_t>(savegame, 0));
    uint32_t const fileCount = mcfile::U32FromBE(Mem::Read<uint32_t>(savegame, 4));
    for (uint32_t i = 0; i < fileCount; i++) {
      uint32_t pos = indexOffset + i * kIndexBytesPerFile;
      if (!ExtractFile(savegame, pos, outputDirectory)) {
        return false;
      }
    }
    return true;
  }

  static constexpr uint32_t kIndexBytesPerFile = 144;
  static constexpr uint32_t kFileNameLength = 56;

  static bool ExtractFile(std::vector<uint8_t> const &buffer, uint32_t indexPosition, std::filesystem::path outputDirectory) {
    using namespace std;
    namespace fs = std::filesystem;
    if (indexPosition + kIndexBytesPerFile > buffer.size()) {
      return false;
    }
    uint32_t size = mcfile::U32FromBE(Mem::Read<uint32_t>(buffer, indexPosition + 0x80));
    uint32_t offset = mcfile::U32FromBE(Mem::Read<uint32_t>(buffer, indexPosition + 0x84));
    if (offset + size > buffer.size()) {
      return false;
    }
    u16string name;
    for (uint32_t i = 0; i < kFileNameLength; i++) {
      char16_t c = mcfile::U16FromBE(Mem::Read<uint16_t>(buffer, indexPosition + i * 2));
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

  static StfsFileEntry *FindSavegameFileEntry(StfsFileListing &listing) {
    for (auto &file : listing.fileEntries) {
      if (file.name == "savegame.dat") {
        return &file;
      }
    }
    for (auto &folder : listing.folderEntries) {
      if (auto entry = FindSavegameFileEntry(folder); entry) {
        return entry;
      }
    }
    return nullptr;
  }

  static std::optional<SavegameInfo> ExtractSavagameFromSaveBin(std::filesystem::path const &saveBinFile, std::vector<uint8_t> &buffer) {
    using namespace std;
    namespace fs = std::filesystem;

    try {
      auto pkg = make_unique<StfsPackage>(new FileIO2(saveBinFile));

      auto listing = pkg->GetFileListing();
      auto entry = FindSavegameFileEntry(listing);
      if (!entry) {
        return nullopt;
      }
      MemoryIO out;
      pkg->Extract(entry, out);
      out.Drain(buffer);

      SavegameInfo info;
      info.fCreatedTime = TimePointFromFatTimestamp(entry->createdTimeStamp);
      if (auto meta = pkg->GetMetaData(); meta) {
        info.fThumbnailImage = std::string((char const *)meta->thumbnailImage, meta->thumbnailImageSize);
      }
      return info;
    } catch (...) {
      return nullopt;
    }
  }

  static std::optional<std::chrono::system_clock::time_point> TimePointFromFatTimestamp(uint32_t fat) {
    using namespace std;
    uint32_t year = (fat >> 25) + 1980;
    uint32_t month = 0xf & (fat >> 21);
    uint32_t day = 0x1f & (fat >> 16);
    uint32_t hour = 0x1f & (fat >> 11);
    uint32_t minute = 0x3f & (fat >> 5);
    uint32_t second = (0x1f & fat) * 2;

#if defined(__GNUC__)
    std::tm tm{};
    tm.tm_year = year - 1900;
    tm.tm_mon = month - 1;
    tm.tm_mday = day;
    tm.tm_hour = hour;
    tm.tm_min = minute;
    tm.tm_sec = second;
    tm.tm_isdst = 0;
#if defined(_MSC_VER)
    std::time_t t = _mkgmtime(&tm);
#else
    std::time_t t = timegm(&tm);
#endif
    if (t == (std::time_t)-1) {
      return nullopt;
    }
    return std::chrono::system_clock::from_time_t(t);
#else
    auto ymd = chrono::year(year) / chrono::month(month) / chrono::day(day);
    if (!ymd.ok()) {
      return nullopt;
    }
    return chrono::sys_days(ymd) + chrono::hours(hour) + chrono::minutes(minute) + chrono::seconds(second);
#endif
  }
};

bool Savegame::DecompressSavegame(std::vector<uint8_t> &buffer) {
  return Impl::DecompressSavegame(buffer);
}

bool Savegame::DecompressRawChunk(std::vector<uint8_t> &buffer) {
  return Impl::DecompressRawChunk(buffer);
}

void Savegame::DecodeDecompressedChunk(std::vector<uint8_t> &buffer) {
  return Impl::DecodeDecompressedChunk(buffer);
}

bool Savegame::ExtractRawChunkFromRegionFile(mcfile::stream::InputStream &stream, int x, int z, std::vector<uint8_t> &buffer) {
  return Impl::ExtractRawChunkFromRegionFile(stream, x, z, buffer);
}

bool Savegame::ExtractFilesFromDecompressedSavegame(std::vector<uint8_t> const &savegame, std::filesystem::path const &outputDirectory) {
  return Impl::ExtractFilesFromDecompressedSavegame(savegame, outputDirectory);
}

bool Savegame::ExtractFile(std::vector<uint8_t> const &buffer, uint32_t indexPosition, std::filesystem::path outputDirectory) {
  return Impl::ExtractFile(buffer, indexPosition, outputDirectory);
}

std::optional<Savegame::SavegameInfo> Savegame::ExtractSavagameFromSaveBin(std::filesystem::path const &saveBinFile, std::vector<uint8_t> &buffer) {
  return Impl::ExtractSavagameFromSaveBin(saveBinFile, buffer);
}

std::optional<std::chrono::system_clock::time_point> Savegame::TimePointFromFatTimestamp(uint32_t fat) {
  return Impl::TimePointFromFatTimestamp(fat);
}

} // namespace je2be::box360
