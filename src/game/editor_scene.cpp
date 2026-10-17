#include "editor_scene.hpp"
#include "../render.hpp"
#include "game.hpp"
#include "sprite_editor_scene.hpp"
#include <random>

namespace aren {

struct VertexDef {
    std::array<bool, 4> pattern;
    int tag;
    int half; // in pixels
    int rotate;
    int mainX, mainY;
};

static const std::vector<VertexDef> VERTEX_DEFS = {
    {
        .pattern = { 1, 0, 0, 0 },
        .tag = 1,
        .half = 5,
        .rotate = 0,
        .mainX = 0,
        .mainY = 0,
    },
    {
        .pattern = { 1, 0, 0, 1 },
        .tag = 2,
        .half = 5,
        .rotate = 0,
        .mainX = 0,
        .mainY = 0,
    },
    {
        .pattern = { 0, 1, 1, 1 },
        .tag = 3,
        .half = 1,
        .rotate = 0,
        .mainX = 1,
        .mainY = 1,
    },
};

static std::vector<VertexDef> s_vertexDefFull;

static const int SURFACE_TAG = 0;
static const int EDGE_TAG = 4;

static const char *getBrushName(EditorBrush type) {
    switch (type) {
        case EditorBrush::Wall: return "Wall";
        case EditorBrush::Barrier: return "Barrier";
        case EditorBrush::Glass: return "Glass";
        case EditorBrush::Sand: return "Sand";
        default: return "N/A";
    }
}

static EditorBlockType getBrushBlockType(EditorBrush type) {
    switch (type) {
        case EditorBrush::Wall: return EditorBlockType::Wall;
        case EditorBrush::Barrier: return EditorBlockType::Barrier;
        case EditorBrush::Glass: return EditorBlockType::Glass;
        case EditorBrush::Sand: return EditorBlockType::Sand;
        default: return EditorBlockType::Empty;
    }
}

EditorScene::EditorScene() : Scene(TYPE) {
    m_cam.m_zoom = GetScreenHeight() / 10.0f;

    if (Sprite::load("brick", m_sprites[EditorBlockType::Wall]) == false) {
        ASSERT(false);
    }
    if (Sprite::load("glass", m_sprites[EditorBlockType::Glass]) == false) {
        ASSERT(false);
    }
    if (Sprite::load("barrier", m_sprites[EditorBlockType::Barrier]) == false) {
        ASSERT(false);
    }
    if (Sprite::load("sand", m_sprites[EditorBlockType::Sand]) == false) {
        ASSERT(false);
    }

    if (Sprite::load("deco01", m_decoSprite) == false) {
        ASSERT(false);
    }

    if (s_vertexDefFull.empty()) {
        const auto rotatePattern = [](std::array<bool, 4> &pattern) {
            // NOTE(aren): rotate clockwise
            auto tmp = pattern;
            pattern[0] = tmp[2];
            pattern[1] = tmp[0];
            pattern[2] = tmp[3];
            pattern[3] = tmp[1];
        };

        for (const auto &it : VERTEX_DEFS) {
            auto copy = it;
            for (int i = 0; i < 4; i++) {
                s_vertexDefFull.push_back(copy);

                rotatePattern(copy.pattern);
                copy.rotate++;

                int tmp = copy.mainX;
                copy.mainX = 1 - copy.mainY;
                copy.mainY = tmp;
            }
        }
    }
}

void EditorScene::update(float dt) {
    m_cam.update(dt);
}

void EditorScene::renderBlockType(Vector2i minPos, Vector2i maxPos, int category) {
    constexpr int PIXEL_SCALER = 2;

    const auto getBlock = [&](const Vector2i &pos) -> EditorBlockType {
        if (pos.x < minPos.x || pos.x > maxPos.x || pos.y < minPos.y || pos.y > maxPos.y) {
            return EditorBlockType::Barrier;
        }

        if (m_blocks.contains(pos)) {
            return m_blocks[pos].m_type;
        }

        return EditorBlockType::Empty;
    };

    const auto getCategory = [&](const Vector2i &pos) -> int {
        switch (getBlock(pos)) {
            case EditorBlockType::Empty:
                return 0;
            case EditorBlockType::Barrier:
            case EditorBlockType::Wall:
                return 1;
            case EditorBlockType::Glass:
                return 2;
            case EditorBlockType::Sand:
                return 3;
            default:
                return 0;
        }
    };

    // draw tiles
    for (int x = minPos.x; x <= maxPos.x; x++) {
        for (int y = minPos.y; y <= maxPos.y; y++) {
            auto v = V2i(x, y);
            if (getCategory(v) == category) {
                auto a = m_cam.worldToScreen(v.f());
                auto b = m_cam.worldToScreen((v + V2I_ONE).f());

                const auto &sprite = m_sprites[getBlock(v)];
                sprite.render(Rect(a.x, a.y, b.x-a.x, b.y-a.y), SURFACE_TAG, PIXEL_SCALER);
            }
        }
    }

    static std::unordered_map<Vector2i, int> s_vertexHalfs;
    s_vertexHalfs.clear();

    for (int x = minPos.x - 1; x <= maxPos.x; x++) {
        for (int y = minPos.y - 1; y <= maxPos.y; y++) {
            std::array<bool, 4> currPattern = {
                getCategory(V2i(x, y)) == category,
                getCategory(V2i(x + 1, y)) == category,
                getCategory(V2i(x, y + 1)) == category,
                getCategory(V2i(x + 1, y + 1)) == category,
            };

            for (const auto &it : s_vertexDefFull) {
                if (it.pattern == currPattern) {
                    auto center = V2i(x + 1, y + 1);
                    s_vertexHalfs[center] = it.half;

                    auto main = V2i(x + it.mainX, y + it.mainY);
                    const auto &sprite = m_sprites[getBlock(main)];

                    auto a = m_cam.worldToScreen(center.f());
                    sprite.renderCorner(a, it.tag, it.rotate * 90, PIXEL_SCALER);
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
            bool p = getCategory(V2i(x, y)) == category;
            bool q = getCategory(V2i(x + 1, y)) == category;
            if (p ^ q) {
                auto a = V2f(x + 1, y);
                auto b = V2f(x + 1, y + 1);
                a += V2f(0, 1) * getVertexHalfWS(V2i(x + 1, y));
                b -= V2f(0, 1) * getVertexHalfWS(V2i(x + 1, y + 1));

                auto aSS = m_cam.worldToScreen(a);
                auto bSS = m_cam.worldToScreen(b);

                const auto &sprite = m_sprites[p ? getBlock(V2i(x, y)) : getBlock(V2i(x + 1, y))];

                sprite.renderEdge((aSS + bSS) * 0.5f, bSS - aSS, EDGE_TAG, q ? 90 : 270, PIXEL_SCALER);
            }

            // horizontal
            q = getCategory(V2i(x, y + 1)) == category;
            if (p ^ q) {
                auto a = V2f(x, y + 1);
                auto b = V2f(x + 1, y + 1);
                a += V2f(1, 0) * getVertexHalfWS(V2i(x, y + 1));
                b -= V2f(1, 0) * getVertexHalfWS(V2i(x + 1, y + 1));

                auto aSS = m_cam.worldToScreen(a);
                auto bSS = m_cam.worldToScreen(b);

                const auto &sprite = m_sprites[p ? getBlock(V2i(x, y)) : getBlock(V2i(x, y + 1))];

                sprite.renderEdge((aSS + bSS) * 0.5f, bSS - aSS, EDGE_TAG, q ? 180 : 0, PIXEL_SCALER);
            }
        }
    }
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
        auto blockType = getBrushBlockType(static_cast<EditorBrush>(m_brushIndex));
        if (blockType != EditorBlockType::Empty) {
            m_blocks[mouse] = EditorBlock(blockType);
        }
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

    // TODO(aren): hardcoded
    for (int i = 0; i < 4; i++) {
        renderBlockType(minPos, maxPos, i);
    }

    std::minstd_rand gen(minPos.x * minPos.y * maxPos.x * maxPos.y);

    for (int x = minPos.x; x <= maxPos.x; x++) {
        for (int y = minPos.y; y <= maxPos.y; y++) {
            if (m_blocks.contains(V2i(x, y)) == false) {
                std::uniform_real_distribution<float> rd(0.0f, 1.0f);
                float v = rd(gen);
                if (v < 0.2f) {
                    auto offset = V2f(rd(gen) * 0.4f - 0.2f, rd(gen) * 0.4f - 0.2f);
                    auto vf = V2i(x, y).f() + offset;
                    auto a = m_cam.worldToScreen(vf);
                    auto b = m_cam.worldToScreen(vf + V2F_ONE);

                    std::uniform_int_distribution<int> rdi(0, 4);
                    m_decoSprite.render(Rect(a.x, a.y, b.x-a.x, b.y-a.y), rdi(gen), 2);
                }
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

}