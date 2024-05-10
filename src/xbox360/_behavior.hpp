#pragma once

#include <je2be/lce/behavior.hpp>

#include "_directory-iterator.hpp"
#include "lce/_lzx-decoder.hpp"

namespace je2be::xbox360 {

class ConverterBehavior : public je2be::lce::Behavior {
public:
  Status decompressChunk(std::vector<uint8_t> &buffer) const override {
    if (buffer.size() < 4) {
      return JE2BE_ERROR;
    }
    // u32 decompressedSize = mcfile::U32FromBE(Mem::Read<u32>(buffer, 0));
    buffer.erase(buffer.begin(), buffer.begin() + 4);
    size_t decodedSize = lce::detail::LzxDecoder::Decode(buffer);
    if (decodedSize == 0) {
      return JE2BE_ERROR;
    } else {
      return Status::Ok();
    }
  }

  Status loadPlayers(std::filesystem::path const &inputDirectory, std::map<std::filesystem::path, CompoundTagPtr> &outBuffer) const override {
    auto playersFrom = inputDirectory / "players";

    for (DirectoryIterator it(playersFrom); it.valid(); it.next()) {
      if (!it->is_regular_file()) {
        continue;
      }
      auto path = it->path();
      auto stream = std::make_shared<mcfile::stream::FileInputStream>(path);
      auto in = CompoundTag::Read(stream, mcfile::Encoding::Java);
      if (in) {
        outBuffer[path] = in;
      }
    }
    return Status::Ok();
  }
};

} // namespace je2be::xbox360
