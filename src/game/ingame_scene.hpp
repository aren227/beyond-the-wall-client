#pragma once

#include "scene.hpp"
#include "../camera.hpp"
#include "sprite_set.hpp"

namespace aren {

class World;

class IngameScene : public Scene {
public:
    static constexpr SceneType TYPE = SceneType::Ingame;

    IngameScene(const World *world);
    virtual ~IngameScene();

    void update(float dt) override;
    void renderUI() override;
    void render() override;

    virtual bool isBackgroundOpaque() override { return true; }

private:
    Camera m_cam;
    std::unique_ptr<World> m_world;
    SpriteSet m_spriteSet;
};

} // namespace aren
