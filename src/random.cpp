#include "random.hpp"
#include <random>

namespace aren {

static std::mt19937& engine() {
    static std::mt19937 e{std::random_device{}()};
    return e;
}

void Random::seed(uint64_t s) {
    engine().seed(s);
}

float Random::value() {
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    return dist(engine());
}

float Random::rangeFloat(float min, float max) {
    std::uniform_real_distribution<float> dist(min, max);
    return dist(engine());
}

int Random::rangeInt(int min, int max) {
    std::uniform_int_distribution<int> dist(min, max - 1);
    return dist(engine());
}

bool Random::chance(float p) {
    return value() < p;
}

int Random::sign() {
    return chance(0.5f) ? 1 : -1;
}

float Random::angle() {
    return value() * TWO_PI;
}

Vector2f Random::insideUnitCircle() {
    float r = std::sqrt(value());
    float a = angle();
    return {r * std::cos(a), r * std::sin(a)};
}

Vector2f Random::onUnitCircle() {
    float a = angle();
    return {std::cos(a), std::sin(a)};
}

} // namespace aren