#pragma once

namespace aren {

class Texture {
public:
    Texture();
    explicit Texture(const char *path, int filter = TEXTURE_FILTER_POINT);
    explicit Texture(::Image image, int filter = TEXTURE_FILTER_POINT);
    ~Texture();

    static const Texture &white();

    Texture(const Texture &) = delete;
    Texture &operator=(const Texture &) = delete;

    Texture(Texture &&other) noexcept;
    Texture &operator=(Texture &&other) noexcept;

    explicit operator const ::Texture2D &() const { return m_texture; }

    int width() const { return m_texture.width; }
    int height() const { return m_texture.height; }
    Vector2f size() const { return V2f(width(), height()); }

private:
    ::Texture2D m_texture = {};
};

} // namespace aren
