#include "render_texture.hpp"
#include "rlgl.h"

namespace aren {

// NOTE(aren): Copied from LoadRenderTexture impl
RenderTexture2D LoadRenderTextureHDRSupport(int width, int height, bool hdr)
{
    RenderTexture2D target = { 0 };

    target.id = rlLoadFramebuffer(); // Load an empty framebuffer

    if (target.id > 0)
    {
        rlEnableFramebuffer(target.id);

        // Create color texture (default to RGBA)
        target.texture.id = rlLoadTexture(NULL, width, height, hdr ? PIXELFORMAT_UNCOMPRESSED_R32G32B32 : PIXELFORMAT_UNCOMPRESSED_R8G8B8A8, 1);
        target.texture.width = width;
        target.texture.height = height;
        target.texture.format = hdr ? PIXELFORMAT_UNCOMPRESSED_R32G32B32 : PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
        target.texture.mipmaps = 1;

        // Create depth renderbuffer/texture
        target.depth.id = rlLoadTextureDepth(width, height, true);
        target.depth.width = width;
        target.depth.height = height;
        target.depth.format = 19;       //DEPTH_COMPONENT_24BIT?
        target.depth.mipmaps = 1;

        // Attach color texture and depth renderbuffer/texture to FBO
        rlFramebufferAttach(target.id, target.texture.id, RL_ATTACHMENT_COLOR_CHANNEL0, RL_ATTACHMENT_TEXTURE2D, 0);
        rlFramebufferAttach(target.id, target.depth.id, RL_ATTACHMENT_DEPTH, RL_ATTACHMENT_RENDERBUFFER, 0);

        // Check if fbo is complete with attachments (valid)
        if (rlFramebufferComplete(target.id)) TRACELOG(LOG_INFO, "FBO: [ID %i] Framebuffer object created successfully", target.id);

        rlDisableFramebuffer();
    }
    else TRACELOG(LOG_WARNING, "FBO: Framebuffer object can not be created");

    return target;
}

RenderTexture::RenderTexture(int width, int height, bool hdr) {
    m_rt = LoadRenderTextureHDRSupport(width, height, hdr);
}

RenderTexture::~RenderTexture() {
    UnloadRenderTexture(m_rt);
}

RenderTexture::RenderTexture(RenderTexture &&other) noexcept : m_rt(other.m_rt) {
    other.m_rt = {};
}

RenderTexture &RenderTexture::operator=(RenderTexture &&other) noexcept {
    if (this == &other) return *this;

    UnloadRenderTexture(m_rt);
    m_rt = other.m_rt;
    other.m_rt = {};
    return *this;
}

} // namespace aren