#ifndef ANIM_EXTEND_HPP
#define ANIM_EXTEND_HPP

#include <cstdint>

namespace anim {

/**
 * @brief How a channel is evaluated for times outside its keyframe range.
 *
 * Set independently for the start (times before the first keyframe) and the
 * end (times after the last keyframe) via Channel::set_extend_start and
 * Channel::set_extend_end.
 */
enum class Extend : uint8_t {
    Hold   = 0, ///< Clamp to the nearest end keyframe's value.
    Repeat = 1, ///< Loop the keyframe range, repeating the curve periodically.
    Mirror = 2  ///< Ping-pong the keyframe range, reflecting the curve each period.
};

} // namespace anim

#endif // ANIM_EXTEND_HPP
