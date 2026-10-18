#pragma once

#include "scene.hpp"

namespace aren {

class IngameScene : public Scene {
public:
    static constexpr SceneType TYPE = SceneType::Ingame;

    IngameScene();

    void update(float dt) override;
    void renderUI() override;
    void render() override;
};

} // namespace aren
