#include "path_query.hpp"
#include "world.hpp"

namespace aren {

void PathQuery::build(const World &world, const Vector2f &to, ObjectLayer layer) {
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
            if (world.getTile(next).checkCollision(layer)) {
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

bool PathQuery::query(const World &world, const Vector2f &from, std::vector<Vector2f> &path) const {
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

    // std::vector<Vector2f> pathTmp;
    // for (int i = 1; i < path.size(); i++) {
    //     constexpr int SPLIT_COUNT = 2;
    //     for (int j = 0; j <= SPLIT_COUNT; j++) {
    //         pathTmp.push_back(Vector2f::lerp(path[i-1], path[i], float(j) / (SPLIT_COUNT+1)));
    //     }
    // }
    // pathTmp.push_back(path.back());
    // path = std::move(pathTmp);

    // NOTE(aren): Remove redundant vertex greedy
    int l = 0;
    while (l + 2 < path.size()) {
        auto from = path[l];
        auto to = path[l + 2];
        auto delta = to - from;
        float t;
        Vector2i hit, normal;
        if (world.raycast(from, delta.normalized(), delta.length(), m_layer, t, hit, normal)) {
            l++;
        } else {
            path.erase(path.begin() + l + 1);
        }
    }

    return true;
}

} // namespace aren