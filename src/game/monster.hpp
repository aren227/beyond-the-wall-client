#pragma once

#include "beam.hpp"
#include "foot.hpp"

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
    float meleeCooldown = 1;
    BeamColor color = BeamColor::White;
    bool chained = false;
};

struct MonsterMelee {
    float duration = 0.6f;
    float time = 0;
    float damage = 10;
    Vector2f fromPos = V2F_ZERO;
    Vector2f targetPos = V2F_ZERO;
    bool hitPlayer = false;

    inline float easeInOutBack(float x) const {
        const float c1 = 1.70158f;
        const float c2 = c1 * 1.525f;

        return x < 0.5f
        ? (std::pow(2 * x, 2) * ((c2 + 1) * 2 * x - c2)) / 2
        : (std::pow(2 * x - 2, 2) * ((c2 + 1) * (x * 2 - 2) + c2) + 2) / 2;
    }

    inline Vector2f currPos() const {
        float eased = easeInOutBack(time / duration);
        if (eased <= 0.5f) return Vector2f::lerp(fromPos, targetPos, eased * 2);
        return Vector2f::lerp(targetPos, fromPos, eased * 2 - 1);
    }
};

class Monster {
public:
    Monster(World *world, const std::string &typeName, const Vector2f &pos);
    virtual ~Monster();

    static const MonsterDefinition &findDef(const std::string &name);

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
    bool m_chained = false;

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

    std::optional<MonsterMelee> m_melee;
    float m_meleeDelay = 0.0f;
    float m_meleeCooldown = 1.0f;
    float m_meleeLength = 2.0f;

    std::array<Foot, 4> m_foots;
};

} // namespace aren;