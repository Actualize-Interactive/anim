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
     * @param in_tangent The incoming Bézier handle
     * @param out_tangent The outgoing Bézier handle
     * @param mode The tangent mode that controls how handles behave
     */
    void set_keyframe(double time, double value, 
                     const BezierHandle& in_tangent, 
                     const BezierHandle& out_tangent, 
                     TangentMode mode);

    /**
     * @brief Change the time of an existing keyframe
     * 
     * @param old_time The current time of the keyframe to modify
     * @param new_time The new time to assign to the keyframe
     * @throws std::out_of_range if no keyframe exists at old_time
     */
    void set_keyframe_time(double old_time, double new_time);
    
    /**
     * @brief Change the value of an existing keyframe
     * 
     * @param time The time of the keyframe to modify
     * @param new_value The new value to assign to the keyframe
     * @throws std::out_of_range if no keyframe exists at the specified time
     */
    void set_keyframe_value(double time, double new_value);
    
    /**
     * @brief Set the incoming tangent handle for a keyframe
     * 
     * @param time The time of the keyframe to modify
     * @param new_in_tangent The new incoming tangent handle
     * @throws std::out_of_range if no keyframe exists at the specified time
     */
    void set_keyframe_in_tangent(double time, const BezierHandle& new_in_tangent);
    
    /**
     * @brief Set the outgoing tangent handle for a keyframe
     * 
     * @param time The time of the keyframe to modify
     * @param new_out_tangent The new outgoing tangent handle
     * @throws std::out_of_range if no keyframe exists at the specified time
     */
    void set_keyframe_out_tangent(double time, const BezierHandle& new_out_tangent);
    
    /**
     * @brief Change the tangent mode of a keyframe
     * 
     * @param time The time of the keyframe to modify
     * @param new_mode The new tangent mode
     * @throws std::out_of_range if no keyframe exists at the specified time
     */
    void set_keyframe_tangent_mode(double time, TangentMode new_mode);
    
    /**
     * @brief Remove a keyframe at the specified time
     * 
     * @param time The time of the keyframe to remove
     * @return true if a keyframe was found and removed, false otherwise
     */
    bool remove_keyframe(double time);
    
    /**
     * @brief Get the keyframe at the specified time
     * 
     * @param time The time to look for a keyframe
     * @return An optional containing the keyframe if found, or empty if not found
     */
    std::optional<Keyframe> get_keyframe(double time) const;
    
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
