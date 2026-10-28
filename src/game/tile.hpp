#pragma once

#include "beam.hpp"

namespace aren {

enum class TileType {
    Empty = 0,
    Wall,
    Barrier,
    Glass,
    Sand,
    __MAX__
};

static constexpr float TILE_MAX_HEALTH = 100;

class Tile {
public:
    TileType m_type = TileType::Empty;
    BeamColor m_color = BeamColor::White;
    float m_health = TILE_MAX_HEALTH;

public:
    explicit Tile() = default;
    Tile(TileType type) : m_type(type) {}

    BeamProp beamProp() const;

    BeamColor beamColor() const;
    bool isTransparent() const;
    bool isDamagable() const;

    void takeDamage(float damage);
};

} // namespace aren