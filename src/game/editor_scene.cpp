#include "editor_scene.hpp"
#include "../render.hpp"
#include "game.hpp"
#include "sprite_editor_scene.hpp"

namespace aren {

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

    if (Input::keyPressed(KEY_Q)) m_drawWireframe = !m_drawWireframe;

    auto mouse = m_cam.screenToWorld(Input::mousePosition()).floor();
    DrawRectangleV(mouse.f(), V2F_ONE, {255, 0, 0, 50});

    if (Input::mouseButtonDown(0)) {
        m_blocks[mouse] = EditorBlock(EditorBlockType::Wall);
    }
    if (Input::mouseButtonDown(1)) {
        m_blocks.erase(mouse);
    }

    // draw tiles
    for (const auto &[k, v] : m_blocks) {
        constexpr float thickness = 0.2f;
        std::array<Vector2f, 4> points = {
            V2f(0, 0),
            V2f(1, 0),
            V2f(1, 1),
            V2f(0, 1),
        };
        std::array<Vector2i, 4> adj = {
            V2i(0, -1),
            V2i(1, 0),
            V2i(0, 1),
            V2i(-1, 0),
        };
        for (int i = 0; i < 4; i++) {
            if (m_blocks.count(k + adj[i]) > 0) continue;
            auto a = points[i];
            auto b = points[(i+1)%4];
            a += (b-a).perp() * thickness * 0.5f;
            b += (b-a).perp() * thickness * 0.5f;
            DrawLineEx(k.f() + a, k.f() + b, thickness, WHITE);
        }
    }

    Render::end2DCam();
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