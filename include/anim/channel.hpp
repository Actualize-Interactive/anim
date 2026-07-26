#ifndef ANIM_CHANNEL_HPP
#define ANIM_CHANNEL_HPP

#include "anim/keyframe.hpp"
#include "anim/handle_utils.hpp"
#include "anim/id.hpp"
#include "anim/extend.hpp"
#include <vector>
#include <algorithm>
#include <optional>
#include <stdexcept>
#include <cmath>
#include <string>
#include <memory>

namespace anim {

// Forward declaration
class Animation;

/**
 * @brief A named, time-ordered sequence of keyframes that can be evaluated as a curve.
 *
 * A Channel owns its keyframes, keeps them sorted by time, and applies the
 * appropriate handle constraints as they are added or edited. Evaluating the
 * channel at a given time interpolates between the surrounding keyframes
 * according to each keyframe's Function; times outside the keyframe range
 * are resolved using the channel's Extend settings.
 *
 * Channels are created and owned by an Animation (the constructor is not
 * public) via Animation::create_channel, which assigns each channel a unique,
 * immutable Id. Channels are non-copyable; use Animation::copy_channel to
 * duplicate one.
 */
class Channel {
    friend class Animation; // Allow Animation to access protected constructor

protected:
    // Construction and copying are restricted: only Animation may create a
    // Channel, which guarantees every channel gets a unique ID.
    Channel() = delete;
    Channel(const Channel&) = delete;
    Channel& operator=(const Channel&) = delete;
    Channel(Channel&&) = default;
    Channel& operator=(Channel&&) = default;

    /// @brief Creates a channel with a name and unique id; used only by Animation.
    explicit Channel(const std::string& name, uint64_t id) : m_name(name), m_id(id) {}

public:
    /// @brief The channel's name.
    inline const std::string& name() const { return m_name; }
    /// @brief Renames the channel.
    inline void set_name(const std::string& name) { m_name = name; }

    /// @brief The channel's unique, immutable identifier.
    inline Id id() const { return m_id; }

    /**
     * @brief Creates a keyframe at a time and value and inserts it in time order.
     *
     * If a keyframe already exists within ~1/200s of @p time it is replaced
     * rather than duplicated. The handles are computed from @p handle_mode.
     * @return A reference to the stored keyframe.
     */
    const Keyframe& create_keyframe(double time, double value,
        Function function = Function::Bezier, HandleMode handle_mode = HandleMode::Smooth);

    /// @brief Creates a keyframe from a position point. @see create_keyframe(double,double,Function,HandleMode)
    const Keyframe& create_keyframe(const Point& position,
        Function function = Function::Bezier, HandleMode handle_mode = HandleMode::Smooth);

    /// @brief Creates a keyframe with explicit in/out handles (defaults to HandleMode::Aligned).
    const Keyframe& create_keyframe(double time, double value,
        const Point& in_handle, const Point& out_handle,
        Function function = Function::Bezier, HandleMode handle_mode = HandleMode::Aligned);

    /// @brief Creates a keyframe from a position point with explicit in/out handles.
    const Keyframe& create_keyframe(const Point& position,
        const Point& in_handle, const Point& out_handle,
        Function function = Function::Bezier, HandleMode handle_mode = HandleMode::Aligned);

    /// @brief Creates a keyframe by copying an existing one (replacing any keyframe at the same time).
    const Keyframe& create_keyframe(const Keyframe& reference_keyframe);

    /// @brief Inserts a keyframe by move, in time order. @see create_keyframe
    const Keyframe& emplace_keyframe(Keyframe&& keyframe);

    /// @brief True if a keyframe exists at (approximately) @p time.
    bool has_keyframe(double time) const;
    /**
     * @brief Removes the keyframe at @p index.
     * @throws std::out_of_range if @p index is out of range.
     */
    void delete_keyframe(size_t index);
    /**
     * @brief Returns the keyframe at @p index.
     * @throws std::out_of_range if @p index is out of range.
     */
    const Keyframe& keyframe(size_t index) const;
    /// @brief Indexed access, equivalent to keyframe(index). @throws std::out_of_range if out of range.
    inline const Keyframe& operator[](size_t index) const { return keyframe(index); }
    /**
     * @brief Returns the last keyframe strictly before @p time.
     * @throws std::out_of_range if the channel is empty or no earlier keyframe exists.
     */
    const Keyframe& prev_keyframe(double time) const;
    /**
     * @brief Returns the first keyframe strictly after @p time.
     * @throws std::out_of_range if the channel is empty or no later keyframe exists.
     */
    const Keyframe& next_keyframe(double time) const;
    /**
     * @brief Returns the keyframe nearest @p time.
     * @throws std::out_of_range if the channel is empty.
     */
    const Keyframe& closest_keyframe(double time) const;
    /// @brief Direct read access to the time-ordered keyframe vector.
    const std::vector<Keyframe>& keyframes() const;
    /// @brief Number of keyframes.
    inline size_t size() const { return m_keyframes.size(); }
    /// @brief Number of keyframes (alias for size()).
    inline size_t num_keyframes() const { return m_keyframes.size(); }
    /// @brief True if the channel has no keyframes.
    inline bool empty() const { return m_keyframes.empty(); }

    /// @brief Replaces the keyframe at @p index, re-sorting and re-applying constraints. @throws std::out_of_range if out of range.
    void update_keyframe(size_t index, const Keyframe& keyframe);
    /// @brief Sets the time of keyframe @p index, clamped between its neighbours. @throws std::out_of_range if out of range.
    void set_keyframe_time(size_t index, double time);
    /// @brief Sets the value of keyframe @p index. @throws std::out_of_range if out of range.
    void set_keyframe_value(size_t index, double value);
    /// @brief Sets the position of keyframe @p index (time clamped between neighbours). @throws std::out_of_range if out of range.
    void set_keyframe_position(size_t index, const Point& position);
    /// @brief Sets the position of keyframe @p index from a time and value. @throws std::out_of_range if out of range.
    void set_keyframe_position(size_t index, double time, double value);
    /// @brief Sets the in-handle of keyframe @p index (re-applying handle constraints). @throws std::out_of_range if out of range.
    void set_keyframe_in_handle(size_t index, const Point& in_handle);
    /// @brief Sets the out-handle of keyframe @p index (re-applying handle constraints). @throws std::out_of_range if out of range.
    void set_keyframe_out_handle(size_t index, const Point& out_handle);
    /// @brief Sets the interpolation function of keyframe @p index. @throws std::out_of_range if out of range.
    void set_keyframe_function(size_t index, Function function);
    /// @brief Sets the handle mode of keyframe @p index. @throws std::out_of_range if out of range.
    void set_keyframe_handle_mode(size_t index, HandleMode handle_mode);

    /**
     * @brief Evaluates the channel's value at @p time.
     *
     * Interpolates between the surrounding keyframes; times outside the
     * keyframe range follow the channel's Extend settings. An empty
     * channel returns 0.
     * @param time The time to evaluate at.
     * @param prev_t Optional Bézier solver seed (a previous parameter in [0,1])
     *        used only as a convergence hint; it is not modified.
     * @return The interpolated value.
     */
    double evaluate(double time, double* prev_t = nullptr) const;
    /**
     * @brief Evaluates @p num_samples evenly spaced values from @p start_time to @p end_time.
     *
     * The range is closed: the first sample is at @p start_time and the last is
     * at @p end_time, so the spacing is (end_time - start_time) / (num_samples - 1).
     * Use evaluate_range_by_rate() to sample at a fixed rate instead.
     * @return A vector of @p num_samples values, or a single value if
     *         @p num_samples is 1 or less.
     */
    std::vector<double> evaluate_range(double start_time, double end_time, int num_samples) const;
    /**
     * @brief Evaluates values from @p start_time to @p end_time at a fixed sample rate.
     *
     * The range is half-open: sample @c i is at <tt>start_time + i / sample_rate</tt>
     * and @p end_time itself is not sampled, so a span of n sample periods gives
     * n samples. Sampling 4 seconds at 30 Hz yields 120 values, the last at
     * 3.9667, not 121 ending on 4.0. A span that is not a whole number of
     * periods is rounded up, so the whole range is covered.
     *
     * To include @p end_time, either use evaluate_range(), or extend the range
     * by one period.
     * @param start_time First time to sample.
     * @param end_time Exclusive upper bound of the range.
     * @param sample_rate Samples per unit time; must be positive.
     * @return A vector of num_samples() values, or a single value if
     *         @p start_time and @p end_time are equal.
     * @throws std::invalid_argument if @p sample_rate is not positive, or if
     *         @p start_time is after @p end_time.
     */
    std::vector<double> evaluate_range_by_rate(double start_time, double end_time, double sample_rate) const;

    /// @brief Time of the first keyframe (0 if empty).
    double start_time() const;
    /// @brief Time of the last keyframe (0 if empty).
    double end_time() const;
    /// @brief Duration spanned by the keyframes (end_time() - start_time()).
    double length() const;
    /**
     * @brief Number of samples that length() would produce at @p sample_rate.
     *
     * Matches what evaluate_range_by_rate() returns over the keyframe range: the
     * span is half-open, so length() * sample_rate rounded up. An empty channel
     * gives 0; a channel with no span gives 1.
     * @throws std::invalid_argument if @p sample_rate is not positive.
     */
    size_t num_samples(double sample_rate) const;

    /// @brief Extend behavior for times before the first keyframe.
    Extend extend_start() const;
    /// @brief Extend behavior for times after the last keyframe.
    Extend extend_end() const;
    /// @brief Sets the extend behavior for times before the first keyframe.
    void set_extend_start(Extend extend);
    /// @brief Sets the extend behavior for times after the last keyframe.
    void set_extend_end(Extend extend);

    /**
     * @brief Replaces this channel's keyframes with a copy of @p source's keyframes.
     *
     * Used by Animation::copy_channel; the channel's own name and id are left
     * unchanged.
     */
    void copy_keyframes_from(const Channel& source);

    /// @brief Equality across name and keyframes (the id is intentionally ignored).
    bool operator==(const Channel& other) const;
    /// @brief Negation of operator==.
    bool operator!=(const Channel& other) const;

private:
    std::string m_name;
    const Id m_id; // Immutable ID - each channel has a unique identity
    mutable std::vector<Keyframe> m_keyframes;
    mutable Keyframe m_last_keyframe_cache; // Cache for the last keyframe's original state
    mutable bool m_cache_valid = false; // Track if cache contains valid data
    mutable size_t m_cached_keyframe_index = SIZE_MAX; // Index of the keyframe in cache

    // Extend behavior settings
    Extend m_extend_start = Extend::Hold;
    Extend m_extend_end = Extend::Hold;

    using KeyframeIt = std::vector<Keyframe>::iterator;

    const Keyframe& create_default_keyframe(const Point& position, Function function, HandleMode handle_mode);
    const Keyframe& insert_keyframe(Keyframe&& keyframe, GrabbedHandle grabbed_handle = GrabbedHandle::None);
    const Keyframe& insert_keyframe(KeyframeIt it, Keyframe&& keyframe, GrabbedHandle grabbed_handle = GrabbedHandle::None);

    void update_keyframe_position(KeyframeIt it, const Point& position);
    void clamp_keyframe_time(KeyframeIt it, double time);

    void update_local_handles(KeyframeIt it, GrabbedHandle grabbed_handle = GrabbedHandle::None);
    void update_handles(
        Keyframe& keyframe,
        Keyframe* prev_keyframe_ptr, Keyframe* next_keyframe_ptr,
        GrabbedHandle grabbed_handle = GrabbedHandle::None);

    void apply_last_keyframe_inheritance(bool restore_cache = true) const;
    void invalidate_cache() const;
};

} // namespace anim

#endif // ANIM_CHANNEL_HPP
