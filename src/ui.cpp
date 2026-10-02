#include "ui.hpp"
#include "audio.hpp"
#include "render.hpp"

namespace aren {

Color4b UI::s_colorTextPrimary      = {255, 255, 255, 255};
Color4b UI::s_colorTextSecondary    = {185, 185, 185, 255};
Color4b UI::s_colorTextButtonIdle   = {200, 200, 200, 255};
Color4b UI::s_colorTextButtonHover  = {255, 255, 255, 255};
Color4b UI::s_colorControlPrimary   = {220, 220, 230, 255};
Color4b UI::s_colorControlAccent    = {90,  170, 255, 255};
Color4b UI::s_colorControlEmpty     = {0,   0,   0,   120};
Color4b UI::s_colorControlHover     = {255, 255, 255, 255};

std::optional<int> UI::s_clickSfxId = std::nullopt;

static int s_enableInput = 0;

// NOTE(aren): Assume that label string pointers are distinct..
static const char *s_dragSlider = nullptr;

void UI::beginInput() {
    s_enableInput++;
}

void UI::endInput() {
    s_enableInput--;
    ASSERT(s_enableInput >= 0);
}

bool UI::textButton(Vector2f center, float size, const char *label) {
    constexpr Vector2f PAD = V2f(12.0f, 4.0f);

    auto dim = Render::measureText(label, size);
    auto hit = Rect(center - dim * 0.5f - PAD, dim + 2 * PAD);

    auto m = Input::mousePosition();
    bool hovered = (s_enableInput > 0) && hit.contains(m);

    Render::drawText(center, size, hovered ? s_colorTextButtonHover : s_colorTextButtonIdle, label,
        Render::TextAlignH::Center, Render::TextAlignV::Center);
    bool clicked = hovered && Input::mouseButtonPressed(MOUSE_BUTTON_LEFT);
    if (clicked && s_clickSfxId) Audio::play(s_clickSfxId.value());
    return clicked;
}

int UI::slider(Vector2f center, float size, const char *label, int value, int steps) {
    constexpr float SEG_W_MUL = 0.55f;
    constexpr Vector2f HANDLE_SIZE = V2f(8.0f, 24.0f);
    constexpr float HANDLE_CLICK_MARGIN = 12.0f;
    constexpr float SEG_H = 8.0f;
    constexpr float SEG_GAP = 2.0f;
    constexpr float LABEL_GAP = 20.0f;

    float segW = size * SEG_W_MUL;
    float stepW = segW + SEG_GAP;
    float barW = float(steps) * segW + float(steps - 1) * SEG_GAP;

    auto labelDim = Render::measureText(label, size);
    float totalW = labelDim.x + LABEL_GAP + barW;
    float startX = center.x - totalW * 0.5f;

    Render::drawText(V2f(startX, center.y), size,
        s_colorTextPrimary, label,
        Render::TextAlignH::Left, Render::TextAlignV::Center);

    float barX = startX + labelDim.x + LABEL_GAP;
    auto mouse = Input::mousePosition();

    int newValue = value;

    float handleX = barX + float(value) * stepW;
    auto handleHit = Rect(V2f(handleX, center.y) - HANDLE_SIZE * 0.5f, HANDLE_SIZE).extends(HANDLE_CLICK_MARGIN);

    bool handleHovered = (s_enableInput > 0) && handleHit.contains(mouse);

    if (handleHovered && Input::mouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        s_dragSlider = label;
    }

    if (s_dragSlider == label) {
        if (Input::mouseButtonDown(MOUSE_BUTTON_LEFT)) {
            int snapped = std::clamp(int(std::round((mouse.x - barX) / stepW)), 0, steps);
            if (snapped != newValue) {
                newValue = snapped;
                if (s_clickSfxId) {
                    Audio::play(s_clickSfxId.value(), {0.33f, 1.0f, 0.5f});
                }
            }
        } else {
            s_dragSlider = nullptr;
        }
    }

    // Segment bar
    for (int i = 0; i < steps; i++) {
        auto r = Rect(
            barX + float(i) * stepW,
            center.y - SEG_H * 0.5f,
            segW,
            SEG_H
        );
        DrawRectangleRec(r, (i < newValue) ? s_colorControlAccent : s_colorControlEmpty);
    }

    // Handle
    {
        float hx = barX + float(newValue) * stepW;
        auto r = Rect(V2f(hx, center.y) - HANDLE_SIZE * 0.5f, HANDLE_SIZE);
        bool active = handleHovered || (s_dragSlider == label);
        DrawRectangleRec(r, active ? s_colorControlHover : s_colorControlPrimary);
    }

    return newValue;
}

bool UI::checkbox(Vector2f center, float size, const char *label, bool value) {
    constexpr float BOX_MUL = 0.7f;
    constexpr float LABEL_GAP = 14.0f;
    constexpr float BORDER = 2.0f;
    constexpr float INNER_PAD = 4.0f;

    float box = size * BOX_MUL;
    auto labelDim = Render::measureText(label, size);
    float totalW = labelDim.x + LABEL_GAP + box;
    float startX = center.x - totalW * 0.5f;

    Render::drawText(V2f(startX, center.y), size, s_colorTextPrimary, label, Render::TextAlignH::Left, Render::TextAlignV::Center);

    auto r = Rect(startX + labelDim.x + LABEL_GAP, center.y - box * 0.5f, box, box);
    auto mouse = Input::mousePosition();
    bool hovered = (s_enableInput > 0) && r.contains(mouse);

    DrawRectangleRec(r, s_colorControlEmpty);
    DrawRectangleLinesEx(r, BORDER, hovered ? s_colorControlHover : s_colorControlPrimary);
    if (value) {
        auto inner = r.extends(-INNER_PAD);
        DrawRectangleRec(inner, s_colorControlAccent);
    }

    bool toggled = hovered && Input::mouseButtonPressed(MOUSE_BUTTON_LEFT);
    if (toggled) {
        value = !value;
        if (s_clickSfxId) {
            Audio::play(s_clickSfxId.value());
        }
    }
    return value;
}

} // namespace aren
