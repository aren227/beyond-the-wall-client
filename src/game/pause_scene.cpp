#include "pause_scene.hpp"
#include "game.hpp"
#include "world_map_scene.hpp"
#include "ingame_scene.hpp"
#include "world.hpp"
#include "transition_scene.hpp"
#include "../render.hpp"
#include "../ui.hpp"

namespace aren {

static void resumeGame() {
    Game::removeScene<PauseScene>();
}

static void retryGame() {
    Game::addScene<TransitionScene>(false);
    auto name = Game::getScene<IngameScene>()->worldName();
    auto scene = Game::getScene<TransitionScene>();
    scene->setOnEnd([name]() {
        Game::removeScene<PauseScene>();
        Game::removeScene<IngameScene>();

        auto loaded = World::load(name);
        ASSERT(loaded);
        Game::addScene<IngameScene>(std::move(loaded));
        Game::addScene<TransitionScene>(true);
    });
}

static void exitGame() {
    Game::addScene<TransitionScene>(false);
    auto scene = Game::getScene<TransitionScene>();
    scene->setOnEnd([]() {
        Game::removeScene<PauseScene>();
        Game::removeScene<IngameScene>();

        Game::addScene<WorldMapScene>();
        Game::addScene<TransitionScene>(true);
    });
}

PauseScene::PauseScene() : Scene(TYPE) {}

PauseScene::~PauseScene() {}

void PauseScene::update(float dt) {
    (void)dt;
    if (Input::keyPressed(KEY_ESCAPE) && Game::getScene<TransitionScene>() == nullptr) {
        resumeGame();
    }
}

void PauseScene::renderUI() {
    int sw = GetScreenWidth();
    int sh = GetScreenHeight();

    DrawRectangleRec(Rect(0, 0, sw, sh), {0, 0, 0, 200});

    Vector2f c = V2f(sw * 0.5f, sh * 0.5f - 120);

    Render::drawText(c, 60, UI::PRIMARY, "Paused",
    Render::TextAlignH::Center, Render::TextAlignV::Center);
    c.y += 60;

    if (UI::textButton(c, 40, "Resume", Render::TextAlignH::Center, Render::TextAlignV::Center)) {
        resumeGame();
        return;
    }
    c.y += 60;

    if (UI::textButton(c, 40, "Retry", Render::TextAlignH::Center, Render::TextAlignV::Center)) {
        retryGame();
        return;
    }
    c.y += 60;

    if (UI::textButton(c, 40, "Exit", Render::TextAlignH::Center, Render::TextAlignV::Center)) {
        exitGame();
        return;
    }
    c.y += 60;
}

void PauseScene::render() {}

} // namespace aren