#pragma once

namespace je2be::box360 {

class Savegame {
  Savegame() = delete;

public:
  static bool DecompressSavegame(std::filesystem::path const &input, std::vector<uint8_t> &output) {
    using namespace std;
    vector<uint8_t> buffer;
    if (!file::GetContents(input, buffer)) {
      return false;
    }
    if (buffer.size() < 12) {
      return false;
    }
    uint32_t inputSize = mcfile::U32FromBE(*(uint32_t *)buffer.data());
    uint32_t outputSize = mcfile::U32FromBE(*(uint32_t *)(buffer.data() + 8));
    for (int i = 0; i < 12; i++) {
      buffer.erase(buffer.begin());
    }
    if (LzxDecoder::Decode(buffer) != outputSize) {
      return false;
    }
    output.swap(buffer);
    return true;
  }

  static bool DecompressRawChunk(std::vector<uint8_t> &buffer) {
    if (buffer.size() < 4) {
      return false;
    }
    uint32_t decompressedSize = mcfile::U32FromBE(*(uint32_t *)buffer.data());
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
    uint32_t const indexOffset = mcfile::U32FromBE(*(uint32_t *)savegame.data());
    uint32_t const fileCount = mcfile::U32FromBE(*(uint32_t *)(savegame.data() + 4));
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
    uint32_t size = mcfile::U32FromBE(*(uint32_t *)(buffer.data() + indexPosition + 0x80));
    uint32_t offset = mcfile::U32FromBE(*(uint32_t *)(buffer.data() + indexPosition + 0x84));
    if (offset + size > buffer.size()) {
      return false;
    }
    string name;
    for (uint32_t i = 1; i < kFileNameLength; i += 2) {
      char c = buffer[indexPosition + i];
      if (c == 0) {
        break;
      }
      name.push_back(c);
    }
    fs::path file;
    if (name.starts_with("DIM-1")) {
      file = outputDirectory / "DIM-1" / "region" / name.substr(5);
    } else if (name.starts_with("DIM1/")) {
      file = outputDirectory / "DIM1" / "region" / name.substr(5);
    } else if (name.ends_with(".mcr")) {
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

  struct SavegameInfo {
    std::optional<std::string> fThumbnailImage;
    std::optional<std::chrono::system_clock::time_point> fCreatedTime;
  };

  static std::optional<SavegameInfo> ExtractSavagameFromSaveBin(std::filesystem::path const &saveBinFile, std::filesystem::path const &outputFile) {
    using namespace std;
    namespace fs = std::filesystem;

    auto pkg = make_unique<StfsPackage>(saveBinFile.string());

    auto listing = pkg->GetFileListing();
    auto entry = FindSavegameFileEntry(listing);
    if (!entry) {
      return nullopt;
    }
    pkg->ExtractFile(entry, outputFile.string());

    SavegameInfo info;
    info.fCreatedTime = TimePointFromFatTimestamp(entry->createdTimeStamp);
    if (auto meta = pkg->GetMetaData(); meta) {
      info.fThumbnailImage = std::string((char const *)meta->thumbnailImage, meta->thumbnailImageSize);
    }
    return info;
  }

  static std::optional<std::chrono::system_clock::time_point> TimePointFromFatTimestamp(uint32_t fat) {
    using namespace std;
    uint32_t year = (fat >> 25) + 1980;
    uint32_t month = 0xf & (fat >> 21);
    uint32_t day = 0x1f & (fat >> 16);
    uint32_t hour = 0x1f & (fat >> 11);
    uint32_t minute = 0x3f & (fat >> 5);
    uint32_t second = (0x1f & fat) * 2;

    auto ymd = chrono::year(year) / chrono::month(month) / chrono::day(day);
    if (!ymd.ok()) {
      return nullopt;
    }
    return chrono::sys_days(ymd) + chrono::hours(hour) + chrono::minutes(minute) + chrono::seconds(second);
  }
};

} // namespace je2be::box360
