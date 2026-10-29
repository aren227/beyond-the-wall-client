#pragma once

namespace aren {

class PathQuery;
class Camera;

class Monster {
public:
    void update(const PathQuery &query, float dt);
    void render(const Camera &cam);

private:
    Vector2f m_pos = V2F_ZERO;
    Vector2f m_velocity = V2F_ZERO;

    float m_speed = 5;
    const float m_accelRate = 1 / 0.015f;
    const float m_brakeRate = 1 / 0.02f;
};

} // namespace aren;