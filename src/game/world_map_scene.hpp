#pragma once

#include "scene.hpp"
#include "../camera.hpp"
#include "sprite_set.hpp"

namespace aren {

class World;

class WorldMapScene : public Scene {
public:
    static constexpr SceneType TYPE = SceneType::WorldMap;

    WorldMapScene();
    virtual ~WorldMapScene();

    void update(float dt) override;
    void renderUI() override;
    void render() override;

    virtual bool isBackgroundOpaque() override { return true; }

    void gotoNextImmediately(const std::string &currentWorldName);

private:
    Camera m_cam;
    std::unique_ptr<World> m_world;
    SpriteSet m_spriteSet;

    const float m_hudHeight = 0;
    float m_hudWidth = 0;

    std::vector<std::string> m_worldNames;
};

} // namespace aren