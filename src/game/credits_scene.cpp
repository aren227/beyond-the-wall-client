#include "credits_scene.hpp"
#include "title_scene.hpp"
#include "game.hpp"
#include "game_audio.hpp"
#include "../settings.hpp"
#include "../render.hpp"
#include "../ui.hpp"

namespace aren {

CreditsScene::CreditsScene() : Scene(TYPE) { }

void CreditsScene::update(float dt) {
    (void)dt;
    if (Input::keyPressed(KEY_ESCAPE)) {
        GameAudio::play(Sfx::Beep);
        Game::removeScene<CreditsScene>();
        Game::addScene<TitleScene>();
    }
}

void CreditsScene::render() {}

void CreditsScene::renderUI() {
    ClearBackground(BLACK);

    int sw = GetScreenWidth();
    int sh = GetScreenHeight();

    Vector2f c = V2f(sw * 0.5f, sh * 0.5f - 120);

    Render::drawText(c, 60, UI::PRIMARY, "Credits",
        Render::TextAlignH::Center, Render::TextAlignV::Center);
    c.y += 60;

    if (UI::textButton(c, 40, "Game by aren227 (itch.io)",
        Render::TextAlignH::Center, Render::TextAlignV::Center)) {
        OpenURL("https://aren227.itch.io/beyond-the-wall");
    }
    c.y += 60;

    if (UI::textButton(c, 40, "Made with raylib (raylib.com)",
        Render::TextAlignH::Center, Render::TextAlignV::Center)) {
        OpenURL("https://www.raylib.com/");
    }
    c.y += 60;

    if (UI::textButton(c, 40, "Music: 'The Cairn' by Kubbi (bandcamp.com)",
        Render::TextAlignH::Center, Render::TextAlignV::Center)) {
        OpenURL("https://kubbi.bandcamp.com/");
    }
    c.y += 60;

    if (UI::textButton(c, 40, "Font: 'Jersey 10' by Sarah Cadigan-Fried (github.com)",
        Render::TextAlignH::Center, Render::TextAlignV::Center)) {
        OpenURL("https://github.com/scfried/soft-type-jersey");
    }
    c.y += 60;

    Render::drawText(c, 40, UI::PRIMARY, "Thanks for playing!",
        Render::TextAlignH::Center, Render::TextAlignV::Center);
    c.y += 60;

    if (UI::textButton(c, 40, "Back", Render::TextAlignH::Center, Render::TextAlignV::Center)) {
        GameAudio::play(Sfx::Beep);
        Game::removeScene<CreditsScene>();
        Game::addScene<TitleScene>();
    }
    c.y += 60;
}

} // namespace aren
