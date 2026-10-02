#include "settings_scene.hpp"
#include "game.hpp"
#include "../settings.hpp"
#include "../render.hpp"
#include "../ui.hpp"

namespace aren {

constexpr float SETTINGS_TITLE_SIZE     = 90.0f;
constexpr float SETTINGS_ROW_SIZE       = 32.0f;
constexpr float SETTINGS_BUTTON_SIZE    = 50.0f;
constexpr float SETTINGS_ROW_GAP        = 60.0f;
constexpr int   SETTINGS_VOLUME_STEPS   = 10;

SettingsScene::SettingsScene() : Scene(TYPE) { }

void SettingsScene::update(float dt) {
    (void)dt;
    if (Input::keyPressed(KEY_ESCAPE)) {
        Game::removeScene<SettingsScene>();
    }
}

void SettingsScene::render() {
    int sw = GetScreenWidth();
    int sh = GetScreenHeight();
    Vector2f c = V2f(sw * 0.5f, sh * 0.5f);

    Render::drawText(V2f(c.x, c.y - SETTINGS_ROW_GAP * 3.0f),
        SETTINGS_TITLE_SIZE, UI::s_colorTextPrimary, "Settings",
        Render::TextAlignH::Center,
        Render::TextAlignV::Center);

    int bgmVolume = std::round(Settings::getBgmVolume() * SETTINGS_VOLUME_STEPS);
    int sfxVolume = std::round(Settings::getSfxVolume() * SETTINGS_VOLUME_STEPS);
    bool cameraShake = Settings::getCameraShake();

    float row = c.y - SETTINGS_ROW_GAP;

    bgmVolume = UI::slider(V2f(c.x, row),
        SETTINGS_ROW_SIZE,
        "BGM Volume",
        bgmVolume,
        SETTINGS_VOLUME_STEPS);
    row += SETTINGS_ROW_GAP;

    sfxVolume = UI::slider(V2f(c.x, row),
        SETTINGS_ROW_SIZE,
        "SFX Volume",
        sfxVolume,
        SETTINGS_VOLUME_STEPS);
    row += SETTINGS_ROW_GAP;

    cameraShake = UI::checkbox(V2f(c.x, row),
        SETTINGS_ROW_SIZE,
        "Camera Shake",
        cameraShake);
    row += SETTINGS_ROW_GAP;

    Settings::setBgmVolume(float(bgmVolume) / SETTINGS_VOLUME_STEPS);
    Settings::setSfxVolume(float(sfxVolume) / SETTINGS_VOLUME_STEPS);
    Settings::setCameraShake(cameraShake);

    row += SETTINGS_ROW_GAP * 0.5f;
    if (UI::textButton(V2f(c.x, row), SETTINGS_BUTTON_SIZE, "Back")) {
        Game::removeScene<SettingsScene>();
    }
}

} // namespace aren
