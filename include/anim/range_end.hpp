#ifndef ANIM_RANGE_END_HPP
#define ANIM_RANGE_END_HPP

#include <cstdint>

namespace anim {

/**
 * @brief Whether a sampled range includes its end time.
 *
 * Passed to Channel::evaluate_range, Channel::evaluate_range_by_rate and the
 * matching num_samples() overloads. The default everywhere is
 * RangeEnd::Exclusive, which treats a sample as covering the interval that
 * follows it -- the convention frame- and audio-rate hosts use, where the end
 * of a range is an edge rather than a sample.
 *
 * RangeEnd::Inclusive treats samples as points on the curve instead, which is
 * what plotting a curve, building an interpolation lookup table, or integrating
 * numerically all need: without it the last point falls short of the end.
 */
enum class RangeEnd : uint8_t {
    Exclusive = 0, ///< The end time is not sampled; the range is [start, end).
    Inclusive = 1  ///< The end time is sampled; the range is [start, end].
};

} // namespace anim

#endif // ANIM_RANGE_END_HPP
