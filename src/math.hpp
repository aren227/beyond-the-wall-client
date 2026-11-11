#pragma once

// raylib.h defines these as macros
#ifdef PI
    #undef PI
#endif
#ifdef DEG2RAD
    #undef DEG2RAD
#endif
#ifdef RAD2DEG
    #undef RAD2DEG
#endif

namespace aren {

inline constexpr float PI      = 3.14159265358979323846f;
inline constexpr float TWO_PI  = PI * 2.0f;
inline constexpr float DEG2RAD = PI / 180.0f;
inline constexpr float RAD2DEG = 180.0f / PI;

struct Vector2i;

struct Vector2f {
    float x = 0.0f;
    float y = 0.0f;

    inline constexpr Vector2f() = default;
    inline constexpr Vector2f(float x_, float y_) : x(x_), y(y_) {}
    inline constexpr Vector2f(::Vector2 v) : x(v.x), y(v.y) {}

    inline constexpr operator ::Vector2() const { return {x, y}; }

    inline constexpr Vector2f operator-() const { return {-x, -y}; }
    inline constexpr Vector2f operator+(Vector2f r) const { return {x + r.x, y + r.y}; }
    inline constexpr Vector2f operator-(Vector2f r) const { return {x - r.x, y - r.y}; }
    inline constexpr Vector2f operator*(float s) const { return {x * s, y * s}; }
    inline constexpr Vector2f operator/(float s) const { return {x / s, y / s}; }

    friend inline constexpr Vector2f operator*(float s, Vector2f v) { return v * s; }

    inline Vector2f& operator+=(Vector2f r) { x += r.x; y += r.y; return *this; }
    inline Vector2f& operator-=(Vector2f r) { x -= r.x; y -= r.y; return *this; }
    inline Vector2f& operator*=(float s)    { x *= s;   y *= s;   return *this; }
    inline Vector2f& operator/=(float s)    { x /= s;   y /= s;   return *this; }

    inline constexpr bool operator==(Vector2f r) const { return x == r.x && y == r.y; }
    inline constexpr bool operator!=(Vector2f r) const { return !(*this == r); }

    inline constexpr float dot(Vector2f r)   const { return x * r.x + y * r.y; }
    inline constexpr float cross(Vector2f r) const { return x * r.y - y * r.x; }
    inline constexpr float lengthSqr()       const { return x * x + y * y; }
    inline float           length()          const { return std::sqrt(lengthSqr()); }
    inline constexpr Vector2f scale(Vector2f r) const { return { x * r.x, y * r.y }; }
    inline constexpr Vector2f min(Vector2f r) const { return { std::min(x, r.x), std::min(y, r.y) }; }
    inline constexpr Vector2f max(Vector2f r) const { return { std::max(x, r.x), std::max(y, r.y) }; }

    inline Vector2f normalized() const {
        float len = length();
        return len > 1e-7f ? Vector2f(x / len, y / len) : Vector2f(1, 0);
    }

    inline Vector2f rotated(float a) const {
        float c = std::cos(a);
        float s = std::sin(a);
        return Vector2f(x * c - y * s, x * s + y * c);
    }

    inline constexpr Vector2f perp() const { return Vector2f(-y, x); }

    inline constexpr Vector2f reflected(Vector2f normal) const {
        return *this - normal * (2.0f * dot(normal));
    }

    static inline constexpr Vector2f lerp(Vector2f a, Vector2f b, float t) {
        return (b-a)*t + a;
    }

    static inline Vector2f moveTorwards(const Vector2f &from, const Vector2f &to, float dist) {
        auto dir = to - from;

        if (dir.lengthSqr() < 1e-6) {
            return to;
        }

        float d = dir.length();

        if (dist >= d) {
            return to;
        }

        return from + dir.normalized() * dist;
    }

    static inline float segmentDist(const Vector2f &from, const Vector2f &to, const Vector2f &point) {
        auto fromToPoint = point - from;
        auto delta = to - from;
        float d = fromToPoint.dot(delta) / delta.dot(delta);
        if (d <= 0) return fromToPoint.length();
        if (d >= 1) return (point - to).length();
        auto perp = fromToPoint - (d * delta);
        return perp.length();
    }

    static inline float lineCircle(const Vector2f &from, const Vector2f &dir, const Vector2f &center, float radius) {
        float d = (center - from).dot(dir);
        auto proj = dir * d;
        auto perp = (center - from) - proj;

        if (perp.lengthSqr() > radius * radius) return -1;
        float b = std::sqrt(radius * radius - perp.lengthSqr());

        return (d - b < 0) ? (d + b) : (d - b);
    }

    inline constexpr Vector2i round() const;
    inline constexpr Vector2i floor() const;
    inline constexpr Vector2i ceil() const;
};

static inline constexpr Vector2f V2f(float x, float y) { return Vector2f(x, y); }

static constexpr Vector2f V2F_ZERO = V2f(0, 0);
static constexpr Vector2f V2F_ONE  = V2f(1, 1);

struct Vector2i {
    int x = 0;
    int y = 0;

    inline constexpr Vector2i() = default;
    inline constexpr Vector2i(int x_, int y_) : x(x_), y(y_) {}

    inline constexpr Vector2i operator-() const { return {-x, -y}; }
    inline constexpr Vector2i operator+(Vector2i r) const { return {x + r.x, y + r.y}; }
    inline constexpr Vector2i operator-(Vector2i r) const { return {x - r.x, y - r.y}; }
    inline constexpr Vector2i operator*(int s) const { return {x * s, y * s}; }

    friend inline constexpr Vector2i operator*(int s, Vector2i v) { return v * s; }

    inline Vector2i& operator+=(Vector2i r) { x += r.x; y += r.y; return *this; }
    inline Vector2i& operator-=(Vector2i r) { x -= r.x; y -= r.y; return *this; }
    inline Vector2i& operator*=(int s)    { x *= s;   y *= s;   return *this; }

    inline constexpr bool operator==(Vector2i r) const { return x == r.x && y == r.y; }
    inline constexpr bool operator!=(Vector2i r) const { return !(*this == r); }

    // NOTE(aren): for std::map
    inline constexpr bool operator<(Vector2i r) const { return (x == r.x) ? (y < r.y) : (x < r.x); }

    constexpr inline Vector2f f() const { return V2f(x, y); }
    inline constexpr Vector2i min(Vector2i r) const { return { std::min(x, r.x), std::min(y, r.y) }; }
    inline constexpr Vector2i max(Vector2i r) const { return { std::max(x, r.x), std::max(y, r.y) }; }
};

static inline constexpr Vector2i V2i(int x, int y) { return Vector2i(x, y); }

static constexpr Vector2i V2I_ZERO = V2i(0, 0);
static constexpr Vector2i V2I_ONE  = V2i(1, 1);

inline constexpr Vector2i Vector2f::round() const {
    return V2i(std::round(x), std::round(y));
}

inline constexpr Vector2i Vector2f::floor() const {
    return V2i(std::floor(x), std::floor(y));
}

inline constexpr Vector2i Vector2f::ceil() const {
    return V2i(std::ceil(x), std::ceil(y));
}

struct Rect {
    float x = 0, y = 0, w = 0, h = 0;

    inline constexpr Rect() = default;
    inline constexpr Rect(float x_, float y_, float w_, float h_) : x(x_), y(y_), w(w_), h(h_) {}
    inline constexpr Rect(Vector2f origin, Vector2f size) : x(origin.x), y(origin.y), w(size.x), h(size.y) {}
    inline constexpr Rect(::Rectangle v) : x(v.x), y(v.y), w(v.width), h(v.height) {}

    inline constexpr operator ::Rectangle() const { return {x, y, w, h}; }

    inline constexpr bool operator==(Rect r) const { return x == r.x && y == r.y && w == r.w && h == r.h; }
    inline constexpr bool operator!=(Rect r) const { return !(*this == r); }

    inline constexpr bool contains(Vector2f v) { return v.x >= x && v.x <= x + w && v.y >= y && v.y <= y + h; }
    inline constexpr Rect extends(float v) { return {x - v, y - v, w + v * 2, h + v * 2}; }
};

struct Color4f;

struct Color4b {
    uint8_t r = 0, g = 0, b = 0, a = 0;

    inline constexpr Color4b() = default;
    inline constexpr Color4b(uint8_t r_, uint8_t g_, uint8_t b_, uint8_t a_) : r(r_), g(g_), b(b_), a(a_) {}
    inline constexpr Color4b(::Color v) : r(v.r), g(v.g), b(v.b), a(v.a) {}

    inline constexpr operator ::Color() const { return {r, g, b, a}; }

    inline constexpr bool operator==(Color4b v) const { return r == v.r && g == v.g && b == v.b && a == v.a; }
    inline constexpr bool operator!=(Color4b v) const { return !(*this == v); }

    inline constexpr uint32_t rgba32() const { return a | (uint32_t(b) << 8) | (uint32_t(g) << 16) | (uint32_t(r) << 24); }

    inline constexpr Color4f f() const;
};

static constexpr Color4b C4B_WHITE = Color4b(255, 255, 255, 255);
static constexpr Color4b C4B_BLACK = Color4b(0, 0, 0, 255);

struct Color4f {
    float r = 0, g = 0, b = 0, a = 0;

    inline constexpr Color4f() = default;
    inline constexpr Color4f(float r_, float g_, float b_, float a_) : r(r_), g(g_), b(b_), a(a_) {}
    inline constexpr Color4f(::Color v) : r(v.r / 255.0f), g(v.g / 255.0f), b(v.b / 255.0f), a(v.a / 255.0f) {}

    inline constexpr operator ::Color() const { return {uint8_t(r * 255), uint8_t(g * 255), uint8_t(b * 255), uint8_t(a * 255)}; }

    inline constexpr Color4f operator+(Color4f v) const { return {r + v.r, g + v.g, b + v.b, a + v.a}; }
    inline constexpr Color4f operator*(Color4f v) const { return {r * v.r, g * v.g, b * v.b, a * v.a}; }
    inline constexpr Color4f operator*(float s) const { return {r * s, g * s, b * s, a * s}; }
    inline constexpr Color4f operator/(float s) const { return {r / s, g / s, b / s, a / s}; }

    inline constexpr bool operator==(Color4f v) const { return r == v.r && g == v.g && b == v.b && a == v.a; }
    inline constexpr bool operator!=(Color4f v) const { return !(*this == v); }

    inline constexpr Color4f brighten(float intensity) const { return { r * intensity, g * intensity, b * intensity, a}; }

    inline constexpr Color4b byte() const;
};

static constexpr Color4f C4F_WHITE = Color4f(1, 1, 1, 1);
static constexpr Color4f C4F_BLACK = Color4f(0, 0, 0, 1);

inline constexpr Color4f Color4b::f() const { return Color4f(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f); }

inline constexpr Color4b Color4f::byte() const {
    return { uint8_t(r * 255), uint8_t(g * 255), uint8_t(b * 255), uint8_t(a * 255) };
}

} // namespace aren

// hashing
template <class T>
inline void hash_combine(std::size_t& seed, T const& v) {
    seed ^= std::hash<T>()(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
}

template <>
struct std::hash<aren::Vector2i> {
    std::size_t operator()(const aren::Vector2i &r) const noexcept {
        size_t seed = 0;
        hash_combine(seed, r.x);
        hash_combine(seed, r.y);
        return seed;
    }
};
