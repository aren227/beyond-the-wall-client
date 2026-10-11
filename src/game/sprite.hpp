#pragma once

namespace aren {

class Layer {
public:
    struct Render {
        enum Method {
            Tiled = 0,
            Patched,
        };

        Method method = Method::Tiled;

        union {
            int border[4];
        } patched;
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
};

} // namespace aren