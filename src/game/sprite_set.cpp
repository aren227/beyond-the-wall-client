#include "sprite_set.hpp"

namespace aren {

void SpriteSet::load() {
    if (Sprite::load("brick", tile(TileType::Wall)) == false) {
        ASSERT(false);
    }
    if (Sprite::load("glass", tile(TileType::Glass)) == false) {
        ASSERT(false);
    }
    if (Sprite::load("barrier", tile(TileType::Barrier)) == false) {
        ASSERT(false);
    }
    if (Sprite::load("sand", tile(TileType::Sand)) == false) {
        ASSERT(false);
    }
    if (Sprite::load("end", tile(TileType::End)) == false) {
        ASSERT(false);
    }
    if (Sprite::load("world_portal", tile(TileType::WorldPortal)) == false) {
        ASSERT(false);
    }

    if (Sprite::load("deco01", deco) == false) {
        ASSERT(false);
    }
    if (Sprite::load("star", star) == false) {
        ASSERT(false);
    }
    if (Sprite::load("bg_brick", bgBrick) == false) {
        ASSERT(false);
    }
    if (Sprite::load("bg_plant", bgPlant) == false) {
        ASSERT(false);
    }
    if (Sprite::load("world_portal_active", worldPortalActive) == false) {
        ASSERT(false);
    }
    if (Sprite::load("world_portal_inactive", worldPortalInactive) == false) {
        ASSERT(false);
    }

    constexpr std::array<std::array<int, 4>, 4> DITHER_MATRIX = {{
        {0, 8, 2, 10},
        {12, 4, 14, 6},
        {3, 11, 1, 9},
        {15, 7, 13, 5},
    }};

    // XXX(aren): Bake all dithering levels into textures (to avoid using shaders)
    for (int i = 0; i <= 16; i++) {
        Image image = GenImageColor(4, 4, {0, 0, 0, 0});
        Image imageOpaque = GenImageColor(4, 4, {0, 0, 0, 255});
        Image imageBlack = GenImageColor(4, 4, {0, 0, 0, 0});
        for (int y = 0; y < 4; y++) {
            for (int x = 0; x < 4; x++) {
                if (DITHER_MATRIX[y][x] >= 16-i) {
                    ImageDrawPixel(&image, x, y, WHITE);
                    ImageDrawPixel(&imageOpaque, x, y, WHITE);
                    ImageDrawPixel(&imageBlack, x, y, BLACK);
                }
            }
        }

        {
            dither.m_name = "Dither";

            Layer layer;
            layer.m_texture = Texture(image);
            layer.m_tag = i;

            dither.m_layers.push_back(std::move(layer));
        }
        {
            ditherOpaque.m_name = "Dither Opaque";

            Layer layer;
            layer.m_texture = Texture(imageOpaque);
            layer.m_tag = i;

            ditherOpaque.m_layers.push_back(std::move(layer));
        }
        {
            ditherBlack.m_name = "Dither Black";

            Layer layer;
            layer.m_texture = Texture(imageBlack);
            layer.m_tag = i;

            ditherBlack.m_layers.push_back(std::move(layer));
        }
    }
}

} // namespace aren