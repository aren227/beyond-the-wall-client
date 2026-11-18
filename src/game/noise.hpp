#pragma once

namespace aren {

class Noise {
public:
    Noise();
    virtual ~Noise();

    float sample(const Vector2f &pos, int levels);

private:
    Image m_image;
};

} // namespace aren