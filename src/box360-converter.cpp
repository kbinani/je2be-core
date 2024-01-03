#include <je2be/box360/converter.hpp>

#include <je2be/fs.hpp>
#include <je2be/lce/converter.hpp>
#include <je2be/lce/options.hpp>
#include <je2be/lce/progress.hpp>

#include <minecraft-file.hpp>

#include <defer.hpp>

#include "box360/_save-bin.hpp"

namespace je2be::box360 {

Status Converter::Run(std::filesystem::path const &inputSaveBin,
                      std::filesystem::path const &outputDirectory,
                      unsigned int concurrency,
                      je2be::lce::Options const &options,
                      je2be::lce::Progress *progress) {
  using namespace std;

  std::vector<u8> buffer;
  auto savegameInfo = box360::SaveBin::ExtractSavagame(inputSaveBin, buffer);
  if (!savegameInfo) {
    return JE2BE_ERROR;
  }
  if (auto thumbnail = savegameInfo->fThumbnailImage; thumbnail) {
    auto iconPath = outputDirectory / "icon.png";
    auto icon = make_shared<mcfile::stream::FileOutputStream>(iconPath);
    if (!icon->write(thumbnail->data(), thumbnail->size())) {
      return JE2BE_ERROR;
    }
  }
  auto lastPlayed = savegameInfo->fCreatedTime;

  if (!box360::SaveBin::DecompressSavegame(buffer)) {
    return JE2BE_ERROR;
  }

  auto tempRoot = options.getTempDirectory();
  auto decompressed = tempRoot / Uuid::Gen().toString();
  {
    mcfile::ScopedFile fp(mcfile::File::Open(decompressed, mcfile::File::Mode::Write));
    if (!fp) {
      return JE2BE_ERROR;
    }
    if (!mcfile::File::Fwrite(buffer.data(), 1, buffer.size(), fp.get())) {
      return JE2BE_ERROR;
    }
    vector<uint8_t>().swap(buffer);
  }
  defer {
    Fs::Delete(decompressed);
  };

  je2be::lce::Options o = options;
  o.fLastPlayed = savegameInfo->fCreatedTime;
  return lce::Converter::Run(decompressed, outputDirectory, concurrency, o, progress);
}

} // namespace je2be::box360
