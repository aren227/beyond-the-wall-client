#include "ingame_scene.hpp"
#include "game.hpp"
#include "world.hpp"
#include "player.hpp"
#include "../render.hpp"

namespace aren {

IngameScene::IngameScene(const World *world) : Scene(TYPE) {
    m_world = std::make_unique<World>(*world);

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
        m_world->renderTiles(m_cam, m_spriteSet, i, 1);
    }

    m_world->renderDecos(m_cam, m_spriteSet);

    m_world->renderMisc(m_cam, m_spriteSet);
}

void IngameScene::renderUI() {
    float sw = GetScreenWidth();
    float sh = GetScreenHeight();
    float sx = (sw - m_hudWidth) * 0.5f;
    const float xMargin = 4;

    Render::drawText(
        V2f(sx + xMargin, sh-36),
        32,
        WHITE,
        m_world->m_name.c_str(),
        Render::TextAlignH::Left,
        Render::TextAlignV::Top
    );

    const float healthWidth = 200;
    const auto player = m_world->player();

    Render::drawText(
        V2f(sx+m_hudWidth-healthWidth-xMargin-50, sh-36),
        32,
        WHITE,
        TextFormat("x%d", player->m_bounces),
        Render::TextAlignH::Left,
        Render::TextAlignV::Top
    );

    auto healthRect = Rect(sx+m_hudWidth-healthWidth-xMargin, sh-30, healthWidth, 22);
    DrawRectangleRec(healthRect, DARKGRAY);
    healthRect.w *= player->m_health / player->m_maxHealth;
    DrawRectangleRec(healthRect, WHITE);

    if (Input::keyPressed(KEY_ESCAPE)) {
        Game::removeScene<IngameScene>();
    }
}

} // namespace aren
