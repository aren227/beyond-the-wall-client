#pragma once

namespace aren {

static constexpr int PIXEL_SCALER = 2;
static constexpr float BEAM_THICKNESS = 0.15f;

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

} // namespace aren