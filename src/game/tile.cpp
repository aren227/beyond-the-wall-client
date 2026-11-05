#include "tile.hpp"
#include "raymath.h"

namespace aren {

static constexpr float MAX_PROGRESS = 5;
static constexpr float PROGRESS_DOWN_SPEED = 5;
static constexpr float PROGRESS_UP_SPEED = 1;

void Tile::update(float dt) {
    if (m_type == TileType::End) {
        if (m_damaged) {
            m_progress = std::min(m_progress + PROGRESS_UP_SPEED * dt, MAX_PROGRESS);
        } else {
            m_progress = std::max(m_progress - PROGRESS_DOWN_SPEED * dt, 0.0f);
        }

        m_life += Lerp(0.1f, 10.0f, progress()) * dt;
    }

    m_damaged = false;
}

float Tile::progress() const {
    return m_progress / MAX_PROGRESS;
}

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
    if (m_type == TileType::End) {
        return BeamColor::Black;
    }
    return m_color;
}

bool Tile::isTransparent() const {
    switch (m_type) {
        case TileType::Empty:
        case TileType::Glass:
        case TileType::Zone:
        case TileType::PlayerSpawn:
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
        case TileType::Zone:
        case TileType::PlayerSpawn:
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

void Tile::takeDamage(float dt, float damage) {
    if (isDamagable()) {
        m_health = std::max(m_health - damage, 0.0f);
    }

    m_damaged = true;
}

} // namespace aren