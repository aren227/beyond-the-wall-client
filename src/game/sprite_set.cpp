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

    if (Sprite::load("deco01", deco) == false) {
        ASSERT(false);
    }
}

} // namespace aren