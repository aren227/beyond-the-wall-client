#include "sprite_editor_scene.hpp"

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
        m_sprites[m_selectedSpriteIndex].render(m_renderRect, -1, 0, 2);
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
            auto name = std::string(GetFileNameWithoutExt(path.c_str()));
            if (Sprite::load(name, sprite)) {
                m_sprites.push_back(std::move(sprite));
            }
        }
        UnloadDirectoryFiles(res);
    }
}

} // namespace aren