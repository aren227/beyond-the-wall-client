#include "sprite.hpp"
#include "consts.hpp"
#include <fstream>

namespace aren {

static std::string getImagePath(const std::string &name) {
    return IMAGE_BASE + "/" + name + ".png";
}

static std::string getSpritePath(const std::string &name) {
    return SPRITE_BASE + "/" + name + ".sprite";
}

void Sprite::render(Rect rect, int tag, float rotation, float scaler, const Color4f &color) const {
    auto rectK = rect;

    rect.x /= scaler;
    rect.y /= scaler;
    rect.w /= scaler;
    rect.h /= scaler;

    for (const auto &layer : m_layers) {
        if (layer.m_tag != tag) {
            continue;
        }

        switch (layer.m_render.method) {
            case Layer::Render::Method::Tiled:
            {
                DrawTexturePro(::Texture2D(layer.m_texture), rect, rectK, V2F_ZERO, 0, color);
                break;
            }
            case Layer::Render::Method::Patched:
            {
                float w = layer.m_texture.width(), h = layer.m_texture.height();
                const auto &border = layer.m_render.params.patched.border;
                float b = h - border[0], t = border[1], l = border[2], r = w - border[3];
                float xs_s[4] = { 0, l, r, w };
                float ys_s[4] = { 0, t, b, h };
                float xs_d[4] = { 0, l * scaler, rectK.w - (w - r) * scaler, rectK.w };
                float ys_d[4] = { 0, t * scaler, rectK.h - (h - b) * scaler, rectK.h };

                for (int i = 0; i < 3; i++) {
                    for (int j = 0; j < 3; j++) {
                        Rect rectS = Rect(
                            xs_s[i],
                            ys_s[j],
                            xs_s[i+1] - xs_s[i],
                            ys_s[j+1] - ys_s[j]
                        );
                        Rect rectD = Rect(
                            xs_d[i] + rectK.x,
                            ys_d[j] + rectK.y,
                            xs_d[i+1] - xs_d[i],
                            ys_d[j+1] - ys_d[j]
                        );
                        DrawTexturePro(::Texture2D(layer.m_texture), rectS, rectD, V2F_ZERO, 0, color);
                    }
                }

                break;
            }
            case Layer::Render::Method::NoRepeat:
            {
                float sw = std::min(rect.w, float(layer.m_texture.width()));
                float sh = std::min(rect.h, float(layer.m_texture.height()));
                float sx = (layer.m_texture.width() - sw) * 0.5f;
                float sy = (layer.m_texture.height() - sh) * 0.5f;

                float dw = sw * scaler;
                float dh = sh * scaler;
                float dx = (rect.w * scaler - dw) * 0.5f + rect.x * scaler;
                float dy = (rect.h * scaler - dh) * 0.5f + rect.y * scaler;

                DrawTexturePro(::Texture2D(layer.m_texture), Rect(sx, sy, sw, sh), Rect(dx + dw * 0.5f, dy + dh * 0.5f, dw, dh), V2f(dw * 0.5f, dh * 0.5f), rotation, color);
                break;
            }
            default:
            {
                ASSERT(false);
                break;
            }
        }
    }
}

void Sprite::renderCorner(Vector2f center, int tag, float rotation, float scaler, const Color4f &color) const {
    const Texture *tex = nullptr;
    for (const auto &it : m_layers) {
        if (it.m_tag == tag) {
            tex = &it.m_texture;
            break;
        }
    }

    if (tex) {
        float w = tex->width();
        float h = tex->height();
        Rect rectS = Rect(0, 0, w, h);
        Rect rectD = Rect(
            center.x,
            center.y,
            w * scaler,
        h * scaler
        );

        DrawTexturePro(::Texture2D(*tex), rectS, rectD, V2f(w, h) * scaler * 0.5f, rotation, color);
    }

}

void Sprite::renderEdge(Vector2f center, Vector2f dirlen, int tag, float rotation, float scaler, const Color4f &color) const {
    const Texture *tex = nullptr;
    for (const auto &it : m_layers) {
        if (it.m_tag == tag) {
            tex = &it.m_texture;
            break;
        }
    }

    if (tex) {
        float w = tex->width();
        float h = tex->height();
        bool isHorizontal = (dirlen.y == 0);

        Rect rectD, rectS;
        if (isHorizontal) {
            rectS = rectD = Rect(center, V2f(dirlen.x, h * scaler));
        } else {
            // XXX(aren): xy was transposed due to 90 degree rotation
            rectS = rectD = Rect(center, V2f(dirlen.y, w * scaler));
        }

        rectS.x /= scaler;
        rectS.y = 0;
        rectS.w /= scaler;
        rectS.h /= scaler;

        DrawTexturePro(::Texture2D(*tex), rectS, rectD, V2f(rectS.w, rectS.h) * scaler * 0.5f, rotation, color);
    }
}

bool Sprite::load(const std::string &name, Sprite &sprite) {
    sprite.m_name = name;

    std::ifstream is(getSpritePath(name));
    std::string tok;

    while (is >> tok) {
        if (tok == "layers") {
            int layerCount; is >> layerCount;
            sprite.m_layers.resize(layerCount);
        } else if (tok == "layer") {
            int layerNum; is >> layerNum;
            if (layerNum >= sprite.m_layers.size()) {
                ASSERTF(false, "Unexpected layer number: %d", layerNum);
                return false;
            }
            auto &layer = sprite.m_layers[layerNum];

            while (is >> tok) {
                if (tok == "filename") {
                    is >> tok;
                    auto image = LoadImage(getImagePath(tok).c_str());
                    if (IsImageValid(image) == false) {
                        UnloadImage(image);
                        return false;
                    }
                    layer.m_texture = Texture(image);
                } else if (tok == "tag") {
                    is >> layer.m_tag;
                } else if (tok == "render") {
                    int num; is >> num;
                    layer.m_render.method = Layer::Render::Method(num);
                } else if (tok == "border") {
                    ASSERT(layer.m_render.method == Layer::Render::Method::Patched);
                    if (layer.m_render.method == Layer::Render::Method::Patched) {
                        for (int i = 0; i < 4; i++) {
                            is >> layer.m_render.params.patched.border[i];
                        }
                    }
                } else if (tok == "exit") {
                    break;
                }
            }
        } else {
            ASSERTF(false, "Unexpected token: %s", tok.c_str());
        }
    }

    return true;
}

} // namespace aren