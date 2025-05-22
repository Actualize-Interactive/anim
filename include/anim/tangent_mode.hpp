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
    flat = 0,       /**< Tangent handles create a flat curve near the keyframe */
    linear = 1,     /**< Tangent handles create a straight line between keyframes */
    stepped = 2,    /**< Value stays constant until the next keyframe, creating a sudden jump */
    smoothAuto = 3, /**< Tangent handles are automatically calculated for smooth transitions */
    smoothManual = 4, /**< Tangent handles can be manually set but maintain curvature continuity */
    broken = 5,       /**< Tangent handles can be manually set with no constraints */
    count = 6 /**< Total number of tangent modes */
};

} // namespace anim

#endif // ANIM_TANGENT_MODE_HPP
