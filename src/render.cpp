#include "render.hpp"
#include "camera.hpp"
#include "render_texture.hpp"
#include "rlgl.h"

namespace aren {

::Font Render::s_defaultFont = {};

Color4b Render::s_backgroundColor = {50, 56, 68, 255};
uint8_t Render::s_vignette = 16;

static std::vector<Color4b> s_tintStack;
static RenderTexture s_mainRt;

constexpr int BLOOM_PASSES = 5;
static struct BloomPass {
    std::array<RenderTexture, BLOOM_PASSES> rts;
    ::Shader downsampleShader;
    ::Shader upsampleShader;
    float intensity = 0.5f;
    int downsampleShaderTextureSizeLoc;
    int upsampleShaderTextureSizeLoc;
} s_bloomPass;

static ::Shader s_tonemappingShader;

static Color4b applyTint(Color4b color) {
    auto tintColor = s_tintStack.empty() ? Color4b(WHITE) : s_tintStack.back();
    return Color4b(Color4f(tintColor) * Color4f(color));
}

void Render::init() {
    constexpr int FONT_ATLAS_SIZE = 96;

    s_defaultFont = LoadFontEx("resources/fonts/Noto_Serif/NotoSerif-SemiBold.ttf",
        FONT_ATLAS_SIZE, nullptr, 0);

    SetTextureFilter(s_defaultFont.texture, TEXTURE_FILTER_BILINEAR);

    // TODO(aren): Properly handle high dpi
    auto framebufferSize = V2f(GetScreenWidth(), GetScreenHeight()).scale(V2F_ONE).floor();

    s_mainRt = RenderTexture(framebufferSize.x, framebufferSize.y);

    // bloom
    auto bloomFramebufferSize = framebufferSize;
    for (int i = 0; i < BLOOM_PASSES; i++) {
        bloomFramebufferSize.x /= 2;
        bloomFramebufferSize.y /= 2;

        s_bloomPass.rts[i] = RenderTexture(bloomFramebufferSize.x, bloomFramebufferSize.y);
        SetTextureFilter(s_bloomPass.rts[i].color(), TEXTURE_FILTER_BILINEAR);
        SetTextureWrap(s_bloomPass.rts[i].color(), TEXTURE_WRAP_CLAMP);
    }

    s_bloomPass.downsampleShader = LoadShader(NULL, TextFormat("resources/shaders/glsl%i/bloom_downsample.fs", GLSL_VERSION));

    s_bloomPass.downsampleShaderTextureSizeLoc = GetShaderLocation(s_bloomPass.downsampleShader, "textureSize");

    s_bloomPass.upsampleShader = LoadShader(NULL, TextFormat("resources/shaders/glsl%i/bloom_upsample.fs", GLSL_VERSION));

    s_bloomPass.upsampleShaderTextureSizeLoc = GetShaderLocation(s_bloomPass.upsampleShader, "textureSize");

    s_tonemappingShader = LoadShader(NULL, TextFormat("resources/shaders/glsl%i/tonemapping.fs", GLSL_VERSION));
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

void Render::beginGameDraw() {
    BeginTextureMode(static_cast<::RenderTexture>(s_mainRt));

    // TODO(aren): Properly handle high dpi
    // rlPushMatrix();
    // rlTranslatef(0.0f, GetScreenHeight(), 0.0f);

    ClearBackground(s_backgroundColor);
}

void Render::endGameDraw() {
    // rlPopMatrix();

    // int sw = GetScreenWidth();
    // int sh = GetScreenHeight();
    // float r = std::sqrt(float(sw * sw + sh * sh)) * 0.5f;
    // DrawCircleGradient(sw / 2, sh / 2, r, {0, 0, 0, 0}, {0, 0, 0, s_vignette});

    EndTextureMode();

    // bloom pass
    {
        // downsample
        auto *prevRT = &s_mainRt;
        for (int i = 0; i < BLOOM_PASSES; i++) {
            auto *nextRT = &s_bloomPass.rts[i];

            float textureSize[4] = {
                float(prevRT->width()), float(prevRT->height()),
                1.0f / prevRT->width(), 1.0f / prevRT->height(),
            };
            SetShaderValue(s_bloomPass.downsampleShader, s_bloomPass.downsampleShaderTextureSizeLoc, textureSize, SHADER_UNIFORM_VEC4);

            BeginTextureMode(static_cast<::RenderTexture>(*nextRT));
            BeginShaderMode(s_bloomPass.downsampleShader);
            DrawTexturePro(prevRT->color(), Rect(0, 0, prevRT->width(), prevRT->height()), Rect(0, 0, nextRT->width(), nextRT->height()), V2F_ZERO, 0, WHITE);
            // DrawTextureRec(prevRT->color(), Rect(0, 0, prevRT->width(), -prevRT->height()), V2F_ZERO, WHITE);
            EndShaderMode();
            EndTextureMode();
            prevRT = nextRT;
        }
        // upsample
        for (int i = BLOOM_PASSES-1; i >= 0; i--) {
            auto *nextRT = &s_bloomPass.rts[i];

            float textureSize[4] = {
                float(prevRT->width()), float(prevRT->height()),
                1.0f / prevRT->width(), 1.0f / prevRT->height(),
            };
            SetShaderValue(s_bloomPass.upsampleShader, s_bloomPass.upsampleShaderTextureSizeLoc, textureSize, SHADER_UNIFORM_VEC4);

            BeginTextureMode(static_cast<::RenderTexture>(*nextRT));
            BeginShaderMode(s_bloomPass.upsampleShader);
            DrawTexturePro(prevRT->color(), Rect(0, 0, prevRT->width(), prevRT->height()), Rect(0, 0, nextRT->width(), nextRT->height()), V2F_ZERO, 0, WHITE);
            // DrawTextureRec(prevRT->color(), Rect(0, 0, prevRT->width(), -prevRT->height()), V2F_ZERO, WHITE);
            EndShaderMode();
            EndTextureMode();
            prevRT = nextRT;
        }
        // blit
        BeginTextureMode(static_cast<::RenderTexture>(s_mainRt));
        BeginBlendMode(BLEND_ADDITIVE);
        DrawTexturePro(
            prevRT->color(),
            Rect(0, 0, prevRT->width(), -prevRT->height()),
            Rect(0, 0, s_mainRt.width(), s_mainRt.height()),
            V2F_ZERO, 0,
            Color4f(1, 1, 1, 0.5f)
        );
        EndBlendMode();
        EndTextureMode();
    }

    BeginDrawing();
    BeginShaderMode(s_tonemappingShader);
    // NOTE(aren): flip y
    DrawTextureRec(s_mainRt.color(), Rect(0, 0, s_mainRt.width(), -s_mainRt.height()), V2F_ZERO, WHITE);
    EndShaderMode();
    // EndDrawing();
}

void Render::begin2DCam(const Camera &cam) {
    rlPushMatrix();

    BeginMode2D(static_cast<Camera2D>(cam));
}

void Render::end2DCam() {
    EndMode2D();

    rlPopMatrix();
}

// NOTE(aren): Intended to be called after game draw
void Render::beginUIDraw() {
    // BeginDrawing();
}

void Render::endUIDraw() {
    EndDrawing();
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