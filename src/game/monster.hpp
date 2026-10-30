#pragma once

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
    inline float collRadius() { return m_collisionRadius; }

private:
    const World *m_world;

    Vector2f m_pos = V2F_ZERO;
    Vector2f m_velocity = V2F_ZERO;
    Vector2f m_dir = V2f(1, 0);

    float m_speed = 5 * 0.5f;
    const float m_accelRate = 1 / 0.1f;
    const float m_brakeRate = 1 / 0.05f;

    const float m_collisionExtents = 0.2f;
    const float m_collisionRadius = m_collisionExtents * std::sqrt(2.0f);
};

} // namespace aren;