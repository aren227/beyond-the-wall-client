#include "tile.hpp"

namespace aren {

BeamProp Tile::beamProp() const {
    return {
        .color = beamColor(),
        .strength = 1.0f,
    };
}

BeamColor Tile::beamColor() const {
    if (m_type == TileType::Barrier) {
        return BeamColor::Black;
    }
    return m_color;
}

bool Tile::isTransparent() const {
    switch (m_type) {
        case TileType::Glass:
            return true;
        default:
            return false;
    }
}

} // namespace aren