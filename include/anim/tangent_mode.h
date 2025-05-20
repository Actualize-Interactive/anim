#ifndef ANIM_TANGENT_MODE_H
#define ANIM_TANGENT_MODE_H

namespace anim {

/**
 * @brief Defines how tangents at a keyframe behave and how they are calculated or interact.
 */
enum class TangentMode {
    /**
     * @brief Tangents are horizontal (zero slope).
     * 
     * Handles are typically (key.time +/- offset, key.value).
     */
    flat,

    /**
     * @brief Tangents point directly to the adjacent keyframes, creating straight line segments.
     * 
     * outTangent of kf_i points towards kf_{i+1}.time/value, and
     * inTangent of kf_{i+1} points towards kf_i.time/value.
     */
    linear,

    /**
     * @brief Value remains constant from this keyframe until the next, then jumps.
     * 
     * The outTangent would be flat, and evaluation should return the current
     * keyframe's value until the next keyframe's time.
     */
    stepped,

    /**
     * @brief Tangents are automatically calculated to ensure C1 continuity (smooth curve).
     * 
     * Typically based on neighboring keyframes (e.g., Catmull-Rom like calculation,
     * or averaging slopes). Both in and out tangents are affected and co-linear.
     */
    smoothAuto,

    /**
     * @brief User can set one tangent handle (in or out), and the other handle is automatically adjusted.
     * 
     * The other handle is automatically adjusted to be co-linear and maintain smoothness.
     * The lengths of the handles can be independent or related.
     */
    smoothManual,

    /**
     * @brief In-tangent and out-tangent are completely independent.
     * 
     * Allows for sharp corners in the animation curve.
     */
    broken
};

} // namespace anim

#endif // ANIM_TANGENT_MODE_H
