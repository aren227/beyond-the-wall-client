#include "particles.hpp"
#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

#include "audio.hpp"
#include "game/game.hpp"
#include "render.hpp"
#include "settings.hpp"
#include "storage.hpp"

namespace aren {

constexpr int DEFAULT_WINDOW_WIDTH  = 1280;
constexpr int DEFAULT_WINDOW_HEIGHT = 720;

static Game s_game;

void UpdateDrawFrame() {
    float dt = std::min(GetFrameTime(), 1 / 30.0f);
    Audio::update();
    s_game.update(dt);

    Render::beginDraw();

    s_game.render();

    Render::endDraw();
}

} // namespace aren

int main() {
    using namespace aren;

#if defined(PLATFORM_WEB)
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);
#else
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT | FLAG_WINDOW_HIGHDPI | FLAG_WINDOW_RESIZABLE);
#endif
    InitWindow(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, PROJECT_NAME);
    SetExitKey(KEY_NULL);
#if !defined(PLATFORM_WEB)
    SetTargetFPS(60);
#endif

#if !defined(PLATFORM_WEB)
    ChangeDirectory(GetApplicationDirectory());
#endif

    Render::init();
    Audio::init();
    Storage::init();
    Settings::init();
    s_game.init();

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
#else
    while (!WindowShouldClose() && !s_game.shouldExit()) {
        UpdateDrawFrame();
    }
#endif

    Storage::deinit();
    Audio::deinit();
    Render::deinit();
    CloseWindow();
    return 0;
}