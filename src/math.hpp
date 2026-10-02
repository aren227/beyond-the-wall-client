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
};

static inline constexpr Vector2f V2f(float x, float y) { return Vector2f(x, y); }

static constexpr Vector2f V2F_ZERO = V2f(0, 0);
static constexpr Vector2f V2F_ONE  = V2f(1, 1);

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

struct Color4b {
    uint8_t r = 0, g = 0, b = 0, a = 0;

    inline constexpr Color4b() = default;
    inline constexpr Color4b(uint8_t r_, uint8_t g_, uint8_t b_, uint8_t a_) : r(r_), g(g_), b(b_), a(a_) {}
    inline constexpr Color4b(::Color v) : r(v.r), g(v.g), b(v.b), a(v.a) {}

    inline constexpr operator ::Color() const { return {r, g, b, a}; }

    inline constexpr bool operator==(Color4b v) const { return r == v.r && g == v.g && b == v.b && a == v.a; }
    inline constexpr bool operator!=(Color4b v) const { return !(*this == v); }
};

struct Color4f {
    float r = 0, g = 0, b = 0, a = 0;

    inline constexpr Color4f() = default;
    inline constexpr Color4f(float r_, float g_, float b_, float a_) : r(r_), g(g_), b(b_), a(a_) {}
    inline constexpr Color4f(::Color v) : r(v.r / 255.0f), g(v.g / 255.0f), b(v.b / 255.0f), a(v.a / 255.0f) {}

    inline constexpr operator ::Color() const { return {uint8_t(r * 255), uint8_t(g * 255), uint8_t(b * 255), uint8_t(a * 255)}; }

    inline constexpr Color4f operator*(Color4f v) const { return {r * v.r, g * v.g, b * v.b, a * v.a}; }
    inline constexpr Color4f operator*(float s) const { return {r * s, g * s, b * s, a * s}; }
    inline constexpr Color4f operator/(float s) const { return {r / s, g / s, b / s, a / s}; }

    inline constexpr bool operator==(Color4b v) const { return r == v.r && g == v.g && b == v.b && a == v.a; }
    inline constexpr bool operator!=(Color4b v) const { return !(*this == v); }
};

} // namespace aren
