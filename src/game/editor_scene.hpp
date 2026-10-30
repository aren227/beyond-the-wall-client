#pragma once

#include "scene.hpp"
#include "../camera.hpp"
#include "sprite_set.hpp"

namespace aren {

class World;

class EditorScene : public Scene {
public:
    static constexpr SceneType TYPE = SceneType::Editor;

    EditorScene();
    virtual ~EditorScene();

    void update(float dt) override;
    void render() override;
    void renderUI() override;

private:
    void refreshScannedWorldNames();

private:
    Camera m_cam;
    std::unique_ptr<World> m_world;
    SpriteSet m_spriteSet;

    std::vector<std::string> m_scannedWorldNames;

    int m_brushIndex = 0;
    bool m_drawWireframe = true;

    std::array<char, 64> m_saveWorldNameTextBox = {};
    bool m_saveWorldNameTextBoxEnabled = false;

    int m_loadWorldNameSelected = 0;
    bool m_loadWorldNameEnabled = false;

    std::array<char, 8> m_zoneTextBox = {};
    bool m_zoneTextBoxEnabled = false;
};

} // namespace aren