#include "texture.hpp"

namespace aren {

Texture::Texture() { }

Texture::Texture(const char *path, int filter) : Texture(LoadImage(path), filter) {}

Texture::Texture(::Image image, int filter) {
    m_texture = LoadTextureFromImage(image);
    SetTextureFilter(m_texture, filter);
    SetTextureWrap(m_texture, TEXTURE_WRAP_REPEAT);
    UnloadImage(image);
}

Texture::~Texture() {
    UnloadTexture(m_texture);
}

Texture::Texture(Texture &&other) noexcept : m_texture(other.m_texture) {
    other.m_texture = {};
}

Texture &Texture::operator=(Texture &&other) noexcept {
    if (this == &other) return *this;

    UnloadTexture(m_texture);
    m_texture = other.m_texture;
    other.m_texture = {};
    return *this;
}

const Texture &Texture::white() {
    static Texture *s_white;
    if (s_white == nullptr) {
        Image image = GenImageColor(16, 16, WHITE);
        s_white = new Texture(image);
    }
    return *s_white;
}

} // namespace aren
