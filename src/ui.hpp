#pragma once

#include "render.hpp"

namespace aren {

class UI {
public:
    static std::optional<int> s_clickSfxId;

    static constexpr Color4b PRIMARY = { 255, 255, 255, 255 };
    static constexpr Color4b SECONDARY = { 190, 190, 190, 255 };
    static constexpr Color4b SELECTED = { 127, 127, 127, 255 };
    static constexpr Color4b ACCENT = { 255, 220, 0, 255 };

    // TODO(aren): Is tint parameter good?
    static bool textButton(Vector2f origin, float size, const char *text, Render::TextAlignH alignH, Render::TextAlignV alignV, Color4b tint = WHITE);
    static int slider(Vector2f center, float width, int maxValue, const char *id, int value);
    static bool checkbox(Vector2f center, bool value);

    static void beginInput();
    static void endInput();
};

} // namespace aren
