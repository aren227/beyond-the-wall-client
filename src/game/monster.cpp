#include "monster.hpp"
#include "world.hpp"
#include "path_query.hpp"
#include "consts.hpp"
#include "player.hpp"
#include "bullet.hpp"
#include "../camera.hpp"
#include "../render.hpp"

namespace aren {

static std::unordered_map<std::string, MonsterDefinition> s_defs;

static void registerDefs() {
    std::unordered_map<std::string, MonsterDefinition> tmp;
    tmp["dumb"] = {
        .maxHealth = 100.0f,
        .speed = 1.5f,
        .bulletSpeed = 8.0f,
        .bulletFireCooldown = 0.5f,
        .bulletSpreadCount = 0,
        .meleeCooldown = 0.5f,
    };
    tmp["dumb_slow"] = {
        .maxHealth = 100.0f,
        .speed = 0.1f,
        .bulletSpeed = 8.0f,
        .bulletFireCooldown = 0.5f,
        .bulletSpreadCount = 0,
        .meleeCooldown = 0.5f,
    };
    tmp["dumb_oneshot"] = {
        .maxHealth = 10.0f,
        .speed = 0.5f,
        .bulletSpeed = 8.0f,
        .bulletFireCooldown = 0.5f,
        .bulletSpreadCount = 0,
        .meleeCooldown = 0.5f,
    };
    tmp["dumb_stop"] = {
        .maxHealth = 100.0f,
        .speed = 0.0f,
        .bulletSpeed = 8.0f,
        .bulletFireCooldown = 0.5f,
        .bulletSpreadCount = 0,
        .meleeCooldown = 0.5f,
    };
    tmp["dumb_stop_chained"] = {
        .maxHealth = 100.0f,
        .speed = 0.0f,
        .bulletSpeed = 8.0f,
        .bulletFireCooldown = 0.5f,
        .bulletSpreadCount = 0,
        .meleeCooldown = 0.5f,
        .chained = true,
    };
    tmp["dumb_fast"] = {
        .maxHealth = 100.0f,
        .speed = 6.0f,
        .bulletSpeed = 8.0f,
        .bulletFireCooldown = 0.5f,
        .bulletSpreadCount = 0,
        .meleeCooldown = 0.5f,
    };
    tmp["shooter"] = {
        .maxHealth = 100.0f,
        .speed = 1.0f,
        .bulletSpeed = 8.0f,
        .bulletFireCooldown = 0.5f,
        .bulletSpreadCount = 1,
        .meleeCooldown = 0,
    };
    tmp["shooter_easy"] = {
        .maxHealth = 100.0f,
        .speed = 1.0f,
        .bulletSpeed = 8.0f,
        .bulletFireCooldown = 1.5f,
        .bulletSpreadCount = 1,
        .meleeCooldown = 0,
    };
    tmp["shooter_hard"] = {
        .maxHealth = 100.0f,
        .speed = 1.0f,
        .bulletSpeed = 15.0f,
        .bulletFireCooldown = 1.0f,
        .bulletSpreadCount = 3,
        .meleeCooldown = 0,
    };

    for (const auto &[k, v] : tmp) {
        s_defs[k] = v;
        s_defs[k + "_r"] = v;
        s_defs[k + "_g"] = v;
        s_defs[k + "_b"] = v;

        s_defs[k].color        = BeamColor::White;
        s_defs[k + "_r"].color = BeamColor::Red;
        s_defs[k + "_g"].color = BeamColor::Green;
        s_defs[k + "_b"].color = BeamColor::Blue;
    }
}

Monster::Monster(World *world, const std::string &typeName, const Vector2f &pos) {
    if (s_defs.empty()) {
        registerDefs();
    }

    m_world = world;
    m_pos = pos;

    ASSERT(s_defs.contains(typeName));
    const auto &def = s_defs[typeName];

    m_health = def.maxHealth;
    m_speed = def.speed;
    m_bulletSpeed = def.bulletSpeed;
    m_bulletFireCooldown = def.bulletFireCooldown;
    m_bulletSpreadCount = def.bulletSpreadCount;
    m_color = def.color;
    m_meleeCooldown = def.meleeCooldown;
    m_chained = def.chained;

    m_foots[0].relative = V2f(-0.25f, -0.25f);
    m_foots[1].relative = V2f(-0.25f, 0.25f);
    m_foots[2].relative = V2f(0.25f, -0.25f);
    m_foots[3].relative = V2f(0.25f, 0.25f);

    m_foots[0].stopWhile = { -1, 1, 2, 3 };
    m_foots[1].stopWhile = { 0, -1, 2, 3 };
    m_foots[2].stopWhile = { 0, 1, -1, 3 };
    m_foots[3].stopWhile = { 0, 1, 2, -1 };

    for (auto &it : m_foots) {
        it.world = m_pos + it.relative;
    }
}

Monster::~Monster() {}

static bool solveForDirection(Vector2f bulletPos, float bulletSpeed, Vector2f targetPos, Vector2f targetVel, Vector2f &res) {
    // make bulletPos origin
    targetPos -= bulletPos;

    // s + dt, rt
    float dd = targetVel.dot(targetVel);
    float rr = bulletSpeed * bulletSpeed;
    float sd = targetPos.dot(targetVel);
    float ss = targetPos.dot(targetPos);

    // At^2 + Bt + C = 0
    float A = dd - rr;
    float B = 2 * sd;
    float C = ss;

    constexpr float EPS = 1e-5f;

    float t = 0;
    if (std::abs(A) < EPS) {
        if (std::abs(B) < EPS) {
            return false;
        }
        t = -C / B;
        if (t < 0) {
            return false;
        }
    } else {
        float D = B * B - 4 * A * C;
        if (D < 0) {
            return false;
        }
        float sqrtD = std::sqrt(D);
        float t1 = (-B - sqrtD) / (2 * A);
        float t2 = (-B + sqrtD) / (2 * A);

        if (std::max(t1, t2) < 0) {
            return false;
        }

        t = (t2 < 0 || (t1 >= 0 && t1 < t2)) ? t1 : t2;
    }

    auto targetT = targetPos + targetVel * t;
    res = targetT + bulletPos;
    return true;
}

void Monster::update(const PathQuery *query, float dt) {
    static std::vector<Vector2f> s_queryRes;
    s_queryRes.clear();
    query->query(m_world, m_pos, m_collRadius, s_queryRes);

    const auto playerPos = m_world->player()->pos();
    const auto playerVel = m_world->player()->vel();

    if (s_queryRes.size() >= 2) {
        auto dir = (s_queryRes[1] - s_queryRes[0]).normalized();

        if (m_melee.has_value() || m_meleeDelay > 0) {
            dir = V2F_ZERO;
        }

        Vector2f velocityTarget = dir * m_speed;
        Vector2f velocityNext = Vector2f::moveTorwards(m_velocity, velocityTarget, ((velocityTarget.lengthSqr() == 0) ? m_brakeRate : m_accelRate) * dt);

        m_velocity = velocityNext;

        moveDelta(m_velocity * dt);
    }

    m_bulletFireDelay -= dt;
    if (m_bulletFireDelay <= 0 && m_bulletSpreadCount > 0) {
        m_bulletFireDelay += m_bulletFireCooldown;

        auto targetPos = playerPos;

        float t;
        Vector2i h, n;

        Vector2f targetFuture;
        if (solveForDirection(m_pos, m_bulletSpeed, playerPos, playerVel, targetFuture)) {
            bool monsterToFutureClear = m_world->raycast(m_pos, (targetFuture - m_pos).normalized(), (targetFuture - m_pos).length(), ObjectLayer::Monster, t, h, n) == false;
            bool playerToFutureClear = m_world->raycast(playerPos, (targetFuture - playerPos).normalized(), (targetFuture - playerPos).length(), ObjectLayer::Player, t, h, n) == false;

            if (monsterToFutureClear && playerToFutureClear) {
                targetPos = targetFuture;
            }
        }

        if (m_world->raycast(m_pos, (targetPos - m_pos).normalized(), (targetPos - m_pos).length(), ObjectLayer::Monster, t, h, n) == false) {
            auto targetDir = (targetPos - m_pos).normalized();

            const float RAD_PER_BULLET = 10 * DEG2RAD;
            float radLo = -RAD_PER_BULLET * (m_bulletSpreadCount - 1) * 0.5f;
            float radHi = -radLo;

            for (int i = 0; i < m_bulletSpreadCount; i++) {
                float rad = (radHi - radLo) * i / std::max(float(m_bulletSpreadCount-1), 1.0f) + radLo;

                m_world->addBullet(std::make_unique<Bullet>(m_pos, targetDir.rotated(rad) * m_bulletSpeed));
            }
        }
    }

    if (m_melee.has_value()) {
        auto &melee = m_melee.value();

        auto prevPos = melee.currPos();
        melee.time += dt;
        auto currPos = melee.currPos();

        if (melee.hitPlayer == false && Vector2f::segmentDist(prevPos, currPos, playerPos) < m_hitRadius) {
            melee.hitPlayer = true;
            m_world->player()->takeDamage(melee.damage);
        }

        if (melee.time >= melee.duration) {
            m_melee.reset();
            m_pos = melee.fromPos;
        } else {
            m_pos = melee.currPos();
        }
    } else {
        m_meleeDelay = std::max(m_meleeDelay - dt, 0.0f);

        auto playerDelta = playerPos - m_pos;
        if (m_melee.has_value() == false && m_meleeCooldown > 0 && m_meleeDelay <= 0 && playerDelta.lengthSqr() < std::pow(m_meleeLength * 0.8f, 2)) {
            m_meleeDelay += m_meleeCooldown;

            m_melee = {
                .fromPos = m_pos,
                .targetPos = playerPos,
            };
        }
    }

    Foot::update(dt, m_speed, m_foots, m_pos);
}

void Monster::render(const Camera &cam) {
    // TODO(aren): Copied from player.cpp

    auto color = getColorForBeam(m_color);

    const float worldToScreenScale = cam.worldToScreen(V2f(1, 0)).x - cam.worldToScreen(V2f(0, 0)).x;

    float thickness = PIXEL_SCALER / worldToScreenScale;

    const auto drawRect = [&](const Vector2f &center, const Vector2f &half) {
        DrawRectangleV(center - half, half * 2, color);
        DrawRectangleV(center - half + V2F_ONE * thickness, half * 2 - V2F_ONE * thickness * 2, BLACK);
    };

    const float footRadius = 0.1f;
    for (const auto &it : m_foots) {
        drawRect(it.world, V2f(1.1f, 1.0f) * footRadius);
        DrawLineEx(it.world, m_pos, thickness, color);
    }

    drawRect(m_pos, V2F_ONE * 0.25f);

    float rot = std::atan2(m_dir.y, m_dir.x) * RAD2DEG;

    const float playerRadius = 0.22f;
    DrawCircleSector(m_pos, playerRadius, rot, 360+rot, 6, color);
    DrawCircleSector(m_pos, playerRadius - thickness, rot, 360+rot, 6, BLACK);

    // if (m_melee.has_value()) {
    //     auto melee = m_melee.value();
    //     auto currDir = melee.currDir();
    //     for (int i = 0; i < 8; i++) {
    //         auto lerpedDir = currDir.rotated(-10 * i * DEG2RAD);
    //         auto opacity = uint8_t((1 - float(i) / 8) * 255);
    //         DrawLineEx(m_pos, m_pos + lerpedDir * m_meleeLength, 0.1f, {255, 255, 255, opacity});
    //     }
    // }
}

void Monster::moveDelta(Vector2f delta) {
    delta = m_world->moveRect(Rect(m_pos - V2F_ONE * m_collExtents, 2 * m_collExtents * V2F_ONE), delta, ObjectLayer::Monster);
    m_pos += delta;
}

void Monster::takeDamage(float damage) {
    m_health = std::max(m_health - damage, 0.0f);
}

} // namespace aren