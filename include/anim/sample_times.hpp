#ifndef ANIM_SAMPLE_TIMES_HPP
#define ANIM_SAMPLE_TIMES_HPP

#include "anim/range_end.hpp"

#include <cstddef>
#include <stdexcept>

namespace anim {

/**
 * @brief The times a range of samples was taken at, as a closed form.
 *
 * Sampling returns values only, because the times they were taken at are fully
 * described by a start, a step and a count -- storing them would double the
 * memory for no information. This holds those three numbers and computes a time
 * on demand, so indexing it costs a multiply and an add and nothing is
 * allocated.
 *
 * Obtain one from Channel::sample_times(), Channel::sample_times_by_rate(), or
 * the free functions of the same names for an arbitrary range. Those apply the
 * same step rule the matching evaluate_range() call does, so element @c i is
 * exactly the time element @c i of the returned values was evaluated at.
 */
class SampleTimes {
public:
    /**
     * @brief Constructs a sequence of @p count times from @p start, @p step apart.
     *
     * Prefer the sample_times() functions, which derive @p step from a range and
     * a RangeEnd rather than leaving it to the caller to restate that rule.
     */
    SampleTimes(double start, double step, size_t count)
        : m_start(start), m_step(step), m_count(count) {}

    /// @brief The time of sample @p index. Not bounds-checked; see at().
    double operator[](size_t index) const {
        return m_start + static_cast<double>(index) * m_step;
    }

    /// @brief The time of sample @p index. @throws std::out_of_range if out of range.
    double at(size_t index) const {
        if (index >= m_count) {
            throw std::out_of_range("Sample index out of range");
        }
        return (*this)[index];
    }

    /// @brief Number of samples described.
    size_t size() const { return m_count; }
    /// @brief Whether there are no samples.
    bool empty() const { return m_count == 0; }
    /// @brief Time of the first sample.
    double front() const { return m_start; }
    /// @brief Time of the last sample. Undefined when empty().
    double back() const { return (*this)[m_count - 1]; }
    /// @brief Spacing between consecutive samples.
    double step() const { return m_step; }

    bool operator==(const SampleTimes& other) const {
        return m_start == other.m_start && m_step == other.m_step && m_count == other.m_count;
    }
    bool operator!=(const SampleTimes& other) const { return !(*this == other); }

private:
    double m_start;
    double m_step;
    size_t m_count;
};

/**
 * @brief Times that Channel::evaluate_range() would sample an arbitrary range at.
 *
 * The step is the span divided by @p num_samples for a half-open range, or by
 * one less for a closed one, matching evaluate_range() exactly.
 * @throws std::invalid_argument if @p num_samples is negative, or if
 *         @p start_time is after @p end_time.
 */
SampleTimes sample_times(double start_time, double end_time, int num_samples,
                         RangeEnd range_end = RangeEnd::Exclusive);

/**
 * @brief Times that Channel::evaluate_range_by_rate() would sample an arbitrary
 *        range at.
 *
 * The step is one sample period, and the count is the one
 * Channel::num_samples() reports for the same range and @p range_end.
 * @throws std::invalid_argument if @p sample_rate is not positive, or if
 *         @p start_time is after @p end_time.
 */
SampleTimes sample_times_by_rate(double start_time, double end_time, double sample_rate,
                                 RangeEnd range_end = RangeEnd::Exclusive);

} // namespace anim

#endif // ANIM_SAMPLE_TIMES_HPP
