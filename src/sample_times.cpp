#include "anim/sample_times.hpp"
#include "sampling.hpp"

namespace anim {

SampleTimes sample_times(double start_time, double end_time, int num_samples, RangeEnd range_end) {
    if (num_samples < 0) {
        throw std::invalid_argument("Sample count cannot be negative");
    }
    if (start_time > end_time) {
        throw std::invalid_argument("Start time must be less than or equal to end time");
    }

    // Mirrors Channel::evaluate_range: a half-open range divides the span by
    // the count, a closed one by one less. With a single sample there is no
    // spacing to derive and the step is never used, so it stays zero rather
    // than dividing by a closed range's zero divisor.
    double step = 0.0;
    if (num_samples > 1) {
        const int divisor = (range_end == RangeEnd::Inclusive) ? num_samples - 1 : num_samples;
        step = (end_time - start_time) / divisor;
    }
    return SampleTimes(start_time, step, static_cast<size_t>(num_samples));
}

SampleTimes sample_times_by_rate(double start_time, double end_time, double sample_rate,
                                 RangeEnd range_end) {
    if (sample_rate <= 0.0) {
        throw std::invalid_argument("Sample rate must be positive");
    }
    if (start_time > end_time) {
        throw std::invalid_argument("Start time must be less than or equal to end time");
    }

    // evaluate_range_by_rate answers an empty range with the single value at
    // that time, whatever the rate.
    if (start_time == end_time) {
        return SampleTimes(start_time, 0.0, 1);
    }

    // Derived the way evaluate_range_by_rate derives it -- the span the samples
    // cover, handed to the same rule as above -- so the two agree exactly
    // rather than merely to within rounding.
    const size_t count = detail::sample_count(end_time - start_time, sample_rate, range_end);
    const size_t divisor = (range_end == RangeEnd::Inclusive) ? count - 1 : count;
    const double covered_end = start_time + static_cast<double>(divisor) / sample_rate;
    return sample_times(start_time, covered_end, static_cast<int>(count), range_end);
}

} // namespace anim
