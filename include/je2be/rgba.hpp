#pragma once

namespace j2b {

struct Rgba {
    Rgba(uint8_t r, uint8_t g, uint8_t b) : fR(r), fG(g), fB(b), fA(255) {}
    Rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a) : fR(r), fG(g), fB(b), fA(a) {}

    uint8_t fR;
    uint8_t fG;
    uint8_t fB;
    uint8_t fA;

    int32_t toRGB() const {
        uint32_t c = ((uint32_t)fR << 16) | ((uint32_t)fG << 8) | ((uint32_t)fB);
        return *(int32_t*)&c;
    }

    int32_t toARGB() const {
        uint32_t c = 0xff000000 | ((uint32_t)fR << 16) | ((uint32_t)fG << 8) | ((uint32_t)fB);
        return *(int32_t*)&c;
    }
};

}
