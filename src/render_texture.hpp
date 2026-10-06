#pragma once

namespace aren {

class RenderTexture {
public:
    RenderTexture() = default;
    RenderTexture(int width, int height, bool hdr = false);
    ~RenderTexture();

    RenderTexture(const RenderTexture &) = delete;
    RenderTexture &operator=(const RenderTexture &) = delete;

    RenderTexture(RenderTexture &&) noexcept;
    RenderTexture &operator=(RenderTexture &&) noexcept;

    explicit operator const ::RenderTexture &() const { return m_rt; }

    int width() const { return m_rt.texture.width; }
    int height() const { return m_rt.texture.height; }

    ::Texture color() const { return m_rt.texture; }
    ::Texture depth() const { return m_rt.depth; }

private:
    ::RenderTexture m_rt = {};
};

} // namespace aren