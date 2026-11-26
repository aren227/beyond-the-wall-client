#include "render.hpp"
#include "camera.hpp"
#include "render_texture.hpp"
#include "rlgl.h"

namespace aren {

struct FontFamily {
    ::Font small;
    float smallSize;
    ::Font large;
    float largeSize;
};

static std::unordered_map<std::string, FontFamily> s_fonts;
static std::vector<std::string> s_fontNameStack = { "default" };

static std::vector<Color4f> s_hdrColorStack = { C4F_WHITE };

Color4b Render::s_backgroundColor = {50, 56, 68, 255};
uint8_t Render::s_vignette = 16;

static RenderTexture s_mainRt;

constexpr int BLOOM_PASSES = 5;
static struct BloomPass {
    std::array<RenderTexture, BLOOM_PASSES> rts;
    ::Shader downsampleShader;
    ::Shader upsampleShader;
    float mix = 0.7f;
    float intensity = 0.1f;
    int downsampleShaderTextureSizeLoc;
    int upsampleShaderTextureSizeLoc;
} s_bloomPass;

static ::Shader s_gammaToLinearShader;
static int s_gammaToLinearShaderDiffuseHDRLoc;

static ::Shader s_tonemappingShader;

static ::Shader s_beamShader;
static int s_beamShaderMiscLoc;

::Font Render::getFontBySize(const std::string &name, float size) {
    const FontFamily *family = nullptr;
    if (s_fonts.contains(name) == false) {
        family = &s_fonts["default"];
    } else {
        family = &s_fonts[name];
    }

    if (size >= family->largeSize) {
        return family->large;
    }
    return family->small;
}

::Font Render::getCurrentFontBySize(float size) {
    return getFontBySize(s_fontNameStack.back(), size);
}

void Render::pushFontName(const std::string &name) {
    s_fontNameStack.push_back(name);
}

void Render::popFontName() {
    ASSERT(s_fontNameStack.size() > 1);
    s_fontNameStack.pop_back();
}

// NOTE(aren): Works only in game render pass
void Render::pushColor(const Color4f &color) {
    const auto prev = s_hdrColorStack.back();

    s_hdrColorStack.push_back(color);

    // XXX(aren): Must manually flush the current batch.
    if (prev != color) {
        rlDrawRenderBatchActive();
        SetShaderValue(s_gammaToLinearShader, s_gammaToLinearShaderDiffuseHDRLoc, &s_hdrColorStack.back(), SHADER_UNIFORM_VEC4);
    }
}

void Render::popColor() {
    const auto prev = s_hdrColorStack.back();

    ASSERT(s_hdrColorStack.size() > 1);
    s_hdrColorStack.pop_back();

    if (prev != s_hdrColorStack.back()) {
        rlDrawRenderBatchActive();
        SetShaderValue(s_gammaToLinearShader, s_gammaToLinearShaderDiffuseHDRLoc, &s_hdrColorStack.back(), SHADER_UNIFORM_VEC4);
    }
}

static FontFamily loadFontFamily(const char *path, float smallSize, float largeSize, bool bilinear) {
    FontFamily family = {
        .small = LoadFontEx(path, smallSize, nullptr, 0),
        .smallSize = smallSize,
        .large = LoadFontEx(path, largeSize, nullptr, 0),
        .largeSize = largeSize,
    };
    if (bilinear) {
        SetTextureFilter(family.small.texture, TEXTURE_FILTER_BILINEAR);
        SetTextureFilter(family.large.texture, TEXTURE_FILTER_BILINEAR);
    }
    return family;
}

void Render::init() {
    s_fonts["default"] = loadFontFamily("resources/fonts/Jersey_10/Jersey10-Regular.ttf", 20, 40, false);
    s_fonts["editor"] = loadFontFamily("resources/fonts/Noto_Serif/NotoSerif-SemiBold.ttf", 16, 96, true);

    // TODO(aren): Properly handle high dpi
    auto framebufferSize = V2f(GetScreenWidth(), GetScreenHeight()).scale(V2F_ONE).floor();

    s_mainRt = RenderTexture(framebufferSize.x, framebufferSize.y, true);

    // bloom
    auto bloomFramebufferSize = framebufferSize;
    for (int i = 0; i < BLOOM_PASSES; i++) {
        bloomFramebufferSize.x = (bloomFramebufferSize.x + 1) / 2;
        bloomFramebufferSize.y = (bloomFramebufferSize.y + 1) / 2;

        s_bloomPass.rts[i] = RenderTexture(bloomFramebufferSize.x, bloomFramebufferSize.y, true);
        SetTextureFilter(s_bloomPass.rts[i].color(), TEXTURE_FILTER_BILINEAR);
        SetTextureWrap(s_bloomPass.rts[i].color(), TEXTURE_WRAP_CLAMP);
    }

    s_bloomPass.downsampleShader = LoadShader(NULL, TextFormat("resources/shaders/glsl%i/bloom_downsample.fs", GLSL_VERSION));

    s_bloomPass.downsampleShaderTextureSizeLoc = GetShaderLocation(s_bloomPass.downsampleShader, "textureSize");

    s_bloomPass.upsampleShader = LoadShader(NULL, TextFormat("resources/shaders/glsl%i/bloom_upsample.fs", GLSL_VERSION));

    s_bloomPass.upsampleShaderTextureSizeLoc = GetShaderLocation(s_bloomPass.upsampleShader, "textureSize");

    s_gammaToLinearShader = LoadShader(
        TextFormat("resources/shaders/glsl%i/gamma_to_linear.vs", GLSL_VERSION),
        TextFormat("resources/shaders/glsl%i/gamma_to_linear.fs", GLSL_VERSION)
    );

    s_gammaToLinearShaderDiffuseHDRLoc = GetShaderLocation(s_gammaToLinearShader, "colDiffuseHDR");

    s_tonemappingShader = LoadShader(NULL, TextFormat("resources/shaders/glsl%i/tonemapping.fs", GLSL_VERSION));

    s_beamShader = LoadShader(NULL, TextFormat("resources/shaders/glsl%i/beam.fs", GLSL_VERSION));
    s_beamShaderMiscLoc = GetShaderLocation(s_beamShader, "misc");
}

void Render::deinit() {
    for (const auto &[k, v] : s_fonts) {
        UnloadFont(v.small);
        UnloadFont(v.large);
    }
}

Vector2f Render::measureText(const char *text, float size) {
    auto font = getCurrentFontBySize(size);
    return MeasureTextEx(font, text, size, 1.0f);
}

void Render::drawText(Vector2f origin, float size, Color4b color, const char *text, TextAlignH alignH, TextAlignV alignV) {
    auto dimension = measureText(text, size);

    if (alignH == TextAlignH::Center) origin.x -= dimension.x * 0.5f;
    else if (alignH == TextAlignH::Right) origin.x -= dimension.x;
    if (alignV == TextAlignV::Center) origin.y -= dimension.y * 0.5f;
    else if (alignV == TextAlignV::Bottom) origin.y -= dimension.y;

    // TODO(aren): shadow

    auto font = getCurrentFontBySize(size);
    DrawTextEx(font, text, origin, size, 1.0f, color);
}

void Render::drawTextRotated(Vector2f origin, float size, float rotationDeg, Color4b color, const char *text, TextAlignH alignH, TextAlignV alignV) {
    auto dimension = measureText(text, size);

    auto pivot = V2F_ZERO;
    if (alignH == TextAlignH::Center) pivot.x = dimension.x * 0.5f;
    else if (alignH == TextAlignH::Right) pivot.x = dimension.x;
    if (alignV == TextAlignV::Center) pivot.y = dimension.y * 0.5f;
    else if (alignV == TextAlignV::Bottom) pivot.y = dimension.y;

    // TODO(aren): shadow

    auto font = getCurrentFontBySize(size);
    DrawTextPro(font, text, origin, pivot, rotationDeg, size, 1.0f, color);
}

void Render::beginGameDraw() {
    BeginTextureMode(static_cast<::RenderTexture>(s_mainRt));

    // TODO(aren): Properly handle high dpi
    // rlPushMatrix();
    // rlTranslatef(0.0f, GetScreenHeight(), 0.0f);

    // TODO(aren): Not in linear space
    ClearBackground(s_backgroundColor);

    BeginShaderMode(s_gammaToLinearShader);

    SetShaderValue(s_gammaToLinearShader, s_gammaToLinearShaderDiffuseHDRLoc, &s_hdrColorStack.back(), SHADER_UNIFORM_VEC4);

    BeginBlendMode(BLEND_ALPHA);
}

void Render::endGameDraw() {
    // rlPopMatrix();

    // int sw = GetScreenWidth();
    // int sh = GetScreenHeight();
    // float r = std::sqrt(float(sw * sw + sh * sh)) * 0.5f;
    // DrawCircleGradient(sw / 2, sh / 2, r, {0, 0, 0, 0}, {0, 0, 0, s_vignette});

    EndBlendMode();

    EndShaderMode();

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
            ClearBackground(BLACK);
            DrawTexturePro(prevRT->color(), Rect(0, 0, prevRT->width(), -prevRT->height()), Rect(0, 0, nextRT->width(), nextRT->height()), V2F_ZERO, 0, WHITE);
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
            BeginBlendMode(BLEND_ADDITIVE);
            DrawTexturePro(prevRT->color(), Rect(0, 0, prevRT->width(), -prevRT->height()), Rect(0, 0, nextRT->width(), nextRT->height()), V2F_ZERO, 0, Color4f(1, 1, 1, s_bloomPass.mix));
            // DrawTextureRec(prevRT->color(), Rect(0, 0, prevRT->width(), -prevRT->height()), V2F_ZERO, WHITE);
            EndBlendMode();
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
            Color4f(1, 1, 1, s_bloomPass.intensity)
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

void Render::beginBeam() {
    float misc[4] = { float(GetScreenWidth()), float(GetScreenHeight()), float(GetTime()), 0.0f };

    BeginShaderMode(s_beamShader);
    SetShaderValue(s_beamShader, s_beamShaderMiscLoc, &misc, SHADER_UNIFORM_VEC4);
}

void Render::endBeam() {
    EndShaderMode();
    BeginShaderMode(s_gammaToLinearShader);
}

void Render::drawTriangleGradient(Vector2f a, Color4b ca, Vector2f b, Color4b cb, Vector2f c, Color4b cc) {
    rlBegin(RL_TRIANGLES);
    rlColor4ub(ca.r, ca.g, ca.b, ca.a); rlVertex2f(a.x, a.y);
    rlColor4ub(cb.r, cb.g, cb.b, cb.a); rlVertex2f(b.x, b.y);
    rlColor4ub(cc.r, cc.g, cc.b, cc.a); rlVertex2f(c.x, c.y);
    rlEnd();
}

} // namespace aren