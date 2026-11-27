#include "ingame_scene.hpp"
#include "game.hpp"
#include "world.hpp"
#include "player.hpp"
#include "pause_scene.hpp"
#include "transition_scene.hpp"
#include "ingame_scene.hpp"
#include "world_map_scene.hpp"
#include "../render.hpp"
#include "../storage.hpp"
#include "consts.hpp"

#if DEBUG
#include "editor_scene.hpp"
#endif

namespace aren {

IngameScene::IngameScene(std::unique_ptr<World> world) : Scene(TYPE) {
    m_world = std::move(world);
    m_worldInitCoied = std::make_unique<World>(m_world.get());

    m_spriteSet.load();

    if (m_world->m_name == SUPER_CONSTELLATION) {
        m_extraHudHeight = 30;
    }
}

IngameScene::~IngameScene() {}

static bool isEditorActive() {
#if DEBUG
    return Game::getScene<EditorScene>() != nullptr;
#else
    return false;
#endif
}

void IngameScene::update(float dt) {
    if (Game::getScene<PauseScene>() == nullptr) {
        bool prevEndFlag = m_world->hasEnded();
        m_world->update(m_cam, dt);
        bool currEndFlag = m_world->hasEnded();

        if (prevEndFlag == false && currEndFlag) {
            // TODO(aren)
            if (Game::getScene<TransitionScene>() != nullptr) {
                Game::removeScene<TransitionScene>();
            }

            Game::addScene<TransitionScene>(false);
            bool win = m_world->hasPlayerWon();
            bool perfectlyWin = m_world->hasPlayerWon() && m_world->player()->m_health >= m_world->player()->m_maxHealth;
            std::string currentWorldName = m_world->m_filename;
            bool shouldRetry = m_world->m_autoRetryMode;
            Game::getScene<TransitionScene>()->setIsWhite(win);
            Game::getScene<TransitionScene>()->setOnEnd([win, perfectlyWin, currentWorldName, shouldRetry]() {
                auto initCopiedWorld = std::move(Game::getScene<IngameScene>()->m_worldInitCoied);
                Game::removeScene<IngameScene>();

                if (isEditorActive() == false) {
                    if (win) {
                        // NOTE(aren): add this level to the set
                        auto names = Storage::getList(COMPLETED_WORLD_KEYS);
                        if (std::find(names.begin(), names.end(), currentWorldName) == names.end()) {
                            names.push_back(currentWorldName);
                        }
                        Storage::setList(COMPLETED_WORLD_KEYS, names);

                        if (perfectlyWin) {
                            auto names = Storage::getList(PERFECTLY_COMPLETED_WORLD_KEYS);
                            if (std::find(names.begin(), names.end(), currentWorldName) == names.end()) {
                                names.push_back(currentWorldName);
                            }
                            Storage::setList(PERFECTLY_COMPLETED_WORLD_KEYS, names);
                        }

                        Game::addScene<WorldMapScene>();
                        if (shouldRetry) {
                            Game::getScene<WorldMapScene>()->gotoSelfImmediately(currentWorldName);
                        } else {
                            Game::getScene<WorldMapScene>()->gotoNextImmediately(currentWorldName);
                        }
                    } else {
                        Game::addScene<IngameScene>(std::move(initCopiedWorld));
                    }
                }
            });
        }
    }

    if (Input::keyPressed(KEY_ESCAPE)) {
        if (isEditorActive()) {
            Game::removeScene<IngameScene>();
        } else if (Game::getScene<PauseScene>() == nullptr && Game::getScene<TransitionScene>() == nullptr) {
            Game::addScene<PauseScene>();
        }
    }

    m_cam.update(dt);
}

void IngameScene::render() {
    ClearBackground(BLACK_BG.f().linear());

    // camera
    {
        auto min = m_world->m_min;
        auto max = m_world->m_max;

        max += V2I_ONE;

        float sw = GetScreenWidth();
        float sh = GetScreenHeight() - m_hudHeight - m_extraHudHeight;

        const float PADDING_WS = 0.25f;

        float xZoom = sw / (max.x - min.x + PADDING_WS * 2);
        float yZoom = sh / (max.y - min.y + PADDING_WS * 2);

        m_cam.m_zoom = std::min(xZoom, yZoom);
        m_cam.m_target = (min + max).f() / 2.0f;
        m_cam.m_offset = V2f(sw, sh) * 0.5f + V2f(0, m_extraHudHeight);

        m_hudWidth = m_cam.m_zoom * (max.x - min.x + 1);
    }

    m_world->renderTilesBg(m_cam, m_spriteSet);
    m_world->renderDecos(m_cam, m_spriteSet);
    // TODO(aren): hardcoded
    for (int i = 0; i < 4; i++) {
        m_world->renderTiles(m_cam, m_spriteSet, i);
    }
    m_world->renderTilesMisc(m_cam, m_spriteSet);

    m_world->renderMisc(m_cam, m_spriteSet);
}

void IngameScene::renderUI() {
    float sw = GetScreenWidth();
    float sh = GetScreenHeight();
    float sx = (sw - m_hudWidth) * 0.5f;
    const float xMargin = 16;

    Render::drawText(
        V2f(sx + xMargin, sh-40),
        40,
        WHITE,
        m_world->m_name.c_str(),
        Render::TextAlignH::Left,
        Render::TextAlignV::Top
    );

    float healthWidth = 200;
    if (m_world->m_name == SUPER_CONSTELLATION) {
        healthWidth = 500;
    }

    const auto player = m_world->player();

    Render::drawText(
        V2f(sx+m_hudWidth-healthWidth-xMargin-50, sh-40),
        40,
        WHITE,
        TextFormat("x%d", player->m_bounces),
        Render::TextAlignH::Left,
        Render::TextAlignV::Top
    );

    auto healthRect = Rect(sx+m_hudWidth-healthWidth-xMargin, sh-30, healthWidth, 22);
    DrawRectangleRec(healthRect, DARKGRAY);
    healthRect.w *= player->m_health / player->m_maxHealth;
    DrawRectangleRec(healthRect, WHITE);

    if (m_world->player()->m_beamDisabled == false) {
        DrawLineEx(Input::mousePosition() + V2f(-3, 0), Input::mousePosition() + V2f(-10, 0), 1, WHITE);
        DrawLineEx(Input::mousePosition() + V2f(3, 0), Input::mousePosition() + V2f(10, 0), 1, WHITE);
        DrawLineEx(Input::mousePosition() + V2f(0, -3), Input::mousePosition() + V2f(0, -10), 1, WHITE);
        DrawLineEx(Input::mousePosition() + V2f(0, 3), Input::mousePosition() + V2f(0, 10), 1, WHITE);
    }

    if (m_world->m_name == SUPER_CONSTELLATION) {
        auto currentColor = C4F_WHITE.byte();
        auto bestColor = Color4b(GOLD); // gold2 is weird in here

        if (m_world->m_scScore > m_world->m_scBestScore) {
            std::swap(currentColor, bestColor);
        }

        Render::drawText(
            V2f(sx + xMargin, 0),
            40,
            currentColor,
            "CURRENT",
            Render::TextAlignH::Left,
            Render::TextAlignV::Top
        );
        Render::drawText(
            V2f(sx + xMargin + 150, 0),
            40,
            currentColor,
            TextFormat("%02d", m_world->m_scScore),
            Render::TextAlignH::Left,
            Render::TextAlignV::Top
        );

        if (m_world->m_autoRetryMode) {
            Render::drawText(
                V2f(sx + m_hudWidth * 0.5f, 0),
                40,
                WHITE,
                "AUTO RETRY MODE",
                Render::TextAlignH::Center,
                Render::TextAlignV::Top
            );
        }

        Render::drawText(
            V2f(sx + m_hudWidth - xMargin - 60, 0),
            40,
            bestColor,
            "BEST",
            Render::TextAlignH::Right,
            Render::TextAlignV::Top
        );
        Render::drawText(
            V2f(sx + m_hudWidth - xMargin, 0),
            40,
            bestColor,
            TextFormat("%02d", m_world->m_scBestScore),
            Render::TextAlignH::Right,
            Render::TextAlignV::Top
        );
    }
}

const std::string &IngameScene::worldName() {
    return m_world->m_filename;
}

} // namespace aren
