#include "ingame_scene.hpp"

namespace aren {

IngameScene::IngameScene() : Scene(TYPE) {}

void IngameScene::update(float dt) {
    (void)dt;
}

void IngameScene::render() {
    ClearBackground(BLACK);
}

void IngameScene::renderUI() {
}

} // namespace aren
