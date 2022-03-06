#pragma once

namespace je2be::box360 {

class Savegame {
  Savegame() = delete;

public:
  // Extract region files, level.dat etc. from "Save{datetime}.bin"
  static bool Extract(std::filesystem::path const &saveBinFile, std::filesystem::path const &outputDirectory) {
    try {
      return UnsafeExtract(saveBinFile, outputDirectory);
    } catch (...) {
      return false;
    }
  }

private:
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
    if (LxzDecoder::Decode(buffer) != outputSize) {
      return false;
    }
    output.swap(buffer);
    return true;
  }

  static void DecodeChunk(std::vector<uint8_t> &buffer) {
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

  static bool RecompressRegionFile(std::filesystem::path const &mcr) {
    using namespace std;
    namespace fs = std::filesystem;

    auto parentDir = mcr.parent_path();
    auto mca = fs::path(mcr).replace_extension(".mca");
    vector<uint32_t> index(1024, 0);
    auto s = make_shared<mcfile::stream::FileInputStream>(mcr);

    auto o = make_shared<mcfile::stream::FileOutputStream>(mca);

    auto filename = mcr.filename().string();
    int rx;
    int rz;
    if (sscanf(filename.c_str(), "r.%d.%d.mcr", &rx, &rz) != 2) {
      return false;
    }

    defer {
      Fs::Delete(mcr);
    };

    int writtenSectors = 2;

    int i = 0;
    for (int z = 0; z < 32; z++) {
      for (int x = 0; x < 32; x++, i++) {
        int cx = rx * 32 + x;
        int cz = rz * 32 + z;

        uint64_t pos = i * 4;
        if (!s->seek(pos)) {
          return false;
        }
        uint32_t location = 0;
        if (!s->read(&location, sizeof(location))) {
          return true;
        }
        location = mcfile::U32FromBE(location);
        uint32_t offset = location >> 8;
        uint8_t sectorCount = 0xff & location;
        if (offset == 0 || sectorCount == 0) {
          continue;
        }
        if (!s->seek(offset * 4096)) {
          return false;
        }
        uint32_t size = 0;
        if (!s->read(&size, sizeof(size))) {
          return false;
        }
        size = 0xffffff & mcfile::U32FromBE(size);
        if (size < 4) {
          continue;
        }
        vector<uint8_t> buffer(size);
        if (!s->read(buffer.data(), size)) {
          return false;
        }
        {
          auto rawFile = parentDir / ("c." + to_string(cx) + "." + to_string(cz) + "-0-raw");
          auto raw = make_shared<mcfile::stream::FileOutputStream>(rawFile);
          if (!raw->write(buffer.data(), buffer.size())) {
            return false;
          }
        }
        uint32_t decompressedSize = mcfile::U32FromBE(*(uint32_t *)buffer.data());
        for (int j = 0; j < 4; j++) {
          buffer.erase(buffer.begin());
        }
        size_t decodedSize = LxzDecoder::Decode(buffer);
        if (decodedSize == 0) {
          return false;
        }
        {
          auto chunkFile = parentDir / ("c." + to_string(cx) + "." + to_string(cz) + "-1-decompressed");
          auto chunk = make_shared<mcfile::stream::FileOutputStream>(chunkFile);
          if (!chunk->write(buffer.data(), buffer.size())) {
            return false;
          }
          // OK. the "*-1-decompressed" file is same as quickbms
        }
        DecodeChunk(buffer);
        {
          auto decodedFile = parentDir / ("c." + to_string(cx) + "." + to_string(cz) + "-2-decoded");
          auto decoded = make_shared<mcfile::stream::FileOutputStream>(decodedFile);
          if (!decoded->write(buffer.data(), buffer.size())) {
            return false;
          }
        }
      }
    }

    s.reset();

    if (!o->seek(0)) {
      return false;
    }
    for (int i = 0; i < 1024; i++) {
      uint32_t idx = mcfile::U32BEFromNative(index[i]);
      if (!o->write(&idx, sizeof(idx))) {
        return false;
      }
    }
    o.reset();

    return true;
  }

  static bool RecompressRegionFiles(std::filesystem::path const &directory) {
    using namespace std;
    namespace fs = std::filesystem;

    error_code ec;
    auto itr = fs::recursive_directory_iterator(directory, ec);
    if (ec) {
      return false;
    }
    for (auto const &entry : itr) {
      auto p = entry.path();
      if (!fs::is_regular_file(p, ec)) {
        continue;
      }
      auto name = p.filename().string();
      if (!name.ends_with(".mcr")) {
        continue;
      }
      if (!RecompressRegionFile(p)) {
        return false;
      }
    }
    return true;
  }

  static bool UnsafeExtract(std::filesystem::path const &saveBinFile, std::filesystem::path const &outputDirectory) {
    using namespace std;
    namespace fs = std::filesystem;

    fs::create_directories(outputDirectory);
    auto savegame = outputDirectory / "savegame.dat";
    if (!ExtractSavagameFromSaveBin(saveBinFile, savegame)) {
      return false;
    }
    vector<uint8_t> decompressed;
    if (!DecompressSavegame(savegame, decompressed)) {
      return false;
    }
    Fs::Delete(savegame);

    if (decompressed.size() < 8) {
      return false;
    }
    uint32_t const indexOffset = mcfile::U32FromBE(*(uint32_t *)decompressed.data());
    uint32_t const fileCount = mcfile::U32FromBE(*(uint32_t *)(decompressed.data() + 4));
    for (uint32_t i = 0; i < fileCount; i++) {
      uint32_t pos = indexOffset + i * kIndexBytesPerFile;
      if (!ExtractFile(decompressed, pos, outputDirectory)) {
        return false;
      }
    }
    return RecompressRegionFiles(outputDirectory);
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

  static bool ExtractSavagameFromSaveBin(std::filesystem::path const &saveBinFile, std::filesystem::path const &outputFile) {
    using namespace std;
    namespace fs = std::filesystem;

    auto pkg = make_unique<StfsPackage>(saveBinFile.string());

    auto listing = pkg->GetFileListing();
    auto entry = FindSavegameFileEntry(listing);
    if (!entry) {
      return false;
    }
    pkg->ExtractFile(entry, outputFile.string());
    return true;
  }
};

} // namespace je2be::box360
