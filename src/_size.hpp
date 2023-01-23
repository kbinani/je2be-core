#pragma once

namespace je2be {

struct Size {
  Size(i32 w, i32 h) : fWidth(w), fHeight(h) {}

  i32 fWidth;
  i32 fHeight;
};

} // namespace je2be
