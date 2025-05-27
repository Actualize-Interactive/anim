#include "anim/animation.hpp"

namespace anim {

Channel* Animation::create_channel(const std::string& name) {
    m_channels.emplace_back(name);
    return &m_channels.back();
}

Channel* Animation::append_channel(const Channel& channel) {
    m_channels.push_back(channel);
    return &m_channels.back();
}

// Channel management by index
Channel* Animation::insert_channel(size_t index, const Channel& channel) {
    if (index > m_channels.size()) {
        throw std::out_of_range("Channel index out of range");
    }
    m_channels.insert(m_channels.begin() + index, channel);
    return &m_channels[index];
}

Channel* Animation::get_channel(size_t index) {
    if (index >= m_channels.size()) {
        return nullptr;
    }
    return &m_channels[index];
}

const Channel* Animation::get_channel(size_t index) const {
    if (index >= m_channels.size()) {
        return nullptr;
    }
    return &m_channels[index];
}

bool Animation::remove_channel(size_t index) {
    if (index >= m_channels.size()) {
        return false;
    }
    m_channels.erase(m_channels.begin() + index);
    return true;
}

// Channel management by name
Channel* Animation::get_channel(const std::string& name) {
    auto it = std::find_if(m_channels.begin(), m_channels.end(),
                          [&name](const Channel& channel) {
                              return channel.name() == name;
                          });
    if (it != m_channels.end()) {
        return &(*it);
    }
    return nullptr;
}

const Channel* Animation::get_channel(const std::string& name) const {
    auto it = std::find_if(m_channels.begin(), m_channels.end(),
                          [&name](const Channel& channel) {
                              return channel.name() == name;
                          });
    if (it != m_channels.end()) {
        return &(*it);
    }
    return nullptr;
}

bool Animation::has_channel(const std::string &name) const
{
    return get_channel(name) != nullptr;
}

size_t Animation::get_channel_count() const 
{
    return m_channels.size();
}

size_t Animation::num_channels() const 
{
    return m_channels.size();
}

bool Animation::remove_channel(const std::string& name) {
    size_t index = find_channel_index(name);
    if (index == static_cast<size_t>(-1)) {
        return false;
    }
    return remove_channel(index);
}

std::vector<std::string> Animation::get_channel_names() const {
    std::vector<std::string> names;
    names.reserve(m_channels.size());
    for (const auto& channel : m_channels) {
        names.push_back(channel.name());
    }
    return names;
}

void Animation::clear_channels() {
    m_channels.clear();
}

size_t Animation::find_channel_index(const std::string& name) const {
    for (size_t i = 0; i < m_channels.size(); ++i) {
        if (m_channels[i].name() == name) {
            return i;
        }
    }
    return static_cast<size_t>(-1);
}

// Keyframe manipulation
bool Animation::insert_keyframe(const std::string& channel_name,
                              double time, double value,
                              const BezierHandle& in_tangent,
                              const BezierHandle& out_tangent,
                              TangentMode mode) {
    Channel* channel = get_channel(channel_name);
    if (!channel) {
        return false;
    }
    
    return channel->insert_keyframe(time, value, in_tangent, out_tangent, mode);
}

bool Animation::insert_keyframe(size_t channel_index,
                              double time, double value,
                              const BezierHandle& in_tangent,
                              const BezierHandle& out_tangent,
                              TangentMode mode) {
    Channel* channel = get_channel(channel_index);
    if (!channel) {
        return false;
    }
    
    return channel->insert_keyframe(time, value, in_tangent, out_tangent, mode);
}

bool Animation::upsert_keyframe(const std::string& channel_name,
                              double time, double value,
                              const BezierHandle& in_tangent,
                              const BezierHandle& out_tangent,
                              TangentMode mode) {
    Channel* channel = get_channel(channel_name);
    if (!channel) {
        return false;
    }
    
    channel->upsert_keyframe(time, value, in_tangent, out_tangent, mode);
    return true;
}

bool Animation::upsert_keyframe(size_t channel_index,
                              double time, double value,
                              const BezierHandle& in_tangent,
                              const BezierHandle& out_tangent,
                              TangentMode mode) {
    Channel* channel = get_channel(channel_index);
    if (!channel) {
        return false;
    }
    
    channel->upsert_keyframe(time, value, in_tangent, out_tangent, mode);
    return true;
}

bool Animation::update_keyframe_at_time(const std::string& channel_name,
                                     double time,
                                     double new_value,
                                     const BezierHandle& new_in_tangent,
                                     const BezierHandle& new_out_tangent,
                                     TangentMode new_mode) {
    Channel* channel = get_channel(channel_name);
    if (!channel) {
        return false;
    }
    
    return channel->update_keyframe_at_time(time, new_value, new_in_tangent, new_out_tangent, new_mode);
}

bool Animation::update_keyframe_at_time(size_t channel_index,
                                     double time,
                                     double new_value,
                                     const BezierHandle& new_in_tangent,
                                     const BezierHandle& new_out_tangent,
                                     TangentMode new_mode) {
    Channel* channel = get_channel(channel_index);
    if (!channel) {
        return false;
    }
    
    return channel->update_keyframe_at_time(time, new_value, new_in_tangent, new_out_tangent, new_mode);
}

bool Animation::update_keyframe_at_time(const std::string& channel_name,
                                     double time,
                                     const std::optional<double>& new_value,
                                     const std::optional<BezierHandle>& new_in_tangent,
                                     const std::optional<BezierHandle>& new_out_tangent,
                                     const std::optional<TangentMode>& new_mode) {
    Channel* channel = get_channel(channel_name);
    if (!channel) {
        return false;
    }
    
    return channel->update_keyframe_at_time(time, new_value, new_in_tangent, new_out_tangent, new_mode);
}

bool Animation::update_keyframe_at_time(size_t channel_index,
                                     double time,
                                     const std::optional<double>& new_value,
                                     const std::optional<BezierHandle>& new_in_tangent,
                                     const std::optional<BezierHandle>& new_out_tangent,
                                     const std::optional<TangentMode>& new_mode) {
    Channel* channel = get_channel(channel_index);
    if (!channel) {
        return false;
    }
    
    return channel->update_keyframe_at_time(time, new_value, new_in_tangent, new_out_tangent, new_mode);
}

bool Animation::update_keyframe(const std::string& channel_name,
                              size_t keyframe_index,
                              double time, double value,
                              const BezierHandle& in_tangent,
                              const BezierHandle& out_tangent,
                              TangentMode mode) {
    Channel* channel = get_channel(channel_name);
    if (!channel) {
        return false;
    }
    
    try {
        channel->update_keyframe(keyframe_index, time, value, in_tangent, out_tangent, mode);
        return true;
    } catch (const std::out_of_range&) {
        return false;
    }
}

bool Animation::update_keyframe(size_t channel_index,
                              size_t keyframe_index,
                              double time, double value,
                              const BezierHandle& in_tangent,
                              const BezierHandle& out_tangent,
                              TangentMode mode) {
    Channel* channel = get_channel(channel_index);
    if (!channel) {
        return false;
    }
    
    try {
        channel->update_keyframe(keyframe_index, time, value, in_tangent, out_tangent, mode);
        return true;
    } catch (const std::out_of_range&) {
        return false;
    }
}

bool Animation::update_keyframe(const std::string& channel_name,
                              size_t keyframe_index,
                              const std::optional<double>& time,
                              const std::optional<double>& value,
                              const std::optional<BezierHandle>& in_tangent,
                              const std::optional<BezierHandle>& out_tangent,
                              const std::optional<TangentMode>& mode) {
    Channel* channel = get_channel(channel_name);
    if (!channel) {
        return false;
    }
    
    try {
        channel->update_keyframe(keyframe_index, time, value, in_tangent, out_tangent, mode);
        return true;
    } catch (const std::out_of_range&) {
        return false;
    }
}

bool Animation::update_keyframe(size_t channel_index,
                              size_t keyframe_index,
                              const std::optional<double>& time,
                              const std::optional<double>& value,
                              const std::optional<BezierHandle>& in_tangent,
                              const std::optional<BezierHandle>& out_tangent,
                              const std::optional<TangentMode>& mode) {
    Channel* channel = get_channel(channel_index);
    if (!channel) {
        return false;
    }
    
    try {
        channel->update_keyframe(keyframe_index, time, value, in_tangent, out_tangent, mode);
        return true;
    } catch (const std::out_of_range&) {
        return false;
    }
}

bool Animation::remove_keyframe(const std::string& channel_name, double time) {
    Channel* channel = get_channel(channel_name);
    if (!channel) {
        return false;
    }
    
    return channel->remove_keyframe_at_time(time);
}

bool Animation::remove_keyframe(size_t channel_index, double time) {
    Channel* channel = get_channel(channel_index);
    if (!channel) {
        return false;
    }
    
    return channel->remove_keyframe_at_time(time);
}

bool Animation::remove_keyframe_at_index(const std::string& channel_name, size_t keyframe_index) {
    Channel* channel = get_channel(channel_name);
    if (!channel) {
        return false;
    }
    
    return channel->remove_keyframe(keyframe_index);
}

bool Animation::remove_keyframe_at_index(size_t channel_index, size_t keyframe_index) {
    Channel* channel = get_channel(channel_index);
    if (!channel) {
        return false;
    }
    
    return channel->remove_keyframe(keyframe_index);
}

// Evaluation functions
std::map<std::string, double> Animation::evaluate_channels(double time) const {
    std::map<std::string, double> results;
    
    for (const auto& channel : m_channels) {
        if (!channel.name().empty()) {
            results[channel.name()] = channel.evaluate(time);
        }
    }
    
    return results;
}

std::map<std::string, std::vector<double>> Animation::evaluate_channels_range(
    double start_time, double end_time, int num_samples) const {
    
    std::map<std::string, std::vector<double>> results;
    
    for (const auto& channel : m_channels) {
        if (!channel.name().empty()) {
            results[channel.name()] = channel.evaluate_range(start_time, end_time, num_samples);
        }
    }
    
    return results;
}

std::map<std::string, std::vector<double>> Animation::evaluate_channels_range_by_rate(
    double start_time, double end_time, double sample_rate) const {
    
    std::map<std::string, std::vector<double>> results;
    
    for (const auto& channel : m_channels) {
        if (!channel.name().empty()) {
            results[channel.name()] = channel.evaluate_range_by_rate(start_time, end_time, sample_rate);
        }
    }
    
    return results;
}

// Timeline information
std::optional<double> Animation::get_start_time() const {
    if (m_channels.empty()) {
        return std::nullopt;
    }
    
    std::optional<double> start_time;
    
    for (const auto& channel : m_channels) {
        auto channel_start = channel.get_start_time();
        if (channel_start) {
            if (!start_time || *channel_start < *start_time) {
                start_time = channel_start;
            }
        }
    }
    
    return start_time;
}

std::optional<double> Animation::get_end_time() const {
    if (m_channels.empty()) {
        return std::nullopt;
    }
    
    std::optional<double> end_time;
    
    for (const auto& channel : m_channels) {
        auto channel_end = channel.get_end_time();
        if (channel_end) {
            if (!end_time || *channel_end > *end_time) {
                end_time = channel_end;
            }
        }
    }
    
    return end_time;
}

double Animation::length() const {
    auto start = get_start_time();
    auto end = get_end_time();
    
    if (!start || !end) {
        return 0.0;
    }
    
    return *end - *start;
}

int Animation::num_samples(double sample_rate) const {
    if (sample_rate <= 0.0) {
        throw std::invalid_argument("Sample rate must be positive");
    }
    
    auto start = get_start_time();
    auto end = get_end_time();
    
    if (!start || !end) {
        return 0;
    }
    
    return static_cast<int>(std::ceil((*end - *start) * sample_rate)) + 1;
}

// State queries
bool Animation::is_empty() const {
    return m_channels.empty();
}

bool Animation::has_no_keyframes() const {
    for (const auto& channel : m_channels) {
        if (!channel.is_empty()) {
            return false;
        }
    }
    return true;
}

} // namespace anim
