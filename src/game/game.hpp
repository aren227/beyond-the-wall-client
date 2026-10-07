#pragma once

#include "scene.hpp"

namespace aren {

class Game {
public:
    Game();

    static Game &instance();

    void init();
    void update(float dt);
    void render();
    void renderUI();

    bool shouldExit() const { return m_shouldExit; }

    static void clearScene();
    static size_t sceneCount();
    static bool isTopScene(const Scene *s);
    static void requestExit();

    template <typename T, typename... Args>
    static void addScene(Args &&...args) {
        auto &inst = instance();
        for (const auto &it : inst.m_scenes) {
            if (it->type() == T::TYPE) {
                ASSERTF(false, "scene type %d already active", (int)T::TYPE);
                return;
            }
        }
        inst.m_scenes.push_back(std::make_shared<T>(std::forward<Args>(args)...));
    }

    template <typename T>
    static void removeScene() {
        auto &inst = instance();
        inst.removeScene(T::TYPE);
    }

    template <typename T>
    static T *getScene() {
        auto &inst = instance();
        return static_cast<T *>(inst.getScene(T::TYPE));
    }

private:
    static Scene *getScene(SceneType type);
    static void removeScene(SceneType type);

    std::vector<std::shared_ptr<Scene>> m_scenes;
    bool m_shouldExit = false;
    float m_lastDt = 0.0f;
};

} // namespace aren
