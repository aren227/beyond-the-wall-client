#pragma once

#include "tile.hpp"
#include "player.hpp"

namespace aren {

class Camera;
class SpriteSet;

class World {
public:
    World();

    void update(float dt);

    void bounds(Vector2i &min, Vector2i &max);

    std::unordered_map<Vector2i, Tile> &tiles() { return m_tiles; }

    bool contains(const Vector2i &pos) { return m_tiles.contains(pos); }
    void erase(const Vector2i &pos) { m_tiles.erase(pos); }
    Tile &at(const Vector2i &pos) { return m_tiles[pos]; }

    void renderTiles(const Camera &cam, const SpriteSet &spriteSet, int category);
    void renderDecos(const Camera &cam, const SpriteSet &spriteSet);
    void renderMisc(const Camera &cam);

private:
    Player m_player;
    std::unordered_map<Vector2i, Tile> m_tiles;
};

} // namespace aren