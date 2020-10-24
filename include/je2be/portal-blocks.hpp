#pragma once

namespace j2b {

class PortalBlocks {
public:
    explicit PortalBlocks(Dimension dim) : fDimension(dim), fX(dim, true), fZ(dim, false)
    {}

    void add(int x, int y, int z, bool xAxis) {
        if (xAxis) {
            fX.add(x, y, z);
        } else {
            fZ.add(x, y, z);
        }
    }

    void extract(std::vector<Portal>& buffer) {
        fX.extract(buffer);
        fZ.extract(buffer);
    }

    void drain(PortalBlocks& out) {
        assert(out.fDimension == fDimension);
        fX.drain(out.fX);
        fZ.drain(out.fZ);
    }

private:
    Dimension const fDimension;
    OrientedPortalBlocks fX;
    OrientedPortalBlocks fZ;
};

}
