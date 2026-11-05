#pragma once

#include "beam.hpp"

namespace aren {

class PathQuery;
class World;
class Camera;

struct MonsterDefinition {
    float maxHealth = 100.0f;
    float speed = 1.0f;
    float bulletSpeed = 8.0f;
    float bulletFireCooldown = 0.5f;
    int bulletSpreadCount = 1; // zero means does not shoot
    BeamColor color = BeamColor::White;
};

class Monster {
public:
    Monster(World *world, const std::string &typeName, const Vector2f &pos);
    virtual ~Monster();

    void update(const PathQuery *query, float dt);
    void render(const Camera &cam);
    void moveDelta(Vector2f delta);

    inline const Vector2f &pos() { return m_pos; }

    void takeDamage(float damage);

public:
    const float m_collExtents = 0.2f;
    const float m_collRadius = m_collExtents * std::sqrt(2.0f);

    const float m_hitExtents = 0.35f;
    const float m_hitRadius = m_hitExtents * std::sqrt(2.0f);

    float m_health = 100;
    BeamColor m_color = BeamColor::White;

private:
    World *m_world;

    Vector2f m_pos = V2F_ZERO;
    Vector2f m_velocity = V2F_ZERO;
    Vector2f m_dir = V2f(1, 0);

    float m_speed = 1.0f;
    const float m_accelRate = 1 / 0.1f;
    const float m_brakeRate = 1 / 0.05f;

    float m_bulletSpeed = 8.0f;
    int m_bulletSpreadCount = 1;

    float m_bulletFireDelay = 1.0f;
    float m_bulletFireCooldown = 0.5f;
};

} // namespace aren;