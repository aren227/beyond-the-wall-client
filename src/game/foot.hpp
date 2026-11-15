#pragma once

namespace aren {

struct Foot {
    Vector2f relative;
    Vector2f bodyJoint;
    Vector2f prevWorld;
    Vector2f world;
    bool isMoving = false;
    float movingDuration = 0.2f;
    float t = 0;
    std::array<int, 4> stopWhile;
    float maxDist = 0.05f;

    template <size_t SIZE>
    inline static void update(float dt, float movingSpeed, std::array<Foot, SIZE> &foots, const Vector2f &pos) {
        for (auto &it : foots) {
            auto target = pos + it.relative;
            auto delta = target - it.world;
            if (it.isMoving) {
                it.t += dt;
                float t = std::clamp(it.t / (it.movingDuration), 0.0f, 1.0f);
                it.world = Vector2f::lerp(it.prevWorld, target, t);
                if (t >= 1.0f) {
                    it.isMoving = false;
                    it.world = target;
                    it.t = 0;
                }
            } else {
                if (delta.lengthSqr() >= std::pow(it.maxDist, 2)) {
                    bool canMove = true;
                    for (auto jt : it.stopWhile) {
                        if (jt == -1) continue;
                        if (foots[jt].isMoving && foots[jt].t < foots[jt].movingDuration * 0.33f) {
                            canMove = false;
                            break;
                        }
                    }

                    if (canMove) {
                        it.isMoving = true;
                        it.prevWorld = it.world;
                    }
                }
            }
        }
    }
};

} // namespace aren