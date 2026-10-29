#include "ingame_scene.hpp"
#include "game.hpp"
#include "world.hpp"

namespace aren {

IngameScene::IngameScene(const World *world) : Scene(TYPE) {
    m_world = std::make_unique<World>();
    m_world->tiles() = world->tiles();

    m_spriteSet.load();
}

IngameScene::~IngameScene() {}

void IngameScene::update(float dt) {
    m_world->update(m_cam, dt);
}

void IngameScene::render() {
    ClearBackground(BLACK);

    // camera
    {
        auto min = m_world->m_min;
        auto max = m_world->m_max;

        max += V2I_ONE;

        float sw = GetScreenWidth();
        float sh = GetScreenHeight();

        const float PADDING_WS = 1.5f;

        float xZoom = sw / (max.x - min.x + PADDING_WS * 2);
        float yZoom = sh / (max.y - min.y + PADDING_WS * 2);

        m_cam.m_zoom = std::min(xZoom, yZoom);
        m_cam.m_target = (min + max).f() / 2.0f;
        m_cam.m_offset = V2f(sw, sh) * 0.5f;
    }

    // TODO(aren): hardcoded
    for (int i = 0; i < 4; i++) {
        m_world->renderTiles(m_cam, m_spriteSet, i);
    }

    m_world->renderDecos(m_cam, m_spriteSet);

    m_world->renderMisc(m_cam, m_spriteSet);
}

void IngameScene::renderUI() {
    if (Input::keyPressed(KEY_ESCAPE)) {
        Game::removeScene<IngameScene>();
    }
}

} // namespace aren
