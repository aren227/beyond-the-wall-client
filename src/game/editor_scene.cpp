#include "editor_scene.hpp"
#include "../render.hpp"
#include "game.hpp"
#include "sprite_editor_scene.hpp"

namespace aren {

struct EdgePlugDef {
    Vector2i dir;
    Vector2i normal;
};

struct EdgePlug {
    Vector2i pos;
    Vector2i dir;
    Vector2i normal;
};

struct CornerTileDef {
    bool pattern[2][2];
    std::string spriteName;
    int tag;
    int rotation;
    std::vector<EdgePlugDef> edgePlugs;
};

std::vector<CornerTileDef> CORNER_PATTERNS = {
    {
        // NOTE(aren): Assum cw rotation
        // 1
        {
            .pattern = {{ 1, 0 }, { 0, 0 }},
            .spriteName = "wall_corner01",
            .tag = 0,
            .rotation = 0,
            .edgePlugs = {
                { { -1, 0 }, { 0, 1 } },
                { { 0, -1 }, { 1, 0 } },
            },
        },
        {
            .pattern = {{ 0, 0 }, { 1, 0 }},
            .spriteName = "wall_corner01",
            .tag = 0,
            .rotation = -90,
            .edgePlugs = {
                { { -1, 0 }, { 0, -1 } },
                { { 0, 1 }, { 1, 0 } },
            },
        },
        {
            .pattern = {{ 0, 0 }, { 0, 1 }},
            .spriteName = "wall_corner01",
            .tag = 0,
            .rotation = -180,
            .edgePlugs = {
                { { 1, 0 }, { 0, -1 } },
                { { 0, 1 }, { -1, 0 } },
            },
        },
        {
            .pattern = {{ 0, 1 }, { 0, 0 }},
            .spriteName = "wall_corner01",
            .tag = 0,
            .rotation = -270,
            .edgePlugs = {
                { { 1, 0 }, { 0, 1 } },
                { { 0, -1 }, { -1, 0 } },
            },
        },
        // 2
        {
            .pattern = {{ 1, 0 }, { 0, 1 }},
            .spriteName = "wall_corner01",
            .tag = 1,
            .rotation = 0,
            .edgePlugs = {
                { { 1, 0 }, { 0, -1 } },
                { { -1, 0 }, { 0, 1 } },
                { { 0, 1 }, { -1, 0 } },
                { { 0, -1 }, { 1, 0 } },
            },
        },
        {
            .pattern = {{ 0, 1 }, { 1, 0 }},
            .spriteName = "wall_corner01",
            .tag = 1,
            .rotation = -90,
            .edgePlugs = {
                { { 1, 0 }, { 0, 1 } },
                { { -1, 0 }, { 0, -1 } },
                { { 0, 1 }, { 1, 0 } },
                { { 0, -1 }, { -1, 0 } },
            },
        },
        // 3
        {
            .pattern = {{ 1, 1 }, { 0, 1 }},
            .spriteName = "wall_corner01",
            .tag = 2,
            .rotation = 0,
            .edgePlugs = {
                { { -1, 0 }, { 0, 1 } },
                { { 0, 1 }, { -1, 0 } },
            },
        },
        {
            .pattern = {{ 1, 1 }, { 1, 0 }},
            .spriteName = "wall_corner01",
            .tag = 2,
            .rotation = -90,
            .edgePlugs = {
                { { 1, 0 }, { 0, 1 } },
                { { 0, 1 }, { 1, 0 } },
            },
        },
        {
            .pattern = {{ 1, 0 }, { 1, 1 }},
            .spriteName = "wall_corner01",
            .tag = 2,
            .rotation = -180,
            .edgePlugs = {
                { { 1, 0 }, { 0, -1 } },
                { { 0, -1 }, { 1, 0 } },
            },
        },
        {
            .pattern = {{ 0, 1 }, { 1, 1 }},
            .spriteName = "wall_corner01",
            .tag = 2,
            .rotation = -270,
            .edgePlugs = {
                { { -1, 0 }, { 0, -1 } },
                { { 0, -1 }, { -1, 0 } },
            },
        },
    }
};

static const char *getBrushName(EditorBrush type) {
    switch (type) {
        case EditorBrush::Wall: return "Wall";
        case EditorBrush::Barrier: return "Barrier";
        case EditorBrush::Glass: return "Glass";
        case EditorBrush::Sand: return "Sand";
        default: return "N/A";
    }
}

EditorScene::EditorScene() : Scene(TYPE) {
    m_cam.m_zoom = GetScreenHeight() / 10.0f;
    if (Sprite::load("brick", m_brickSprite) == false) {
        ASSERT(false);
    }
    if (Sprite::load("wall_corner", m_cornerSprite) == false) {
        ASSERT(false);
    }
    if (Sprite::load("wall_edge", m_edgeSprite) == false) {
        ASSERT(false);
    }
}

void EditorScene::update(float dt) {
    m_cam.update(dt);
}

void EditorScene::render() {
    ClearBackground(BLACK);

    float sw = GetScreenWidth();
    float sh = GetScreenHeight();

    float zoomMin = sh / 10.0f;
    float zoomMax = sh / 40.0f;

    m_cam.m_zoom = std::clamp(m_cam.m_zoom + Input::mouseWheel(), zoomMax, zoomMin);
    m_cam.m_offset = V2f(sw, sh) * 0.5f;

    if (Input::mouseButtonDown(2)) {
        m_cam.m_target -= Input::mouseDelta() / m_cam.m_zoom;
    }

    Vector2i minPos = V2I_ZERO;
    Vector2i maxPos = V2I_ZERO;
    if (m_blocks.size() > 0) {
        maxPos = minPos = m_blocks.begin()->first;

        for (const auto &[k, _] : m_blocks) {
            maxPos = maxPos.max(k);
            minPos = minPos.min(k);
        }
    }

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
        auto m = minPos, M = maxPos + V2I_ONE;
        DrawLineV(V2f(m.x, m.y), V2f(M.x, m.y), BLUE);
        DrawLineV(V2f(M.x, m.y), V2f(M.x, M.y), BLUE);
        DrawLineV(V2f(M.x, M.y), V2f(m.x, M.y), BLUE);
        DrawLineV(V2f(m.x, M.y), V2f(m.x, m.y), BLUE);
    }

    if (Input::keyPressed(KEY_Q)) m_drawWireframe = !m_drawWireframe;

    auto mouse = m_cam.screenToWorld(Input::mousePosition()).floor();
    DrawRectangleV(mouse.f(), V2F_ONE, {255, 0, 0, 50});

    if (Input::mouseButtonDown(0)) {
        m_blocks[mouse] = EditorBlock(EditorBlockType::Wall);
    }
    if (Input::mouseButtonDown(1)) {
        m_blocks.erase(mouse);
    }

    Render::end2DCam();

    constexpr int PIXEL_SCALER = 3;

    // draw tiles
    for (const auto &[k, v] : m_blocks) {
        if (v.m_type == EditorBlockType::Wall) {
            auto a = m_cam.worldToScreen(k.f());
            auto b = m_cam.worldToScreen((k + V2I_ONE).f());

            m_brickSprite.render(Rect(a.x, a.y, b.x-a.x, b.y-a.y), PIXEL_SCALER);
        }
    }

    static std::vector<EdgePlug> s_horizontalPlugs;
    static std::vector<EdgePlug> s_verticalPLugs;
    static std::vector<EdgePlug> s_plugs;

    s_horizontalPlugs.clear();
    s_verticalPLugs.clear();
    s_plugs.clear();

    // draw corners
    for (int x = minPos.x - 1; x <= maxPos.x; x++) {
        for (int y = minPos.y - 1; y <= maxPos.y; y++) {
            bool v00 = getBlock(V2i(x, y)).m_type == EditorBlockType::Wall;
            bool v01 = getBlock(V2i(x + 1, y)).m_type == EditorBlockType::Wall;
            bool v10 = getBlock(V2i(x, y + 1)).m_type == EditorBlockType::Wall;
            bool v11 = getBlock(V2i(x + 1, y + 1)).m_type == EditorBlockType::Wall;

            for (const auto &jt : CORNER_PATTERNS) {
                if (jt.pattern[0][0] == v00 && jt.pattern[0][1] == v01
                    && jt.pattern[1][0] == v10 && jt.pattern[1][1] == v11) {
                    auto center = V2i(x + 1, y + 1);
                    auto a = m_cam.worldToScreen(center.f());
                    m_cornerSprite.renderCorner(a, jt.tag, jt.rotation, PIXEL_SCALER);

                    for (const auto &kt : jt.edgePlugs) {
                        auto &plugs = (kt.dir.x == 1 || kt.dir.x == -1) ? s_horizontalPlugs : s_verticalPLugs;
                        plugs.push_back({
                            .pos = center,
                            .dir = kt.dir,
                            .normal = kt.normal,
                        });
                    }
                }
            }
        }
    }

    std::sort(s_horizontalPlugs.begin(), s_horizontalPlugs.end(), [](const EdgePlug &a, const EdgePlug &b) {
        return (a.pos == b.pos) ? (a.dir < b.dir) : ((a.pos.y == b.pos.y) ? (a.pos.x < b.pos.x) : (a.pos.y < b.pos.y));
    });
    std::sort(s_verticalPLugs.begin(), s_verticalPLugs.end(), [](const EdgePlug &a, const EdgePlug &b) {
        return (a.pos == b.pos) ? (a.dir < b.dir) : ((a.pos.x == b.pos.x) ? (a.pos.y < b.pos.y) : (a.pos.x < b.pos.x));
    });

    // NOTE(aren): Must be paired
    ASSERT(s_horizontalPlugs.size() % 2 == 0);
    ASSERT(s_verticalPLugs.size() % 2 == 0);

    // NOTE(aren): Merge to use same codes
    for (const auto &it : s_horizontalPlugs) {
        s_plugs.push_back(it);
    }
    for (const auto &it : s_verticalPLugs) {
        s_plugs.push_back(it);
    }

    for (int i = 0; i < s_plugs.size(); i += 2) {
        auto start = s_plugs[i];
        bool isHorizontal = (start.dir.y == 0);
        auto dirlen = (s_plugs[i+1].pos - start.pos).f();

        auto centerW = m_cam.worldToScreen(start.pos.f() + dirlen * 0.5f);
        auto dirlenW = m_cam.worldToScreen(dirlen) - m_cam.worldToScreen(V2F_ZERO);
        // TODO(aren): Hardcoded border width(?)
        dirlenW = V2F_ZERO.max(dirlenW - V2F_ONE * 5 * PIXEL_SCALER);

        // NOTE(aren): for numerical stability
        (isHorizontal ? dirlenW.y : dirlenW.x) = 0;

        int rot = 0;
        if (start.normal == V2i(0, 1)) rot = 0;
        else if (start.normal == V2i(-1, 0)) rot = 90;
        else if (start.normal == V2i(0, -1)) rot = 180;
        else if (start.normal == V2i(1, 0)) rot = 270;
        else ASSERT(false);

        m_edgeSprite.renderEdge(centerW, dirlenW, rot, PIXEL_SCALER);
    }

}

void EditorScene::renderUI() {
    GuiPanel({0, 0, 200, 800}, "Brush List");

    float y = 40;
    if (GuiButton({10, y, 180, 30}, "Goto Sprite Editor")) {
        Game::removeScene<EditorScene>();
        Game::addScene<SpriteEditorScene>();
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

        if (i < 9 && Input::keyPressed(KEY_ONE + i)) {
            m_brushIndex = i;
        }

        GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, prev);
        y += 40;
    }

    if (Input::keyPressed(KEY_ONE)) {
        m_brushIndex = 0;
    }
}

EditorBlock &EditorScene::getBlock(const Vector2i &pos) {
    static EditorBlock s_dummyBlock;
    if (m_blocks.contains(pos)) {
        return m_blocks[pos];
    }

    s_dummyBlock.m_type = EditorBlockType::Empty;
    return s_dummyBlock;
}

}