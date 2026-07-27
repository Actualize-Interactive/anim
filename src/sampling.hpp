#ifndef ANIM_SRC_SAMPLING_HPP
#define ANIM_SRC_SAMPLING_HPP

#include "anim/range_end.hpp"

#include <cmath>
#include <cstddef>

// Internal helper shared by Channel and Animation. Not installed, not part of
// the public API.
namespace anim {
namespace detail {

/**
 * @brief How many points spaced 1 / @p sample_rate apart, starting at 0, fall
 *        within a span of @p duration.
 *
 * With RangeEnd::Exclusive the span is [0, duration); with RangeEnd::Inclusive
 * it is [0, duration]. The two differ only when the span is a whole number of
 * sample periods, because that is the only case where a point lands exactly on
 * the end: 4 seconds at 30 Hz gives 120 points half-open and 121 closed, while
 * 1.05 seconds gives 32 either way, the next point falling past the end
 * regardless of which end is asked for.
 *
 * A product that misses a whole number only by floating-point noise counts as
 * that whole number, so 4.0 seconds at 30 Hz cannot come out as 121 points
 * because the multiplication landed a few ulps high.
 *
 * @param duration Length of the span; must not be negative.
 * @param sample_rate Samples per unit time; must be positive.
 */
inline std::size_t sample_count(double duration, double sample_rate, RangeEnd range_end) {
    const bool include_end = (range_end == RangeEnd::Inclusive);
    const double exact = duration * sample_rate;
    if (exact <= 0.0) {
        return include_end ? 1 : 0;
    }
    const double nearest = std::round(exact);
    // Scale the tolerance with the magnitude, so it stays meaningful for long
    // spans where the absolute representation error is correspondingly larger.
    const double tolerance = 1e-9 * std::fmax(1.0, exact);
    if (std::fabs(exact - nearest) <= tolerance) {
        return static_cast<std::size_t>(nearest) + (include_end ? 1 : 0);
    }
    return static_cast<std::size_t>(std::ceil(exact));
}

} // namespace detail
} // namespace anim

#endif // ANIM_SRC_SAMPLING_HPP
