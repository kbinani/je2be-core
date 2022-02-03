#pragma once

namespace je2be::toje {

class Entity {
public:
  static std::shared_ptr<CompoundTag> ItemFrameFromBedrock(mcfile::Dimension d, Pos3i pos, mcfile::be::Block const &blockJ, CompoundTag const &blockEntityB, Context &ctx) {
    using namespace props;
    auto ret = std::make_shared<CompoundTag>();
    CompoundTag &t = *ret;
    if (blockJ.fName == "minecraft:glow_frame") {
      t["id"] = String("minecraft:glow_item_frame");
    } else {
      t["id"] = String("minecraft:item_frame");
    }

    int32_t facingDirectionA = blockJ.fStates->int32("facing_direction", 0);
    Facing6 f6 = Facing6FromBedrockFacingDirectionA(facingDirectionA);
    Rotation rot(0, 0);
    switch (f6) {
    case Facing6::Up:
      rot.fPitch = -90;
      break;
    case Facing6::North:
      rot.fYaw = 180;
      break;
    case Facing6::East:
      rot.fYaw = 270;
      break;
    case Facing6::South:
      rot.fYaw = 0;
      break;
    case Facing6::West:
      rot.fYaw = 90;
      break;
    case Facing6::Down:
      rot.fPitch = 90;
      break;
    }
    t["Rotation"] = rot.toListTag();
    t["Facing"] = Byte(facingDirectionA);

    t["TileX"] = Int(pos.fX);
    t["TileY"] = Int(pos.fY);
    t["TileZ"] = Int(pos.fZ);

    Pos3i direction = Pos3iFromFacing6(f6);
    double thickness = 15.0 / 32.0;
    Pos3d position(pos.fX + 0.5 - direction.fX * thickness, pos.fY + 0.5 - direction.fY * thickness, pos.fZ + 0.5 - direction.fZ * thickness);
    t["Pos"] = position.toListTag();

    XXHash64 h(0);
    h.add(&d, sizeof(d));
    h.add(&position.fX, sizeof(position.fX));
    h.add(&position.fY, sizeof(position.fY));
    h.add(&position.fZ, sizeof(position.fZ));
    auto uuid = Uuid::GenWithU64Seed(h.hash());
    t["UUID"] = uuid.toListTag();

    auto itemB = blockEntityB.compoundTag("Item");
    if (itemB) {
      auto itemJ = Item::From(*itemB, ctx);
      if (itemJ) {
        t["Item"] = itemJ;
      }
    }

    auto itemRotationB = blockEntityB.float32("ItemRotation", 0);
    t["ItemRotation"] = Byte(static_cast<int8_t>(std::roundf(itemRotationB / 45.0f)));

    Pos3d motion(0, 0, 0);
    t["Motion"] = motion.toListTag();

    t["Air"] = Short(300);
    t["FallDistance"] = Float(0);
    t["Fire"] = Short(-1);
    t["Fixed"] = Bool(false);
    t["Invisible"] = Bool(false);
    t["Invulnerable"] = Bool(false);
    t["ItemDropChance"] = Bool(true);
    t["OnGround"] = Bool(false);
    t["PortalCooldown"] = Int(0);

    return ret;
  }
};

} // namespace je2be::toje
