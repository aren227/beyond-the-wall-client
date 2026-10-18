#pragma once

#include "scene.hpp"
#include "../camera.hpp"
#include "world.hpp"
#include "sprite_set.hpp"

namespace aren {

class IngameScene : public Scene {
public:
    static constexpr SceneType TYPE = SceneType::Ingame;

    IngameScene(const World &world);

    void update(float dt) override;
    void renderUI() override;
    void render() override;

private:
    Camera m_cam;
    World m_world;
    SpriteSet m_spriteSet;
};

} // namespace aren
