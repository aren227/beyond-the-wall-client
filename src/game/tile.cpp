#include "tile.hpp"
#include "world.hpp"
#include "player.hpp"
#include "raymath.h"

namespace aren {

static constexpr float MAX_PROGRESS = 5;
static constexpr float PROGRESS_DOWN_SPEED = 5;
static constexpr float PROGRESS_UP_SPEED = 1;

void Tile::update(const World *world, const Vector2i &pos, float dt) {
    if (m_type == TileType::End) {
        if (m_damaged && world->checkWorldCanBeDone()) {
            m_progress = std::min(m_progress + PROGRESS_UP_SPEED * dt, MAX_PROGRESS);
        } else {
            m_progress = std::max(m_progress - PROGRESS_DOWN_SPEED * dt, 0.0f);
        }

        m_life += Lerp(0.1f, 10.0f, progress()) * dt;
    } else if (m_type == TileType::Consumable) {
        if (world->player()->pos().floor() == pos) {
            m_health = 0;
            world->player()->m_consumedConsumable = true;

            if (m_name == "beam") {
                world->player()->m_beamDisabled = false;
            } else if (m_name == "add_bounces") {
                world->player()->m_bounces++;
            }
        }
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
    if (m_type == TileType::Barrier || m_type == TileType::Void) {
        return BeamColor::Black;
    }
    if (m_type == TileType::End) {
        return BeamColor::Black;
    }
    if (m_type == TileType::WorldPortal) {
        return m_completed ? BeamColor::Yellow : BeamColor::Black;
    }
    return m_color;
}

bool Tile::isTransparent() const {
    switch (m_type) {
        case TileType::Empty:
        case TileType::Glass:
        case TileType::Zone:
        case TileType::PlayerSpawn:
        case TileType::Consumable:
        case TileType::InvisibleBarrier:
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
        case TileType::Consumable:
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