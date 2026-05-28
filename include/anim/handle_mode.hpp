#ifndef ANIM_HANDLE_TYPE_HPP
#define ANIM_HANDLE_TYPE_HPP

#include <cstdint>

namespace anim {

/**
 * @brief How a keyframe's Bézier handles (tangents) are computed and constrained.
 *
 * The handle mode only applies to keyframes whose Function is Function::Bezier;
 * for Function::Linear and Function::Constant the handles are derived
 * automatically. In every mode the handle times are clamped so they cannot
 * cross the neighbouring keyframes.
 */
enum class HandleMode : uint8_t {
    Flat            = 0, ///< Horizontal tangents: handles share the keyframe value; their time is adjustable.
    Smooth          = 1, ///< Auto-computed collinear handles for a smooth (C1) curve, scaled by neighbour distance.
    Aligned         = 2, ///< In/out handles kept collinear (one straight tangent); magnitudes may differ.
    Free            = 3, ///< Fully independent ("broken") handles; only time-clamped to neighbours.
    AlignStrict     = 4, ///< Aligned and symmetric: both handles forced to equal magnitude.
    AlignFlex       = 5, ///< Aligned, but each handle's magnitude is clamped independently (symmetry may break).
    AlignAdjustable = 6, ///< Aligned, where the shared magnitude follows the opposite handle while adjusting.
    Count           = 7  ///< Number of handle modes; not a selectable value.
};

} // namespace anim

#endif // ANIM_HANDLE_TYPE_HPP
