#pragma once

namespace j2b {

class Structures {
public:
  void add(StructurePiece p, Dimension dim) {
    switch (dim) {
    case Dimension::Overworld:
      fOverworld.add(p);
      break;
    case Dimension::Nether:
      fNether.add(p);
      break;
    case Dimension::End:
      fEnd.add(p);
      break;
    }
  }

  void put(DbInterface &db) {
    fOverworld.put(db, Dimension::Overworld);
    fNether.put(db, Dimension::Nether);
    fEnd.put(db, Dimension::End);
  }

private:
  StructurePieceCollection fOverworld;
  StructurePieceCollection fNether;
  StructurePieceCollection fEnd;
};

} // namespace j2b
