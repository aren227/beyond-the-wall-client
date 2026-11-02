#pragma once

#include "beam.hpp"

namespace aren {

class PathQuery;
class World;
class Camera;

class Monster {
public:
    Monster(const World *world, const Vector2f &pos);
    virtual ~Monster();

    void update(const PathQuery *query, float dt);
    void render(const Camera &cam);
    void moveDelta(Vector2f delta);

    inline const Vector2f &pos() { return m_pos; }

public:
    const float m_collExtents = 0.2f;
    const float m_collRadius = m_collExtents * std::sqrt(2.0f);

    const float m_hitExtents = 0.35f;
    const float m_hitRadius = m_hitExtents * std::sqrt(2.0f);

private:
    const World *m_world;

    Vector2f m_pos = V2F_ZERO;
    Vector2f m_velocity = V2F_ZERO;
    Vector2f m_dir = V2f(1, 0);

    float m_speed = 5 * 0.5f;
    const float m_accelRate = 1 / 0.1f;
    const float m_brakeRate = 1 / 0.05f;

    BeamColor m_color = BeamColor::White;
};

} // namespace aren;