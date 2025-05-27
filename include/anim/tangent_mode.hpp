#ifndef ANIM_TANGENT_MODE_HPP
#define ANIM_TANGENT_MODE_HPP

namespace anim {

enum class TangentMode {
    flat = 0,
    linear = 1,
    constant = 2,
    smooth = 3,
    manual = 4,
    broken = 5,
    count = 6
};

} // namespace anim

#endif // ANIM_TANGENT_MODE_HPP
