#pragma once

#include "scene.hpp"
#include "../camera.hpp"
#include "sprite_set.hpp"
#include "beam.hpp"

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

    struct DebugBeam {
        Beam beam;
        Vector2f start;
        Vector2f dir;
    };
    std::optional<DebugBeam> m_debugBeam;

    template <size_t SIZE = 256>
    struct TextBoxProps {
        static constexpr size_t len = SIZE;
        std::array<char, SIZE> str = {};
        bool enabled = false;

        TextBoxProps<>() = default;
    };

    Vector2 m_panelScroll = {0, 0};
    Rectangle m_panelView = {0, 0, 0, 0};

    TextBoxProps<> m_savedWorldNameTB;

    int m_loadWorldNameSelected = 0;
    bool m_loadWorldNameEnabled = false;

    TextBoxProps<> m_worldNameTB;
    TextBoxProps<> m_worldCommentTB;

    TextBoxProps<8> m_zoneTB;

    TextBoxProps<> m_beamBouncesTB;

    enum class Tab : int {
        World,
        Brush,
        Wave,
    };
    Tab m_tab = Tab::World;
    bool m_tabEnabled = false;

    struct MonsterWaveProps {
        std::string zoneStr;
        TextBoxProps<> zoneTB;
        TextBoxProps<> monsterNameTB;
        bool conditionTypeEnabled = false;
        TextBoxProps<> conditionValueTB;
    };
    std::vector<MonsterWaveProps> m_monsterWaveProps;
};

} // namespace aren