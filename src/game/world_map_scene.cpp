#include "world_map_scene.hpp"
#include "world.hpp"
#include "../render.hpp"
#include "game.hpp"
#include "transition_scene.hpp"
#include "ingame_scene.hpp"
#include "title_scene.hpp"
#include "player.hpp"
#include <map>
#include "../storage.hpp"
#include "consts.hpp"

namespace aren {

WorldMapScene::WorldMapScene() : Scene(TYPE) {
    m_world = World::load("world_map");
    ASSERT(m_world);

    m_world->player()->m_visible = false;

    m_world->m_onWorldPortalClick = [](std::string name) {
        Game::addScene<TransitionScene>(false);
        auto scene = Game::getScene<TransitionScene>();
        scene->setOnEnd([name]() {
            auto loaded = World::load(name);
            ASSERT(loaded);
            Game::removeScene<WorldMapScene>();
            Game::addScene<IngameScene>(std::move(loaded));
            Game::addScene<TransitionScene>(true);
        });
    };

    auto compNames = Storage::getList(COMPLETED_WORLD_KEYS);

    std::map<int, std::string> sorted;
    for (auto &[k, tile] : m_world->tiles()) {
        // TODO(aren): Basically copied from world.cpp
        if (tile.m_type == TileType::WorldPortal) {
            auto idx = tile.m_name.find(',');
            if (idx != std::string::npos) {
                int num = std::stoi(tile.m_name.substr(0, idx));
                auto filename = tile.m_name.substr(idx+1);
                sorted[num] = filename;

                tile.m_completed = (std::find(compNames.begin(), compNames.end(), filename) != compNames.end());
            }
        }
    }

    for (auto it = sorted.begin(); it != sorted.end(); ++it) {
        m_worldNames.push_back(it->second);
    }

    m_spriteSet.load();
}

WorldMapScene::~WorldMapScene() {}

void WorldMapScene::update(float dt) {
    m_world->update(m_cam, dt);

    if (Input::keyPressed(KEY_ESCAPE)) {
        Game::removeScene<WorldMapScene>();
        Game::addScene<TitleScene>();
    }
}

void WorldMapScene::render() {
    ClearBackground(BLACK_BG.f().linear());

    // camera
    {
        auto min = m_world->m_min;
        auto max = m_world->m_max;

        max += V2I_ONE;

        float sw = GetScreenWidth();
        float sh = GetScreenHeight() - m_hudHeight;

        const float PADDING_WS = 1.0f;

        float xZoom = sw / (max.x - min.x + PADDING_WS * 2);
        float yZoom = sh / (max.y - min.y + PADDING_WS * 2);

        m_cam.m_zoom = std::min(xZoom, yZoom);
        m_cam.m_target = (min + max).f() / 2.0f;
        m_cam.m_offset = V2f(sw, sh - m_hudHeight) * 0.5f;

        m_hudWidth = m_cam.m_zoom * (max.x - min.x + 1);
    }

    // TODO(aren): hardcoded
    for (int i = 0; i < 4; i++) {
        m_world->renderTiles(m_cam, m_spriteSet, i);
    }
    m_world->renderTilesMisc(m_cam, m_spriteSet);

    m_world->renderDecos(m_cam, m_spriteSet);

    m_world->renderMisc(m_cam, m_spriteSet);
}

void WorldMapScene::renderUI() {
    float sw = GetScreenWidth();
    float sh = GetScreenHeight();
    float sx = (sw - m_hudWidth) * 0.5f;
    const float xMargin = 4;

    Render::drawText(
        V2f(sx + xMargin, sh-40),
        40,
        WHITE,
        m_world->m_name.c_str(),
        Render::TextAlignH::Left,
        Render::TextAlignV::Top
    );
}

void WorldMapScene::gotoNextImmediately(const std::string &currentWorldName) {
    auto index = std::find(m_worldNames.begin(), m_worldNames.end(), currentWorldName);

    if (index == m_worldNames.end() || (index + 1) == m_worldNames.end()) {
        return;
    }

    const auto &next = m_worldNames[index - m_worldNames.begin() + 1];

    auto loaded = World::load(next);
    ASSERT(loaded);
    Game::removeScene<WorldMapScene>();
    Game::addScene<IngameScene>(std::move(loaded));

    Game::addScene<TransitionScene>(true);
    auto scene = Game::getScene<TransitionScene>();
    scene->setIsWhite(true);
}

} // namespace aren