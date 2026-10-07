#pragma once

namespace aren {

enum class SceneType {
    Title,
    Settings,
    World,
    Editor,
};

class Scene {
public:
    virtual ~Scene() = default;
    virtual void update(float dt) = 0;
    virtual void render() = 0;
    virtual void renderUI() = 0;

    SceneType type() const { return m_type; }

protected:
    explicit Scene(SceneType t) : m_type(t) {}

private:
    SceneType m_type;
};

} // namespace aren
