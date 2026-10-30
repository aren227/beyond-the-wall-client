#include "path_query.hpp"
#include "world.hpp"

namespace aren {

void PathQuery::build(const World *world, const Vector2f &to, ObjectLayer layer) {
    m_to = to;
    m_toI = to.floor();
    m_dirs.clear();
    m_layer = layer;

    constexpr std::array<Vector2i, 4> DELTAS = {
        V2i(-1, 0),
        V2i(1, 0),
        V2i(0, -1),
        V2i(0, 1),
    };

    std::queue<Vector2i> queue;
    queue.push(m_toI);
    m_dirs[m_toI] = V2I_ZERO;

    while (queue.size() > 0) {
        auto curr = queue.front();
        queue.pop();

        for (const auto &delta : DELTAS) {
            auto next = curr + delta;
            if (world->getTile(next).checkCollision(layer)) {
                continue;
            }
            if (m_dirs.contains(next)) {
                continue;
            }

            m_dirs[next] = -delta;
            queue.push(next);
        }
    }
}

bool PathQuery::query(const World *world, const Vector2f &from, float radius, std::vector<Vector2f> &path) const {
    auto fromI = from.floor();
    if (m_dirs.contains(fromI) == false) {
        return false;
    }

    path.push_back(from);
    path.push_back(fromI.f() + V2f(0.5f, 0.5f));

    auto curr = fromI;
    while (m_dirs.at(curr) != V2I_ZERO) {
        auto next = curr + m_dirs.at(curr);
        path.push_back(next.f() + V2f(0.5f, 0.5f));
        curr = next;
    }

    ASSERT(curr == m_toI);
    path.push_back(m_to);

    // NOTE(aren): Approximated circle cast check. Works only if radius < 0.5.
    const auto circleCast = [&](const Vector2f &from, Vector2f delta) {
        float len = delta.length();
        delta = delta.normalized();

        float t;
        Vector2i hit, normal;
        auto perp = delta.perp();
        auto a = from + perp * radius;
        auto b = from - perp * radius;

        return world->raycast(from, delta, len, m_layer, t, hit, normal)
            || world->raycast(a,    delta, len, m_layer, t, hit, normal)
            || world->raycast(b,    delta, len, m_layer, t, hit, normal);
    };

    // NOTE(aren): Remove redundant vertex greedy
    int l = 0;
    while (l + 2 < path.size()) {
        auto from = path[l];
        auto to = path[l + 2];
        auto delta = to - from;

        if (circleCast(from, delta)) {
            l++;
        } else {
            path.erase(path.begin() + l + 1);
        }
    }

    return true;
}

} // namespace aren