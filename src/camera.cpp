#include "camera.hpp"
#include "random.hpp"
#include "settings.hpp"

namespace aren {

void Camera::update(float dt) {
    // shake
    if (m_shakeRemaining > 0) {
        m_shakeRemaining = std::max(0.0f, m_shakeRemaining - dt);
        float intensity = m_shakeRemaining / std::max(m_shakeDuration, 1e-3f);
        float angle = Random::rangeFloat(0.0f, 2.0f * PI);
        m_shakeOffset = Vector2f(std::cos(angle), std::sin(angle)) * (m_shakeAmplitude * intensity);
    } else {
        m_shakeOffset = V2F_ZERO;
    }
}

void Camera::shake(float amplitude, float duration) {
    if (!Settings::getCameraShake()) return;
    m_shakeAmplitude = amplitude;
    m_shakeDuration = duration;
    m_shakeRemaining = duration;
}

Vector2f Camera::screenToWorld(Vector2f screen) const {
    return GetScreenToWorld2D(screen, static_cast<Camera2D>(*this));
}

Vector2f Camera::worldToScreen(Vector2f world) const {
    return GetWorldToScreen2D(world, static_cast<Camera2D>(*this));
}

void Camera::moveToInside(const Rect &bounds) {
    float screenW = float(GetScreenWidth());
    float screenH = float(GetScreenHeight());

    const auto clampAxis = [](float t, float lo, float hi) -> float {
        return lo > hi ? (lo + hi) * 0.5f : std::clamp(t, lo, hi);
    };

    m_target.x = clampAxis(m_target.x, bounds.x, bounds.x + bounds.w - screenW / m_zoom);
    m_target.y = clampAxis(m_target.y, bounds.y, bounds.y + bounds.h - screenH / m_zoom);
}

void Camera::zoomToFit(const Rect &bounds) {
    m_target.x = bounds.x + bounds.w * 0.5f;
    m_target.y = bounds.y + bounds.h * 0.5f;
    m_zoom = std::min(GetScreenWidth() / bounds.w, GetScreenHeight() / bounds.h);
}

Camera::operator ::Camera2D() const {
    return {
        .offset = m_shakeOffset,
        .target = m_target,
        .rotation = m_rotation,
        .zoom = m_zoom,
    };
}

} // namespace aren
