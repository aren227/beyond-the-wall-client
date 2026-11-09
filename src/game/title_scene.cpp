#include "title_scene.hpp"
#include "game.hpp"
#include "../ui.hpp"
#include "../render.hpp"
#include "settings_scene.hpp"
#include "world_map_scene.hpp"

#if DEBUG
#include "editor_scene.hpp"
#endif

namespace aren {

static void startGame() {
    Game::removeScene<TitleScene>();
    Game::addScene<WorldMapScene>();
}

#if DEBUG
static void startEditor() {
    Game::removeScene<TitleScene>();
    Game::addScene<EditorScene>();
}
#endif

static void startSettings() {
    Game::removeScene<TitleScene>();
    Game::addScene<SettingsScene>();
}

TitleScene::TitleScene() : Scene(TYPE) {}

void TitleScene::update(float dt) {
    (void)dt;
    if (Input::keyPressed(KEY_ENTER) || Input::keyPressed(KEY_KP_ENTER)) {
        startGame();
    }
}

void TitleScene::render() {}

void TitleScene::renderUI() {
    ClearBackground(BLACK);

    int sw = GetScreenWidth();
    int sh = GetScreenHeight();

    Vector2f c = V2f(sw * 0.5f, sh * 0.5f - 120);

    Render::drawText(c, 60, UI::PRIMARY, "Beyond The Wall",
        Render::TextAlignH::Center, Render::TextAlignV::Center);
    c.y += 60;

    if (UI::textButton(c, 40, "Start", Render::TextAlignH::Center, Render::TextAlignV::Center)) {
        startGame();
        return;
    }
    c.y += 60;

#if DEBUG
    if (UI::textButton(c, 40, "Editor", Render::TextAlignH::Center, Render::TextAlignV::Center)) {
        startEditor();
        return;
    }
    c.y += 60;
#endif

    if (UI::textButton(c, 40, "Settings", Render::TextAlignH::Center, Render::TextAlignV::Center)) {
        startSettings();
        return;
    }
    c.y += 60;

#if !defined(PLATFORM_WEB)
    if (UI::textButton(c, 40, "Exit", Render::TextAlignH::Center, Render::TextAlignV::Center)) {
        Game::requestExit();
    }
    c.y += 60;
#endif

    Render::drawText(V2f(10, sh - 10), 20, UI::SECONDARY,
        TextFormat("version %s", GAME_VERSION),
        Render::TextAlignH::Left, Render::TextAlignV::Bottom);

    Render::drawText(V2f(sw - 10, sh - 10), 20, UI::SECONDARY,
        "A game by aren227",
        Render::TextAlignH::Right, Render::TextAlignV::Bottom);
}

} // namespace aren
