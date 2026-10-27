#include "player.hpp"
#include "world.hpp"
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

void Player::update(const Camera &cam, const World &world, float dt) {
    Vector2f input = V2F_ZERO;

    if (Input::keyDown(KEY_W)) input += V2f(0, -1);
    if (Input::keyDown(KEY_S)) input += V2f(0, 1);
    if (Input::keyDown(KEY_A)) input += V2f(-1, 0);
    if (Input::keyDown(KEY_D)) input += V2f(1, 0);

    Vector2f velocityTarget = ((input == V2F_ZERO) ? V2F_ZERO : input.normalized()) * m_speed;
    Vector2f velocityNext = moveTorwards(m_velocity, velocityTarget, ((input == V2F_ZERO) ? m_brakeRate : m_accelRate) * dt);

    m_velocity = velocityNext;

    auto delta = m_velocity * dt;
    delta = world.moveRect(Rect(m_pos.x - 0.25f, m_pos.y - 0.25f, 0.5f, 0.5f), delta);
    m_pos += delta;

    auto camPos = cam.screenToWorld(Input::mousePosition());
    m_dir = (camPos - m_pos).normalized();

    // beam update
    const auto traversal = [&](const Vector2f &pos, const Vector2f &dir, const Vector2i &min, const Vector2i &max, std::vector<Vector2i> &res) {
        auto cell = pos.floor();

        Vector2i step;
        step.x = dir.x > 0 ? 1 : (dir.x < 0 ? -1 : 0);
        step.y = dir.y > 0 ? 1 : (dir.y < 0 ? -1 : 0);

        Vector2f tDelta, tMin;
        tDelta = V2f(1.0f / std::abs(dir.x), 1.0f / std::abs(dir.y));

        tMin.x =  dir.x > 0 ? ((cell.x+1 - pos.x) / dir.x)
                : dir.x < 0 ? ((pos.x - cell.x) / -dir.x)
                : 1.0f/0.0f;
        tMin.y =  dir.y > 0 ? ((cell.y+1 - pos.y) / dir.y)
                : dir.y < 0 ? ((pos.y - cell.y) / -dir.y)
                : 1.0f/0.0f;

        do {
            res.push_back(cell);

            bool x = tMin.x < tMin.y;
            (x ? cell.x : cell.y) += (x ? step.x : step.y);
            (x ? tMin.x : tMin.y) += (x ? tDelta.x : tDelta.y);

        } while (cell.x >= min.x && cell.x <= max.x && cell.y >= min.y && cell.y <= max.y);
    };

    const auto segmentIntersect = [&](const Vector2i &a, const Vector2i &b, const Vector2f &pos, const Vector2f &dir, float &t) -> bool {
        ASSERT(a.x == b.x || a.y == b.y);
        if (a.x == b.x && dir.x != 0) {
            t = (a.x - pos.x) / dir.x;
            float y = pos.y + dir.y * t;
            if (t >= 0 && std::min(a.y, b.y) <= y && y <= std::max(a.y, b.y)) {
                return true;
            }
        }
        if (a.y == b.y && dir.y != 0) {
            t = (a.y - pos.y) / dir.y;
            float x = pos.x + dir.x * t;
            if (t >= 0 && std::min(a.x, b.x) <= x && x <= std::max(a.x, b.x)) {
                return true;
            }
        }
        return false;
    };

    const auto cellIntersect = [&](const Vector2i &cell, const Vector2f &pos, const Vector2f &dir, float &t, Vector2f &normal) -> bool {
        if (dir.x < 0) {
            if (segmentIntersect(cell + V2i(1, 0), cell + V2i(1, 1), pos, dir, t)) {
                normal = V2f(1, 0);
                return true;
            }
        }
        if (dir.x > 0) {
            if (segmentIntersect(cell + V2i(0, 0), cell + V2i(0, 1), pos, dir, t)) {
                normal = V2f(-1, 0);
                return true;
            }
        }
        if (dir.y < 0) {
            if (segmentIntersect(cell + V2i(0, 1), cell + V2i(1, 1), pos, dir, t)) {
                normal = V2f(0, 1);
                return true;
            }
        }
        if (dir.y > 0) {
            if (segmentIntersect(cell + V2i(0, 0), cell + V2i(1, 0), pos, dir, t)) {
                normal = V2f(0, -1);
                return true;
            }
        }
        return false;
    };

    // beam raycast
    {
        BeamSegment segment;
        segment.start = m_pos;
        segment.dir = m_dir;

        m_beam.segments.clear();

        for (int i = 0; i <= m_bounces; i++) {
            static std::vector<Vector2i> s_visit;
            s_visit.clear();

            traversal(segment.start, segment.dir,
                world.m_min - V2I_ONE, world.m_max + V2I_ONE, s_visit);

            for (const auto &it : s_visit) {
                if (world.getTileType(it) != TileType::Empty) {
                    const auto &tile = world.getTile(it);
                    if (tile.isTransparent() == false) {
                        float t;
                        Vector2f normal;
                        if (cellIntersect(it, segment.start, segment.dir, t, normal)) {
                            segment.end = segment.start + segment.dir * std::max(t - 0.01f, 0.0f);

                            m_beam.segments.push_back(segment);

                            segment.start = segment.end;
                            segment.dir = segment.dir.reflected(normal);
                            segment.prop = segment.prop.merge(world.getTile(it).beamProp());

                            break;
                        }
                    }
                }
            }
        }
    }
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

    float rot = std::atan2(m_dir.y, m_dir.x) * RAD2DEG;

    const float playerRadius = 0.22f;
    DrawCircleSector(m_pos, playerRadius, rot, 360+rot, 6, WHITE);
    DrawCircleSector(m_pos, playerRadius - thickness, rot, 360+rot, 6, BLACK);

    // NOTE(aren): Trick to render hdr color
    if (m_beam.segments.size() > 0) {
        BeginBlendMode(BLEND_ADDITIVE);
        for (const auto &it : m_beam.segments) {
            for (int j = 0; j < 5; j++) {
                DrawLineEx(it.start, it.end, 0.1f, getColorForBeam(it.prop.color));
            }
        }
        EndBlendMode();
    }

    Render::end2DCam();
}

} // namespace aren