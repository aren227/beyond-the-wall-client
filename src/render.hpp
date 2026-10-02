#pragma once

namespace aren {

class Camera;

class Render {
public:
    enum class TextAlignH {
        Left, Center, Right
    };

    enum class TextAlignV {
        Top, Center, Bottom
    };

    static Color4b s_backgroundColor;
    static uint8_t s_vignette;

    static void init();
    static void deinit();

    static Vector2f measureText(const char *text, float size);
    static void drawText(Vector2f origin, float size, Color4b color, const char *text, TextAlignH alignH, TextAlignV alignV);
    static void drawTextRotated(Vector2f origin, float size, float rotationDeg, Color4b color, const char *text, TextAlignH alignH, TextAlignV alignV);

    static void beginDraw();
    static void endDraw();
    static void begin2DCam(const Camera &cam);
    static void end2DCam();

    static void pushTint(Color4b color);
    static void popTint();

    static void drawTriangleGradient(Vector2f a, Color4b ca, Vector2f b, Color4b cb, Vector2f c, Color4b cc);

private:
    static Font s_defaultFont;
};

} // namespace aren