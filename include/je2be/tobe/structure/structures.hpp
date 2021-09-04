#pragma once

namespace je2be::tobe {

class Structures {
public:
  void add(StructurePiece p, mcfile::Dimension dim) {
    switch (dim) {
    case mcfile::Dimension::Overworld:
      fOverworld.add(p);
      break;
    case mcfile::Dimension::Nether:
      fNether.add(p);
      break;
    case mcfile::Dimension::End:
      fEnd.add(p);
      break;
    }
  }

  void put(DbInterface &db) {
    fOverworld.put(db, mcfile::Dimension::Overworld);
    fNether.put(db, mcfile::Dimension::Nether);
    fEnd.put(db, mcfile::Dimension::End);
  }

private:
  StructurePieceCollection fOverworld;
  StructurePieceCollection fNether;
  StructurePieceCollection fEnd;
};

} // namespace je2be::tobe
