#pragma once

#include "beam.hpp"
#include "object_layer.hpp"

namespace aren {

class World;

enum class TileType {
    Empty = 0,
    Wall,
    Barrier,
    Glass,
    Sand,
    Zone,
    PlayerSpawn,
    End,
    WorldPortal,
    Consumable,
    InvisibleBarrier,
    __MAX__
};

static constexpr float TILE_MAX_HEALTH = 100;

class Tile {
public:
    TileType m_type = TileType::Empty;
    BeamColor m_color = BeamColor::White;
    int m_multiplier = 1;
    float m_health = TILE_MAX_HEALTH;
    std::string m_name;

    bool m_damaged = false;

    // TODO(aren): Very tile specific code..
    float m_progress = 0;
    float m_life = 0;

public:
    explicit Tile() = default;
    Tile(TileType type) : m_type(type) {}

    void update(const World *world, const Vector2i &pos, float dt);

    BeamProp beamProp() const;

    BeamColor beamColor() const;
    bool isTransparent() const;
    bool isDamagable() const;
    bool isSolid() const;

    bool checkCollision(ObjectLayer layer) const;

    void takeDamage(float dt, float damage);
    float progress() const;
};

} // namespace aren