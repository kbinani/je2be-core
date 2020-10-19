#pragma once

namespace j2b {

class Keys {
public:
    static std::string SubChunk(int32_t chunkX, int32_t chunkY, int32_t chunkZ, Dimension dim) {
        std::vector<char> b;
        PlaceXZTag(b, chunkX, chunkZ, 0x2f);
        SetDimension(b, dim);
        uint8_t const y = (uint8_t)(std::min)((std::max)(chunkY, 0), 255);
        b.push_back(y);
        return std::string(b.data(), b.size());
    }

    static std::string Version(int32_t chunkX, int32_t chunkZ, Dimension dim) {
        std::vector<char> b;
        PlaceXZTag(b, chunkX, chunkZ, 0x76);
        SetDimension(b, dim);
        return std::string(b.data(), b.size());
    }

    static std::string Data2D(int32_t chunkX, int32_t chunkZ, Dimension dim) {
        std::vector<char> b;
        PlaceXZTag(b, chunkX, chunkZ, 0x2d);
        SetDimension(b, dim);
        return std::string(b.data(), b.size());
    }

    static std::string BiomeState(int32_t chunkX, int32_t chunkZ, Dimension dim) {
        std::vector<char> b;
        PlaceXZTag(b, chunkX, chunkZ, 0x35);
        SetDimension(b, dim);
        return std::string(b.data(), b.size());
    }

private:
    static void PlaceXZTag(std::vector<char>& out, int32_t chunkX, int32_t chunkZ, uint8_t tag) {
        out.clear();
        out.resize(9);
        *(uint32_t*)out.data() = mcfile::detail::Int32LEFromNative(*(uint32_t*)&chunkX);
        *(uint32_t*)(out.data() + 4) = mcfile::detail::Int32LEFromNative(*(uint32_t*)&chunkZ);
        out[8] = tag;
    }

    static void SetDimension(std::vector<char>& out, Dimension dim) {
        if (dim == Dimension::Overworld) {
            return;
        }
        assert(out.size() == 9);
        uint8_t tag = out[8];
        out.resize(13);
        uint32_t const v = static_cast<uint8_t>(dim);
        *(uint32_t*)(out.data() + 8) = mcfile::detail::Int32LEFromNative(v);
        out[12] = tag;
    }

private:
    Keys() = delete;
};

}
