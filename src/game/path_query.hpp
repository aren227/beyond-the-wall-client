#pragma once

#include "object_layer.hpp"

namespace aren {

class World;

class PathQuery {
public:
    PathQuery() = default;
    virtual ~PathQuery() = default;

    void build(const World *world, const Vector2f &to, ObjectLayer layer);
    bool query(const World *world, const Vector2f &from, float radius, std::vector<Vector2f> &path) const;

private:
    Vector2f m_to;
    Vector2i m_toI;
    ObjectLayer m_layer;
    std::unordered_map<Vector2i, Vector2i> m_dirs;
};

} // namespace aren