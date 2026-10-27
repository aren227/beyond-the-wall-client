#pragma once

#include "tile.hpp"
#include "player.hpp"

namespace aren {

class Camera;
class SpriteSet;

class World {
public:
    World();

    void update(const Camera &cam, float dt);
    void updateBounds();

    std::unordered_map<Vector2i, Tile> &tiles() { return m_tiles; }
    const std::unordered_map<Vector2i, Tile> &tiles() const { return m_tiles; }

    bool contains(const Vector2i &pos) const { return m_tiles.contains(pos); }
    void erase(const Vector2i &pos) { m_tiles.erase(pos); }
    Tile &at(const Vector2i &pos) { return m_tiles[pos]; }

    void renderTiles(const Camera &cam, const SpriteSet &spriteSet, int category);
    void renderDecos(const Camera &cam, const SpriteSet &spriteSet);
    void renderMisc(const Camera &cam, const SpriteSet &spriteSet);

    Vector2f moveRect(const Rect &rect, const Vector2f &delta) const;

    TileType getTileType(const Vector2i &pos) const;
    const Tile &getTile(const Vector2i &pos) const;

private:
    Color4f getColor(const Vector2i &pos) const;

public:
    // XXX(aren): lazy
    Vector2i m_min = V2I_ZERO, m_max = V2I_ZERO;

private:
    float m_time = 0;
    Player m_player;
    std::unordered_map<Vector2i, Tile> m_tiles;
};

} // namespace aren