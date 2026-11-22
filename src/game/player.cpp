#include "player.hpp"
#include "world.hpp"
#include "path_query.hpp"
#include "consts.hpp"
#include "monster.hpp"
#include "game_audio.hpp"
#include "../camera.hpp"
#include "../render.hpp"
#include "../particles.hpp"

namespace aren {

Player::Player(World *world) : m_world(world) {}

Player::~Player() {
    GameAudio::laser(0.0f);
}

void Player::update(const Camera &cam, float dt) {
    if (m_controlDisabled == false) {
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
            m_hasDashed = true;

            GameAudio::play(Sfx::Dash);
        }

        if (m_dashTime > m_dashCooldown) {
            velocityNext = m_dashDir * m_dashSpeed;

            m_world->particles()->emit(m_pos, 2, {
                .color = {255, 255, 255, 20},
                .size = 0.4f,
            });
        }

        if (m_immortalTime > 0) {
            m_immortalTime = std::max(m_immortalTime - dt, 0.0f);
        }

        m_velocity = velocityNext;

        auto delta = m_velocity * dt;
        delta = m_world->moveRect(Rect(m_pos - V2F_ONE * m_collExtents, 2 * m_collExtents * V2F_ONE), delta, ObjectLayer::Player);
        m_pos += delta;

        auto camPos = cam.screenToWorld(Input::mousePosition());
        m_dir = (camPos - m_pos).normalized();
    }

    bool somethingHit = false;

    // beam raycast
    if (isDashing() == false && m_beamDisabled == false) {
        BeamSegment segment;
        segment.start = m_pos;
        segment.dir = m_dir;

        m_beam.segments.clear();

        std::vector<Monster *> chainedMonsters;

        for (int i = 0; i <= m_bounces; i++) {
            float minT = INFINITY;
            std::variant<std::monostate, Vector2i, Monster *> hit;
            Vector2f normal;
            BeamProp beamProp;

            { // world
                float t;
                Vector2i h, n;

                if (m_world->raycast(segment.start, segment.dir, -1, ObjectLayer::Light, t, h, n)) {
                    minT = t;
                    hit = h;
                    normal = n.f();
                    beamProp = m_world->getTile(h).beamProp();
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
            for (const auto &monster : m_world->monsters()) {
                float t = Vector2f::lineCircle(segment.start, segment.dir, monster->pos(), monster->m_hitRadius);
                if (t >= 0 && t < minT) {
                    if (monster->m_chained) {
                        // NOTE(aren): pierce
                        chainedMonsters.push_back(monster.get());
                    } else {
                        minT = t;
                        hit = monster.get();
                        normal = V2f(1, 0);
                        beamProp = {
                            .color = BeamColor::Black,
                        };
                    }
                }
            }

            if (minT < INFINITY) {
                segment.end = segment.start + segment.dir * std::max(minT - 0.001f, 0.0f);

                m_beam.segments.push_back(segment);

                float damage = segment.prop.strength * m_baseAttackDamage * dt;
                auto prevColor = segment.prop.color;

                segment.start = segment.end;
                segment.dir = segment.dir.reflected(normal);
                segment.prop = segment.prop.merge(beamProp);

                if (Vector2i *pos = std::get_if<Vector2i>(&hit)) {
                    if (m_world->contains(*pos)) {
                        m_world->at(*pos).takeDamage(dt, damage);
                        auto particleColor = getColorForBeam(prevColor);
                        // XXX(aren): gold exception
                        if (m_world->m_isWorldMap && prevColor == BeamColor::Yellow) {
                            particleColor = GOLD2.f();
                        }
                        particleColor.a = 0.2f;
                        m_world->particles()->emit(segment.end, 2, {
                            .color = particleColor.byte(),
                            .normal = normal,
                            .angle = 90.0f,
                        });

                        // somethingHit = true;
                    }
                } else if (Monster **monster = std::get_if<Monster *>(&hit)) {
                    if ((*monster)->m_color == prevColor) {
                        (*monster)->takeDamage(damage);
                        auto particleColor = getColorForBeam(prevColor);
                        particleColor.a = 0.5f;
                        m_world->particles()->emit(segment.end, 5, {
                            .color = particleColor.byte(),
                        });

                        somethingHit = true;
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

        std::sort(chainedMonsters.begin(), chainedMonsters.end());
        chainedMonsters.erase(std::unique(chainedMonsters.begin(), chainedMonsters.end()), chainedMonsters.end());

        size_t spawnedChainedCount = 0;
        for (const auto &it : m_world->monsters()) {
            if (it.get()->m_chained) spawnedChainedCount++;
        }

        if (chainedMonsters.size() == spawnedChainedCount) {
            float damage = m_baseAttackDamage * dt;
            for (const auto &it : chainedMonsters) {
                it->takeDamage(damage);
                auto particleColor = getColorForBeam(BeamColor::White);
                particleColor.a = 0.5f;
                m_world->particles()->emit(it->pos(), 5, {
                    .color = particleColor.byte(),
                });

                somethingHit = true;
            }
        }

    } else {
        m_beam.segments.clear();
    }

    // NOTE(aren): Laser volume controller
    {
        float minDist = 1e9f;
        for (const auto &it : m_beam.segments) {
            float dist = (m_pos - it.end).length();
            minDist = std::min(minDist, dist);
        }

        float maxVolume = 0.3f;
        float maxVolumeDist = 1.0f;
        float fadeDist = 10.0f;

        float t = std::clamp((minDist - fadeDist) / (maxVolumeDist - fadeDist), 0.0f, 1.0f);
        float volume = maxVolume * t;
        if (somethingHit) {
            volume = std::max(volume, 0.8f);
        }

        if (m_laserVolume < volume) {
            m_laserVolume = std::min(m_laserVolume + dt * 10.0f, volume);
        } else {
            m_laserVolume = std::max(m_laserVolume - dt * 10.0f, volume);
        }

        GameAudio::laser(m_laserVolume);
    }
}

void Player::render(const Camera &cam) {
    if (m_visible == false) {
        return;
    }

    // NOTE(aren): changing opacity is too weak..
    if ((m_immortalTime > 0) && (int(m_immortalTime * 24) % 2 == 0)) {
        return;
    }

    const float worldToScreenScale = cam.worldToScreen(V2f(1, 0)).x - cam.worldToScreen(V2f(0, 0)).x;

    float thickness = PIXEL_SCALER / worldToScreenScale;

    const auto drawRect = [&](const Vector2f &center, const Vector2f &half) {
        DrawRectangleV(center - half, half * 2, WHITE);
        DrawRectangleV(center - half + V2F_ONE * thickness, half * 2 - V2F_ONE * thickness * 2, BLACK);
    };

    // NOTE(aren): player moving speed is too fast for animation...
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

    for (const auto &it : m_beam.segments) {
        if (it.prop.color == BeamColor::Black) {
            break;
        }

        // XXX(aren): gold exception
        auto color = getColorForBeam(it.prop.color);
        if (m_world->m_isWorldMap && it.prop.color == BeamColor::Yellow) {
            color = GOLD2.f();
        }

        Render::pushColor(m_beamBaseColor);
        DrawLineEx(it.start, it.end, BEAM_THICKNESS, color);
        Render::popColor();
    }
}

void Player::teleport(const Vector2f &dst) {
    m_pos = dst;
    m_velocity = V2F_ZERO;
}

void Player::look(const Vector2f &dst) {
    m_dir = (dst - m_pos).normalized();
}

void Player::takeDamage(float damage) {
    if (isDashing() == false && m_immortalTime == 0) {
        GameAudio::play(Sfx::Hit);

        m_health = std::max(m_health - damage, 0.0f);
        m_immortalTime = m_hitImmortalDuration;
    }
}

} // namespace aren