#include "sprite_editor_scene.hpp"
#include <fstream>

namespace aren {

SpriteEditorScene::SpriteEditorScene() : Scene(SpriteEditorScene::TYPE) {
    refreshAssets();
}

void SpriteEditorScene::update(float dt) {
    (void)dt;

    if (Input::mouseButtonDown(0) == false && m_selectedEdge != -1) {
        m_selectedEdge = -1;
    }
    if (Input::mouseButtonPressed(0) && m_selectedEdge == -1) {
        std::array<Vector2f, 4> points = {
            V2f(m_renderRect.x, m_renderRect.y),
            V2f(m_renderRect.x + m_renderRect.w, m_renderRect.y),
            V2f(m_renderRect.x + m_renderRect.w, m_renderRect.y + m_renderRect.h),
            V2f(m_renderRect.x, m_renderRect.y + m_renderRect.h),
        };
        for (int i = 0; i < 4; i++) {
            auto a = points[i];
            auto b = points[(i + 1) % 4];
            if (i >= 2) std::swap(a, b);
            auto rect = Rect(a.x, a.y, b.x - a.x, b.y - a.y).extends(10);
            if (rect.contains(Input::mousePosition())) {
                m_selectedEdge = i;
                break;
            }
        }
    }
    if (m_selectedEdge != -1) {
        switch (m_selectedEdge) {
            case 0:
                m_renderRect.y += Input::mouseDelta().y;
                m_renderRect.h -= Input::mouseDelta().y;
                break;
            case 1:
                m_renderRect.w += Input::mouseDelta().x;
                break;
            case 2:
                m_renderRect.h += Input::mouseDelta().y;
                break;
            case 3:
                m_renderRect.x += Input::mouseDelta().x;
                m_renderRect.w -= Input::mouseDelta().x;
                break;
            default:
                ASSERT(false);
                break;
        }
    }
}

void SpriteEditorScene::render() {
    if (m_selectedSpriteIndex >= 0 && m_selectedSpriteIndex < m_sprites.size()) {
        constexpr float SCALER = 2;
        auto rect = m_renderRect;
        rect.x /= SCALER;
        rect.y /= SCALER;
        rect.w /= SCALER;
        rect.h /= SCALER;
        auto rectK = rect;
        rectK.x *= SCALER;
        rectK.y *= SCALER;
        rectK.w *= SCALER;
        rectK.h *= SCALER;

        const auto &sprite = m_sprites[m_selectedSpriteIndex];
        for (const auto &layer : sprite.m_layers) {
            switch (layer.m_render.method) {
                case Layer::Render::Method::Tiled:
                {
                    DrawTexturePro(::Texture2D(layer.m_texture), rect, rectK, V2F_ZERO, 0, WHITE);
                    break;
                }
                case Layer::Render::Method::Patched:
                {
                    float w = layer.m_texture.width(), h = layer.m_texture.height();
                    const auto &border = layer.m_render.patched.border;
                    float b = h - border[0], t = border[1], l = border[2], r = w - border[3];
                    float xs_s[4] = { 0, l, r, w };
                    float ys_s[4] = { 0, t, b, h };
                    float xs_d[4] = { 0, l * SCALER, rectK.w - (w - r) * SCALER, rectK.w };
                    float ys_d[4] = { 0, t * SCALER, rectK.h - (h - b) * SCALER, rectK.h };

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
                            DrawTexturePro(::Texture2D(layer.m_texture), rectS, rectD, V2F_ZERO, 0, WHITE);
                        }
                    }

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
}

void SpriteEditorScene::renderUI() {
    static std::string spriteNameJoined;
    spriteNameJoined.clear();
    for (size_t i = 0; i < m_sprites.size(); i++) {
        spriteNameJoined += m_sprites[i].m_name;
        if (i + 1 < m_sprites.size()) {
            spriteNameJoined += ";";
        }
    }

    float y = 10;
    if (GuiButton({10, y, 180, 30}, "Refresh")) {
        refreshAssets();
    }
    y += 40;

    if (GuiDropdownBox({10, y, 180, 30}, spriteNameJoined.c_str(), &m_selectedSpriteIndex, m_spriteDropdownActive)) {
        m_spriteDropdownActive = !m_spriteDropdownActive;
    }
    y += 40;
}

void SpriteEditorScene::refreshAssets() {
    m_sprites.clear();

    const std::string path = "./resources/sprites";
    if (DirectoryExists(path.c_str())) {
        auto res = LoadDirectoryFilesEx(path.c_str(), ".sprite", false);
        for (size_t i = 0; i < res.count; i++) {
            Sprite sprite;
            if (loadSprite(res.paths[i], sprite)) {
                m_sprites.push_back(std::move(sprite));
            }
        }
        UnloadDirectoryFiles(res);
    }
}

bool SpriteEditorScene::loadSprite(const std::string &path, Sprite &sprite) {
    sprite.m_name = std::string(GetFileNameWithoutExt(path.c_str()));

    std::ifstream is(path);
    std::string tok;

    while (is >> tok) {
        if (tok == "layers") {
            int layerCount; is >> layerCount;
            sprite.m_layers.resize(layerCount);
        } else if (tok == "layer") {
            int layerNum; is >> layerNum;
            if (layerNum >= sprite.m_layers.size()) return false;
            auto &layer = sprite.m_layers[layerNum];

            while (is >> tok) {
                if (tok == "filename") {
                    is >> tok;
                    auto image = LoadImage(("./resources/images/" + tok + ".png").c_str());
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
                            is >> layer.m_render.patched.border[i];
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