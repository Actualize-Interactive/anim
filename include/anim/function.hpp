#ifndef ANIM_FUNCTION_TYPE_HPP
#define ANIM_FUNCTION_TYPE_HPP

#include <cstdint>

namespace anim {

enum class Function : uint8_t {
    constant = 0,
    linear = 1,
    bezier = 2,
    count = 3
};

} // namespace anim

#endif // ANIM_FUNCTION_TYPE_HPP