#pragma once

namespace aren {

class Camera;

class Player {
public:
    void update(float dt);
    void render(const Camera &cam);

private:
    Vector2f m_pos = V2F_ZERO;
    Vector2f m_velocity = V2F_ZERO;

    float m_speed = 5;
    float m_accelRate = 1 / 0.015f;
    float m_brakeRate = 1 / 0.02f;
};

} // namespace aren