#ifndef ANIM_TANGENT_MODE_HPP
#define ANIM_TANGENT_MODE_HPP

namespace anim {

/**
 * @brief Defines how keyframe tangent handles behave when interpolating between keyframes.
 * 
 * Different tangent modes control the shape of the animation curve between keyframes,
 * affecting how smooth or sharp transitions are.
 */
enum class TangentMode {
    flat,       /**< Tangent handles create a flat curve near the keyframe */
    linear,     /**< Tangent handles create a straight line between keyframes */
    stepped,    /**< Value stays constant until the next keyframe, creating a sudden jump */
    smoothAuto, /**< Tangent handles are automatically calculated for smooth transitions */
    smoothManual, /**< Tangent handles can be manually set but maintain curvature continuity */
    broken      /**< Tangent handles can be manually set with no constraints */
};

} // namespace anim

#endif // ANIM_TANGENT_MODE_HPP
