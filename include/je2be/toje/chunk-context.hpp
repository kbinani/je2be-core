#pragma once

namespace je2be::toje {

class ChunkContext {
public:
  explicit ChunkContext(Context &ctx) : fCtx(ctx) {}

  Context &fCtx;
  std::unordered_map<Uuid, std::map<size_t, Uuid>, UuidHasher, UuidPred> fPassengers;
};

} // namespace je2be::toje
