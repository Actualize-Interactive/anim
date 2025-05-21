#ifndef ANIM_TANGENT_MODE_HPP
#define ANIM_TANGENT_MODE_HPP

namespace anim {

enum class TangentMode {
    flat,
    linear,
    stepped,
    smoothAuto,
    smoothManual,
    broken
};

} // namespace anim

#endif // ANIM_TANGENT_MODE_HPP
