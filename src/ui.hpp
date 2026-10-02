#pragma once

namespace aren {

class UI {
public:
    static Color4b s_colorTextPrimary;
    static Color4b s_colorTextSecondary;
    static Color4b s_colorTextButtonIdle;
    static Color4b s_colorTextButtonHover;
    static Color4b s_colorControlPrimary;
    static Color4b s_colorControlAccent;
    static Color4b s_colorControlEmpty;
    static Color4b s_colorControlHover;

    static std::optional<int> s_clickSfxId;

    static bool textButton(Vector2f center, float size, const char *label);
    static int slider(Vector2f center, float size, const char *label, int value, int steps);
    static bool checkbox(Vector2f center, float size, const char *label, bool value);

    static void beginInput();
    static void endInput();
};

} // namespace aren
