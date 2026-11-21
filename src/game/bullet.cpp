#include "bullet.hpp"
#include "world.hpp"
#include "player.hpp"
#include "../render.hpp"

namespace aren {

Bullet::Bullet(const Vector2f &pos, const Vector2f &vel) {
    m_pos = pos;
    m_velocity = vel;
}

void Bullet::update(const World *world, float dt) {
    auto curr = m_pos;
    auto next = m_pos + m_velocity * dt;
    auto delta = next - curr;
    float dist = delta.length();
    auto dir = delta.normalized();

    auto perp = dir.perp();
    auto a = curr - perp * m_radius;
    auto b = curr + dir * m_radius;
    auto c = curr + perp * m_radius;

    float minT = INFINITY;
    std::optional<Player *> target;

    for (const auto &start : {a, b, c}) {
        float t;
        Vector2i h, n;
        if (world->raycast(start, dir, dist, ObjectLayer::Monster, t, h, n)) {
            if (t < minT) {
                minT = t;
                target = std::nullopt;
            }
        }
        auto player = world->player();
        t = Vector2f::lineCircle(start, dir, player->pos(), player->m_hitRadius);
        if (t >= 0 && t < dist && t < minT) {
            minT = t;
            target = player;
        }
    }

    if (minT < INFINITY) {
        m_destroy = true;

        if (target.has_value()) {
            target.value()->takeDamage(m_damage);
        }
    }

    m_pos += m_velocity * dt;
}

void Bullet::render(float outline) {
    DrawCircleV(m_pos, m_radius + outline, BLACK);
    Render::pushColor(m_color);
    DrawCircleV(m_pos, m_radius, WHITE);
    Render::popColor();
}

} // namespace aren