#pragma once

#include "scene.hpp"
#include "sprite.hpp"

namespace aren {

class SpriteEditorScene : public Scene {
public:
    static constexpr SceneType TYPE = SceneType::SpriteEditor;

    SpriteEditorScene();

    void update(float dt) override;
    void render() override;
    void renderUI() override;

    void refreshAssets();

private:
    std::vector<Sprite> m_sprites;

    Rect m_renderRect = Rect(100, 100, 100, 100);
    int m_selectedEdge = -1;

    int m_selectedSpriteIndex = 0;
    bool m_spriteDropdownActive = false;
};

} // namespace aren