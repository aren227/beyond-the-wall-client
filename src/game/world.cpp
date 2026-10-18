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

void World::bounds(Vector2i &min, Vector2i &max) {
    min = max = V2I_ZERO;
    if (m_tiles.size() > 0) {
        min = max = m_tiles.begin()->first;

        for (const auto &[k, _] : m_tiles) {
            max = max.max(k);
            min = min.min(k);
        }
    }
}

void World::renderTiles(const Camera &cam, const SpriteSet &spriteSet, int category) {
    Vector2i minPos, maxPos;
    bounds(minPos, maxPos);

    const auto getBlock = [&](const Vector2i &pos) -> TileType {
        if (pos.x < minPos.x || pos.x > maxPos.x || pos.y < minPos.y || pos.y > maxPos.y) {
            return TileType::Barrier;
        }

        if (m_tiles.contains(pos)) {
            return m_tiles.at(pos).m_type;
        }

        return TileType::Empty;
    };

    const auto getCategory = [&](const Vector2i &pos) -> int {
        switch (getBlock(pos)) {
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
    for (int x = minPos.x; x <= maxPos.x; x++) {
        for (int y = minPos.y; y <= maxPos.y; y++) {
            auto v = V2i(x, y);
            if (getCategory(v) == category) {
                auto a = cam.worldToScreen(v.f());
                auto b = cam.worldToScreen((v + V2I_ONE).f());

                const auto &sprite = spriteSet.tile(getBlock(v));
                sprite.render(Rect(a.x, a.y, b.x-a.x, b.y-a.y), SURFACE_TAG, 0, PIXEL_SCALER);
            }
        }
    }

    static std::unordered_map<Vector2i, int> s_vertexHalfs;
    s_vertexHalfs.clear();

    for (int x = minPos.x - 1; x <= maxPos.x; x++) {
        for (int y = minPos.y - 1; y <= maxPos.y; y++) {
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
                    const auto &sprite = spriteSet.tile(getBlock(main));

                    auto a = cam.worldToScreen(center.f());
                    sprite.renderCorner(a, it.tag, it.rotate * 90, PIXEL_SCALER);
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

    for (int x = minPos.x - 1; x <= maxPos.x; x++) {
        for (int y = minPos.y - 1; y <= maxPos.y; y++) {
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

                const auto &sprite = spriteSet.tile(p ? getBlock(V2i(x, y)) : getBlock(V2i(x + 1, y)));

                sprite.renderEdge((aSS + bSS) * 0.5f, bSS - aSS, EDGE_TAG, q ? 90 : 270, PIXEL_SCALER);
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

                const auto &sprite = spriteSet.tile(p ? getBlock(V2i(x, y)) : getBlock(V2i(x, y + 1)));

                sprite.renderEdge((aSS + bSS) * 0.5f, bSS - aSS, EDGE_TAG, q ? 180 : 0, PIXEL_SCALER);
            }
        }
    }
}

void World::renderDecos(const Camera &cam, const SpriteSet &spriteSet) {
    Vector2i minPos, maxPos;
    bounds(minPos, maxPos);

    std::minstd_rand gen(minPos.x * minPos.y * maxPos.x * maxPos.y);

    for (int x = minPos.x; x <= maxPos.x; x++) {
        for (int y = minPos.y; y <= maxPos.y; y++) {
            if (m_tiles.contains(V2i(x, y)) == false) {
                std::uniform_real_distribution<float> rd(0.0f, 1.0f);
                float v = rd(gen);
                if (v < 0.2f) {
                    auto offset = V2f(rd(gen) * 0.4f - 0.2f, rd(gen) * 0.4f - 0.2f);
                    auto vf = V2i(x, y).f() + offset;
                    auto a = cam.worldToScreen(vf);
                    auto b = cam.worldToScreen(vf + V2F_ONE);

                    std::uniform_int_distribution<int> rdi(0, 4);
                    spriteSet.deco.render(Rect(a.x, a.y, b.x-a.x, b.y-a.y), rdi(gen), 0, PIXEL_SCALER);
                }
            }
        }
    }
}

} // namespace aren