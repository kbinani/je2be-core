#pragma once

namespace je2be::tobe {

class PortalBlocks {
public:
  PortalBlocks() : fX(true), fZ(false) {}

  void add(int x, int y, int z, bool xAxis) {
    if (xAxis) {
      fX.add(x, y, z);
    } else {
      fZ.add(x, y, z);
    }
  }

  void extract(std::vector<Portal> &buffer, mcfile::Dimension dim) {
    fX.extract(buffer, dim);
    fZ.extract(buffer, dim);
  }

  void drain(PortalBlocks &out) {
    fX.drain(out.fX);
    fZ.drain(out.fZ);
  }

  std::shared_ptr<mcfile::nbt::Tag> toNbt() const {
    auto tag = std::make_shared<mcfile::nbt::CompoundTag>();
    (*tag)["x"] = fX.toNbt();
    (*tag)["z"] = fZ.toNbt();
    return tag;
  }

  static std::optional<PortalBlocks> FromNbt(mcfile::nbt::Tag const &nbt) {
    using namespace std;
    auto tag = nbt.asCompound();
    if (!tag) {
      return nullopt;
    }
    auto xTag = tag->compoundTag("x");
    auto zTag = tag->compoundTag("z");
    if (!xTag || !zTag) {
      return nullopt;
    }
    auto x = OrientedPortalBlocks::FromNbt(*xTag);
    auto z = OrientedPortalBlocks::FromNbt(*zTag);
    if (!x || !z) {
      return nullopt;
    }
    return PortalBlocks(*x, *z);
  }

private:
  PortalBlocks(OrientedPortalBlocks x, OrientedPortalBlocks z) : fX(x), fZ(z) {}

private:
  OrientedPortalBlocks fX;
  OrientedPortalBlocks fZ;
};

} // namespace je2be::tobe
