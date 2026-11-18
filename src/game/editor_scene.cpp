#if DEBUG

#include "editor_scene.hpp"
#include "../render.hpp"
#include "game.hpp"
#include "world.hpp"
#include "ingame_scene.hpp"
#include "transition_scene.hpp"
#include "title_scene.hpp"
#include "player.hpp"
#include "consts.hpp"

namespace aren {

enum class EditorBrush {
    Wall = 0,
    Barrier,
    Glass,
    Sand,
    Zone,
    PlayerSpawn,
    End,
    WorldPortal,
    Consumable,
    InvisibleBarrier,
    White,
    Red,
    Green,
    Blue,
    Yellow,
    Magenta,
    Cyan,
    x1,
    x2,
    x4,
    __MAX__,
};

static const char *getBrushName(EditorBrush type) {
    switch (type) {
        case EditorBrush::Wall: return "Wall";
        case EditorBrush::Barrier: return "Barrier";
        case EditorBrush::Glass: return "Glass";
        case EditorBrush::Sand: return "Sand";
        case EditorBrush::Zone: return "Zone";
        case EditorBrush::PlayerSpawn: return "Player Spawn";
        case EditorBrush::End: return "End";
        case EditorBrush::WorldPortal: return "World Portal";
        case EditorBrush::Consumable: return "Consumable";
        case EditorBrush::InvisibleBarrier: return "Invisible Barrier";
        case EditorBrush::White: return "White";
        case EditorBrush::Red: return "Red";
        case EditorBrush::Green: return "Green";
        case EditorBrush::Blue: return "Blue";
        case EditorBrush::Yellow: return "Yellow";
        case EditorBrush::Magenta: return "Magenta";
        case EditorBrush::Cyan: return "Cyan";
        case EditorBrush::x1: return "x1";
        case EditorBrush::x2: return "x2";
        case EditorBrush::x4: return "x4";
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
        case EditorBrush::PlayerSpawn: return TileType::PlayerSpawn;
        case EditorBrush::End: return TileType::End;
        case EditorBrush::WorldPortal: return TileType::WorldPortal;
        case EditorBrush::Consumable: return TileType::Consumable;
        case EditorBrush::InvisibleBarrier: return TileType::InvisibleBarrier;
        default: return TileType::Empty;
    }
}

static BeamColor getBeamColorFromBrush(EditorBrush type) {
    switch (type) {
        case EditorBrush::Red: return BeamColor::Red;
        case EditorBrush::Green: return BeamColor::Green;
        case EditorBrush::Blue: return BeamColor::Blue;
        case EditorBrush::Yellow: return BeamColor::Yellow;
        case EditorBrush::Magenta: return BeamColor::Magenta;
        case EditorBrush::Cyan: return BeamColor::Cyan;
        case EditorBrush::White:
        default:
            return BeamColor::White;
    }
}

static int getMultiplierFromBrush(EditorBrush type) {
    switch (type) {
        case EditorBrush::x1: return 1;
        case EditorBrush::x2: return 2;
        case EditorBrush::x4: return 4;
        default: return -1;
    }
}

template <typename TBProps>
static void strcpyTB(TBProps &tbProps, const std::string_view &str) {
    strcpy(tbProps.str.data(), str.data());
}

EditorScene::EditorScene() : Scene(TYPE) {
    m_cam.m_zoom = GetScreenHeight() / 10.0f;
    m_spriteSet.load();

    m_world = std::make_unique<World>();

    strcpyTB(m_beamBouncesTB, std::to_string(m_world->player()->m_bounces).c_str());

    refreshScannedWorldNames();
}

EditorScene::~EditorScene() {}

void EditorScene::update(float dt) {
    m_cam.update(dt);

    // beam raycast
    if (m_debugBeam) {
        auto &debug = m_debugBeam.value();
        BeamSegment segment;
        segment.start = debug.start;
        segment.dir = debug.dir;

        debug.beam.segments.clear();

        for (int i = 0; i <= m_world->player()->m_bounces; i++) {
            float t;
            Vector2i hit, normal;
            if (m_world->raycast(segment.start, segment.dir, -1, ObjectLayer::Light, t, hit, normal)) {
                segment.end = segment.start + segment.dir * std::max(t - 0.01f, 0.0f);

                debug.beam.segments.push_back(segment);

                segment.start = segment.end;
                segment.dir = segment.dir.reflected(normal.f());
                segment.prop = segment.prop.merge(m_world->getTile(hit).beamProp());
            }
        }
    }

    if (Input::keyPressed(KEY_F5) && Game::isTopScene(this)) {
        Game::addScene<IngameScene>(std::make_unique<World>(m_world.get()));
        Game::addScene<TransitionScene>(true);
    }

    if (Input::keyPressed(KEY_ESCAPE) && Game::isTopScene(this)) {
        Game::removeScene<EditorScene>();
        Game::addScene<TitleScene>();
    }
}

void EditorScene::render() {
    ClearBackground(BLACK_BG.f().linear());

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
    m_world->renderTilesMisc(m_cam, m_spriteSet);

    m_world->renderDecos(m_cam, m_spriteSet);;

    m_world->renderMisc(m_cam, m_spriteSet);

    if (m_debugBeam) {
        Render::begin2DCam(m_cam);
        const auto &beam = m_debugBeam.value().beam;
        for (const auto &it : beam.segments) {
            if (it.prop.color == BeamColor::Black) {
                break;
            }
            for (int j = 0; j < 5; j++) {
                DrawLineEx(it.start, it.end, BEAM_THICKNESS, getColorForBeam(it.prop.color));
            }
        }
        Render::end2DCam();
    }
}

static void join(const std::vector<std::string> &vec, std::string &str, char delim) {
    str.clear();
    for (size_t i = 0; i < vec.size(); i++) {
        str += vec[i];
        if (i + 1 < vec.size()) {
            str += delim;
        }
    }
}

static void split(std::vector<std::string> &vec, const std::string &str, char delim) {
    vec.clear();
    std::string tmp;
    for (char c : str) {
        if (c == delim) {
            vec.push_back(tmp);
            tmp.clear();
        } else {
            tmp += c;
        }
    }
    vec.push_back(tmp);
}

template <typename TBProps>
static bool GuiTextBoxHelper(const Rect &rect, TBProps &tbProps, std::string *str = nullptr) {
    bool changed = false;
    if (GuiTextBox(rect, tbProps.str.data(), tbProps.len, tbProps.enabled)) {
        tbProps.enabled = !tbProps.enabled;
        if (str) {
            *str = std::string(tbProps.str.data());
        }
        changed = true;
    }
    return changed;
}

void EditorScene::renderUI() {
    m_world->renderDebugAsUI(m_cam, m_spriteSet);

    constexpr float UI_PANEL_WIDTH = 200 + 16;
    if (Input::mousePosition().x > UI_PANEL_WIDTH) {
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
                    // NOTE(aren): Erase previous tile
                    if (tileType == TileType::PlayerSpawn) {
                        for (const auto &[k, v] : m_world->tiles()) {
                            if (v.m_type == TileType::PlayerSpawn) {
                                m_world->erase(k);
                                break;
                            }
                        }
                    }

                    m_world->at(mouse) = Tile(tileType);
                    if (tileType == TileType::Zone || tileType == TileType::WorldPortal || tileType == TileType::Consumable) {
                        m_world->at(mouse).m_name = std::string(m_tileNameTB.str.data());
                    }
                }

                auto beamColor = getBeamColorFromBrush(static_cast<EditorBrush>(m_brushIndex));
                if (m_world->contains(mouse)) {
                    auto &tile = m_world->at(mouse);
                    tile.m_color = beamColor;
                }

                int multiplier = getMultiplierFromBrush(static_cast<EditorBrush>(m_brushIndex));
                if (multiplier > 0 && m_world->contains(mouse)) {
                    auto &tile = m_world->at(mouse);
                    tile.m_multiplier = multiplier;
                }
            }
            if (Input::mouseButtonDown(1)) {
                m_world->erase(mouse);
            }

            m_world->updateBounds();
        }

        // debug beam
        if (Input::keyPressed(KEY_LEFT_CONTROL)) {
            m_debugBeam = {
                .beam = {},
                .start = m_cam.screenToWorld(Input::mousePosition()),
                .dir = V2f(1, 0),
            };
        }
        if (Input::keyReleased(KEY_LEFT_CONTROL)) {
            m_debugBeam.reset();
        }
        if (m_debugBeam) {
            auto curr = m_cam.screenToWorld(Input::mousePosition());
            auto dir = (curr - m_debugBeam.value().start);
            dir = (dir == V2F_ZERO) ? V2f(1, 0) : dir.normalized();
            m_debugBeam.value().dir = dir;
        }
    }

    if (m_tabEnabled) GuiLock();

    GuiScrollPanel({0, 0, UI_PANEL_WIDTH, float(GetScreenHeight())}, "Tools", {0, 0, UI_PANEL_WIDTH-16, 3000}, &m_panelScroll, &m_panelView);

    float y = 30;
    y += m_panelScroll.y;

    auto tabDropdownRect = Rect(10, y, 180, 30);
    y += 35;

    if (m_tab == Tab::World) {
        if (m_loadWorldNameEnabled) GuiLock();

        GuiTextBoxHelper({10, y, 180, 30}, m_savedWorldNameTB);
        y += 35;

        if (GuiButton({10, y, 180, 30}, "Save")) {
            m_world->m_name = std::string(m_worldNameTB.str.data());
            m_world->m_comment = std::string(m_worldCommentTB.str.data());

            m_world->save(m_savedWorldNameTB.str.data());
        }
        y += 35;

        auto worldDropdownRect = Rect(10, y, 180, 30);
        // NOTE(aren): Draw later due to z index
        y += 35;

        if (GuiButton({10, y, 180, 30}, "Load")) {
            if (m_loadWorldNameSelected < m_scannedWorldNames.size()) {
                m_world = World::load(m_scannedWorldNames[m_loadWorldNameSelected]);

                strcpyTB(m_savedWorldNameTB, m_scannedWorldNames[m_loadWorldNameSelected]);
                strcpyTB(m_worldNameTB, m_world->m_name);
                strcpyTB(m_worldCommentTB, m_world->m_comment);
                strcpyTB(m_beamBouncesTB, std::to_string(m_world->player()->m_bounces).c_str());
            }
        }
        y += 35;

        GuiLabel({10, y, 180, 30}, "World Name");
        y += 25;
        GuiTextBoxHelper({10, y, 180, 30}, m_worldNameTB);
        y += 35;

        GuiLabel({10, y, 180, 30}, "Comment");
        y += 25;
        GuiTextBoxHelper({10, y, 180, 30}, m_worldCommentTB);
        y += 35;

        GuiLabel({10, y, 180, 30}, "Beam Bounces");
        y += 25;
        if (GuiTextBoxHelper({10, y, 180, 30}, m_beamBouncesTB)) {
            m_world->player()->m_bounces = std::atoi(m_beamBouncesTB.str.data());
        }
        y += 35;

        GuiCheckBox({10, y, 30, 30}, "Beam Disabled", &m_world->player()->m_beamDisabled);
        y += 35;

        // NOTE(aren): Pass 2, need to stack backward
        if (m_loadWorldNameEnabled) GuiUnlock();

        static std::string s_joinedStr;
        join(m_scannedWorldNames, s_joinedStr, ';');

        if (GuiDropdownBox(worldDropdownRect, s_joinedStr.c_str(), &m_loadWorldNameSelected, m_loadWorldNameEnabled)) {
            m_loadWorldNameEnabled = !m_loadWorldNameEnabled;
        }
    } else if (m_tab == Tab::Brush) {
        for (int i = 0; i < int(EditorBrush::__MAX__); i++) {
            int prev = GuiGetStyle(BUTTON, BASE_COLOR_NORMAL);
            if (i == m_brushIndex) {
                GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, Color4b(255, 127, 127, 255).rgba32());
            }

            auto label = ((i < 9) ? (std::to_string(i+1) + ": ") : "") + getBrushName(static_cast<EditorBrush>(i));

            if (GuiButton({10, y, 180, 30}, label.c_str())) {
                m_brushIndex = i;
            }

            if (i < 9 && Input::keyPressed(KEY_ONE + i) && m_tileNameTB.enabled == false) {
                m_brushIndex = i;
            }

            GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, prev);
            y += 35;
        }

        if (static_cast<EditorBrush>(m_brushIndex) == EditorBrush::Zone || static_cast<EditorBrush>(m_brushIndex) == EditorBrush::WorldPortal || static_cast<EditorBrush>(m_brushIndex) == EditorBrush::Consumable) {
            GuiLabel({10, y, 180, 30}, "Tile name");
            y += 25;
            GuiTextBoxHelper({10, y, 180, 30}, m_tileNameTB);
            y += 35;
        }
    } else if (m_tab == Tab::Wave) {
        auto &waves = m_world->m_monsterWaves;
        if (waves.size() != m_monsterWaveProps.size()) {
            m_monsterWaveProps.clear();
            for (const auto &it : waves) {
                MonsterWaveProps props;
                join(it.zones, props.zoneStr, ',');
                strcpy(props.zoneTB.str.data(), props.zoneStr.c_str());
                strcpy(props.monsterNameTB.str.data(), it.monsterName.c_str());
                strcpy(props.conditionValueTB.str.data(), it.conditionValue.c_str());
                m_monsterWaveProps.push_back(std::move(props));
            }
        }

        int toRemove = -1;
        for (int i = 0; i < waves.size(); i++) {
            if (m_monsterWaveProps[i].conditionTypeEnabled) GuiLock();

            GuiLabel({10, y, 180, 30}, TextFormat("%d", i + 1));
            y += 25;
            GuiLabel({10, y, 180, 30}, "Zones");
            y += 25;
            if (GuiTextBoxHelper({10, y, 180, 30}, m_monsterWaveProps[i].zoneTB, &m_monsterWaveProps[i].zoneStr)) {
                split(waves[i].zones, m_monsterWaveProps[i].zoneStr, ',');
            }
            y += 35;
            GuiLabel({10, y, 180, 30}, "Monster Name");
            y += 25;
            GuiTextBoxHelper({10, y, 180, 30}, m_monsterWaveProps[i].monsterNameTB, &waves[i].monsterName);
            y += 35;
            GuiLabel({10, y, 180, 30}, "Condition Type");
            y += 25;
            auto conditionValueRect = Rect(10, y, 180, 30);
            y += 35;
            GuiLabel({10, y, 180, 30}, "Condition Value");
            y += 25;
            GuiTextBoxHelper({10, y, 180, 30}, m_monsterWaveProps[i].conditionValueTB, &waves[i].conditionValue);
            y += 35;
            if (GuiButton({10, y, 180, 30}, "X")) {
                toRemove = i;
            }
            y += 35;

            // NOTE(aren): Pass 2, need to stack backward
            if (m_monsterWaveProps[i].conditionTypeEnabled) GuiUnlock();

            if (GuiDropdownBox(conditionValueRect, "Always;MonsterCleared;EnterZone;ConsumedConsumable", reinterpret_cast<int *>(&waves[i].conditionType), m_monsterWaveProps[i].conditionTypeEnabled)) {
                m_monsterWaveProps[i].conditionTypeEnabled = !m_monsterWaveProps[i].conditionTypeEnabled;
            }
        }

        if (toRemove != -1) {
            waves.erase(waves.begin() + toRemove);
        }

        if (GuiButton({10, y, 180, 30}, "+")) {
            waves.push_back({
                .monsterName = "dumb",
            });
        }
        y += 35;
    }

    // NOTE(aren): Pass 2, need to stack backward
    if (m_tabEnabled) GuiUnlock();

    if (GuiDropdownBox(tabDropdownRect, "World;Brush;Wave", reinterpret_cast<int *>(&m_tab), m_tabEnabled)) {
        m_tabEnabled = !m_tabEnabled;
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

#endif