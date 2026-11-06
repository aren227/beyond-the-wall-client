#pragma once

#include "scene.hpp"

namespace aren {

class PauseScene : public Scene {
public:
    static constexpr SceneType TYPE = SceneType::Pause;

    PauseScene();
    virtual ~PauseScene();

    void update(float dt) override;
    void renderUI() override;
    void render() override;
};

} // namespace aren