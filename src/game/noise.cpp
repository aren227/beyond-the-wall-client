#include "noise.hpp"
#include "consts.hpp"

namespace aren {

Noise::Noise() {
    m_image = LoadImage((IMAGE_BASE + "/perlin.png").c_str());
    ASSERT(m_image.width == m_image.height);
}

Noise::~Noise() {
    UnloadImage(m_image);
}

float Noise::sample(const Vector2f &pos, int levels) {
    const auto at = [&](float x, float y) {
        int x0 = std::floor(x);
        int y0 = std::floor(y);
        int x1 = x0 + 1;
        int y1 = y0 + 1;
        int w1 = m_image.width - 1;
        float v_x0y0 = GetImageColor(m_image, x0 & w1, y0 & w1).r / 255.0f;
        float v_x1y0 = GetImageColor(m_image, x1 & w1, y0 & w1).r / 255.0f;
        float v_x0y1 = GetImageColor(m_image, x0 & w1, y1 & w1).r / 255.0f;
        float v_x1y1 = GetImageColor(m_image, x1 & w1, y1 & w1).r / 255.0f;

        float xx = x - x0;
        float yy = y - y0;
        float v_y0 = std::lerp(v_x0y0, v_x1y0, xx);
        float v_y1 = std::lerp(v_x0y1, v_x1y1, xx);
        float v = std::lerp(v_y0, v_y1, yy);
        return v;
    };

    float sum = 0, max = 0;
    float weight = 1.0f;
    float scale = 1.0f;
    for (int i = 0; i < levels; i++) {
        sum += at(pos.x * scale, pos.y * scale) * weight;
        max += weight;

        weight *= 0.5f;
        scale *= 2.0f;
    }

    const auto smoothStep = [](float t) {
        return t * t * (3.0f - 2.0f * t);
    };

    return smoothStep(sum / max);
}

} // namespace aren