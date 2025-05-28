#ifndef ANIM_ANIMATION_HPP
#define ANIM_ANIMATION_HPP

#include "anim/channel.hpp"

namespace anim {
class Animation {
public:
    Animation() = default;
    explicit Animation(const std::string& name) : m_name(name) {}

    inline const std::string& name() const { return m_name; }
    inline void set_name(const std::string& name) { m_name = name; }

    inline Channel& create_channel(const std::string& channel_name) {
        m_channels.emplace_back(channel_name);
        return m_channels.back();
    }
    inline Channel& create_channel(const std::string& channel_name, size_t index) {
        if (index > m_channels.size()) {
            throw std::out_of_range("Channel index out of range");
        }
        m_channels.insert(m_channels.begin() + index, Channel(channel_name));
        return m_channels[index];
    }

    inline Channel& emplace_channel(Channel&& channel) {
        m_channels.emplace_back(std::move(channel));
        return m_channels.back();
    }

    inline Channel& insert_channel(size_t index, const Channel& channel) {
        if (index > m_channels.size()) {
            throw std::out_of_range("Channel index out of range");
        }
        m_channels.insert(m_channels.begin() + index, channel);
        return m_channels[index];
    }
    inline Channel& insert_channel(size_t index, Channel&& channel) {
        if (index > m_channels.size()) {
            throw std::out_of_range("Channel index out of range");
        }
        m_channels.insert(m_channels.begin() + index, std::move(channel));
        return m_channels[index];
    }

    inline const Channel& channel(size_t index) const {
        if (index >= m_channels.size()) {
            throw std::out_of_range("Channel index out of range");
        }
        return m_channels[index];
    }
    inline const Channel& operator[](size_t index) const {
        return channel(index);
    }
    inline Channel& channel(size_t index) {
        if (index >= m_channels.size()) {
            throw std::out_of_range("Channel index out of range");
        }
        return m_channels[index];
    }
    inline Channel& operator[](size_t index) {
        return channel(index);
    }

    inline Channel& channel(const std::string& channel_name) {
        auto it = std::find_if(m_channels.begin(), m_channels.end(),
                               [&channel_name](const Channel& ch) { return ch.name() == channel_name; });
        if (it != m_channels.end()) {
            return *it;
        } else {
            throw std::out_of_range("Channel with the specified name does not exist");
        }
    }
    inline Channel& operator[](const std::string& channel_name) {
        return channel(channel_name);
    }

    inline const Channel& channel(const std::string& channel_name) const {
        auto it = std::find_if(m_channels.begin(), m_channels.end(),
                               [&channel_name](const Channel& ch) { return ch.name() == channel_name; });
        if (it != m_channels.end()) {
            return *it;
        } else {
            throw std::out_of_range("Channel with the specified name does not exist");
        }
    }
    inline const Channel& operator[](const std::string& channel_name) const {
        return channel(channel_name);
    }

    inline size_t size() const { return m_channels.size(); }
    inline size_t num_channels() const { return m_channels.size(); }
    inline bool has_channel(const std::string& channel_name) const {
        return std::any_of(m_channels.begin(), m_channels.end(),
                           [&channel_name](const Channel& ch) { return ch.name() == channel_name; });
    }
    inline bool empty() const { return m_channels.empty(); }
    inline void clear() { m_channels.clear(); }

    inline void remove_channel(size_t index) {
        if (index >= m_channels.size()) {
            throw std::out_of_range("Channel index out of range");
        }
        m_channels.erase(m_channels.begin() + index);
    } 

    inline void remove_channel(const std::string& channel_name) {
        auto it = std::remove_if(m_channels.begin(), m_channels.end(),
                                 [&channel_name](const Channel& ch) { return ch.name() == channel_name; });
        if (it != m_channels.end()) {
            m_channels.erase(it, m_channels.end());
        } else {
            throw std::out_of_range("Channel with the specified name does not exist");
        }
    }

    inline std::vector<Channel>& channels() { return m_channels; }
    inline const std::vector<Channel>& channels() const { return m_channels; }
    std::vector<std::string> channel_names() const {
        std::vector<std::string> names;
        names.reserve(m_channels.size());
        for (const auto& channel : m_channels) {
            names.push_back(channel.name());
        }
        return names;
    }

    inline double start_time() const { return m_start_time;  }
    inline void set_start_time(double start_time) { m_start_time = std::min(start_time, m_end_time); }
    
    inline double end_time() const { return m_end_time; }
    inline void set_end_time(double end_time) { m_end_time = std::max(end_time, m_start_time); }

    inline double length() const { 
        if (m_start_time > m_end_time) {
            throw std::invalid_argument("Start time must be less than or equal to end time");
        }
        return m_end_time - m_start_time;
    }
    inline void set_length(double length) {
        if (length < 0.0) {
            throw std::invalid_argument("Length cannot be negative");
        }
        m_end_time = m_start_time + length;
    }

    int num_samples(double sample_rate) const {
        if (sample_rate <= 0.0) {
            throw std::invalid_argument("Sample rate must be positive");
        }
        if (m_channels.empty()) {
            return 0;
        }
        return static_cast<int>(std::ceil(length() * sample_rate)) + 1; // +1 to include the start time
    }



private:
    std::string m_name;
    std::vector<Channel> m_channels;
    double m_start_time { 0.0 };
    double m_end_time { 30.0 };
};
} // namespace anim 

#endif // ANIM_ANIMATION_HPP