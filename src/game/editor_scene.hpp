#pragma once

#include "scene.hpp"
#include "../camera.hpp"
#include "sprite.hpp"

namespace aren {

enum class EditorBrush {
    Wall = 0,
    Barrier,
    Glass,
    Sand,
    __MAX__,
};

enum class EditorBlockType {
    Empty = 0,
    Wall,
    Barrier,
    Glass,
    Sand,
};

class EditorBlock {
public:
    EditorBlockType m_type = EditorBlockType::Empty;

    explicit EditorBlock() = default;
    EditorBlock(EditorBlockType type) : m_type(type) {}
};

class EditorScene : public Scene {
public:
    static constexpr SceneType TYPE = SceneType::Editor;

    EditorScene();

    void update(float dt) override;
    void render() override;
    void renderUI() override;

private:
    EditorBlock &getBlock(const Vector2i &pos);

private:
    Camera m_cam;
    int m_brushIndex = 0;
    std::unordered_map<Vector2i, EditorBlock> m_blocks;
    bool m_drawWireframe = true;

    Sprite m_brickSprite;
    Sprite m_cornerSprite;
    Sprite m_edgeSprite;
};

} // namespace aren