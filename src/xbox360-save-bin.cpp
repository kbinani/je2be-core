#include "xbox360/_save-bin.hpp"

#include <minecraft-file.hpp>

#include "xbox360/_stfs.hpp"

#include "lce/_lzx-decoder.hpp"
#include "xbox360/_stfs-ext.hpp"

namespace je2be::xbox360 {

class Impl {
public:
  static bool DecompressSavegame(std::vector<u8> &buffer) {
    using namespace std;
    if (buffer.size() < 12) {
      return false;
    }
    // u32 inputSize = mcfile::U32FromBE(Mem::Read<u32>(buffer, 0));
    u32 outputSize = mcfile::U32FromBE(mcfile::Mem::Read<u32>(buffer, 8));
    for (int i = 0; i < 12; i++) {
      buffer.erase(buffer.begin());
    }
    if (lce::detail::LzxDecoder::Decode(buffer) != outputSize) {
      return false;
    }
    return true;
  }

  static xbox360::detail::StfsFileEntry *FindSavegameFileEntry(xbox360::detail::StfsFileListing &listing) {
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

  static std::optional<SaveBin::SavegameInfo> ExtractSavagame(std::filesystem::path const &saveBinFile, std::vector<u8> &buffer) {
    using namespace std;
    namespace fs = std::filesystem;

    try {
      auto pkg = make_unique<xbox360::detail::StfsPackage>(new xbox360::detail::FileIO2(saveBinFile));

      auto listing = pkg->GetFileListing();
      auto entry = FindSavegameFileEntry(listing);
      if (!entry) {
        return nullopt;
      }
      xbox360::detail::MemoryIO out;
      pkg->Extract(entry, out);
      out.Drain(buffer);

      SaveBin::SavegameInfo info;
      info.fCreatedTime = TimePointFromFatTimestamp(entry->createdTimeStamp);
      if (auto meta = pkg->GetMetaData(); meta && meta->thumbnailImage && meta->thumbnailImageSize > 0) {
        info.fThumbnailImage = std::string((char const *)meta->thumbnailImage.get(), meta->thumbnailImageSize);
      }
      return info;
    } catch (...) {
      return nullopt;
    }
  }

  static std::optional<std::chrono::system_clock::time_point> TimePointFromFatTimestamp(u32 fat) {
    using namespace std;
    u32 year = (fat >> 25) + 1980;
    u32 month = 0xf & (fat >> 21);
    u32 day = 0x1f & (fat >> 16);
    u32 hour = 0x1f & (fat >> 11);
    u32 minute = 0x3f & (fat >> 5);
    u32 second = (0x1f & fat) * 2;

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

std::optional<SaveBin::SavegameInfo> SaveBin::ExtractSavagame(std::filesystem::path const &saveBinFile, std::vector<u8> &buffer) {
  return Impl::ExtractSavagame(saveBinFile, buffer);
}

bool SaveBin::DecompressSavegame(std::vector<u8> &buffer) {
  return Impl::DecompressSavegame(buffer);
}

} // namespace je2be::xbox360
