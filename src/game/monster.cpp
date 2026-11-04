#include "monster.hpp"
#include "world.hpp"
#include "path_query.hpp"
#include "consts.hpp"
#include "player.hpp"
#include "bullet.hpp"
#include "../camera.hpp"
#include "../render.hpp"

namespace aren {

Monster::Monster(World *world, const Vector2f &pos) {
    m_world = world;
    m_pos = pos;
}

Monster::~Monster() {}

static bool solveForDirection(Vector2f bulletPos, float bulletSpeed, Vector2f targetPos, Vector2f targetVel, Vector2f &res) {
    // make bulletPos origin
    targetPos -= bulletPos;
    bulletPos = V2F_ZERO;

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
    res = (targetT - bulletPos).normalized();
    return true;
}

void Monster::update(const PathQuery *query, float dt) {
    static std::vector<Vector2f> s_queryRes;
    s_queryRes.clear();
    query->query(m_world, m_pos, m_collRadius, s_queryRes);

    if (s_queryRes.size() >= 2) {
        auto dir = (s_queryRes[1] - s_queryRes[0]).normalized();

        Vector2f velocityTarget = dir * m_speed;
        Vector2f velocityNext = Vector2f::moveTorwards(m_velocity, velocityTarget, ((velocityTarget.lengthSqr() == 0) ? m_brakeRate : m_accelRate) * dt);

        m_velocity = velocityNext;

        moveDelta(m_velocity * dt);
    }

    m_bulletFireDelay -= dt;
    if (m_bulletFireDelay <= 0) {
        m_bulletFireDelay += m_bulletFireCooldown;

        auto playerPos = m_world->player()->pos();
        auto playerVel = m_world->player()->vel();
        auto targetDir = (playerPos - m_pos).normalized();

        Vector2f targetDirFuture;
        if (solveForDirection(m_pos, m_bulletSpeed, playerPos, playerVel, targetDirFuture)) {
            targetDir = targetDirFuture;
        }

        m_world->addBullet(std::make_unique<Bullet>(m_pos, targetDir * m_bulletSpeed));
    }
}

void Monster::render(const Camera &cam) {
    // TODO(aren): Copied from player.cpp

    const float worldToScreenScale = cam.worldToScreen(V2f(1, 0)).x - cam.worldToScreen(V2f(0, 0)).x;

    float thickness = PIXEL_SCALER / worldToScreenScale;

    const auto drawRect = [&](const Vector2f &center, const Vector2f &half) {
        DrawRectangleV(center - half, half * 2, WHITE);
        DrawRectangleV(center - half + V2F_ONE * thickness, half * 2 - V2F_ONE * thickness * 2, BLACK);
    };

    auto footPoses = {
        V2f(-0.25f, -0.25f),
        V2f(-0.25f, 0.25f),
        V2f(0.25f, -0.25f),
        V2f(0.25f, 0.25f),
    };
    const float footRadius = 0.1f;
    for (const auto &it : footPoses) {
        drawRect(m_pos + it, V2f(1.1f, 1.0f) * footRadius);
    }

    drawRect(m_pos, V2F_ONE * 0.25f);

    float rot = std::atan2(m_dir.y, m_dir.x) * RAD2DEG;

    const float playerRadius = 0.22f;
    DrawCircleSector(m_pos, playerRadius, rot, 360+rot, 6, WHITE);
    DrawCircleSector(m_pos, playerRadius - thickness, rot, 360+rot, 6, BLACK);
}

void Monster::moveDelta(Vector2f delta) {
    delta = m_world->moveRect(Rect(m_pos - V2F_ONE * m_collExtents, 2 * m_collExtents * V2F_ONE), delta, ObjectLayer::Monster);
    m_pos += delta;
}

void Monster::takeDamage(float damage) {
    m_health = std::max(m_health - damage, 0.0f);
}

} // namespace aren