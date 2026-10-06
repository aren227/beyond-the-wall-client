#pragma once

#include "scene.hpp"
#include "../camera.hpp"

namespace aren {

enum class EditorBrush {
    Wall = 0,
    Barrier,
    Glass,
    Sand,
    __MAX__,
};

enum class EditorBlockType {
    Wall = 0,
    Barrier,
    Glass,
    Sand,
};

class EditorBlock {
public:
    EditorBlockType m_type;

    explicit EditorBlock() = default;
    EditorBlock(EditorBlockType type) : m_type(type) {}
};

class EditorScene : public Scene {
public:
    static constexpr SceneType TYPE = SceneType::Editor;

    EditorScene();

    void update(float dt) override;
    void render() override;

private:
    Camera m_cam;
    int m_brushIndex = 0;
    std::unordered_map<Vector2i, EditorBlock> m_blocks;
};

} // namespace aren