#pragma once

#include "scene.hpp"
#include "../camera.hpp"
#include "sprite_set.hpp"

namespace aren {

class World;

class IngameScene : public Scene {
public:
    static constexpr SceneType TYPE = SceneType::Ingame;

    IngameScene(std::unique_ptr<World> world);
    virtual ~IngameScene();

    void update(float dt) override;
    void renderUI() override;
    void render() override;

    virtual bool isBackgroundOpaque() override { return true; }

private:
    Camera m_cam;
    std::unique_ptr<World> m_world;
    std::unique_ptr<World> m_worldInitCoied;
    SpriteSet m_spriteSet;

    const float m_hudHeight = 16;
    float m_hudWidth = 0;
};

} // namespace aren
