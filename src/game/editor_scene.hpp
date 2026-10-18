#pragma once

#include "scene.hpp"
#include "../camera.hpp"
#include "world.hpp"
#include "sprite_set.hpp"

namespace aren {

enum class EditorBrush {
    Wall = 0,
    Barrier,
    Glass,
    Sand,
    __MAX__,
};

class EditorScene : public Scene {
public:
    static constexpr SceneType TYPE = SceneType::Editor;

    EditorScene();

    void update(float dt) override;
    void render() override;
    void renderUI() override;

private:
    Camera m_cam;
    World m_world;
    SpriteSet m_spriteSet;

    int m_brushIndex = 0;
    bool m_drawWireframe = true;
};

} // namespace aren