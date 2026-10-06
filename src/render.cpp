#include "render.hpp"
#include "camera.hpp"
#include "rlgl.h"

namespace aren {

::Font Render::s_defaultFont = {};

Color4b Render::s_backgroundColor = {50, 56, 68, 255};
uint8_t Render::s_vignette = 16;

static std::vector<Color4b> s_tintStack;

static Color4b applyTint(Color4b color) {
    auto tintColor = s_tintStack.empty() ? Color4b(WHITE) : s_tintStack.back();
    return Color4b(Color4f(tintColor) * Color4f(color));
}

void Render::init() {
    constexpr int FONT_ATLAS_SIZE = 96;

    s_defaultFont = LoadFontEx("resources/fonts/Noto_Serif/NotoSerif-SemiBold.ttf",
        FONT_ATLAS_SIZE, nullptr, 0);

    SetTextureFilter(s_defaultFont.texture, TEXTURE_FILTER_BILINEAR);
}

void Render::deinit() {
    UnloadFont(s_defaultFont);
}

Vector2f Render::measureText(const char *text, float size) {
    return MeasureTextEx(s_defaultFont, text, size, 1.0f);
}

void Render::drawText(Vector2f origin, float size, Color4b color, const char *text, TextAlignH alignH, TextAlignV alignV) {
    auto dimension = measureText(text, size);

    if (alignH == TextAlignH::Center) origin.x -= dimension.x * 0.5f;
    else if (alignH == TextAlignH::Right) origin.x -= dimension.x;
    if (alignV == TextAlignV::Center) origin.y -= dimension.y * 0.5f;
    else if (alignV == TextAlignV::Bottom) origin.y -= dimension.y;

    // TODO(aren): shadow

    DrawTextEx(s_defaultFont, text, origin, size, 1.0f, applyTint(color));
}

void Render::drawTextRotated(Vector2f origin, float size, float rotationDeg, Color4b color, const char *text, TextAlignH alignH, TextAlignV alignV) {
    auto dimension = measureText(text, size);

    auto pivot = V2F_ZERO;
    if (alignH == TextAlignH::Center) pivot.x = dimension.x * 0.5f;
    else if (alignH == TextAlignH::Right) pivot.x = dimension.x;
    if (alignV == TextAlignV::Center) pivot.y = dimension.y * 0.5f;
    else if (alignV == TextAlignV::Bottom) pivot.y = dimension.y;

    // TODO(aren): shadow

    DrawTextPro(s_defaultFont, text, origin, pivot, rotationDeg, size, 1.0f, applyTint(color));
}

void Render::beginDraw() {
    BeginDrawing();
    ClearBackground(s_backgroundColor);
}

void Render::endDraw() {
    int sw = GetScreenWidth();
    int sh = GetScreenHeight();
    float r = std::sqrt(float(sw * sw + sh * sh)) * 0.5f;
    DrawCircleGradient(sw / 2, sh / 2, r, {0, 0, 0, 0}, {0, 0, 0, s_vignette});

    EndDrawing();
}

void Render::begin2DCam(const Camera &cam) {
    rlPushMatrix();

    BeginMode2D(static_cast<Camera2D>(cam));
}

void Render::end2DCam() {
    EndMode2D();

    rlPopMatrix();
}

void Render::pushTint(Color4b color) {
    auto next = applyTint(color);
    s_tintStack.push_back(next);
}

void Render::popTint() {
    ASSERT(s_tintStack.size() > 0);
    s_tintStack.pop_back();
}

void Render::drawTriangleGradient(Vector2f a, Color4b ca, Vector2f b, Color4b cb, Vector2f c, Color4b cc) {
    rlBegin(RL_TRIANGLES);
    ca = applyTint(ca);
    cb = applyTint(cb);
    cc = applyTint(cc);
    rlColor4ub(ca.r, ca.g, ca.b, ca.a); rlVertex2f(a.x, a.y);
    rlColor4ub(cb.r, cb.g, cb.b, cb.a); rlVertex2f(b.x, b.y);
    rlColor4ub(cc.r, cc.g, cc.b, cc.a); rlVertex2f(c.x, c.y);
    rlEnd();
}

} // namespace aren