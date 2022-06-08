#pragma once

namespace je2be::tobe {

class OrientedPortalBlocks {
public:
  explicit OrientedPortalBlocks(bool xAxis) : fXAxis(xAxis) {}

  void add(int x, int y, int z) { fBlocks.emplace(x, y, z); }

  void extract(std::vector<Portal> &buffer, mcfile::Dimension dim) {
    while (!fBlocks.empty()) {
      Pos3i start = *fBlocks.begin();
      Pos3i bottomNorthWest = lookupBottomNorthWestCorner(start);
      Pos3i topSouthEast = lookupTopSouthEastCorner(start);
      for (int x = bottomNorthWest.fX; x <= topSouthEast.fX; x++) {
        for (int z = bottomNorthWest.fZ; z <= topSouthEast.fZ; z++) {
          for (int y = bottomNorthWest.fY; y <= topSouthEast.fY; y++) {
            Pos3i p(x, y, z);
            fBlocks.erase(p);
          }
        }
      }
      uint8_t span = (uint8_t)(std::max(topSouthEast.fX - bottomNorthWest.fX, topSouthEast.fZ - bottomNorthWest.fZ) + 1);
      Portal portal((int32_t)dim, span, bottomNorthWest.fX, bottomNorthWest.fY, bottomNorthWest.fZ, fXAxis ? 1 : 0, fXAxis ? 0 : 1);
      buffer.push_back(portal);
    }
  }

  void drain(OrientedPortalBlocks &out) {
    assert(out.fXAxis == fXAxis);
    for (auto b : fBlocks) {
      out.fBlocks.insert(b);
    }
    std::unordered_set<Pos3i, Pos3iHasher>().swap(fBlocks);
  }

  std::shared_ptr<Tag> toNbt() const {
    using namespace std;
    auto tag = Compound();
    CompoundTag &c = *tag;
    c["x"] = make_shared<ByteTag>(fXAxis ? 1 : 0);
    auto blocks = List<Tag::Type::Compound>();
    for (auto const &pos : fBlocks) {
      auto p = Compound();
      (*p)["x"] = make_shared<IntTag>(pos.fX);
      (*p)["y"] = make_shared<IntTag>(pos.fY);
      (*p)["z"] = make_shared<IntTag>(pos.fZ);
      blocks->push_back(p);
    }
    c["blocks"] = blocks;
    return tag;
  }

  static std::optional<OrientedPortalBlocks> FromNbt(Tag const &nbt) {
    using namespace std;
    auto tag = nbt.asCompound();
    if (!tag) {
      return nullopt;
    }
    auto xAxis = tag->boolean("x");
    if (!xAxis) {
      return nullopt;
    }
    auto blocks = tag->listTag("blocks");
    OrientedPortalBlocks ret(*xAxis);
    if (blocks) {
      for (auto const &t : *blocks) {
        auto c = t->asCompound();
        if (!c) {
          return nullopt;
        }
        auto x = c->int32("x");
        auto y = c->int32("y");
        auto z = c->int32("z");
        if (!x || !y || !z) {
          return nullopt;
        }
        Pos3i pos(*x, *y, *z);
        ret.fBlocks.insert(pos);
      }
    }
    return ret;
  }

private:
  Pos3i lookupBottomNorthWestCorner(Pos3i start) {
    if (fXAxis) {
      return lookupCorner<-1, -1, 0>(start);
    } else {
      return lookupCorner<0, -1, -1>(start);
    }
  }

  Pos3i lookupTopSouthEastCorner(Pos3i start) {
    if (fXAxis) {
      return lookupCorner<1, 1, 0>(start);
    } else {
      return lookupCorner<0, 1, 1>(start);
    }
  }

  template <int dx, int dy, int dz>
  Pos3i lookupCorner(Pos3i p) {
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
        Pos3i test(x0 + dx * xz, currentY + dy, z0 + dz * xz);
        if (fBlocks.find(test) == fBlocks.end()) {
          expandVertical = false;
          break;
        }
      }
      if (expandVertical) {
        currentY += dy;
        height++;
      }

      bool expandHorizontal = true;
      for (int y = 0; y < height; y++) {
        Pos3i test(currentX + dx, y0 + dy * y, currentZ + dz);
        if (fBlocks.find(test) == fBlocks.end()) {
          expandHorizontal = false;
          break;
        }
      }
      if (!expandVertical && !expandHorizontal) {
        break;
      }

      if (expandHorizontal) {
        width++;
        currentX += dx;
        currentZ += dz;
      }
    }
    return Pos3i(currentX, currentY, currentZ);
  }

private:
  bool fXAxis;
  std::unordered_set<Pos3i, Pos3iHasher> fBlocks;
};

} // namespace je2be::tobe
