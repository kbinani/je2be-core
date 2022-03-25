#pragma once

namespace je2be::box360 {

class MinecraftSaveInfo {
  MinecraftSaveInfo() = delete;

public:
  struct SaveBin {
    std::u16string fTitle;
    std::string fFileName;
    std::string fThumbnailData;
  };

  static bool Parse(std::filesystem::path const &saveInfoFilePath, std::vector<SaveBin> &bins) {
    using namespace std;
    try {
      auto pkg = make_unique<StfsPackage>(new FileIO2(saveInfoFilePath));
      auto const &listing = pkg->GetFileListing();
      if (listing.fileEntries.size() != 1) {
        return true;
      }
      auto entry = listing.fileEntries[0];
      MemoryIO io;
      pkg->Extract(&entry, io);
      vector<uint8_t> buffer;
      io.Drain(buffer);
      if (buffer.size() < 4) {
        return false;
      }
      size_t pos = 0;
      uint32_t numFiles = mcfile::U32FromBE(*(uint32_t *)buffer.data());
      pos += 4;

      vector<uint32_t> thumbnailSizeList;

      for (uint32_t i = 0; i < numFiles; i++) {
        if (pos + 256 + 0x34 > buffer.size()) {
          return false;
        }
        u16string title;
        for (int j = 0; j < 128; j++) {
          char16_t ch = mcfile::U16FromBE(*(uint16_t *)(buffer.data() + pos + 2 * j));
          if (ch == 0) {
            break;
          }
          title.push_back(ch);
        }
        pos += 256;

        string fileName;
        for (int j = 0; j < 22; j++) {
          fileName.push_back(buffer[pos]);
          pos++;
        }

        pos += 26;

        uint32_t thumbnailSize = mcfile::U32FromBE(*(uint32_t *)(buffer.data() + pos));
        thumbnailSizeList.push_back(thumbnailSize);
        pos += 4;

        SaveBin bin;
        bin.fTitle = title;
        bin.fFileName = fileName;
        bins.push_back(bin);
      }

      assert(thumbnailSizeList.size() == numFiles);
      for (uint32_t i = 0; i < numFiles; i++) {
        uint32_t size = thumbnailSizeList[i];
        if (pos + size > buffer.size()) {
          return false;
        }
        copy_n(buffer.data() + pos, size, back_inserter(bins[i].fThumbnailData));
        pos += size;
      }

      return true;
    } catch (...) {
      return false;
    }
  }
};

} // namespace je2be::box360
