#pragma once

namespace aren {

enum class TileType {
    Empty = 0,
    Wall,
    Barrier,
    Glass,
    Sand,
    __MAX__
};

class Tile {
public:
    TileType m_type = TileType::Empty;

    explicit Tile() = default;
    Tile(TileType type) : m_type(type) {}
};

} // namespace aren