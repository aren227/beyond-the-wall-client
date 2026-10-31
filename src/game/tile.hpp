#pragma once

#include "beam.hpp"
#include "object_layer.hpp"

namespace aren {

enum class TileType {
    Empty = 0,
    Wall,
    Barrier,
    Glass,
    Sand,
    Zone,
    __MAX__
};

static constexpr float TILE_MAX_HEALTH = 100;

class Tile {
public:
    TileType m_type = TileType::Empty;
    BeamColor m_color = BeamColor::White;
    int m_multiplier = 1;
    float m_health = TILE_MAX_HEALTH;
    std::array<char, 8> m_name = {};

public:
    explicit Tile() = default;
    Tile(TileType type) : m_type(type) {}

    BeamProp beamProp() const;

    BeamColor beamColor() const;
    bool isTransparent() const;
    bool isDamagable() const;
    bool isSolid() const;

    bool checkCollision(ObjectLayer layer) const;

    void takeDamage(float damage);
};

} // namespace aren