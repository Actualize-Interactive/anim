#ifndef ANIM_SRC_SAMPLING_HPP
#define ANIM_SRC_SAMPLING_HPP

#include <cmath>
#include <cstddef>

// Internal helper shared by Channel and Animation. Not installed, not part of
// the public API.
namespace anim {
namespace detail {

/**
 * @brief Number of samples covering the half-open span [0, duration) at
 *        @p sample_rate, with samples at 0, 1/rate, 2/rate, ...
 *
 * The count is duration * sample_rate rounded up, so a span that is not a whole
 * number of sample periods is still covered end to end. A product that misses a
 * whole number only by floating-point noise is treated as that whole number: 4.0
 * seconds at 30 Hz must be 120 samples, and computing it as 120.00000000000001
 * must not silently add a 121st.
 *
 * @param duration Length of the span; must not be negative.
 * @param sample_rate Samples per unit time; must be positive.
 */
inline std::size_t sample_count(double duration, double sample_rate) {
    const double exact = duration * sample_rate;
    if (exact <= 0.0) {
        return 0;
    }
    const double nearest = std::round(exact);
    // Scale the tolerance with the magnitude, so it stays meaningful for long
    // spans where the absolute representation error is correspondingly larger.
    const double tolerance = 1e-9 * std::fmax(1.0, exact);
    if (std::fabs(exact - nearest) <= tolerance) {
        return static_cast<std::size_t>(nearest);
    }
    return static_cast<std::size_t>(std::ceil(exact));
}

} // namespace detail
} // namespace anim

#endif // ANIM_SRC_SAMPLING_HPP
