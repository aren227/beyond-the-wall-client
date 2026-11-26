#pragma once

#include "../math.hpp"

namespace aren {

static constexpr int PIXEL_SCALER = 2;
static constexpr float BEAM_THICKNESS = 0.2f;

#if DEBUG
static const auto _evalBaseResourcePath = []() {
    auto ptr = std::getenv("RESOURCES_BASE");
    if (ptr) {
        return std::string(ptr);
    } else {
        return std::string("resources/");
    }
};
#else
static const auto _evalBaseResourcePath = []() {
    return std::string("resources/");
};
#endif

static const std::string IMAGE_BASE  = _evalBaseResourcePath() + "images";
static const std::string SPRITE_BASE = _evalBaseResourcePath() + "sprites";
static const std::string WORLD_BASE  = _evalBaseResourcePath() + "worlds";
static const std::string SOUND_BASE  = _evalBaseResourcePath() + "sounds";

static constexpr const char *COMPLETED_WORLD_KEYS = "game_completed_world_keys";
static constexpr const char *PERFECTLY_COMPLETED_WORLD_KEYS = "game_perfectly_completed_world_keys";

static constexpr Color4b BLACK_BG = Color4b(0, 0, 0, 255);

static constexpr Color4b GOLD2 = Color4b(255, 186, 59, 255);

static constexpr float REFLECT_MARGIN = 0.001f;

static const std::string SUPER_CONSTELLATION = "Super Constellation";
static constexpr const char *SUPER_CONSTELLATION_BEST = "scBestScore";

} // namespace aren