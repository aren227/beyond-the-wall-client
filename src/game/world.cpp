#include "world.hpp"
#include "../camera.hpp"
#include "sprite_set.hpp"
#include "player.hpp"
#include "monster.hpp"
#include "path_query.hpp"
#include "consts.hpp"
#include "bullet.hpp"
#include "../particles.hpp"
#include "../render.hpp"
#include "../ui.hpp"
#include "raymath.h"
#include <random>
#include <fstream>
#include <map>

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

    m_player = std::make_unique<Player>();
    m_prevPlayerHealth = m_player->m_health;

    m_pathQuery = std::make_unique<PathQuery>();

    m_particles = std::make_unique<Particles>();
}

World::World(const World *world) : World() {
    m_tiles = world->tiles();
    m_monsterWaves = world->m_monsterWaves;

    m_player->m_bounces = world->m_player->m_bounces;
    m_player->m_beamDisabled = world->m_player->m_beamDisabled;

    m_name = world->m_name;
    m_comment = world->m_comment;

    resetPlayerPos();
}

void World::resetPlayerPos() {
    for (const auto &[k, v] : m_tiles) {
        if (v.m_type == TileType::PlayerSpawn) {
            m_player->teleport(k.f() + V2f(0.5f, 0.5f));
            break;
        }
    }
}

World::~World() {}

bool World::checkWaveCondition(const MonsterWave &wave) {
    switch (wave.conditionType) {
        case MonsterWaveCondition::Always:
        {
            return true;
        }
        case MonsterWaveCondition::MonsterCleared:
        {
            return m_monsters.size() == 0;
        }
        case MonsterWaveCondition::EnterZone:
        {
            auto tile = getTile(m_player->pos().floor());
            return (tile.m_type == TileType::Zone && tile.m_name == wave.conditionValue);
        }
        case MonsterWaveCondition::ConsumedConsumable:
        {
            return m_player->m_consumedConsumable;
        }
        default:
        {
            UNREACHABLE();
        }
    }
    return true;
}

void World::activateWave(const MonsterWave &wave) {
    for (const auto &zone : wave.zones) {
        for (const auto &[pos, tile] : m_tiles) {
            if (tile.m_type == TileType::Zone && zone == tile.m_name) {
                m_monsters.push_back(std::make_shared<Monster>(this, wave.monsterName, pos.f() + V2f(0.5f, 0.5f)));
            }
        }
    }
}

bool World::checkWorldCanBeDone() const {
    return m_monsterWaves.empty() && m_monsters.empty();
}

void World::update(Camera &cam, float dt) {
    m_time += dt;

    updateBounds();

    m_player->update(cam, this, dt);

    if (m_prevPlayerHealth > m_player->m_health) {
        cam.shake(5.0f, 0.15f);
    }
    m_prevPlayerHealth = m_player->m_health;

    static std::vector<Vector2i> s_toRemove; s_toRemove.clear();
    for (auto &[k, v] : m_tiles) {
        v.update(this, k, dt);
        if (v.m_health <= 0) {
            s_toRemove.push_back(k);
        }

        if (v.m_type == TileType::End) {
            if (v.progress() >= 1) {
                m_endFlag = true;
                m_playerWin = true;
            }
        }
    }

    for (const auto &k : s_toRemove) {
        m_tiles.erase(k);
    }

    for (auto it = m_monsterWaves.begin(); it != m_monsterWaves.end();) {
        if (checkWaveCondition(*it)) {
            activateWave(*it);
            it = m_monsterWaves.erase(it);
        } else {
            break;
        }
    }

    m_pathQuery->build(this, m_player->pos(), ObjectLayer::Monster);

    for (const auto &it : m_monsters) {
        it->update(m_pathQuery.get(), dt);
    }
    for (auto it = m_monsters.begin(); it != m_monsters.end();) {
        if ((*it)->m_health <= 0) {
            m_particles->emit((*it)->pos(), 20, {
                .speed = 10.0f,
                .life = 0.5f,
                .size = 0.5f,
            });
            it = m_monsters.erase(it);
            cam.shake(5.0f, 0.15f);
        } else {
            ++it;
        }
    }

    // XXX(aren): O(N^2)
    for (const auto &it : m_monsters) {
        auto accumDelta = V2F_ZERO;
        for (const auto &jt : m_monsters) {
            if (it == jt) continue;

            auto delta = it->pos() - jt->pos();
            float len = delta.length();

            if (len < it->m_collRadius + jt->m_collRadius) {
                float penerated = it->m_collRadius + jt->m_collRadius - len;
                accumDelta += delta.normalized() * penerated * 0.5f;
            }
        }

        it->moveDelta(accumDelta);
    }

    m_particles->update(dt);

    for (const auto &it : m_bullets) {
        it->update(this, dt);
    }
    std::erase_if(m_bullets, [](const auto &it){
        return it->done();
    });

    if (m_player->m_health <= 0) {
        m_endFlag = true;
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
            case TileType::WorldPortal:
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

                Render::pushColor(C4F_WHITE.brighten(getTile(v).m_multiplier));
                sprite.render(Rect(a.x, a.y, b.x-a.x, b.y-a.y), SURFACE_TAG, 0, PIXEL_SCALER, color);
                Render::popColor();
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

                    Render::pushColor(C4F_WHITE.brighten(getTile(main).m_multiplier));
                    sprite.renderCorner(a, it.tag, it.rotate * 90, PIXEL_SCALER, color);
                    Render::popColor();
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

                Render::pushColor(C4F_WHITE.brighten(getTile(p ? V2i(x, y) : V2i(x + 1, y)).m_multiplier));
                sprite.renderEdge((aSS + bSS) * 0.5f, bSS - aSS, EDGE_TAG, q ? 90 : 270, PIXEL_SCALER, color);
                Render::popColor();
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

                Render::pushColor(C4F_WHITE.brighten(getTile(p ? V2i(x, y) : V2i(x, y + 1)).m_multiplier));
                sprite.renderEdge((aSS + bSS) * 0.5f, bSS - aSS, EDGE_TAG, q ? 180 : 0, PIXEL_SCALER, color);
                Render::popColor();
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

    BeginBlendMode(BLEND_ADDITIVE);
    for (int x = m_min.x; x <= m_max.x; x++) {
        for (int y = m_min.y; y <= m_max.y; y++) {
            auto v = V2i(x, y);
            if (getTile(v).isTransparent()) {
                auto vf = v.f() + V2f(0.5f, 0.5f);
                float light = 0;
                { // player
                    float playerDist = (vf - m_player->pos()).length();
                    if (playerDist < 4) {
                        light = 1 - playerDist / 4;
                    }
                }
                for (const auto &it : m_player->beam().segments) {
                    if (it.prop.color == BeamColor::Black) {
                        break;
                    }

                    float beamDist = Vector2f::segmentDist(it.start, it.end, vf);
                    if (beamDist < 2) {
                        light = std::max(light, 1 - beamDist / 2);
                    }
                }

                if (light > 0) {
                    auto a = cam.worldToScreen(v.f());
                    auto b = cam.worldToScreen((v + V2I_ONE).f());

                    const auto &sprite = spriteSet.ditherOpaque;
                    const int tag = std::ceil(light * 6);
                    sprite.render(Rect(a.x, a.y, b.x-a.x, b.y-a.y), tag, 0, PIXEL_SCALER, C4F_WHITE * 0.2f);
                }
            }
        }
    }
    EndBlendMode();
}

void World::renderDecos(const Camera &cam, const SpriteSet &spriteSet) {
    std::minstd_rand gen(m_min.x * m_min.y * m_max.x * m_max.y);

    for (int x = m_min.x; x <= m_max.x; x++) {
        for (int y = m_min.y; y <= m_max.y; y++) {
            if (getTile(V2i(x, y)).isSolid() == false) {
                std::uniform_real_distribution<float> rd(0.0f, 1.0f);
                float v = rd(gen);
                if (v < 0.05f) {
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

void World::renderDebugAsUI(const Camera &cam, const SpriteSet &spriteSet) {
    for (int x = m_min.x; x <= m_max.x; x++) {
        for (int y = m_min.y; y <= m_max.y; y++) {
            auto v = V2i(x, y);
            const auto &tile = getTile(v);
            if (tile.m_type == TileType::Zone || tile.m_type == TileType::PlayerSpawn || tile.m_type == TileType::WorldPortal || tile.m_type == TileType::Consumable || tile.m_type == TileType::InvisibleBarrier) {
                auto a = cam.worldToScreen(v.f());
                auto b = cam.worldToScreen((v + V2I_ONE).f());
                auto rect = Rect(a.x, a.y, b.x-a.x, b.y-a.y);

                auto color = WHITE;
                const char *label = "?";
                if (tile.m_type == TileType::Zone) {
                    color = {255, 255, 0, 128};
                    label = tile.m_name.c_str();
                } else if (tile.m_type == TileType::PlayerSpawn) {
                    color = {0, 255, 255, 128};
                    label = "Spawn";
                } else if (tile.m_type == TileType::WorldPortal) {
                    color = {0, 0, 0, 0};
                    label = tile.m_name.c_str();
                } else if (tile.m_type == TileType::Consumable) {
                    color = {255, 0, 255, 128};
                    label = tile.m_name.c_str();
                } else if (tile.m_type == TileType::InvisibleBarrier) {
                    color = {255, 0, 0, 128};
                    label = "";
                }

                DrawRectangleRec(rect, color);

                int prev = GuiGetStyle(LABEL, TEXT_COLOR_NORMAL);
                GuiSetStyle(LABEL, TEXT_COLOR_NORMAL, Color4b(255, 255, 255, 255).rgba32());
                GuiLabel(rect, label);
                GuiSetStyle(LABEL, TEXT_COLOR_NORMAL, prev);
            }
        }
    }
}

void World::renderMisc(const Camera &cam, const SpriteSet &spriteSet) {
    for (const auto &[v, tile] : m_tiles) {
        if (tile.m_type == TileType::WorldPortal) {
            auto idx = tile.m_name.find(',');
            if (idx != std::string::npos) {
                auto a = cam.worldToScreen(v.f() + V2f(0.5f, 0.5f));
                if (UI::textButton(a, 40, tile.m_name.substr(0, idx).c_str(), Render::TextAlignH::Center, Render::TextAlignV::Center)) {
                    if (m_onWorldPortalClick) {
                        m_onWorldPortalClick(tile.m_name.substr(idx+1));
                    }

                    break;
                }
            }
        } else if (tile.m_type == TileType::Consumable) {
            auto a = cam.worldToScreen(v.f());
            auto b = cam.worldToScreen(v.f() + V2F_ONE);

            Render::pushColor(C4F_WHITE.brighten(2 + std::sin(m_time * 2)));
            spriteSet.star.render(Rect(a.x, a.y, b.x-a.x, b.y-a.y), 0, 0, PIXEL_SCALER, WHITE);
            Render::popColor();
        }
    }

    Render::begin2DCam(cam);

    const float worldToScreenScale = cam.worldToScreen(V2f(1, 0)).x - cam.worldToScreen(V2f(0, 0)).x;
    const float thickness = PIXEL_SCALER / worldToScreenScale;

    for (const auto &[v, tile] : m_tiles) {
        if (tile.m_type == TileType::End) {
            Color4f color = C4F_WHITE.brighten(tile.progress() * 15 + 1);
            Render::pushColor(color);

            std::array<Vector3, 6> hexes = {{
                { 1, 0, 0 },
                { 0.5f, 0.866f, 0 },
                { -0.5f, 0.866f, 0 },
                { -1, 0, 0 },
                { -0.5f, -0.866f, 0 },
                { 0.5f, -0.866f, 0 },
            }};

            auto mat1 = MatrixRotate(Vector3Normalize({0.5f, 0.866f, 0.0f}), 1.7f*tile.m_life);
            auto mat2 = MatrixRotate(Vector3Normalize({-0.5f, 0.866f, 0.0f}), 1.3f*tile.m_life);
            auto mat3 = MatrixRotate(Vector3Normalize({1.0f, 0.0f, 0.0f}), 1.1f*tile.m_life);
            mat1 = MatrixMultiply(mat1, MatrixTranslate(v.x + 0.5f, v.y + 0.5f, 0));
            mat2 = MatrixMultiply(mat2, mat1);
            mat3 = MatrixMultiply(mat3, mat2);
            for (int i = 0; i < 6; i++) {
                auto a3 = Vector3Scale(hexes[i], 0.4f);
                auto b3 = Vector3Scale(hexes[(i + 1) % 6], 0.4f);
                a3 = Vector3Transform(a3, mat1);
                b3 = Vector3Transform(b3, mat1);
                DrawLineEx({a3.x, a3.y}, {b3.x, b3.y}, thickness, getColorForBeam(BeamColor::Red));

                a3 = Vector3Scale(hexes[i], 0.3f);
                b3 = Vector3Scale(hexes[(i + 1) % 6], 0.3f);
                a3 = Vector3Transform(a3, mat2);
                b3 = Vector3Transform(b3, mat2);
                DrawLineEx({a3.x, a3.y}, {b3.x, b3.y}, thickness, getColorForBeam(BeamColor::Green));

                a3 = Vector3Scale(hexes[i], 0.2f);
                b3 = Vector3Scale(hexes[(i + 1) % 6], 0.2f);
                a3 = Vector3Transform(a3, mat3);
                b3 = Vector3Transform(b3, mat3);
                DrawLineEx({a3.x, a3.y}, {b3.x, b3.y}, thickness, getColorForBeam(BeamColor::Blue));
            }

            Render::popColor();
        }
    }

    for (const auto &it : m_monsters) {
        it->render(cam);
    }

    m_player->render(cam);

    for (const auto &it : m_bullets) {
        it->render();
    }

    m_particles->render();

    Render::end2DCam();

    // XXX(aren): World specific logic
    if (m_name == "" && m_player->m_consumedConsumable) {
        auto a = cam.worldToScreen(V2f(11.5f, -5));
        Render::drawText(a, 60, WHITE, "Beyond The Wall", Render::TextAlignH::Center, Render::TextAlignV::Center);
    }
}

Vector2f World::moveRect(const Rect &rect, const Vector2f &delta, ObjectLayer layer) const {
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
            if (getTile(V2i(x, y)).checkCollision(layer)) {
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
            if (getTile(V2i(x, y)).checkCollision(layer)) {
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

static constexpr int LVL_VERSION = 1;

static std::string getWorldPath(const std::string &name) {
    return WORLD_BASE + "/" + name + ".lvl";
}

// TODO(aren): Copied from storage.cpp

static std::string escape(const std::string_view &s) {
    std::string out;
    out.reserve(s.size() + 1);
    out += ';';
    for (char c : s) {
        switch (c) {
            case '\\':
                out += "\\\\";
                break;
            case '\n':
                out += "\\n";
                break;
            case '\r':
                out += "\\r";
                break;
            case ' ':
                out += "\\_";
                break;
            default:
                out += c;
                break;
        }
    }
    return out;
}

static std::string unescape(const std::string_view &s) {
    std::string out;
    out.reserve(s.size());
    ASSERT(s.size() > 0 && s[0] == ';');
    for (size_t i = 1; i < s.size(); i++) {
        char c = s[i];
        if (c == '\\' && i + 1 < s.size()) {
            switch (s[i + 1]) {
                case '\\':
                    out += '\\';
                    i++;
                    break;
                case 'n':
                    out += '\n';
                    i++;
                    break;
                case 'r':
                    out += '\r';
                    i++;
                    break;
                case '_':
                    out += ' ';
                    i++;
                    break;
                default:
                    ASSERT(false);
                    i++;
                    break;
            }
        } else {
            out += c;
        }
    }
    return out;
}

static bool unescapeIs(std::istream &is, std::string &value) {
    bool res = !!(is >> value);
    if (res) {
        value = unescape(value);
    }
    return res;
}

void World::save(const std::string &name) {
    if (MakeDirectory(WORLD_BASE.c_str()) != 0) {
        ASSERT(false);
    }

    std::ofstream os(getWorldPath(name));

    os << "version " << LVL_VERSION << std::endl;

    os << "name " << escape(this->m_name) << std::endl;
    os << "comment " << escape(m_comment) << std::endl;

    os << "balance" << std::endl;
    {
        os << "beam_bounces " << m_player->m_bounces << std::endl;
        os << "beam_disabled " << int(m_player->m_beamDisabled) << std::endl;
        os << "exit" << std::endl;
    }

    std::map<Vector2i, Tile> sortedTiles;
    for (const auto &[k, v] : m_tiles) {
        sortedTiles[k] = v;
    }

    os << "tiles " << sortedTiles.size() << std::endl;
    for (const auto [k, v] : sortedTiles) {
        os << "pos " << k.x << " " << k.y << std::endl;
        os << "type " << int(v.m_type) << std::endl;
        os << "color " << int(v.m_color) << std::endl;
        os << "mult " << v.m_multiplier << std::endl;
        os << "name " << escape(v.m_name) << std::endl;
        os << "exit" << std::endl;
    }

    os << "waves " << m_monsterWaves.size() << std::endl;

    for (const auto &it : m_monsterWaves) {
        os << "zones " << it.zones.size();
        for (const auto &zone : it.zones) {
            os << " " << escape(zone);
        }
        os << std::endl;

        os << "monster_name " << escape(it.monsterName) << std::endl;
        os << "condition_type " << int(it.conditionType) << std::endl;
        os << "condition_value " << escape(it.conditionValue) << std::endl;
        os << "exit" << std::endl;
    }

    os.close();
}

std::unique_ptr<World> World::load(const std::string &name) {
    std::ifstream is(getWorldPath(name));
    std::string tok;

    is >> tok;
    if (tok != "version") return nullptr;
    is >> tok;
    if (tok != std::to_string(LVL_VERSION)) return nullptr;

    auto world = std::make_unique<World>();

    while (is >> tok) {
        if (tok == "name") {
            unescapeIs(is, world->m_name);
        } else if (tok == "comment") {
            unescapeIs(is, world->m_comment);
        } else if (tok == "balance") {
            while (is >> tok) {
                if (tok == "beam_bounces") {
                    int c; is >> c;
                    world->m_player->m_bounces = c;
                } else if (tok == "beam_disabled") {
                    int b; is >> b;
                    world->m_player->m_beamDisabled = (b != 0);
                } else if (tok == "exit") {
                    break;
                }
            }
        } else if (tok == "tiles") {
            int count; is >> count;
            for (int i = 0; i < count; i++) {
                std::optional<Vector2i> pos;
                Tile tile;

                while (is >> tok) {
                    if (tok == "pos") {
                        int x, y; is >> x >> y;
                        pos = Vector2i(x, y);
                    } else if (tok == "type") {
                        int t; is >> t;
                        tile.m_type = TileType(t);
                    } else if (tok == "color") {
                        int c; is >> c;
                        tile.m_color = BeamColor(c);
                    } else if (tok == "mult") {
                        int m; is >> m;
                        tile.m_multiplier = m;
                    } else if (tok == "name") {
                        unescapeIs(is, tile.m_name);
                    } else if (tok == "exit") {
                        break;
                    }
                }

                if (!pos) return nullptr;

                world->m_tiles[pos.value()] = std::move(tile);
            }
        } else if (tok == "waves") {
            int count; is >> count;
            for (int i = 0; i < count; i++) {
                MonsterWave wave;

                while (is >> tok) {
                    if (tok == "zones") {
                        int c; is >> c;

                        wave.zones.resize(c);
                        for (int j = 0; j < c; j++) {
                            unescapeIs(is, wave.zones[j]);
                        }
                    } else if (tok == "monster_name") {
                        unescapeIs(is, wave.monsterName);
                    } else if (tok == "condition_type") {
                        int t; is >> t;
                        wave.conditionType = MonsterWaveCondition(t);
                    } else if (tok == "condition_value") {
                        unescapeIs(is, wave.conditionValue);
                    } else if (tok == "exit") {
                        break;
                    }
                }

                world->m_monsterWaves.push_back(std::move(wave));
            }
        }
    }

    is.close();

    world->resetPlayerPos();

    return world;
}

void World::addBullet(std::unique_ptr<Bullet> bullet) {
    m_bullets.push_back(std::move(bullet));
}

} // namespace aren