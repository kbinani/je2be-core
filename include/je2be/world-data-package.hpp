#pragma once

namespace j2b {

class WorldDataPackage {
public:
    WorldDataPackage() : fPortalBlocks(std::make_shared<PortalBlocks>())
    {}

    std::shared_ptr<PortalBlocks> fPortalBlocks;
};

}
