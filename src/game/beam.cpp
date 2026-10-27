#include "beam.hpp"

namespace aren {

BeamProp BeamProp::merge(const BeamProp &o) const {
    return {
        .color = BeamColor(int(color) & int(o.color)),
        .strength = strength * o.strength,
    };
}

} // namespace aren