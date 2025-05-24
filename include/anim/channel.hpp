#ifndef ANIM_CHANNEL_HPP
#define ANIM_CHANNEL_HPP

#include "anim/keyframe.hpp"
#include "anim/bezier_utils.hpp"
#include <vector>
#include <algorithm>
#include <optional>
#include <stdexcept>
#include <cmath>
#include <string>

namespace anim {

/**
 * @brief A Channel represents a single animatable property over time.
 * 
 * Channels contain keyframes that define values at specific times,
 * with interpolation between keyframes creating a continuous curve.
 * Each channel has a name that identifies what property it controls.
 */
class Channel {
public:
    /**
     * @brief Default constructor creates an empty channel with no name
     */
    Channel() = default;
    
    /**
     * @brief Construct a named channel
     * @param name The name identifying this channel
     */
    explicit Channel(const std::string& name) : m_name(name) {}

    /**
     * @brief Set a keyframe at the specified time and value
     * 
     * If a keyframe already exists at this time, it will be updated with the new values.
     * Otherwise, a new keyframe will be created and the internal keyframe list will be sorted.
     * 
     * @param time The time position for the keyframe
     * @param value The value at this keyframe
     * @param in_handle The incoming Bézier handle
     * @param out_handle The outgoing Bézier handle
     * @param mode The tangent mode that controls how handles behave
     */
    void upsert_keyframe(double time, double value, 
                     const BezierHandle& in_handle, 
                     const BezierHandle& out_handle, 
                     TangentMode mode);
    
    /**
     * @brief Alias for upsert_keyframe to maintain backward compatibility
     * @deprecated Use upsert_keyframe instead
     */
    void set_keyframe_at_time(double time, double value, 
                     const BezierHandle& in_handle, 
                     const BezierHandle& out_handle, 
                     TangentMode mode);
    
    /**
     * @brief Insert a new keyframe at the specified time
     * 
     * Fails if a keyframe already exists at the specified time.
     * 
     * @param time The time position for the keyframe
     * @param value The value at this keyframe
     * @param in_handle The incoming Bézier handle
     * @param out_handle The outgoing Bézier handle
     * @param mode The tangent mode that controls how handles behave
     * @return true if keyframe was inserted, false if a keyframe already exists at the time
     */
    bool insert_keyframe(double time, double value, 
                     const BezierHandle& in_handle, 
                     const BezierHandle& out_handle, 
                     TangentMode mode);
    
    /**
     * @brief Append a keyframe at the end of the timeline
     * 
     * Fails if a keyframe already exists at the specified time.
     * 
     * @param time The time position for the keyframe (must be greater than any existing keyframe time)
     * @param value The value at this keyframe
     * @param in_handle The incoming Bézier handle
     * @param out_handle The outgoing Bézier handle
     * @param mode The tangent mode that controls how handles behave
     * @return true if keyframe was appended, false if couldn't be appended
     * @throws std::invalid_argument if time is not greater than all existing keyframe times
     */
    bool append_keyframe(double time, double value, 
                     const BezierHandle& in_handle, 
                     const BezierHandle& out_handle, 
                     TangentMode mode);
    
    /**
     * @brief Update all properties of an existing keyframe at the specified index
     *
     * @param index The index of the keyframe to update
     * @param time The new time of the keyframe
     * @param value The new value of the keyframe
     * @param in_handle The new incoming Bézier handle
     * @param out_handle The new outgoing Bézier handle
     * @param mode The new tangent mode
     * @throws std::out_of_range if no keyframe exists at the specified index
     */
    void update_keyframe(size_t index, double time, double value, 
                    const BezierHandle& in_handle, 
                    const BezierHandle& out_handle, 
                    TangentMode mode);
    
    /**
     * @brief Update selected properties of an existing keyframe at the specified index
     *
     * Only updates the properties for which a value is provided.
     *
     * @param index The index of the keyframe to update
     * @param time Optional new time for the keyframe
     * @param value Optional new value for the keyframe
     * @param in_handle Optional new incoming Bézier handle
     * @param out_handle Optional new outgoing Bézier handle
     * @param mode Optional new tangent mode
     * @throws std::out_of_range if no keyframe exists at the specified index
     */
    void update_keyframe(size_t index, 
                    const std::optional<double>& time = std::nullopt, 
                    const std::optional<double>& value = std::nullopt, 
                    const std::optional<BezierHandle>& in_handle = std::nullopt, 
                    const std::optional<BezierHandle>& out_handle = std::nullopt, 
                    const std::optional<TangentMode>& mode = std::nullopt);
    
    /**
     * @brief Update an existing keyframe by time
     *
     * @param time The time of the keyframe to update
     * @param new_value The new value for the keyframe
     * @param new_in_handle The new incoming Bézier handle
     * @param new_out_handle The new outgoing Bézier handle
     * @param new_mode The new tangent mode
     * @return true if keyframe was updated, false if no keyframe exists at the specified time
     */
    bool update_keyframe_at_time(double time, 
                            double new_value, 
                            const BezierHandle& new_in_handle, 
                            const BezierHandle& new_out_handle, 
                            TangentMode new_mode);
    
    /**
     * @brief Update selected properties of an existing keyframe at the specified time
     *
     * Only updates the properties for which a value is provided.
     *
     * @param time The time of the keyframe to update
     * @param new_value Optional new value for the keyframe
     * @param new_in_handle Optional new incoming Bézier handle
     * @param new_out_handle Optional new outgoing Bézier handle
     * @param new_mode Optional new tangent mode
     * @return true if keyframe was updated, false if no keyframe exists at the specified time
     */
    bool update_keyframe_at_time(double time, 
                            const std::optional<double>& new_value = std::nullopt, 
                            const std::optional<BezierHandle>& new_in_handle = std::nullopt, 
                            const std::optional<BezierHandle>& new_out_handle = std::nullopt, 
                            const std::optional<TangentMode>& new_mode = std::nullopt);

    /**
     * @brief Set an existing keyframe's time, value, and tangents
     * @param index The index of the keyframe to modify
     * @param time The time of the keyframe to modify
     * @param new_value The new value to assign to the keyframe
     * @param new_in_handle The new incoming Bézier handle
     * @param new_out_handle The new outgoing Bézier handle
     * @param new_mode The new tangent mode
     * @throws std::out_of_range if no keyframe exists at the specified time
     * @deprecated Use update_keyframe instead
     */
    void set_keyframe(size_t index, double time, double new_value, 
                     const BezierHandle& new_in_handle, 
                     const BezierHandle& new_out_handle, 
                     TangentMode new_mode);

    
    bool has_keyframe_at_time(double time) const;
    bool has_keyframe(size_t index) const;
    size_t keyframe_count() const { return m_keyframes.size(); }



    /**
     * @brief Remove a keyframe at the specified time
     * 
     * @param time The time of the keyframe to remove
     * @return true if a keyframe was found and removed, false otherwise
     */
    bool remove_keyframe_at_time(double time);

    /**
     * @brief Remove a keyframe at the specified index
     * 
     * @param index The index of the keyframe to remove
     * @return true if a keyframe was found and removed, false otherwise
     */
    bool remove_keyframe(size_t index);

    /**
     * @brief Get the keyframe at the specified time
     * 
     * @param time The time to look for a keyframe
     * @return An optional containing the keyframe if found, or empty if not found
     */
    std::optional<Keyframe> get_keyframe_at_time(double time) const;

    /**
     * @brief Get the keyframe at the specified index
     * 
     * @param index The index of the keyframe to retrieve
     * @return A reference to the keyframe at the specified index
     * @throws std::out_of_range if index is out of range
     */
    Keyframe& get_keyframe(size_t index);
    
    /**
     * @brief Get all keyframes in this channel
     * 
     * @return A const reference to the vector of keyframes
     */
    const std::vector<Keyframe>& get_all_keyframes() const;

    /**
     * @brief Evaluate the channel at a specific time
     * 
     * @param time The time at which to evaluate the channel
     * @return The interpolated value at the specified time
     */
    double evaluate(double time) const;
    
    /**
     * @brief Evaluate the channel over a range with a fixed number of samples
     * 
     * @param start_time The beginning of the time range
     * @param end_time The end of the time range
     * @param num_samples The number of samples to generate
     * @return A vector of values representing the channel evaluation at each sample point
     */
    std::vector<double> evaluate_range(double start_time, double end_time, int num_samples) const;
    
    /**
     * @brief Evaluate the channel over a range at a specific sample rate
     * 
     * @param start_time The beginning of the time range
     * @param end_time The end of the time range
     * @param sample_rate The number of samples per unit of time
     * @return A vector of values representing the channel evaluation at each sample point
     * @throws std::invalid_argument if sample_rate is not positive
     */
    std::vector<double> evaluate_range_by_rate(double start_time, double end_time, double sample_rate) const;

    /**
     * @brief Check if the channel has any keyframes
     * 
     * @return true if the channel has no keyframes, false otherwise
     */
    bool is_empty() const;
    
    /**
     * @brief Get the time of the first keyframe
     * 
     * @return An optional containing the start time, or empty if the channel is empty
     */
    std::optional<double> get_start_time() const;
    
    /**
     * @brief Get the time of the last keyframe
     * 
     * @return An optional containing the end time, or empty if the channel is empty
     */
    std::optional<double> get_end_time() const;

    /**
     * @brief Get the name of this channel
     * 
     * @return The channel name
     */
    const std::string& name() const { return m_name; }
    
    /**
     * @brief Set the name of this channel
     * 
     * @param name The new channel name
     */
    void set_name(const std::string& name) { m_name = name; }

    /**
     * @brief Set the time of an existing keyframe
     * 
     * @param old_time The time of the keyframe to modify
     * @param new_time The new time to assign
     * @return true if a keyframe was found and updated, false otherwise
     */
    bool set_keyframe_time(double old_time, double new_time);
    
    /**
     * @brief Set the value of an existing keyframe
     * 
     * @param time The time of the keyframe to modify
     * @param new_value The new value to assign
     * @return true if a keyframe was found and updated, false otherwise
     */
    bool set_keyframe_value(double time, double new_value);
    
    /**
     * @brief Set the in-handle of an existing keyframe
     * 
     * @param time The time of the keyframe to modify
     * @param new_in_handle The new in-handle to assign
     * @return true if a keyframe was found and updated, false otherwise
     */
    bool set_keyframe_in_handle(double time, const BezierHandle& new_in_handle);
    
    /**
     * @brief Set the out-handle of an existing keyframe
     * 
     * @param time The time of the keyframe to modify
     * @param new_out_handle The new out-handle to assign
     * @return true if a keyframe was found and updated, false otherwise
     */
    bool set_keyframe_out_handle(double time, const BezierHandle& new_out_handle);
    
    /**
     * @brief Set the tangent mode of an existing keyframe
     * 
     * @param time The time of the keyframe to modify
     * @param new_mode The new tangent mode to assign
     * @return true if a keyframe was found and updated, false otherwise
     */
    bool set_keyframe_tangent_mode(double time, TangentMode new_mode);

private:
    std::string m_name;
    std::vector<Keyframe> m_keyframes;

    /**
     * @brief Sort keyframes by time (internal method)
     */
    void sort_keyframes_internal();
    
    /**
     * @brief Recalculate tangent handles based on tangent mode (internal method)
     */
    void recalculate_dependent_tangents_internal();
};

} // namespace anim

#endif // ANIM_CHANNEL_HPP
