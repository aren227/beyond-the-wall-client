#include "particles.hpp"
#include "random.hpp"

namespace aren {

Particles::Particles() {
    m_particles.reserve(1024);
}

void Particles::emit(Vector2f pos, int count, const ParticleDef &def) {
    for (int i = 0; i < count; i++) {
        auto velocity = Random::onUnitCircle() * Random::rangeFloat(0.5f, 1.0f) * def.speed;
        m_particles.push_back({
            .pos = pos,
            .velocity = velocity,
            .damping = def.damping,
            .life = def.life,
            .maxLife = def.life,
            .size = def.size,
            .color = def.color,
        });
    }
}

void Particles::clear() {
    m_particles.clear();
}

void Particles::update(float dt) {
    for (auto &it : m_particles) {
        it.life -= dt;
        it.pos += it.velocity * dt;
        it.velocity = it.velocity * (1.0f / (1.0f + it.damping * dt));
    }

    m_particles.erase(std::remove_if(m_particles.begin(), m_particles.end(),
        [](const Particle &p) { return p.life <= 0.0f; }), m_particles.end());
}

void Particles::render() const {
    for (const auto &it : m_particles) {
        auto color = it.color;
        color.a = uint8_t(color.a * (it.life / it.maxLife));

        DrawRectangleV(it.pos - V2F_ONE * (it.size * 0.5f), V2F_ONE * it.size, color);
    }
}

} // namespace aren
