#pragma once

namespace je2be {

template <class Container, int ContainerSize = 16, int ContainerNum = 3>
class LatticeContainerWrapper {
public:
  using ValueType = typename Container::ValueType;

  static_assert(ContainerSize > 0);
  static_assert(ContainerNum > 0);

  explicit LatticeContainerWrapper(Pos3i const &start, int maxY)
      : fStart(start),                                                                                        //
        fEnd(start.fX + ContainerSize * ContainerNum - 1, maxY, start.fZ + ContainerSize * ContainerNum - 1), //
        fChunkStart(LatticeIndex(start.fX), LatticeIndex(start.fZ)),                                          //
        fChunkEnd(LatticeIndex(fStart.fX) + ContainerNum - 1, LatticeIndex(fStart.fZ) + ContainerNum - 1),    //
        fContainers(Pos2i(LatticeIndex(start.fX), LatticeIndex(start.fZ)), ContainerNum, ContainerNum, nullptr) {
    assert(start.fX % ContainerSize == 0);
    assert(start.fZ % ContainerSize == 0);
  }

  ValueType operator[](Pos3i const &p) const {
    int cx = LatticeIndex(p.fX);
    int cz = LatticeIndex(p.fZ);
    assert(fChunkStart.fX <= cx && cx <= fChunkEnd.fX && fChunkStart.fZ <= cz && cz <= fChunkEnd.fZ);
    auto const &container = fContainers[{cx, cz}];
    assert(container);
    return (*container)[p];
  }

  void store(int cx, int cz, std::shared_ptr<Container> container) {
    assert(fChunkStart.fX <= cx && cx <= fChunkEnd.fX && fChunkStart.fZ <= cz && cz <= fChunkEnd.fZ);
    fContainers[{cx, cz}] = container;
  }

  std::shared_ptr<Container> get(int cx, int cz) {
    assert(fChunkStart.fX <= cx && cx <= fChunkEnd.fX && fChunkStart.fZ <= cz && cz <= fChunkEnd.fZ);
    return fContainers[{cx, cz}];
  }

  bool contains(Pos3i const &p) const {
    return fStart.fX <= p.fX && p.fX <= fEnd.fX &&
           fStart.fY <= p.fY && p.fY <= fEnd.fY &&
           fStart.fZ <= p.fZ && p.fZ <= fEnd.fZ;
  }

private:
  static int LatticeIndex(int x) {
    return x < 0 ? ((x + 1) / ContainerSize - 1) : (x / ContainerSize);
  }

public:
  Pos3i const fStart;
  Pos3i const fEnd;
  Pos2i const fChunkStart;
  Pos2i const fChunkEnd;

private:
  Data2d<std::shared_ptr<Container>> fContainers;
};

} // namespace je2be
