#pragma once

namespace j2b {

struct Size {
    Size(int32_t w, int32_t h) : fWidth(w), fHeight(h) {}

    int32_t fWidth;
    int32_t fHeight;
};

}
