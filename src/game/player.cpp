#include "player.hpp"
#include "world.hpp"
#include "path_query.hpp"
#include "../camera.hpp"
#include "../render.hpp"

namespace aren {

static constexpr int PIXEL_SCALER = 2;

Player::Player() {}

Player::~Player() {}

void Player::update(const Camera &cam, World *world, float dt) {
    Vector2f input = V2F_ZERO;

    if (Input::keyDown(KEY_W)) input += V2f(0, -1);
    if (Input::keyDown(KEY_S)) input += V2f(0, 1);
    if (Input::keyDown(KEY_A)) input += V2f(-1, 0);
    if (Input::keyDown(KEY_D)) input += V2f(1, 0);

    Vector2f velocityTarget = ((input == V2F_ZERO) ? V2F_ZERO : input.normalized()) * m_speed;
    Vector2f velocityNext = Vector2f::moveTorwards(m_velocity, velocityTarget, ((input == V2F_ZERO) ? m_brakeRate : m_accelRate) * dt);

    m_velocity = velocityNext;

    auto delta = m_velocity * dt;
    delta = world->moveRect(Rect(m_pos.x - 0.25f, m_pos.y - 0.25f, 0.5f, 0.5f), delta, ObjectLayer::Player);
    m_pos += delta;

    auto camPos = cam.screenToWorld(Input::mousePosition());
    m_dir = (camPos - m_pos).normalized();

    // beam raycast
    {
        BeamSegment segment;
        segment.start = m_pos;
        segment.dir = m_dir;

        m_beam.segments.clear();

        for (int i = 0; i <= m_bounces; i++) {
            float t;
            Vector2i hit, normal;
            if (world->raycast(segment.start, segment.dir, -1, ObjectLayer::Light, t, hit, normal)) {
                segment.end = segment.start + segment.dir * std::max(t - 0.01f, 0.0f);

                m_beam.segments.push_back(segment);

                segment.start = segment.end;
                segment.dir = segment.dir.reflected(normal.f());

                auto prevColor = segment.prop.color;
                segment.prop = segment.prop.merge(world->getTile(hit).beamProp());

                if (world->contains(hit) && prevColor != BeamColor::Black) {
                    world->at(hit).takeDamage(dt * m_baseAttackDamage);
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
            if (it.prop.color == BeamColor::Black) {
                break;
            }
            for (int j = 0; j < 5; j++) {
                DrawLineEx(it.start, it.end, 0.1f, getColorForBeam(it.prop.color));
            }
        }
        EndBlendMode();
    }

    Render::end2DCam();
}

void Player::teleport(const Vector2f &dst) {
    m_pos = dst;
    m_velocity = V2F_ZERO;
}

} // namespace aren