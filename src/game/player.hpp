#pragma once

#include "beam.hpp"

namespace aren {

class Camera;
class World;

class Player {
public:
    Player();
    virtual ~Player();

    void update(const Camera &cam, World *world, float dt);
    void render(const Camera &cam);

    inline const Vector2f &pos() const { return m_pos; }
    inline const Vector2f &vel() const { return m_velocity; }
    void teleport(const Vector2f &dst);

    inline const Beam &beam() { return m_beam; }

    void takeDamage(float damage);

public:
    int m_bounces = 2;

    const float m_collExtents = 0.25f;
    const float m_collRadius = m_collExtents * std::sqrt(2.0f);

    const float m_hitExtents = 0.2f;
    const float m_hitRadius = m_hitExtents * std::sqrt(2.0f);

    float m_maxHealth = 100;
    float m_health = m_maxHealth;

private:
    Vector2f m_pos = V2F_ZERO;
    Vector2f m_velocity = V2F_ZERO;

    float m_speed = 5;
    const float m_accelRate = 1 / 0.015f;
    const float m_brakeRate = 1 / 0.02f;

    Vector2f m_dir = V2f(1, 0);

    Beam m_beam;

    float m_baseAttackDamage = 100;
};

} // namespace aren