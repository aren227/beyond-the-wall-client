#pragma once

#include "scene.hpp"

namespace aren {

class SettingsScene : public Scene {
public:
    static constexpr SceneType TYPE = SceneType::Settings;

    SettingsScene();

    void update(float dt) override;
    void renderUI() override;
    void render() override;
};

} // namespace aren
