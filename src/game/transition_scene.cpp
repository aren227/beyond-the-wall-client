#include "transition_scene.hpp"
#include "game.hpp"

namespace aren {

TransitionScene::TransitionScene(bool isOpen) : Scene(TYPE) {
    m_isOpen = isOpen;
    m_spriteSet.load();
}

void TransitionScene::update(float dt) {
    m_time += dt;
}

void TransitionScene::render() {
}

void TransitionScene::renderUI() {
    constexpr float duration = 0.7f;
    if (m_time > duration) {
        Game::removeScene<TransitionScene>();
        return;
    }

    int r = std::floor(m_time / duration / 17);

    if (m_isOpen) {
        r = 16 - r;
    }

    m_spriteSet.dither.render(Rect(0, 0, GetScreenWidth(), GetScreenHeight()), r, 0, 2, WHITE);
}

} // namespace aren