#include "ui.hpp"
#include "audio.hpp"
#include "render.hpp"

namespace aren {

std::optional<int> UI::s_clickSfxId = std::nullopt;

static int s_enableInput = 0;

// NOTE(aren): Assume that label string pointers are distinct..
static const char *s_selectedSlider = nullptr;

void UI::beginInput() {
    s_enableInput++;
}

void UI::endInput() {
    s_enableInput--;
    ASSERT(s_enableInput >= 0);
}

bool UI::textButton(Vector2f origin, float size, const char *text, Render::TextAlignH alignH, Render::TextAlignV alignV, Color4b tint) {
    auto dimension = Render::measureText(text, size);

    if (alignH == Render::TextAlignH::Center) origin.x -= dimension.x * 0.5f;
    else if (alignH == Render::TextAlignH::Right) origin.x -= dimension.x;
    if (alignV == Render::TextAlignV::Center) origin.y -= dimension.y * 0.5f;
    else if (alignV == Render::TextAlignV::Bottom) origin.y -= dimension.y;

    auto hit = Rect(origin, dimension);
    bool hovered = (s_enableInput > 0) && hit.contains(Input::mousePosition());
    bool selected = hovered && Input::mouseButtonDown(MOUSE_BUTTON_LEFT);
    bool clicked = hovered && Input::mouseButtonReleased(MOUSE_BUTTON_LEFT);

    auto color = (tint.f() * (selected ? SELECTED : hovered ? SECONDARY : PRIMARY).f());

    auto font = Render::getCurrentFontBySize(size);
    DrawTextEx(font, text, origin, size, 1.0f, color);

    if (clicked && s_clickSfxId) {
        Audio::play(s_clickSfxId.value());
    }

    return clicked;
}

int UI::slider(Vector2f center, float width, int maxValue, const char *id, int value) {
    Vector2f start = center - V2f(width * 0.5f, 0);
    Vector2f end = center + V2f(width * 0.5f, 0);
    auto rect = Rect(start.x, start.y - 16, width, 32);

    bool hovered = (s_enableInput > 0) && rect.contains(Input::mousePosition());

    if (hovered && Input::mouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        s_selectedSlider = id;
    }
    if (s_selectedSlider == id && (Input::mouseButtonDown(MOUSE_BUTTON_LEFT) == false || Input::mouseButtonReleased(MOUSE_BUTTON_LEFT))) {
        s_selectedSlider = nullptr;
    }

    bool selected = (s_selectedSlider == id) && Input::mouseButtonDown(MOUSE_BUTTON_LEFT);

    auto color = selected ? SELECTED : hovered ? SECONDARY : PRIMARY;
    auto innerColor = selected ? SELECTED : ACCENT;

    DrawLineEx(start, end, 4, innerColor);

    float rate = float(value) / maxValue;
    auto curr = Vector2f::lerp(start, end, rate);
    DrawLineEx(curr + V2f(0, 16), curr - V2f(0, 16), 4, color);

    if (selected) {
        float nextRate = std::clamp((Input::mousePosition().x - start.x) / (end.x - start.x), 0.0f, 1.0f);
        value = std::round(nextRate * maxValue);
    }

    return value;
}

bool UI::checkbox(Vector2f center, bool value) {
    auto size = V2f(32, 32);
    auto rect = Rect(center - size * 0.5f, size);
    bool hovered = (s_enableInput > 0) && rect.contains(Input::mousePosition());
    bool selected = hovered && Input::mouseButtonDown(MOUSE_BUTTON_LEFT);
    bool clicked = hovered && Input::mouseButtonReleased(MOUSE_BUTTON_LEFT);

    auto color = selected ? SELECTED : hovered ? SECONDARY : PRIMARY;
    auto innerColor = selected ? SELECTED : ACCENT;

    DrawRectangleLinesEx(rect, 3, color);
    if (value) {
        DrawRectangleRec(rect.extends(-8), innerColor);
    }

    if (clicked) {
        value = !value;
        if (s_clickSfxId) {
            Audio::play(s_clickSfxId.value());
        }
    }

    return value;
}

} // namespace aren
