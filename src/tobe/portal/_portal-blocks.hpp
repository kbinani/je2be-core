#pragma once

#include "tobe/portal/_oriented-portal-blocks.hpp"

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

private:
  PortalBlocks(OrientedPortalBlocks x, OrientedPortalBlocks z) : fX(x), fZ(z) {}

private:
  OrientedPortalBlocks fX;
  OrientedPortalBlocks fZ;
};

} // namespace je2be::tobe
