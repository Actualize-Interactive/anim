#ifndef ANIM_EXTEND_HPP
#define ANIM_EXTEND_HPP

#include <cstdint>

namespace anim {

enum class Extend : uint8_t {
    Hold = 0,
    Repeat = 1,
    Mirror = 2,
};

} // namespace anim

#endif // ANIM_EXTEND_HPP