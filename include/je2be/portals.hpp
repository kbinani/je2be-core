#pragma once

namespace j2b {

class Portals {
public:
    Portals()
        : fOverworldX(Dimension::Overworld, true)
        , fOverworldZ(Dimension::Overworld, false)
        , fNetherX(Dimension::Nether, true)
        , fNetherZ(Dimension::Nether, false)
    {}

	void add(int32_t x, int32_t y, int32_t z, mcfile::Block const& block, Dimension dim) {
		auto axis = block.property("axis", "x");
		if (dim == Dimension::Overworld) {
			if (axis == "x") {
				fOverworldX.add(x, y, z);
			} else {
				fOverworldZ.add(x, y, z);
			}
		} else if (dim == Dimension::Nether) {
			if (axis == "x") {
				fNetherX.add(x, y, z);
			} else {
				fNetherZ.add(x, y, z);
			}
		}
	}

	void putInto(Db& db) {
		using namespace std;
        using namespace mcfile::stream;
        using namespace mcfile::nbt;
		vector<Portal> portals;
		fOverworldX.drain(portals);
		fOverworldZ.drain(portals);
		fNetherX.drain(portals);
		fNetherZ.drain(portals);

        auto root = make_shared<CompoundTag>();
        auto data = make_shared<CompoundTag>();
        auto portalRecords = make_shared<ListTag>();
        portalRecords->fType = Tag::TAG_Compound;
        for (auto portal : portals) {
            portalRecords->fValue.push_back(portal.toCompoundTag());
        }
        data->fValue.emplace("PortalRecords", portalRecords);
        root->fValue.emplace("data", data);

        auto s = make_shared<ByteStream>();
        OutputStreamWriter w(s, {.fLittleEndian = true});
        w.write((uint8_t)Tag::TAG_Compound);
        w.write(string());
        root->write(w);
        w.write((uint8_t)Tag::TAG_End);

        auto key = Key::Portals();
        vector<uint8_t> buffer;
        s->drain(buffer);

        leveldb::Slice v((char*)buffer.data(), buffer.size());
        db.put(key, v);
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

	class Portal {
	public:
		Portal(int32_t dimId, uint8_t span, int32_t tpX, int32_t tpY, int32_t tpZ, uint8_t xa, uint8_t za)
			: fDimId(dimId), fSpan(span), fTpX(tpX), fTpY(tpY), fTpZ(tpZ), fXa(xa), fZa(za)
		{}

        std::shared_ptr<mcfile::nbt::CompoundTag> toCompoundTag() const {
            using namespace std;
            using namespace mcfile::nbt;
            using namespace props;
            auto tag = make_shared<CompoundTag>();
            tag->fValue = {
                {"DimId", Int(fDimId)},
                {"Span", Byte(fSpan)},
                {"TpX", Int(fTpX)},
                {"TpY", Int(fTpY)},
                {"TpZ", Int(fTpZ)},
                {"Xa", Byte(fXa)},
                {"Za", Byte(fZa)},
            };
            return tag;
        }

	private:
		int32_t const fDimId;
		uint8_t const fSpan;
		int32_t const fTpX;
		int32_t const fTpY;
		int32_t const fTpZ;
		uint8_t const fXa;
		uint8_t const fZa;
	};

	struct PosHasher {
		size_t operator()(Pos const& k) const {
			size_t res = 17;
            res = res * 31 + std::hash<int>{}(k.fX);
            res = res * 31 + std::hash<int>{}(k.fY);
            res = res * 31 + std::hash<int>{}(k.fZ);
			return res;
		}
	};

	class PortalCandidates {
	public:
		PortalCandidates(Dimension dim, bool xAxis) : fDimension(dim), fXAxis(xAxis) {}

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
                int nextX = currentX;
                int nextZ = currentZ;
                if (expandHorizontal) {
                    nextX += dx;
                    nextZ += dz;
                    width++;
                }
                
                if (!expandVertical && !expandHorizontal) {
                    break;
                }
                
                currentX = nextX;
                currentY = nextY;
                currentZ = nextZ;
            }
            return Pos(currentX, currentY, currentZ);
        }
        
	private:
		Dimension const fDimension;
		bool const fXAxis;
		std::unordered_set<Pos, PosHasher> fBlocks;
	};

private:
	PortalCandidates fOverworldX;
	PortalCandidates fOverworldZ;
	PortalCandidates fNetherX;
	PortalCandidates fNetherZ;
};

}
