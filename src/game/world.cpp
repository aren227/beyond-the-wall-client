#include "world.hpp"
#include "../camera.hpp"
#include "sprite_set.hpp"
#include <random>

namespace aren {

struct VertexDef {
    std::array<bool, 4> pattern;
    int tag;
    int half; // in pixels
    int rotate;
    int mainX, mainY;
};

static const std::vector<VertexDef> VERTEX_DEFS = {
    {
        .pattern = { 1, 0, 0, 0 },
        .tag = 1,
        .half = 5,
        .rotate = 0,
        .mainX = 0,
        .mainY = 0,
    },
    {
        .pattern = { 1, 0, 0, 1 },
        .tag = 2,
        .half = 5,
        .rotate = 0,
        .mainX = 0,
        .mainY = 0,
    },
    {
        .pattern = { 0, 1, 1, 1 },
        .tag = 3,
        .half = 1,
        .rotate = 0,
        .mainX = 1,
        .mainY = 1,
    },
};

static std::vector<VertexDef> s_vertexDefFull;

static const int SURFACE_TAG = 0;
static const int EDGE_TAG = 4;

static constexpr int PIXEL_SCALER = 2;

World::World() {
    if (s_vertexDefFull.empty()) {
        const auto rotatePattern = [](std::array<bool, 4> &pattern) {
            // NOTE(aren): rotate clockwise
            auto tmp = pattern;
            pattern[0] = tmp[2];
            pattern[1] = tmp[0];
            pattern[2] = tmp[3];
            pattern[3] = tmp[1];
        };

        for (const auto &it : VERTEX_DEFS) {
            auto copy = it;
            for (int i = 0; i < 4; i++) {
                s_vertexDefFull.push_back(copy);

                rotatePattern(copy.pattern);
                copy.rotate++;

                int tmp = copy.mainX;
                copy.mainX = 1 - copy.mainY;
                copy.mainY = tmp;
            }
        }
    }
}

void World::update(const Camera &cam, float dt) {
    m_time += dt;

    updateBounds();

    m_player.update(cam, *this, dt);

    static std::vector<Vector2i> s_toRemove; s_toRemove.clear();
    for (const auto &[k, v] : m_tiles) {
        if (v.m_health <= 0) {
            s_toRemove.push_back(k);
        }
    }

    for (const auto &k : s_toRemove) {
        m_tiles.erase(k);
    }
}

void World::updateBounds() {
    m_min = m_max = V2I_ZERO;
    if (m_tiles.size() > 0) {
        m_min = m_max = m_tiles.begin()->first;

        for (const auto &[k, _] : m_tiles) {
            m_max = m_max.max(k);
            m_min = m_min.min(k);
        }
    }
}

const Tile &World::getTile(const Vector2i &pos) const {
    static Tile s_tile;

    if (pos.x < m_min.x || pos.x > m_max.x || pos.y < m_min.y || pos.y > m_max.y) {
        s_tile = Tile(TileType::Barrier);
        return s_tile;
    }

    if (m_tiles.contains(pos)) {
        return m_tiles.at(pos);
    }

    s_tile = Tile(TileType::Empty);
    return s_tile;
}

TileType World::getTileType(const Vector2i &pos) const {
    return getTile(pos).m_type;
}

Color4f World::getColor(const Vector2i &pos) const {
    auto color = getColorForBeam(getTile(pos).beamColor());
    if (color == C4F_BLACK) {
        // XXX(aren): black has dedicated sprite..
        return C4F_WHITE;
    }
    return color;
}

void World::renderTiles(const Camera &cam, const SpriteSet &spriteSet, int category) {
    const auto getCategory = [&](const Vector2i &pos) -> int {
        switch (getTileType(pos)) {
            case TileType::Empty:
                return 0;
            case TileType::Barrier:
            case TileType::Wall:
                return 1;
            case TileType::Glass:
                return 2;
            case TileType::Sand:
                return 3;
            default:
                return 0;
        }
    };

    // draw tiles
    for (int x = m_min.x; x <= m_max.x; x++) {
        for (int y = m_min.y; y <= m_max.y; y++) {
            auto v = V2i(x, y);
            if (getCategory(v) == category) {
                auto a = cam.worldToScreen(v.f());
                auto b = cam.worldToScreen((v + V2I_ONE).f());

                const auto &sprite = spriteSet.tile(getTileType(v));
                const Color4f color = getColor(v);
                sprite.render(Rect(a.x, a.y, b.x-a.x, b.y-a.y), SURFACE_TAG, 0, PIXEL_SCALER, color);
            }
        }
    }

    static std::unordered_map<Vector2i, int> s_vertexHalfs;
    s_vertexHalfs.clear();

    for (int x = m_min.x - 1; x <= m_max.x; x++) {
        for (int y = m_min.y - 1; y <= m_max.y; y++) {
            std::array<bool, 4> currPattern = {
                getCategory(V2i(x, y)) == category,
                getCategory(V2i(x + 1, y)) == category,
                getCategory(V2i(x, y + 1)) == category,
                getCategory(V2i(x + 1, y + 1)) == category,
            };

            for (const auto &it : s_vertexDefFull) {
                if (it.pattern == currPattern) {
                    auto center = V2i(x + 1, y + 1);
                    s_vertexHalfs[center] = it.half;

                    auto main = V2i(x + it.mainX, y + it.mainY);
                    const auto &sprite = spriteSet.tile(getTileType(main));
                    const auto color = getColor(main);

                    auto a = cam.worldToScreen(center.f());
                    sprite.renderCorner(a, it.tag, it.rotate * 90, PIXEL_SCALER, color);
                }
            }
        }
    }

    const float worldToScreenScale = cam.worldToScreen(V2f(1, 0)).x - cam.worldToScreen(V2f(0, 0)).x;

    const auto getVertexHalfWS = [&](const Vector2i &at) {
        if (s_vertexHalfs.contains(at)) {
            float screenSpace = s_vertexHalfs[at] * PIXEL_SCALER;
            return screenSpace / worldToScreenScale;
        }
        return 0.0f;
    };

    for (int x = m_min.x - 1; x <= m_max.x; x++) {
        for (int y = m_min.y - 1; y <= m_max.y; y++) {
            // vertical
            bool p = getCategory(V2i(x, y)) == category;
            bool q = getCategory(V2i(x + 1, y)) == category;
            if (p ^ q) {
                auto a = V2f(x + 1, y);
                auto b = V2f(x + 1, y + 1);
                a += V2f(0, 1) * getVertexHalfWS(V2i(x + 1, y));
                b -= V2f(0, 1) * getVertexHalfWS(V2i(x + 1, y + 1));

                auto aSS = cam.worldToScreen(a);
                auto bSS = cam.worldToScreen(b);

                const auto &sprite = spriteSet.tile(p ? getTileType(V2i(x, y)) : getTileType(V2i(x + 1, y)));
                const auto color = getColor(p ? V2i(x, y) : V2i(x + 1, y));

                sprite.renderEdge((aSS + bSS) * 0.5f, bSS - aSS, EDGE_TAG, q ? 90 : 270, PIXEL_SCALER, color);
            }

            // horizontal
            q = getCategory(V2i(x, y + 1)) == category;
            if (p ^ q) {
                auto a = V2f(x, y + 1);
                auto b = V2f(x + 1, y + 1);
                a += V2f(1, 0) * getVertexHalfWS(V2i(x, y + 1));
                b -= V2f(1, 0) * getVertexHalfWS(V2i(x + 1, y + 1));

                auto aSS = cam.worldToScreen(a);
                auto bSS = cam.worldToScreen(b);

                const auto &sprite = spriteSet.tile(p ? getTileType(V2i(x, y)) : getTileType(V2i(x, y + 1)));
                const auto color = getColor(p ? V2i(x, y) : V2i(x, y + 1));

                sprite.renderEdge((aSS + bSS) * 0.5f, bSS - aSS, EDGE_TAG, q ? 180 : 0, PIXEL_SCALER, color);
            }
        }
    }

    // NOTE(aren): Overlay after render all stuffs
    BeginBlendMode(BLEND_MULTIPLIED);
    for (int x = m_min.x; x <= m_max.x; x++) {
        for (int y = m_min.y; y <= m_max.y; y++) {
            auto v = V2i(x, y);
            float health = getTile(v).m_health;
            if (health < TILE_MAX_HEALTH) {
                auto a = cam.worldToScreen(v.f());
                auto b = cam.worldToScreen((v + V2I_ONE).f());

                const auto &sprite = spriteSet.ditherOpaque;
                const int tag = std::floor((health / TILE_MAX_HEALTH) * 15) + 1;
                sprite.render(Rect(a.x, a.y, b.x-a.x, b.y-a.y), tag, 0, PIXEL_SCALER, WHITE);
            }
        }
    }
    EndBlendMode();
}

void World::renderDecos(const Camera &cam, const SpriteSet &spriteSet) {
    std::minstd_rand gen(m_min.x * m_min.y * m_max.x * m_max.y);

    for (int x = m_min.x; x <= m_max.x; x++) {
        for (int y = m_min.y; y <= m_max.y; y++) {
            if (m_tiles.contains(V2i(x, y)) == false) {
                std::uniform_real_distribution<float> rd(0.0f, 1.0f);
                float v = rd(gen);
                if (v < 0.2f) {
                    auto offset = V2f(rd(gen) * 0.4f - 0.2f, rd(gen) * 0.4f - 0.2f);
                    auto vf = V2i(x, y).f() + offset;
                    auto a = cam.worldToScreen(vf);
                    auto b = cam.worldToScreen(vf + V2F_ONE);

                    std::uniform_int_distribution<int> rdi(0, 4);
                    spriteSet.deco.render(Rect(a.x, a.y, b.x-a.x, b.y-a.y), rdi(gen), 0, PIXEL_SCALER, WHITE);
                }
            }
        }
    }
}

void World::renderMisc(const Camera &cam, const SpriteSet &spriteSet) {
    m_player.render(cam);
}

Vector2f World::moveRect(const Rect &rect, const Vector2f &delta) const {
    if (delta.lengthSqr() < 1e-6) {
        return V2F_ZERO;
    }

    Vector2f res = V2F_ZERO;

    float minX = rect.x;
    float maxX = rect.x + rect.w;
    float minY = rect.y;
    float maxY = rect.y + rect.h;

    // NOTE(aren): move x axis first.
    float targetMinX = minX + std::min(delta.x, 0.0f);
    float targetMaxX = maxX + std::max(delta.x, 0.0f);

    int minXi = std::floor(targetMinX);
    int maxXi = std::floor(targetMaxX);
    int minYi = std::floor(minY);
    int maxYi = std::floor(maxY);

    constexpr float MARGIN = 0.01f;

    for (int x = minXi; x <= maxXi; x++) {
        for (int y = minYi; y <= maxYi; y++) {
            if (getTileType(V2i(x, y)) != TileType::Empty) {
                if (delta.x > 0) targetMaxX = std::min(targetMaxX, x - MARGIN);
                else targetMinX = std::max(targetMinX, x + 1 + MARGIN);
            }
        }
    }

    res.x = (delta.x > 0) ? targetMaxX - maxX : targetMinX - minX;

    minX += res.x;
    maxX += res.x;

    // NOTE(aren): move y axis.
    float targetMinY = minY + std::min(delta.y, 0.0f);
    float targetMaxY = maxY + std::max(delta.y, 0.0f);

    minXi = std::floor(minX);
    maxXi = std::floor(maxX);
    minYi = std::floor(targetMinY);
    maxYi = std::floor(targetMaxY);

    for (int x = minXi; x <= maxXi; x++) {
        for (int y = minYi; y <= maxYi; y++) {
            if (getTileType(V2i(x, y)) != TileType::Empty) {
                if (delta.y > 0) targetMaxY = std::min(targetMaxY, y - MARGIN);
                else targetMinY = std::max(targetMinY, y + 1 + MARGIN);
            }
        }
    }

    res.y = (delta.y > 0) ? (targetMaxY - maxY) : (targetMinY - minY);

    return res;
}

bool World::raycast(const Vector2f &start, const Vector2f &dir, float maxDist, ObjectLayer layer, float &t, Vector2i &hit, Vector2i &normal) const {
    // helper
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

    const auto cellIntersect = [&](const Vector2i &cell, const Vector2f &pos, const Vector2f &dir, float &t, Vector2i &normal) -> bool {
        if (dir.x < 0) {
            if (segmentIntersect(cell + V2i(1, 0), cell + V2i(1, 1), pos, dir, t)) {
                normal = V2i(1, 0);
                return true;
            }
        }
        if (dir.x > 0) {
            if (segmentIntersect(cell + V2i(0, 0), cell + V2i(0, 1), pos, dir, t)) {
                normal = V2i(-1, 0);
                return true;
            }
        }
        if (dir.y < 0) {
            if (segmentIntersect(cell + V2i(0, 1), cell + V2i(1, 1), pos, dir, t)) {
                normal = V2i(0, 1);
                return true;
            }
        }
        if (dir.y > 0) {
            if (segmentIntersect(cell + V2i(0, 0), cell + V2i(1, 0), pos, dir, t)) {
                normal = V2i(0, -1);
                return true;
            }
        }
        return false;
    };

    static std::vector<Vector2i> s_visit;
    s_visit.clear();

    auto minBound = m_min - V2I_ONE;
    auto maxBound = m_max + V2I_ONE;

    if (maxDist >= 0) {
        auto end = start + dir * maxDist;
        minBound = minBound.max(start.min(end).floor());
        maxBound = maxBound.min(start.max(end).floor());
    }

    traversal(start, dir, minBound, maxBound, s_visit);

    for (const auto &it : s_visit) {
        const auto &tile = getTile(it);
        if (tile.checkCollision(layer)) {
            if (cellIntersect(it, start, dir, t, normal)) {
                if (maxDist < 0 || t <= maxDist) {
                    hit = it;
                    return true;
                }
            }
        }
    }

    return false;
}

} // namespace aren