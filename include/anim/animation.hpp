#ifndef ANIM_ANIMATION_HPP
#define ANIM_ANIMATION_HPP

#include "anim/channel.hpp"
#include <string>
#include <map>
#include <optional>
#include <vector>
#include <limits>
#include <algorithm>
#include <functional>

namespace anim {

/**
 * @brief An Animation manages a collection of Channels that form a complete animation.
 * 
 * The Animation class provides functionality to store, access, and evaluate multiple
 * animation channels. It allows accessing channels by index or by name, and can
 * evaluate all channels at a given time or time range.
 */
class Animation {
public:
    /**
     * @brief Default constructor creates an empty animation
     */
    Animation() = default;
   
    /**
     * @brief Create a new channel with a specified name
     * @param name The name identifying this channel
     * @return Pointer to the created channel
     */
    Channel* create_channel(const std::string& name);

    /**
     * @brief Append a channel to the end of the animation
     * @param channel The channel to append
     * @return Pointer to the appended channel
     */
    Channel* append_channel(const Channel& channel);

    /**
     * @brief Insert a channel at a specific position
     * @param index The position to insert the channel
     * @param channel The channel to insert
     * @return Pointer to the inserted channel
     * @throws std::out_of_range if index is out of range
     */
    Channel* insert_channel(size_t index, const Channel& channel);

    /**
     * @brief Get a channel by its index
     * @param index The index of the channel to retrieve
     * @return Pointer to the channel, or nullptr if index is out of range
     */
    Channel* get_channel(size_t index);

    /**
     * @brief Get a channel by its index (const version)
     * @param index The index of the channel to retrieve
     * @return Pointer to the channel, or nullptr if index is out of range
     */
    const Channel* get_channel(size_t index) const;

    // Channel management by name
    /**
     * @brief Get a channel by its name
     * @param name The name of the channel to retrieve
     * @return Pointer to the channel, or nullptr if no channel with that name exists
     */
    Channel* get_channel(const std::string& name);

    /**
     * @brief Get a channel by its name (const version)
     * @param name The name of the channel to retrieve
     * @return Pointer to the channel, or nullptr if no channel with that name exists
     */
    const Channel* get_channel(const std::string& name) const;

    /**
     * @brief Check if a channel with the specified name exists
     * @param name The name of the channel to check
     * @return true if the channel exists, false otherwise
     */
    bool has_channel(const std::string& name) const;

    
    /**
     * @brief Get the number of channels in the animation
     * @return The channel count
     */
    size_t get_channel_count() const;
    

    /**
     * @brief Remove a channel by its name
     * @param name The name of the channel to remove
     * @return true if the channel was found and removed, false otherwise
     */
    bool remove_channel(const std::string& name);
    /**
     * @brief Remove a channel by its index
     * @param index The index of the channel to remove
     * @return true if the channel was removed, false if index was out of range
     */
    bool remove_channel(size_t index);
    
    /**
     * @brief Get a list of all channel names
     * @return Vector of channel names
     */
    std::vector<std::string> get_channel_names() const;
    
    /**
     * @brief Find the index of a channel by its name
     * @param name The name of the channel to find
     * @return The index of the channel, or -1 if not found
     */
    size_t find_channel_index(const std::string& name) const;
    
    // Evaluation
    /**
     * @brief Evaluate all channels at a specific time
     * @param time The time at which to evaluate
     * @return Map of channel names to their evaluated values
     */
    std::map<std::string, double> evaluate_channels(double time) const;
    
    /**
     * @brief Evaluate all channels over a time range with fixed sample count
     * @param start_time The beginning of the time range
     * @param end_time The end of the time range
     * @param num_samples The number of samples to generate
     * @return Map of channel names to vectors of their evaluated values
     */
    std::map<std::string, std::vector<double>> evaluate_channels_range(
        double start_time, double end_time, int num_samples) const;
    
    /**
     * @brief Evaluate all channels over a time range with a specific sample rate
     * @param start_time The beginning of the time range
     * @param end_time The end of the time range
     * @param sample_rate The number of samples per unit of time
     * @return Map of channel names to vectors of their evaluated values
     */
    std::map<std::string, std::vector<double>> evaluate_channels_range_by_rate(
        double start_time, double end_time, double sample_rate) const;
    
    // Timeline information
    /**
     * @brief Get the earliest keyframe time across all channels
     * @return Optional containing the start time, or empty if animation has no keyframes
     */
    std::optional<double> get_start_time() const;
    
    /**
     * @brief Get the latest keyframe time across all channels
     * @return Optional containing the end time, or empty if animation has no keyframes
     */
    std::optional<double> get_end_time() const;
    
    /**
     * @brief Get the total duration of the animation
     * @return The length of the animation (end_time - start_time), or 0 if empty
     */
    double length() const;
    
    /**
     * @brief Calculate the number of samples needed at a given sample rate
     * @param sample_rate The number of samples per unit of time
     * @return The number of samples that would be generated for the animation's duration
     * @throws std::invalid_argument if sample_rate is not positive
     */
    int num_samples(double sample_rate) const;
    
    // State queries
    /**
     * @brief Check if the animation has any channels
     * @return true if animation has no channels, false otherwise
     */
    bool is_empty() const;
    
    /**
     * @brief Check if all channels in the animation are empty
     * @return true if all channels have no keyframes, false otherwise
     */
    bool has_no_keyframes() const;

private:
    std::vector<Channel> m_channels;
};

} // namespace anim

#endif // ANIM_ANIMATION_HPP
