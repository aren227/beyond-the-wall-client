#pragma once

#include "scene.hpp"

namespace aren {

class CreditsScene : public Scene {
public:
    static constexpr SceneType TYPE = SceneType::Settings;

    CreditsScene();

    void update(float dt) override;
    void renderUI() override;
    void render() override;
};

} // namespace aren
