#include <je2be/box360/minecraft-save-info.hpp>

// clang-format off
#include <fstream>

#include "_mem.hpp"
#include "box360/_stfs.hpp"
#include "box360/_stfs-ext.hpp"

#include <minecraft-file.hpp>
// clang-format on

namespace je2be::box360 {

using namespace detail;

class MinecraftSaveInfo::Impl {
  Impl() = delete;

public:
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
      vector<u8> buffer;
      io.Drain(buffer);
      if (buffer.size() < 4) {
        return false;
      }
      size_t pos = 0;
      u32 numFiles = mcfile::U32FromBE(Mem::Read<u32>(buffer, 0));
      pos += 4;

      vector<u32> thumbnailSizeList;

      for (u32 i = 0; i < numFiles; i++) {
        if (pos + 256 + 0x34 > buffer.size()) {
          return false;
        }
        u16string title;
        for (int j = 0; j < 128; j++) {
          char16_t ch = mcfile::U16FromBE(Mem::Read<u16>(buffer, pos + 2 * j));
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

        u32 thumbnailSize = mcfile::U32FromBE(Mem::Read<u32>(buffer, pos));
        thumbnailSizeList.push_back(thumbnailSize);
        pos += 4;

        SaveBin bin;
        bin.fTitle = title;
        bin.fFileName = fileName;
        bins.push_back(bin);
      }

      assert(thumbnailSizeList.size() == numFiles);
      for (u32 i = 0; i < numFiles; i++) {
        u32 size = thumbnailSizeList[i];
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

bool MinecraftSaveInfo::Parse(std::filesystem::path const &saveInfoFilePath, std::vector<SaveBin> &bins) {
  return Impl::Parse(saveInfoFilePath, bins);
}

} // namespace je2be::box360
