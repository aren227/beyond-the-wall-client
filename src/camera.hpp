#pragma once

namespace aren {

class Camera {
public:
    Vector2f m_target = V2F_ZERO;
    float m_rotation = 0.0f;
    float m_zoom = 1.0f;

    void update(float dt);

    void shake(float amplitude, float duration);

    explicit operator ::Camera2D() const;

    Vector2f screenToWorld(Vector2f screen) const;
    Vector2f worldToScreen(Vector2f world) const;

    void moveToInside(const Rect &bounds);
    void zoomToFit(const Rect &bounds);

private:
    float m_shakeAmplitude = 0.0f;
    float m_shakeDuration = 0.0f;
    float m_shakeRemaining = 0.0f;
    Vector2f m_shakeOffset = V2F_ZERO;
};

} // namespace aren
