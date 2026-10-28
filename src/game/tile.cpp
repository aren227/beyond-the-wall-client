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
        case TileType::Empty:
        case TileType::Glass:
            return true;
        default:
            return false;
    }
}

bool Tile::isDamagable() const {
    switch (m_type) {
        case TileType::Sand:
            return true;
        default:
            return false;
    }
}

bool Tile::isSolid() const {
    switch (m_type) {
        case TileType::Empty:
            return false;
        default:
            return true;
    }
}

bool Tile::checkCollision(ObjectLayer layer) const {
    switch (layer) {
        case ObjectLayer::Light:
            return isTransparent() == false;
        case ObjectLayer::Others:
        case ObjectLayer::Player:
        case ObjectLayer::Monster:
        default:
            return isSolid();
    }
}

void Tile::takeDamage(float damage) {
    if (isDamagable()) {
        m_health = std::max(m_health - damage, 0.0f);
    }
}

} // namespace aren