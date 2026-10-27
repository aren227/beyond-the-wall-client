#pragma once

#include "beam.hpp"

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
    BeamColor m_color = BeamColor::White;

public:
    explicit Tile() = default;
    Tile(TileType type) : m_type(type) {}

    BeamProp beamProp() const;

    BeamColor beamColor() const;
    bool isTransparent() const;
};

} // namespace aren