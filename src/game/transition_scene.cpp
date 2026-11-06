#include "transition_scene.hpp"
#include "game.hpp"

namespace aren {

static constexpr float DURATION = 0.7f;

TransitionScene::TransitionScene(bool isOpen) : Scene(TYPE) {
    m_isOpen = isOpen;
    m_spriteSet.load();
}

void TransitionScene::update(float dt) {
    m_time += dt;

    if (m_time > DURATION) {
        Game::removeScene<TransitionScene>();

        if (m_onEnd) {
            m_onEnd();
        }
    }
}

void TransitionScene::render() {
}

void TransitionScene::renderUI() {
    int r = std::floor(m_time / DURATION * 17);

    if (m_isOpen) {
        r = 16 - r;
    }

    const auto &sprite = m_isWhite ? m_spriteSet.dither : m_spriteSet.ditherBlack;

    sprite.render(Rect(0, 0, GetScreenWidth(), GetScreenHeight()), r, 0, 2, WHITE);
}

} // namespace aren