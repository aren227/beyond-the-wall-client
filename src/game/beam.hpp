#pragma once

namespace aren {

// flags
enum class BeamColor : int {
    Black = 0,
    Red = 1,
    Green = 2,
    Yellow = 3,
    Blue = 4,
    Magenta = 5,
    Cyan = 6,
    White = 7,
};

static constexpr Color4f getColorForBeam(BeamColor color) {
    switch (color) {
        case BeamColor::Black:
            return Color4f(0, 0, 0, 1);
        case BeamColor::Red:
            return Color4b(242, 73, 73, 255).f();
        case BeamColor::Green:
            return Color4b(73, 242, 73, 255).f();
        case BeamColor::Yellow:
            return Color4f(1, 1, 0, 1);
        case BeamColor::Blue:
            return Color4b(73, 73, 242, 255).f();
        case BeamColor::Magenta:
            return Color4f(1, 0, 1, 1);
        case BeamColor::Cyan:
            return Color4f(0, 1, 1, 1);
        case BeamColor::White:
            return Color4f(1, 1, 1, 1);
        default:
            ASSERT(false);
            return Color4f(1, 1, 1, 1);
    }
}

static constexpr Color4f getPureColorForBeam(BeamColor color) {
    switch (color) {
        case BeamColor::Black:
            return Color4f(0, 0, 0, 1);
        case BeamColor::Red:
            return Color4f(1, 0, 0, 1);
        case BeamColor::Green:
            return Color4f(0, 1, 0, 1);
        case BeamColor::Yellow:
            return Color4f(1, 1, 0, 1);
        case BeamColor::Blue:
            return Color4f(0, 0, 1, 1);
        case BeamColor::Magenta:
            return Color4f(1, 0, 1, 1);
        case BeamColor::Cyan:
            return Color4f(0, 1, 1, 1);
        case BeamColor::White:
            return Color4f(1, 1, 1, 1);
        default:
            ASSERT(false);
            return Color4f(1, 1, 1, 1);
    }
}

struct BeamProp {
    BeamColor color = BeamColor::White;
    float strength = 1.0f;

    BeamProp merge(const BeamProp &o) const;
};

struct BeamSegment {
    Vector2f start;
    Vector2f end;
    Vector2f dir;
    BeamProp prop;
};

struct Beam {
    std::vector<BeamSegment> segments;
};

} // namespace aren