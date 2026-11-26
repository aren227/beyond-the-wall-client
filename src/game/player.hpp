#pragma once

#include "beam.hpp"

namespace aren {

class Camera;
class World;

class Player {
public:
    Player(World *world);
    virtual ~Player();

    void update(Camera &cam, float dt);
    void render(const Camera &cam);

    inline const Vector2f &pos() const { return m_pos; }
    inline const Vector2f &vel() const { return m_velocity; }
    void teleport(const Vector2f &dst);
    void look(const Vector2f &dst);

    inline const Beam &beam() const { return m_beam; }
    inline bool isDashing() const { return m_dashTime > m_dashCooldown; }

    void takeDamage(float damage);
    void setHealth(float health);

public:
    bool m_visible = true;

    int m_bounces = 2;

    const float m_collExtents = 0.25f;
    const float m_collRadius = m_collExtents * std::sqrt(2.0f);

    const float m_hitExtents = 0.2f;
    const float m_hitRadius = m_hitExtents * std::sqrt(2.0f);

    float m_maxHealth = 100;
    float m_health = m_maxHealth;
    float m_prevHealth = m_maxHealth;

    bool m_beamDisabled = false;
    bool m_consumedConsumable = false;
    bool m_hasDashed = false;
    bool m_controlDisabled = false;

private:
    World *m_world = nullptr;

    Vector2f m_pos = V2F_ZERO;
    Vector2f m_velocity = V2F_ZERO;

    float m_speed = 5;
    const float m_accelRate = 1 / 0.01f;
    const float m_brakeRate = 1 / 0.02f;

    const float m_dashSpeed = 17;
    const float m_dashDuration = 0.2f;
    const float m_dashCooldown = 0.5f;
    Vector2f m_dashDir = V2F_ZERO;
    float m_dashTime = 0;

    float m_immortalTime = 0;
    const float m_hitImmortalDuration = 1.0f;

    Vector2f m_dir = V2f(1, 0);

    Beam m_beam;
    // const Color4f m_beamBaseColor = C4F_WHITE.brighten(2);

    float m_baseAttackDamage = 100;

    float m_laserVolume = 0.0f;

    float m_afterimageDelay = 0.0f;
    struct PoseSnapshot {
        float maxTimeLeft = 0.5f;
        float timeLeft = maxTimeLeft;
        Vector2f pos;
        Vector2f dir;
    };
    std::vector<PoseSnapshot> m_afterimages;
};

} // namespace aren