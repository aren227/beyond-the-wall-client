#include "title_scene.hpp"
#include "game.hpp"
#include "../ui.hpp"
#include "../render.hpp"
#include "settings_scene.hpp"

namespace aren {

constexpr float TITLE_NAME_SIZE         = 120.0f;
constexpr float TITLE_BUTTON_SIZE       = 50.0f;
constexpr float TITLE_BUTTON_GAP        = 70.0f;
constexpr float TITLE_BLOCK_OFFSET_Y    = 30.0f;
constexpr float TITLE_VERSION_SIZE      = 30.0f;
constexpr float TITLE_VERSION_PAD       = 20.0f;

static void startGame() {
    Game::removeScene<TitleScene>();
}

TitleScene::TitleScene() : Scene(TYPE) {}

void TitleScene::update(float dt) {
    (void)dt;
    if (Input::keyPressed(KEY_ENTER) || Input::keyPressed(KEY_KP_ENTER)) {
        startGame();
    }
}

void TitleScene::render() {
    int sw = GetScreenWidth();
    int sh = GetScreenHeight();
    Vector2f c = V2f(sw * 0.5f, sh * 0.5f + TITLE_BLOCK_OFFSET_Y);

    Render::drawText(V2f(c.x, c.y - TITLE_BUTTON_GAP * 3.0f),
        TITLE_NAME_SIZE, UI::s_colorTextPrimary, "Beyond The Wall",
        Render::TextAlignH::Center, Render::TextAlignV::Center);

    float row = c.y - TITLE_BUTTON_GAP * 1.5f;
    if (UI::textButton(V2f(c.x, row), TITLE_BUTTON_SIZE, "Start")) {
        startGame();
        return;
    }
    row += TITLE_BUTTON_GAP;

    if (UI::textButton(V2f(c.x, row), TITLE_BUTTON_SIZE, "Settings")) {
        Game::addScene<SettingsScene>();
        return;
    }
    row += TITLE_BUTTON_GAP;

#if !defined(PLATFORM_WEB)
    if (UI::textButton(V2f(c.x, row), TITLE_BUTTON_SIZE, "Exit")) {
        Game::requestExit();
    }
    row += TITLE_BUTTON_GAP;
#endif

    Render::drawText(V2f(TITLE_VERSION_PAD, sh - TITLE_VERSION_PAD),
        TITLE_VERSION_SIZE, UI::s_colorTextSecondary,
        TextFormat("version %s", GAME_VERSION),
        Render::TextAlignH::Left, Render::TextAlignV::Bottom);

    Render::drawText(V2f(sw - TITLE_VERSION_PAD, sh - TITLE_VERSION_PAD),
        TITLE_VERSION_SIZE, UI::s_colorTextSecondary,
        "A game by aren227",
        Render::TextAlignH::Right, Render::TextAlignV::Bottom);
}

} // namespace aren
