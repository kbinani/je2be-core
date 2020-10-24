#pragma once

namespace j2b {

class PortalBlocks {
public:
    explicit PortalBlocks() : fX(true), fZ(false)
    {}

    void add(int x, int y, int z, bool xAxis) {
        if (xAxis) {
            fX.add(x, y, z);
        } else {
            fZ.add(x, y, z);
        }
    }

    void extract(std::vector<Portal>& buffer, Dimension dim) {
        fX.extract(buffer, dim);
        fZ.extract(buffer, dim);
    }

    void drain(PortalBlocks& out) {
        fX.drain(out.fX);
        fZ.drain(out.fZ);
    }

private:
    OrientedPortalBlocks fX;
    OrientedPortalBlocks fZ;
};

}
