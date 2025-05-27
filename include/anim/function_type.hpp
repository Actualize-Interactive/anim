#ifndef ANIM_FUNCTION_TYPE_HPP
#define ANIM_FUNCTION_TYPE_HPP

namespace anim {

enum class FunctionType {
    constant = 0,
    linear = 1,
    bezier = 2,
    count = 3
};

} // namespace anim

#endif // ANIM_FUNCTION_TYPE_HPP