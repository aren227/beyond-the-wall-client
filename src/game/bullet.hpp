#pragma once

namespace aren {

class World;

class Bullet {
public:
    Bullet(const Vector2f &pos, const Vector2f &vel);

    void update(const World *world, float dt);
    void render();

    inline bool done() const { return m_destroy; }

public:
    const float m_radius = 0.15f;

private:
    Vector2f m_pos = V2F_ZERO;
    Vector2f m_velocity = V2F_ZERO;
    bool m_destroy = false;
    float m_damage = 10;
};

};