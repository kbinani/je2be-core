#include <je2be/ps3/converter.hpp>

#include <je2be/fs.hpp>
#include <je2be/lce/converter.hpp>
#include <je2be/lce/options.hpp>
#include <je2be/lce/progress.hpp>

#include "_file.hpp"
#include "ps3/_chunk-decompressor.hpp"

namespace je2be::ps3 {

Status Converter::Run(std::filesystem::path const &inputDirectory,
                      std::filesystem::path const &outputDirectory,
                      unsigned int concurrency,
                      je2be::lce::Options const &options,
                      je2be::lce::Progress *progress) {
  using namespace std;

  auto gamedataFile = inputDirectory / u8"GAMEDATA";
  if (!Fs::Exists(gamedataFile)) {
    return JE2BE_ERROR;
  }

  auto iconFile = inputDirectory / u8"ICON0.PNG";
  if (Fs::Exists(iconFile)) {
    auto iconPath = outputDirectory / "icon.png";
    Fs::Delete(iconPath);
    if (!Fs::Copy(iconFile, iconPath)) {
      return JE2BE_ERROR;
    }
  }

  vector<uint8_t> buffer;
  if (!file::GetContents(gamedataFile, buffer)) {
    return JE2BE_ERROR;
  }
  je2be::lce::Options o = options;
  o.fLastPlayed = Fs::LastWriteTime(gamedataFile);
  je2be::ps3::ChunkDecompressor decompressor;
  return lce::Converter::Run(buffer, outputDirectory, concurrency, decompressor, o, progress);
}

} // namespace je2be::ps3
