#pragma once

#include "tile.hpp"
#include "object_layer.hpp"
#include "monster_wave.hpp"

namespace aren {

class Camera;
class SpriteSet;
class Player;
class Monster;
class PathQuery;

class World {
public:
    World();
    virtual ~World();

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
    void renderDebugAsUI(const Camera &cam, const SpriteSet &spriteSet);

    Vector2f moveRect(const Rect &rect, const Vector2f &delta, ObjectLayer layer) const;

    TileType getTileType(const Vector2i &pos) const;
    const Tile &getTile(const Vector2i &pos) const;

    bool raycast(const Vector2f &start, const Vector2f &dir, float maxDist, ObjectLayer layer, float &t, Vector2i &hit, Vector2i &normal) const;

    void save(const std::string &name);
    static std::unique_ptr<World> load(const std::string &name);

private:
    Color4f getColor(const Vector2i &pos) const;
    bool checkWaveCondition(const MonsterWave &wave);
    void activateWave(const MonsterWave &wvae);

public:
    // XXX(aren): lazy
    Vector2i m_min = V2I_ZERO, m_max = V2I_ZERO;
    std::string m_name;
    std::string m_comment;

private:
    float m_time = 0;
    std::unique_ptr<Player> m_player;
    std::unordered_map<Vector2i, Tile> m_tiles;
    std::vector<std::shared_ptr<Monster>> m_monsters;
    std::vector<MonsterWave> m_monsterWaves;
    std::unique_ptr<PathQuery> m_pathQuery;
};

} // namespace aren