#include "editor_scene.hpp"
#include "../render.hpp"
#include "game.hpp"
#include "sprite_editor_scene.hpp"

namespace aren {

struct VertexDef {
    std::array<bool, 4> pattern;
    std::string spriteName;
    int tag;
    int half; // in pixels
    int rotate;
};

static const std::vector<VertexDef> VERTEX_DEFS = {
    {
        .pattern = { 1, 0, 0, 0 },
        .spriteName = "edge01",
        .tag = 0,
        .half = 5,
        .rotate = 0,
    },
    {
        .pattern = { 1, 0, 0, 1 },
        .spriteName = "edge01",
        .tag = 1,
        .half = 5,
        .rotate = 0,
    },
    {
        .pattern = { 0, 1, 1, 1 },
        .spriteName = "edge01",
        .tag = 2,
        .half = 1,
        .rotate = 0,
    },
};

static std::vector<VertexDef> s_vertexDefFull;

static const int EDGE_TAG = 3;

static void rotatePattern(std::array<bool, 4> &pattern, int rotate) {
    // NOTE(aren): rotate clockwise
    for (int i = 0; i < rotate; i++) {
        auto tmp = pattern;
        pattern[0] = tmp[2];
        pattern[1] = tmp[0];
        pattern[2] = tmp[3];
        pattern[3] = tmp[1];
    }
}

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
    if (Sprite::load("edge01", m_edgeSprite) == false) {
        ASSERT(false);
    }

    if (s_vertexDefFull.empty()) {
        for (const auto &it : VERTEX_DEFS) {
            for (int i = 0; i < 4; i++) {
                auto copy = it;
                rotatePattern(copy.pattern, i);
                copy.rotate = i;
                s_vertexDefFull.push_back(copy);
            }
        }
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

    Render::begin2DCam(m_cam);

    if (Input::keyPressed(KEY_Q)) m_drawWireframe = !m_drawWireframe;

    auto mouse = m_cam.screenToWorld(Input::mousePosition()).floor();
    DrawRectangleV(mouse.f(), V2F_ONE, {255, 0, 0, 50});

    if (Input::mouseButtonDown(0)) {
        m_blocks[mouse] = EditorBlock(EditorBlockType::Wall);
    }
    if (Input::mouseButtonDown(1)) {
        m_blocks.erase(mouse);
    }

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

    Vector2i minPos = V2I_ZERO;
    Vector2i maxPos = V2I_ZERO;
    if (m_blocks.size() > 0) {
        maxPos = minPos = m_blocks.begin()->first;

        for (const auto &[k, _] : m_blocks) {
            maxPos = maxPos.max(k);
            minPos = minPos.min(k);
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

    static std::unordered_map<Vector2i, int> s_vertexHalfs;
    s_vertexHalfs.clear();

    for (int x = minPos.x - 1; x <= maxPos.x; x++) {
        for (int y = minPos.y - 1; y <= maxPos.y; y++) {
            std::array<bool, 4> currPattern = {
                getBlock(V2i(x, y)).m_type == EditorBlockType::Wall,
                getBlock(V2i(x + 1, y)).m_type == EditorBlockType::Wall,
                getBlock(V2i(x, y + 1)).m_type == EditorBlockType::Wall,
                getBlock(V2i(x + 1, y + 1)).m_type == EditorBlockType::Wall,
            };

            for (const auto &it : s_vertexDefFull) {
                if (it.pattern == currPattern) {
                    auto center = V2i(x + 1, y + 1);
                    s_vertexHalfs[center] = it.half;

                    auto a = m_cam.worldToScreen(center.f());
                    m_edgeSprite.renderCorner(a, it.tag, it.rotate * 90, PIXEL_SCALER);
                }
            }
        }
    }

    const float worldToScreenScale = m_cam.worldToScreen(V2f(1, 0)).x - m_cam.worldToScreen(V2f(0, 0)).x;

    const auto getVertexHalfWS = [&](const Vector2i &at) {
        if (s_vertexHalfs.contains(at)) {
            float screenSpace = s_vertexHalfs[at] * PIXEL_SCALER;
            return screenSpace / worldToScreenScale;
        }
        return 0.0f;
    };

    for (int x = minPos.x - 1; x <= maxPos.x; x++) {
        for (int y = minPos.y - 1; y <= maxPos.y; y++) {
            // vertical
            bool p = getBlock(V2i(x, y)).m_type == EditorBlockType::Wall;
            bool q = getBlock(V2i(x + 1, y)).m_type == EditorBlockType::Wall;
            if (p ^ q) {
                auto a = V2f(x + 1, y);
                auto b = V2f(x + 1, y + 1);
                a += V2f(0, 1) * getVertexHalfWS(V2i(x + 1, y));
                b -= V2f(0, 1) * getVertexHalfWS(V2i(x + 1, y + 1));

                auto aSS = m_cam.worldToScreen(a);
                auto bSS = m_cam.worldToScreen(b);

                m_edgeSprite.renderEdge((aSS + bSS) * 0.5f, bSS - aSS, EDGE_TAG, q ? 90 : 270, PIXEL_SCALER);
            }

            // horizontal
            q = getBlock(V2i(x, y + 1)).m_type == EditorBlockType::Wall;
            if (p ^ q) {
                auto a = V2f(x, y + 1);
                auto b = V2f(x + 1, y + 1);
                a += V2f(1, 0) * getVertexHalfWS(V2i(x, y + 1));
                b -= V2f(1, 0) * getVertexHalfWS(V2i(x + 1, y + 1));

                auto aSS = m_cam.worldToScreen(a);
                auto bSS = m_cam.worldToScreen(b);

                m_edgeSprite.renderEdge((aSS + bSS) * 0.5f, bSS - aSS, EDGE_TAG, q ? 180 : 0, PIXEL_SCALER);
            }
        }
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