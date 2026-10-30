#include "editor_scene.hpp"
#include "../render.hpp"
#include "game.hpp"
#include "world.hpp"
#include "sprite_editor_scene.hpp"
#include "ingame_scene.hpp"
#include "transition_scene.hpp"

namespace aren {

enum class EditorBrush {
    Wall = 0,
    Barrier,
    Glass,
    Sand,
    Zone,
    White,
    Red,
    Green,
    Blue,
    __MAX__,
};

static const char *getBrushName(EditorBrush type) {
    switch (type) {
        case EditorBrush::Wall: return "Wall";
        case EditorBrush::Barrier: return "Barrier";
        case EditorBrush::Glass: return "Glass";
        case EditorBrush::Sand: return "Sand";
        case EditorBrush::Zone: return "Zone";
        case EditorBrush::White: return "White";
        case EditorBrush::Red: return "Red";
        case EditorBrush::Green: return "Green";
        case EditorBrush::Blue: return "Blue";
        default: return "N/A";
    }
}

static TileType getTileTypeFromBrush(EditorBrush type) {
    switch (type) {
        case EditorBrush::Wall: return TileType::Wall;
        case EditorBrush::Barrier: return TileType::Barrier;
        case EditorBrush::Glass: return TileType::Glass;
        case EditorBrush::Sand: return TileType::Sand;
        case EditorBrush::Zone: return TileType::Zone;
        default: return TileType::Empty;
    }
}

static BeamColor getBeamColorFromBrush(EditorBrush type) {
    switch (type) {
        case EditorBrush::Red: return BeamColor::Red;
        case EditorBrush::Green: return BeamColor::Green;
        case EditorBrush::Blue: return BeamColor::Blue;
        case EditorBrush::White:
        default:
            return BeamColor::White;
    }
}

static const std::string WORLD_BASE = "./resources/worlds";

EditorScene::EditorScene() : Scene(TYPE) {
    m_cam.m_zoom = GetScreenHeight() / 10.0f;
    m_spriteSet.load();

    m_world = std::make_unique<World>();

    refreshScannedWorldNames();
}

EditorScene::~EditorScene() {}

void EditorScene::update(float dt) {
    m_cam.update(dt);
}

void EditorScene::render() {
    ClearBackground(BLACK);

    Render::begin2DCam(m_cam);

    auto tl = m_cam.viewportToWorld(V2F_ZERO);
    auto br = m_cam.viewportToWorld(V2F_ONE);

    int t = std::ceil(tl.y);
    int b = std::floor(br.y);
    int l = std::ceil(tl.x);
    int r = std::floor(br.x);

    if (m_drawWireframe) {
        for (int i = t; i <= b; i++) {
            DrawLineV(V2f(tl.x, i), V2f(br.x, i), (i == 0) ? RED : DARKGRAY);
        }
        for (int i = l; i <= r; i++) {
            DrawLineV(V2f(i, tl.y), V2f(i, br.y), (i == 0) ? GREEN : DARKGRAY);
        }
    }

    // bounds
    {
        auto m = m_world->m_min, M = m_world->m_max + V2I_ONE;
        DrawLineV(V2f(m.x, m.y), V2f(M.x, m.y), BLUE);
        DrawLineV(V2f(M.x, m.y), V2f(M.x, M.y), BLUE);
        DrawLineV(V2f(M.x, M.y), V2f(m.x, M.y), BLUE);
        DrawLineV(V2f(m.x, M.y), V2f(m.x, m.y), BLUE);
    }

    Render::end2DCam();

    // TODO(aren): hardcoded
    // TODO(aren): We need a world space sprite render method!!
    for (int i = 0; i < 4; i++) {
        m_world->renderTiles(m_cam, m_spriteSet, i);
    }

    m_world->renderDecos(m_cam, m_spriteSet);;

    m_world->renderMisc(m_cam, m_spriteSet);
}

void EditorScene::renderUI() {
    m_world->renderDebugAsUI(m_cam, m_spriteSet);

    // camera control
    {
        const float sw = GetScreenWidth();
        const float sh = GetScreenHeight();

        const float zoomMin = sh / 10.0f;
        const float zoomMax = sh / 40.0f;

        m_cam.m_zoom = std::clamp(m_cam.m_zoom + Input::mouseWheel(), zoomMax, zoomMin);
        m_cam.m_offset = V2f(sw, sh) * 0.5f;

        if (Input::mouseButtonDown(2)) {
            m_cam.m_target -= Input::mouseDelta() / m_cam.m_zoom;
        }
    }

    if (Input::keyPressed(KEY_Q)) m_drawWireframe = !m_drawWireframe;

    // tile placement
    {
        auto mouse = m_cam.screenToWorld(Input::mousePosition()).floor();
        DrawRectangleV(mouse.f(), V2F_ONE, {255, 0, 0, 50});

        if (Input::mouseButtonDown(0)) {
            auto tileType = getTileTypeFromBrush(static_cast<EditorBrush>(m_brushIndex));
            if (tileType != TileType::Empty) {
                m_world->at(mouse) = Tile(tileType);
                if (tileType == TileType::Zone) {
                    m_world->at(mouse).m_name = m_zoneTextBox;
                }
            }

            auto beamColor = getBeamColorFromBrush(static_cast<EditorBrush>(m_brushIndex));
            if (m_world->contains(mouse)) {
                auto &tile = m_world->at(mouse);
                tile.m_color = beamColor;
            }
        }
        if (Input::mouseButtonDown(1)) {
            m_world->erase(mouse);
        }

        m_world->updateBounds();
    }

    GuiPanel({0, 0, 200, 800}, "Brush List");

    float y = 40;

    if (GuiTextBox({10, y, 100, 30}, m_saveWorldNameTextBox.data(), m_saveWorldNameTextBox.size(), m_saveWorldNameTextBoxEnabled)) {
        m_saveWorldNameTextBoxEnabled = !m_saveWorldNameTextBoxEnabled;
    }
    y += 40;

    if (GuiButton({10, y, 100, 30}, "Save")) {
        m_world->save(m_saveWorldNameTextBox.data());
    }
    y += 40;

    auto worldDropdownRect = Rect(10, y, 100, 30);
    // NOTE(aren): Draw later due to z index
    y += 40;

    if (GuiButton({10, y, 100, 30}, "Load")) {
        if (m_loadWorldNameSelected < m_scannedWorldNames.size()) {
            m_world = World::load(m_scannedWorldNames[m_loadWorldNameSelected]);
        }
    }
    y += 40;

    for (int i = 0; i < int(EditorBrush::__MAX__); i++) {
        int prev = GuiGetStyle(BUTTON, BASE_COLOR_NORMAL);
        if (i == m_brushIndex) {
            GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, Color4b(255, 127, 127, 255).rgba32());
        }

        auto label = ((i < 9) ? (std::to_string(i+1) + ": ") : "") + getBrushName(static_cast<EditorBrush>(i));

        if (GuiButton({10, y, 100, 30}, label.c_str())) {
            m_brushIndex = i;
        }

        if (i < 9 && Input::keyPressed(KEY_ONE + i) && m_zoneTextBoxEnabled == false) {
            m_brushIndex = i;
        }

        GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, prev);
        y += 40;
    }

    if (static_cast<EditorBrush>(m_brushIndex) == EditorBrush::Zone) {
        GuiLabel({10, y, 100, 30}, "Zone name");
        y += 40;
        if (GuiTextBox({10, y, 100, 30}, m_zoneTextBox.data(), m_zoneTextBox.size(), m_zoneTextBoxEnabled)) {
            m_zoneTextBoxEnabled = !m_zoneTextBoxEnabled;
        }
        y += 40;
    }

    // NOTE(aren): Pass 2
    static std::string s_joinedStr;
    s_joinedStr.clear();
    for (size_t i = 0; i < m_scannedWorldNames.size(); i++) {
        s_joinedStr += m_scannedWorldNames[i];
        if (i + 1 < m_scannedWorldNames.size()) {
            s_joinedStr += ";";
        }
    }

    if (GuiDropdownBox(worldDropdownRect, s_joinedStr.c_str(), &m_loadWorldNameSelected, m_loadWorldNameSelected)) {
        m_loadWorldNameEnabled = !m_loadWorldNameEnabled;
    }

    if (Input::keyPressed(KEY_ONE)) {
        m_brushIndex = 0;
    }

    if (Input::keyPressed(KEY_F5)) {
        Game::addScene<IngameScene>(m_world.get());
        Game::addScene<TransitionScene>(true);
    }
}

void EditorScene::refreshScannedWorldNames() {
    m_scannedWorldNames.clear();

    if (DirectoryExists(WORLD_BASE.c_str())) {
        auto res = LoadDirectoryFilesEx(WORLD_BASE.c_str(), ".lvl", false);
        for (size_t i = 0; i < res.count; i++) {
            auto name = std::string(GetFileNameWithoutExt(res.paths[i]));
            m_scannedWorldNames.push_back(std::move(name));
        }
        UnloadDirectoryFiles(res);
    }
}

}