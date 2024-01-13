#include <je2be/xbox360/converter.hpp>

#include <je2be/lce/converter.hpp>
#include <je2be/lce/options.hpp>
#include <je2be/lce/progress.hpp>

#include "xbox360/_behavior.hpp"
#include "xbox360/_save-bin.hpp"

namespace je2be::xbox360 {

Status Converter::Run(std::filesystem::path const &inputSaveBin,
                      std::filesystem::path const &outputDirectory,
                      unsigned int concurrency,
                      je2be::lce::Options const &options,
                      je2be::lce::Progress *progress) {
  using namespace std;

  std::vector<u8> buffer;
  auto savegameInfo = xbox360::SaveBin::ExtractSavagame(inputSaveBin, buffer);
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

  if (!xbox360::SaveBin::DecompressSavegame(buffer)) {
    return JE2BE_ERROR;
  }

  je2be::lce::Options o = options;
  o.fLastPlayed = savegameInfo->fCreatedTime;
  xbox360::ConverterBehavior behavior;
  return lce::Converter::Run(buffer, outputDirectory, concurrency, behavior, o, progress);
}

} // namespace je2be::xbox360
