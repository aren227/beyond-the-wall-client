#pragma once

#include "sprite.hpp"
#include "tile.hpp"

namespace aren {

class SpriteSet {
public:
    SpriteSet() = default;

    void load();

    const Sprite &tile(TileType tile) const { return tiles[int(tile)]; }
    Sprite &tile(TileType tile) { return tiles[int(tile)]; }

public:
    std::array<Sprite, int(TileType::__MAX__)> tiles;
    Sprite deco;
    Sprite star;
    Sprite bgBrick;
    Sprite dither;
    Sprite ditherOpaque;
    Sprite ditherBlack;
    Sprite worldPortalActive;
    Sprite worldPortalInactive;
};

} // namespace aren