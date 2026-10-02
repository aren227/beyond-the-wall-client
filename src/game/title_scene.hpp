#pragma once

#include "scene.hpp"

namespace aren {

class TitleScene : public Scene {
public:
    static constexpr SceneType TYPE = SceneType::Title;

    TitleScene();

    void update(float dt) override;
    void render() override;
};

} // namespace aren
