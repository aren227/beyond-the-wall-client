#pragma once

namespace aren {

class PathQuery;
class World;
class Camera;

class Monster {
public:
    Monster(const Vector2f &pos);
    virtual ~Monster();

    void update(const PathQuery *query, const World *world, float dt);
    void render(const Camera &cam);

private:
    Vector2f m_pos = V2F_ZERO;
    Vector2f m_velocity = V2F_ZERO;
    Vector2f m_dir = V2f(1, 0);

    float m_speed = 5 * 0.5f;
    const float m_accelRate = 1 / 0.015f;
    const float m_brakeRate = 1 / 0.02f;
};

} // namespace aren;