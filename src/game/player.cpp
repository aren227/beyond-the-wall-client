#include "player.hpp"
#include "../camera.hpp"
#include "../render.hpp"

namespace aren {

static constexpr int PIXEL_SCALER = 2;

static Vector2f moveTorwards(const Vector2f &from, const Vector2f &to, float dist) {
    auto dir = to - from;

    if (dir.lengthSqr() < 1e-6) {
        return to;
    }

    float d = dir.length();

    if (dist >= d) {
        return to;
    }

    return from + dir.normalized() * dist;
}

void Player::update(float dt) {
    Vector2f input = V2F_ZERO;

    if (Input::keyDown(KEY_W)) input += V2f(0, -1);
    if (Input::keyDown(KEY_S)) input += V2f(0, 1);
    if (Input::keyDown(KEY_A)) input += V2f(-1, 0);
    if (Input::keyDown(KEY_D)) input += V2f(1, 0);

    Vector2f velocityTarget = ((input == V2F_ZERO) ? V2F_ZERO : input.normalized()) * m_speed;
    Vector2f velocityNext = moveTorwards(m_velocity, velocityTarget, ((input == V2F_ZERO) ? m_brakeRate : m_accelRate) * dt);

    m_velocity = velocityNext;
    m_pos += m_velocity * dt;
}

void Player::render(const Camera &cam) {
    Render::begin2DCam(cam);

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

    float rot = Input::mousePosition().x;

    const float playerRadius = 0.22f;
    DrawCircleSector(m_pos, playerRadius, rot, 360+rot, 6, WHITE);
    DrawCircleSector(m_pos, playerRadius - thickness, rot, 360+rot, 6, BLACK);

    // DrawCircleV(playerPos, playerRadius, WHITE);
    // DrawCircleV(playerPos, playerRadius - thickness, BLACK);

    Render::end2DCam();
}

} // namespace aren