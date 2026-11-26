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
class Particles;
class Bullet;
class Noise;

class World {
public:
    World();
    explicit World(const World *world);
    virtual ~World();

    void update(Camera &cam, float dt);
    void updateBounds();

    std::unordered_map<Vector2i, Tile> &tiles() { return m_tiles; }
    const std::unordered_map<Vector2i, Tile> &tiles() const { return m_tiles; }

    bool contains(const Vector2i &pos) const { return m_tiles.contains(pos); }
    void erase(const Vector2i &pos) { m_tiles.erase(pos); }
    Tile &at(const Vector2i &pos) { return m_tiles[pos]; }

    void renderTiles(const Camera &cam, const SpriteSet &spriteSet, int category);
    void renderTilesMisc(const Camera &cam, const SpriteSet &spriteSet);
    void renderDecos(const Camera &cam, const SpriteSet &spriteSet);
    void renderMisc(const Camera &cam, const SpriteSet &spriteSet);
#if DEBUG
    void renderDebugAsUI(const Camera &cam, const SpriteSet &spriteSet);
#endif

    Vector2f moveRect(const Rect &rect, const Vector2f &delta, ObjectLayer layer) const;

    TileType getTileType(const Vector2i &pos) const;
    const Tile &getTile(const Vector2i &pos) const;

    bool raycast(const Vector2f &start, const Vector2f &dir, float maxDist, ObjectLayer layer, float &t, Vector2i &hit, Vector2i &normal) const;
    bool raycast(const Vector2f &start, const Vector2f &dir, float maxDist, ObjectLayer layer);
    bool raycastToEnd(const Vector2f &start, const Vector2f &end, ObjectLayer layer);

    void save(const std::string &name);
    static std::unique_ptr<World> load(const std::string &name);

    void addBullet(std::unique_ptr<Bullet> bullet);

    inline bool hasEnded() const { return m_endFlag; }
    inline bool hasPlayerWon() const { return m_playerWin; }
    bool checkWorldCanBeDone() const;

    inline Player *player() const { return m_player.get(); }
    inline Particles *particles() const { return m_particles.get(); }
    inline const std::vector<std::shared_ptr<Monster>> &monsters() const { return m_monsters; }

private:
    void resetPlayerPos();
    Color4f getColor(const Vector2i &pos) const;
    bool checkWaveCondition(const MonsterWave &wave);
    void activateWave(const MonsterWave &wvae);
    void generateMonstersForSC();

public:
    // XXX(aren): lazy
    Vector2i m_min = V2I_ZERO, m_max = V2I_ZERO;
    std::string m_filename;
    std::string m_name;
    std::string m_comment;
    std::vector<MonsterWave> m_monsterWaves;
    std::function<void(std::string)> m_onWorldPortalClick;
    bool m_isWorldMap = false;
    bool m_autoRetryMode = false;

    int m_scScore = -1;
    int m_scBestScore = 0;

private:
    float m_time = 0;
    bool m_endFlag = false;
    bool m_playerWin = false;
    float m_winDelayTimer = 0; // NOTE(aren): Instant transition is bad
    std::unique_ptr<Player> m_player;
    std::unordered_map<Vector2i, Tile> m_tiles;
    std::vector<std::shared_ptr<Monster>> m_monsters;
    std::unique_ptr<PathQuery> m_pathQuery;
    std::unique_ptr<Particles> m_particles;
    std::vector<std::unique_ptr<Bullet>> m_bullets;
    std::unique_ptr<Noise> m_noise;

    struct MonsterSpawnAnimation {
        Vector2f pos;
        std::string name;

        float timeLeft = 3;
        Color4f color = C4F_WHITE;
    };
    std::vector<MonsterSpawnAnimation> m_monsterSpawnAnims;
};

} // namespace aren