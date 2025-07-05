#ifndef ANIM_FUNCTION_TYPE_HPP
#define ANIM_FUNCTION_TYPE_HPP

#include <cstdint>

namespace anim {

enum class Function : uint8_t {
    Constant = 0,
    Linear = 1,
    Bezier = 2,
    Count = 3
};

} // namespace anim

#endif // ANIM_FUNCTION_TYPE_HPP