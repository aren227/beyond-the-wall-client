#pragma once

#include "scene.hpp"
#include "sprite_set.hpp"

namespace aren {

class TransitionScene : public Scene {
public:
    static constexpr SceneType TYPE = SceneType::TransitionScene;

    TransitionScene(bool isOpen);

    void update(float dt) override;
    void render() override;
    void renderUI() override;

    void setOnEnd(std::function<void()> onEnd) { m_onEnd = onEnd; }

private:
    bool m_isOpen;
    SpriteSet m_spriteSet;
    float m_time = 0;
    std::function<void()> m_onEnd;
};

} // namespace aren