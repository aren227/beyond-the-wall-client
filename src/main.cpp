#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#endif

#include "audio.hpp"
#include "game/game.hpp"
#include "render.hpp"
#include "settings.hpp"
#include "storage.hpp"

#if DEBUG
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#endif

namespace aren {

constexpr int DEFAULT_WINDOW_WIDTH  = 1280;
constexpr int DEFAULT_WINDOW_HEIGHT = 720;

static Game s_game;

void UpdateDrawFrame() {
    float dt = std::min(GetFrameTime(), 1 / 30.0f);
    Audio::update();
    s_game.update(dt);

    Render::beginGameDraw();
    s_game.render();
    Render::endGameDraw();

    Render::beginUIDraw();
    s_game.renderUI();
    Render::endUIDraw();
}

} // namespace aren

#if defined(PLATFORM_WEB)
static EM_BOOL consumeSpace(int, const EmscriptenKeyboardEvent* e, void*) {
    return std::strcmp(e->code, "Space") == 0;
}
#endif

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

#if defined(PLATFORM_WEB)
    emscripten_set_keypress_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, EM_TRUE, consumeSpace);
#endif

    Render::init();
    Audio::init();
    Storage::init();
    Settings::init();
    s_game.init();

#if DEBUG
    // NOTE(aren): Set default ui styles
    GuiSetFont(Render::getFontBySize("editor", 16));
    GuiSetStyle(DEFAULT, TEXT_SIZE, 16);
#endif

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