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

    inline const Vector2f &pos() { return m_pos; }
    void teleport(const Vector2f &dst);

public:
    int m_bounces = 2;

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