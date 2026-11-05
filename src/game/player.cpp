#include "player.hpp"
#include "world.hpp"
#include "path_query.hpp"
#include "consts.hpp"
#include "monster.hpp"
#include "../camera.hpp"
#include "../render.hpp"
#include "../particles.hpp"

namespace aren {

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

    m_dashTime = std::max(m_dashTime - dt, 0.0f);

    if (m_dashTime <= 0 && input.lengthSqr() > 0 && Input::keyPressed(KEY_SPACE)) {
        m_dashTime = m_dashDuration + m_dashCooldown;
        m_dashDir = input.normalized();
    }

    if (m_dashTime > m_dashCooldown) {
        velocityNext = m_dashDir * m_dashSpeed;

        world->particles()->emit(m_pos, 2, {
            .color = {255, 255, 255, 20},
            .size = 0.4f,
        });
    }

    m_velocity = velocityNext;

    auto delta = m_velocity * dt;
    delta = world->moveRect(Rect(m_pos - V2F_ONE * m_collExtents, 2 * m_collExtents * V2F_ONE), delta, ObjectLayer::Player);
    m_pos += delta;

    auto camPos = cam.screenToWorld(Input::mousePosition());
    m_dir = (camPos - m_pos).normalized();

    // beam raycast
    if (isDashing() == false) {
        BeamSegment segment;
        segment.start = m_pos;
        segment.dir = m_dir;

        m_beam.segments.clear();

        for (int i = 0; i <= m_bounces; i++) {
            float minT = INFINITY;
            std::variant<std::monostate, Vector2i, Monster *> hit;
            Vector2f normal;
            BeamProp beamProp;

            { // world
                float t;
                Vector2i h, n;

                if (world->raycast(segment.start, segment.dir, -1, ObjectLayer::Light, t, h, n)) {
                    minT = t;
                    hit = h;
                    normal = n.f();
                    beamProp = world->getTile(h).beamProp();
                } else {
                    ASSERT(false);
                }
            }
            if (i > 0) { // the first beam starts from the player inside
                float t = Vector2f::lineCircle(segment.start, segment.dir, m_pos, m_hitRadius);
                if (t >= 0 && t < minT) {
                    minT = t;
                    hit = {};
                    normal = V2f(1, 0);
                    beamProp = {
                        .color = BeamColor::Black,
                    };
                }
            }
            for (const auto &monster : world->monsters()) {
                float t = Vector2f::lineCircle(segment.start, segment.dir, monster->pos(), monster->m_hitRadius);
                if (t >= 0 && t < minT) {
                    minT = t;
                    hit = monster.get();
                    normal = V2f(1, 0);
                    beamProp = {
                        .color = BeamColor::Black,
                    };
                }
            }

            if (minT < INFINITY) {
                segment.end = segment.start + segment.dir * std::max(minT - 0.01f, 0.0f);

                m_beam.segments.push_back(segment);

                float damage = segment.prop.strength * m_baseAttackDamage * dt;
                auto prevColor = segment.prop.color;

                segment.start = segment.end;
                segment.dir = segment.dir.reflected(normal);
                segment.prop = segment.prop.merge(beamProp);

                if (Vector2i *pos = std::get_if<Vector2i>(&hit)) {
                    if (world->contains(*pos)) {
                        world->at(*pos).takeDamage(dt, damage);
                    }
                } else if (Monster **monster = std::get_if<Monster *>(&hit)) {
                    if ((*monster)->m_color == prevColor) {
                        (*monster)->takeDamage(damage);
                        world->particles()->emit((*monster)->pos(), 5, {});
                    }
                }
            } else {
                UNREACHABLE();
                break;
            }

            if (segment.prop.color == BeamColor::Black || segment.prop.strength <= 0) {
                break;
            }
        }
    } else {
        m_beam.segments.clear();
    }
}

void Player::render(const Camera &cam) {
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
                DrawLineEx(it.start, it.end, BEAM_THICKNESS, getColorForBeam(it.prop.color));
            }
        }
        EndBlendMode();
    }
}

void Player::teleport(const Vector2f &dst) {
    m_pos = dst;
    m_velocity = V2F_ZERO;
}

void Player::takeDamage(float damage) {
    m_health = std::max(m_health - damage, 0.0f);
}

} // namespace aren