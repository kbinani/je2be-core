#pragma once

namespace j2b {

class DimensionDataFragment {
public:
    explicit DimensionDataFragment(Dimension dim) : fDim(dim) {}

    void addPortalBlock(int32_t x, int32_t y, int32_t z, bool xAxis) {
        fPortalBlocks.add(x, y, z, xAxis);
    }

    void addMapId(int32_t id) {
        fMapIdList.insert(id);
    }

    void drain(WorldData &wd) {
        wd.fPortals.add(fPortalBlocks, fDim);
        for (auto id : fMapIdList) {
            wd.fMapIdList.insert(id);
        }
    }

public:
    Dimension const fDim;

private:
    PortalBlocks fPortalBlocks;
    std::unordered_set<int32_t> fMapIdList;
};

}
