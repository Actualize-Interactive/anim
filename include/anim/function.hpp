#ifndef ANIM_FUNCTION_TYPE_HPP
#define ANIM_FUNCTION_TYPE_HPP

#include <cstdint>

namespace anim {

/**
 * @brief Interpolation function used between a keyframe and the next one.
 *
 * The function belongs to the keyframe at the start of a segment and
 * determines how values are interpolated up to the following keyframe.
 */
enum class Function : uint8_t {
    Constant = 0, ///< Hold the keyframe's value until the next keyframe (step).
    Linear   = 1, ///< Straight-line interpolation to the next keyframe.
    Bezier   = 2, ///< Cubic Bézier interpolation driven by the keyframe handles.
    Count    = 3  ///< Number of interpolation functions; not a selectable value.
};

} // namespace anim

#endif // ANIM_FUNCTION_TYPE_HPP
