#pragma once

namespace j2b {

class PortalBlocks {
public:
    PortalBlocks(Dimension dim, bool xAxis) : fDimension(dim), fXAxis(xAxis) {}

    void add(int x, int y, int z) {
        fBlocks.emplace(x, y, z);
    }

    void drain(std::vector<Portal>& buffer) {
        while (!fBlocks.empty()) {
            Pos start = *fBlocks.begin();
            Pos bottomNorthWest = lookupBottomNorthWestCorner(start);
            Pos topSouthEast = lookupTopSouthEastCorner(start);
            for (int x = bottomNorthWest.fX; x <= topSouthEast.fX; x++) {
                for (int z = bottomNorthWest.fZ; z <= topSouthEast.fZ; z++) {
                    for (int y = bottomNorthWest.fY; y <= topSouthEast.fY; y++) {
                        Pos p(x, y, z);
                        fBlocks.erase(p);
                    }
                }
            }
            uint8_t span = (uint8_t)(std::max(topSouthEast.fX - bottomNorthWest.fX, topSouthEast.fZ - bottomNorthWest.fZ) + 1);
            Portal portal((int32_t)fDimension, span, bottomNorthWest.fX, bottomNorthWest.fY, bottomNorthWest.fZ, fXAxis ? 1 : 0, fXAxis ? 0 : 1);
            buffer.push_back(portal);
        }
    }

private:
    class Pos {
    public:
        Pos(int x, int y, int z)
            : fX(x), fZ(z), fY(y)
        {}

        bool operator==(Pos const& other) const {
            return fX == other.fX && fZ == other.fZ && fY == other.fY;
        }

    public:
        int fX;
        int fZ;
        int fY;
    };

    class PosHasher {
    public:
        size_t operator()(Pos const& k) const {
            size_t res = 17;
            res = res * 31 + std::hash<int>{}(k.fX);
            res = res * 31 + std::hash<int>{}(k.fY);
            res = res * 31 + std::hash<int>{}(k.fZ);
            return res;
        }
    };

    Pos lookupBottomNorthWestCorner(Pos start) {
        if (fXAxis) {
            return lookupCorner<-1, -1, 0>(start);
        } else {
            return lookupCorner<0, -1, -1>(start);
        }
    }

    Pos lookupTopSouthEastCorner(Pos start) {
        if (fXAxis) {
            return lookupCorner<1, 1, 0>(start);
        } else {
            return lookupCorner<0, 1, 1>(start);
        }
    }

    template<int dx, int dy, int dz>
    Pos lookupCorner(Pos p) {
        int const x0 = p.fX;
        int const y0 = p.fY;
        int const z0 = p.fZ;
        int currentX = x0;
        int currentY = y0;
        int currentZ = z0;
        int width = 1;
        int height = 1;
        while (true) {
            bool expandVertical = true;
            for (int xz = 0; xz < width; xz++) {
                Pos test(x0 + dx * xz, currentY + dy, z0 + dz * xz);
                if (fBlocks.find(test) == fBlocks.end()) {
                    expandVertical = false;
                    break;
                }
            }
            int nextY = currentY;
            if (expandVertical) {
                nextY += dy;
                height++;
            }

            bool expandHorizontal = true;
            for (int y = 0; y < height; y++) {
                Pos test(currentX + dx, y0 + dy * y, currentZ + dz);
                if (fBlocks.find(test) == fBlocks.end()) {
                    expandHorizontal = false;
                    break;
                }
            }
            if (!expandVertical && !expandHorizontal) {
                break;
            }

            width++;
            currentX += dx;
            currentZ += dz;
        }
        return Pos(currentX, currentY, currentZ);
    }

private:
    Dimension const fDimension;
    bool const fXAxis;
    std::unordered_set<Pos, PosHasher> fBlocks;
};

}
