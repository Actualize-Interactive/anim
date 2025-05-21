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

class Animation {
public:
    Animation() = default;

    // Channel management by index
    void add_channel(const Channel& channel);
    void insert_channel(size_t index, const Channel& channel);
    void append_channel(const Channel& channel);
    Channel* get_channel(size_t index);
    const Channel* get_channel(size_t index) const;
    bool remove_channel(size_t index);
    size_t get_channel_count() const;
    
    // Channel management by name
    Channel* get_channel(const std::string& name);
    const Channel* get_channel(const std::string& name) const;
    bool remove_channel(const std::string& name);
    std::vector<std::string> get_channel_names() const;
    size_t find_channel_index(const std::string& name) const;
    
    // Evaluation
    std::map<std::string, double> evaluate_channels(double time) const;
    std::map<std::string, std::vector<double>> evaluate_channels_range(
        double start_time, double end_time, int num_samples) const;
    std::map<std::string, std::vector<double>> evaluate_channels_range_by_rate(
        double start_time, double end_time, double sample_rate) const;
    
    // Timeline information
    std::optional<double> get_start_time() const;
    std::optional<double> get_end_time() const;
    double length() const;
    int num_samples(double sample_rate) const;
    
    // State queries
    bool is_empty() const;
    bool has_no_keyframes() const;

private:
    std::vector<Channel> m_channels;
};

} // namespace anim

#endif // ANIM_ANIMATION_HPP

