#pragma once

namespace aren {

struct ParticleDef {
    Color4b color = WHITE;
    float speed = 4.0f;
    float damping = 0.5f;
    float life = 0.25f;
    float size = 0.25f;
    Vector2f normal = V2f(1, 0);
    float angle = 360.0f;
};

class Particles {
public:
    Particles();

    void emit(Vector2f pos, int count, const ParticleDef &def);
    void clear();

    void update(float dt);
    void render() const;

private:
    struct Particle {
        Vector2f pos;
        Vector2f velocity;
        float damping;
        float life;
        float maxLife;
        float size;
        Color4b color;
    };

    std::vector<Particle> m_particles;
};

} // namespace aren
