#include "monster.hpp"
#include "path_query.hpp"
#include "../camera.hpp"
#include "../render.hpp"

namespace aren {

static constexpr int PIXEL_SCALER = 2;

Monster::Monster(const Vector2f &pos) {
    m_pos = pos;
}

Monster::~Monster() {}

void Monster::update(const PathQuery *query, const World *world, float dt) {
    static std::vector<Vector2f> s_queryRes;
    s_queryRes.clear();
    query->query(world, m_pos, s_queryRes);

    ASSERT(s_queryRes.size() >= 2);

    auto dir = (s_queryRes[1] - s_queryRes[0]).normalized();

    m_pos += dir * m_speed * dt;
}

void Monster::render(const Camera &cam) {
    Render::begin2DCam(cam);

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

    Render::end2DCam();
}

} // namespace aren