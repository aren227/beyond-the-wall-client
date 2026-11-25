#pragma once

#include "math.hpp"

namespace aren {

class Random {
public:
    static void seed(uint64_t s);

    static float value();

    static float rangeFloat(float min, float max);
    static int rangeInt(int min, int max);

    static bool chance(float p);
    static int sign();
    static float angle();

    static Vector2f insideUnitCircle();
    static Vector2f onUnitCircle();

    template <typename T>
    static const T& pick(const std::vector<T>& v) {
        if (v.empty()) {
            static T placeholder;
            return placeholder;
        }
        return v[rangeInt(0, static_cast<int>(v.size()))];
    }

    template <typename T>
    static void shuffle(std::vector<T> &v) {
        for (size_t i = 0; i < v.size(); i++) {
            int j = Random::rangeInt(i, static_cast<int>(v.size()));
            std::swap(v[i], v[j]);
        }
    }
};

} // namespace aren
