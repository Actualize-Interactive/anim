#ifndef ANIM_ANIMATION_H
#define ANIM_ANIMATION_H

#include "animation_channel.h"
#include <string>
#include <map>
#include <optional>
#include <limits>

namespace anim {

/**
 * @brief Manages a collection of named AnimationChannels
 */
class Animation {
public:
    /**
     * @brief Constructor for an empty animation
     */
    Animation() = default;

    /**
     * @brief Add or replace a channel with the given name
     * 
     * @param name The name of the channel
     * @param channel The animation channel
     */
    void add_channel(const std::string& name, const AnimationChannel& channel) {
        m_channels[name] = channel;
    }

    /**
     * @brief Get a modifiable channel by name
     * 
     * @param name The name of the channel
     * @return AnimationChannel* Pointer to the channel, or nullptr if not found
     */
    AnimationChannel* get_channel(const std::string& name) {
        auto it = m_channels.find(name);
        if (it != m_channels.end()) {
            return &(it->second);
        }
        return nullptr;
    }

    /**
     * @brief Get a const channel by name
     * 
     * @param name The name of the channel
     * @return const AnimationChannel* Const pointer to the channel, or nullptr if not found
     */
    const AnimationChannel* get_channel(const std::string& name) const {
        auto it = m_channels.find(name);
        if (it != m_channels.end()) {
            return &(it->second);
        }
        return nullptr;
    }

    /**
     * @brief Remove a channel by name
     * 
     * @param name The name of the channel to remove
     * @return bool True if the channel was removed, false if not found
     */
    bool remove_channel(const std::string& name) {
        auto it = m_channels.find(name);
        if (it != m_channels.end()) {
            m_channels.erase(it);
            return true;
        }
        return false;
    }

    /**
     * @brief Get all channel names
     * 
     * @return std::vector<std::string> The names of all channels
     */
    std::vector<std::string> get_channel_names() const {
        std::vector<std::string> names;
        names.reserve(m_channels.size());
        for (const auto& pair : m_channels) {
            names.push_back(pair.first);
        }
        return names;
    }

    /**
     * @brief Evaluate all channels at a specific time
     * 
     * @param time The time to evaluate at
     * @return std::map<std::string, double> Map of channel names to evaluated values
     */
    std::map<std::string, double> evaluate_channels(double time) const {
        std::map<std::string, double> results;
        
        for (const auto& pair : m_channels) {
            results[pair.first] = pair.second.evaluate(time);
        }
        
        return results;
    }

    /**
     * @brief Evaluate all channels over a range with a fixed number of samples
     * 
     * @param start_time Start of the time range
     * @param end_time End of the time range
     * @param num_samples Number of samples to generate
     * @return std::map<std::string, std::vector<double>> Map of channel names to vectors of sampled values
     */
    std::map<std::string, std::vector<double>> evaluate_channels_range(
        double start_time, double end_time, int num_samples) const {
        
        std::map<std::string, std::vector<double>> results;
        
        for (const auto& pair : m_channels) {
            results[pair.first] = pair.second.evaluate_range(start_time, end_time, num_samples);
        }
        
        return results;
    }

    /**
     * @brief Evaluate all channels over a range with a specific sample rate
     * 
     * @param start_time Start of the time range
     * @param end_time End of the time range
     * @param sample_rate Number of samples per unit of time
     * @return std::map<std::string, std::vector<double>> Map of channel names to vectors of sampled values
     */
    std::map<std::string, std::vector<double>> evaluate_channels_range_by_rate(
        double start_time, double end_time, double sample_rate) const {
        
        std::map<std::string, std::vector<double>> results;
        
        for (const auto& pair : m_channels) {
            results[pair.first] = pair.second.evaluate_range_by_rate(start_time, end_time, sample_rate);
        }
        
        return results;
    }

    /**
     * @brief Get the overall start time (earliest keyframe across all channels)
     * 
     * @return std::optional<double> The start time, or std::nullopt if there are no keyframes
     */
    std::optional<double> get_start_time() const {
        if (m_channels.empty()) {
            return std::nullopt;
        }
        
        std::optional<double> start_time;
        
        for (const auto& pair : m_channels) {
            auto channel_start = pair.second.get_start_time();
            if (channel_start) {
                if (!start_time || *channel_start < *start_time) {
                    start_time = channel_start;
                }
            }
        }
        
        return start_time;
    }

    /**
     * @brief Get the overall end time (latest keyframe across all channels)
     * 
     * @return std::optional<double> The end time, or std::nullopt if there are no keyframes
     */
    std::optional<double> get_end_time() const {
        if (m_channels.empty()) {
            return std::nullopt;
        }
        
        std::optional<double> end_time;
        
        for (const auto& pair : m_channels) {
            auto channel_end = pair.second.get_end_time();
            if (channel_end) {
                if (!end_time || *channel_end > *end_time) {
                    end_time = channel_end;
                }
            }
        }
        
        return end_time;
    }

    /**
     * @brief Check if the animation has any channels
     * 
     * @return bool True if there are no channels
     */
    bool is_empty() const {
        return m_channels.empty();
    }

    /**
     * @brief Check if the animation has any keyframes across all channels
     * 
     * @return bool True if there are no keyframes in any channel
     */
    bool has_no_keyframes() const {
        for (const auto& pair : m_channels) {
            if (!pair.second.is_empty()) {
                return false;
            }
        }
        return true;
    }

private:
    std::map<std::string, AnimationChannel> m_channels; ///< The named animation channels
};

} // namespace anim

#endif // ANIM_ANIMATION_H
