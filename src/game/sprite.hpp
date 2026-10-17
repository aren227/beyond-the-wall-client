#pragma once

namespace aren {

class Layer {
public:
    struct Render {
        enum Method {
            Tiled = 0,
            Patched,
            NoRepeat,
        };

        Method method = Method::Tiled;

        union {
            struct {
                int border[4];
            } patched;
        } params;
    };
public:
    Texture m_texture;
    int m_tag = -1;
    Render m_render;
};

class Sprite {
public:
    std::string m_name;
    std::vector<Layer> m_layers;

public:
    Sprite() = default;
    void render(Rect rect, int tag, float scaler) const;
    void renderCorner(Vector2f center, int tag, int rotation, float scaler) const;
    void renderEdge(Vector2f center, Vector2f dirlen, int tag, int rotation, float scaler) const;

    static bool load(const std::string &name, Sprite &sprite);
};

} // namespace aren