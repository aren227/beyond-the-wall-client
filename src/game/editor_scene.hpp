#pragma once

#include "scene.hpp"
#include "../camera.hpp"
#include "world.hpp"
#include "sprite_set.hpp"

namespace aren {

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

    std::array<char, 64> m_levelNameTextBox = {};
    bool m_levelNameTextBoxEnabled = false;

    std::array<char, 8> m_zoneTextBox = {};
    bool m_zoneTextBoxEnabled = false;
};

} // namespace aren