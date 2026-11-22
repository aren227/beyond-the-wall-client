#include "settings_scene.hpp"
#include "title_scene.hpp"
#include "game.hpp"
#include "game_audio.hpp"
#include "../settings.hpp"
#include "../render.hpp"
#include "../ui.hpp"

namespace aren {

SettingsScene::SettingsScene() : Scene(TYPE) { }

void SettingsScene::update(float dt) {
    (void)dt;
    if (Input::keyPressed(KEY_ESCAPE)) {
        GameAudio::play(Sfx::Beep);
        Game::removeScene<SettingsScene>();
        Game::addScene<TitleScene>();
    }
}

void SettingsScene::render() {}

void SettingsScene::renderUI() {
    ClearBackground(BLACK);

    int sw = GetScreenWidth();
    int sh = GetScreenHeight();

    Vector2f c = V2f(sw * 0.5f, sh * 0.5f - 120);

    Render::drawText(c, 60, UI::PRIMARY, "Settings",
        Render::TextAlignH::Center, Render::TextAlignV::Center);
    c.y += 60;

    constexpr int VOLUME_STEPS = 10;
    int bgm = std::round(Settings::getBgmVolume() * VOLUME_STEPS);
    int sfx = std::round(Settings::getSfxVolume() * VOLUME_STEPS);

    constexpr float SLIDER_WIDTH = 200;

    Render::drawText(c, 40, UI::PRIMARY, "Music Volume",
        Render::TextAlignH::Right, Render::TextAlignV::Center);

    bgm = UI::slider(c + V2f(20 + SLIDER_WIDTH * 0.5f, 0), SLIDER_WIDTH, VOLUME_STEPS, "bgm", bgm);
    Settings::setBgmVolume(float(bgm) / VOLUME_STEPS);

    c.y += 60;

    Render::drawText(c, 40, UI::PRIMARY, "Effect Volume",
        Render::TextAlignH::Right, Render::TextAlignV::Center);

    sfx = UI::slider(c + V2f(20 + SLIDER_WIDTH * 0.5f, 0), SLIDER_WIDTH, VOLUME_STEPS, "sfx", sfx);
    Settings::setSfxVolume(float(sfx) / VOLUME_STEPS);

    c.y += 60;

    if (UI::textButton(c, 40, "Back", Render::TextAlignH::Center, Render::TextAlignV::Center)) {
        GameAudio::play(Sfx::Beep);
        Game::removeScene<SettingsScene>();
        Game::addScene<TitleScene>();
    }
    c.y += 60;
}

} // namespace aren
