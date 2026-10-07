#include "game.hpp"
#include "title_scene.hpp"
#include "../ui.hpp"
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#ifdef DEBUG
#include "../render.hpp"
#endif

namespace aren {

static Game *s_instance;

Game::Game() {
    s_instance = this;
}

Game &Game::instance() {
    return *s_instance;
}

void Game::init() {
    Game::addScene<TitleScene>();
}

void Game::update(float dt) {
    m_lastDt = dt;

#if DEBUG
    if (Input::keyPressed(KEY_F12)) {
        auto t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        std::ostringstream name;
        name << std::put_time(std::localtime(&t), "screenshot_%Y%m%d_%H%M%S.png");
        TakeScreenshot(name.str().c_str());
    }
#endif

    if (m_scenes.empty()) return;
    std::shared_ptr<Scene> top = m_scenes.back();
    top->update(dt);
}

void Game::render() {
    // TODO(aren): May require better handling..
    static std::vector<std::shared_ptr<Scene>> s_tempScenes;
    s_tempScenes.reserve(8);
    for (const auto &it : m_scenes) {
        s_tempScenes.push_back(it);
    }

    for (size_t i = 0; i < s_tempScenes.size(); i++) {
        bool isTop = (i + 1 == s_tempScenes.size());
        if (isTop) UI::beginInput();
        Render::pushTint(isTop ? WHITE : Color{0, 0, 0, 0});
        s_tempScenes[i]->render();
        Render::popTint();
        if (isTop) UI::endInput();
    }

    s_tempScenes.clear();
}

void Game::renderUI() {
    // TODO(aren): Garbage code needs to fix
    // TODO(aren): May require better handling..
    static std::vector<std::shared_ptr<Scene>> s_tempScenes;
    s_tempScenes.reserve(8);
    for (const auto &it : m_scenes) {
        s_tempScenes.push_back(it);
    }

    for (size_t i = 0; i < s_tempScenes.size(); i++) {
        bool isTop = (i + 1 == s_tempScenes.size());
        if (isTop) UI::beginInput();
        Render::pushTint(isTop ? WHITE : Color{0, 0, 0, 0});
        s_tempScenes[i]->renderUI();
        Render::popTint();
        if (isTop) UI::endInput();
    }

    s_tempScenes.clear();

#ifdef DEBUG
    int fps = m_lastDt > 0.0f ? (int)std::round(1.0f / m_lastDt) : 0;
    Render::drawText(V2f(8, 8), 20.0f,
        {255, 255, 255, 200},
        TextFormat("fps: %d", fps),
        Render::TextAlignH::Left,
        Render::TextAlignV::Top);
#endif
}

void Game::removeScene(SceneType type) {
    Game &self = instance();
    for (auto it = self.m_scenes.begin(); it != self.m_scenes.end(); ++it) {
        if ((*it)->type() == type) {
            self.m_scenes.erase(it);
            return;
        }
    }
    ASSERTF(false, "removeScene: type %d not active", (int)type);
}

void Game::clearScene() {
    Game &self = instance();
    while (!self.m_scenes.empty()) {
        self.m_scenes.pop_back();
    }
}

Scene *Game::getScene(SceneType type) {
    Game &self = instance();
    for (const auto &it : self.m_scenes) {
        if (it->type() == type) return it.get();
    }
    return nullptr;
}

size_t Game::sceneCount() {
    return instance().m_scenes.size();
}

bool Game::isTopScene(const Scene *s) {
    Game &self = instance();
    if (self.m_scenes.empty()) return false;
    return self.m_scenes.back().get() == s;
}

void Game::requestExit() {
    instance().m_shouldExit = true;
}

} // namespace aren
