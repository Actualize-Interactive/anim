#ifndef ANIM_ANIMATION_HPP
#define ANIM_ANIMATION_HPP

#include "anim/channel.hpp"
#include "anim/id.hpp"
#include <unordered_map>
#include <memory>
#include <vector>
#include <string>
#include <stdexcept>
#include <algorithm>
#include <functional>

namespace anim {

class Animation {
public:
    Animation() = default;
    explicit Animation(const std::string& name) : m_name(name) {}

    inline const std::string& name() const { return m_name; }
    inline void set_name(const std::string& name) { m_name = name; }    inline Channel& create_channel(const std::string& channel_name) {
        uint64_t new_id = s_next_channel_id++;
        auto channel_ptr = std::unique_ptr<Channel>(new Channel(channel_name, new_id));
        Channel* raw_ptr = channel_ptr.get();
        m_channel_map[Id(new_id)] = raw_ptr;
        m_channels.emplace_back(std::move(channel_ptr));
        return *raw_ptr;
    }

    inline Channel& create_channel(const std::string& channel_name, size_t index) {
        if (index > m_channels.size()) {
            throw std::out_of_range("Channel index out of range");
        }
        uint64_t new_id = s_next_channel_id++;
        auto channel_ptr = std::unique_ptr<Channel>(new Channel(channel_name, new_id));
        Channel* raw_ptr = channel_ptr.get();
        m_channel_map[Id(new_id)] = raw_ptr;
        m_channels.insert(m_channels.begin() + index, std::move(channel_ptr));
        return *raw_ptr;
    }

    // Create a new channel by copying data from an existing channel (gets new ID)
    inline Channel& copy_channel(const Channel& source_channel, const std::string& new_name = "") {
        uint64_t new_id = s_next_channel_id++;
        std::string channel_name = new_name.empty() ? source_channel.name() + "_copy" : new_name;
        
        // Create new channel with new ID
        auto channel_ptr = std::unique_ptr<Channel>(new Channel(channel_name, new_id));
        Channel* raw_ptr = channel_ptr.get();
        
        // Copy keyframes from source channel
        raw_ptr->copy_keyframes_from(source_channel);
        
        m_channel_map[Id(new_id)] = raw_ptr;
        m_channels.emplace_back(std::move(channel_ptr));
        return *raw_ptr;
    }

    inline const Channel& channel(size_t index) const {
        if (index >= m_channels.size()) {
            throw std::out_of_range("Channel index out of range");
        }
        return *m_channels[index];
    }
    inline const Channel& operator[](size_t index) const {
        return channel(index);
    }
    inline Channel& channel(size_t index) {
        if (index >= m_channels.size()) {
            throw std::out_of_range("Channel index out of range");
        }
        return *m_channels[index];
    }
    inline Channel& operator[](size_t index) {
        return channel(index);
    }

    inline Channel& channel(const std::string& channel_name) {
        auto it = std::find_if(m_channels.begin(), m_channels.end(),
                               [&channel_name](const std::unique_ptr<Channel>& ch) { return ch->name() == channel_name; });
        if (it != m_channels.end()) {
            return **it;
        } else {
            throw std::out_of_range("Channel with the specified name does not exist");
        }
    }
    inline Channel& operator[](const std::string& channel_name) {
        return channel(channel_name);
    }

    // Id-based channel accessor
    inline Channel& channel(Id channel_id) {
        auto it = m_channel_map.find(channel_id);
        if (it != m_channel_map.end()) {
            return *(it->second);
        } else {
            throw std::out_of_range("Channel with the specified ID does not exist");
        }
    }
    inline Channel& operator[](Id channel_id) {
        return channel(channel_id);
    }

    inline const Channel& channel(const std::string& channel_name) const {
        auto it = std::find_if(m_channels.begin(), m_channels.end(),
                               [&channel_name](const std::unique_ptr<Channel>& ch) { return ch->name() == channel_name; });
        if (it != m_channels.end()) {
            return **it;
        } else {
            throw std::out_of_range("Channel with the specified name does not exist");
        }
    }
    inline const Channel& operator[](const std::string& channel_name) const {
        return channel(channel_name);
    }

    // Const Id-based channel accessor
    inline const Channel& channel(Id channel_id) const {
        auto it = m_channel_map.find(channel_id);
        if (it != m_channel_map.end()) {
            return *(it->second);
        } else {
            throw std::out_of_range("Channel with the specified ID does not exist");
        }
    }
    inline const Channel& operator[](Id channel_id) const {
        return channel(channel_id);
    }

    inline size_t size() const { return m_channels.size(); }
    inline size_t num_channels() const { return m_channels.size(); }
    inline bool has_channel(const std::string& channel_name) const {
        return std::any_of(m_channels.begin(), m_channels.end(),
                           [&channel_name](const std::unique_ptr<Channel>& ch) { return ch->name() == channel_name; });
    }
    inline bool empty() const { return m_channels.empty(); }    // Reorder channel to a new position
    inline void reorder_channel(size_t from_index, size_t to_index) {
        if (from_index >= m_channels.size() || to_index >= m_channels.size()) {
            throw std::out_of_range("Channel index out of range");
        }
        if (from_index == to_index) return;
        
        size_t original_size = m_channels.size();
        auto temp = std::move(m_channels[from_index]);
        m_channels.erase(m_channels.begin() + from_index);
        
        // Adjust target index if it was after the removed element
        // Special case: if target was the last position originally, keep it at the end
        if (to_index == original_size - 1) {
            // Target was last position - place at new end
            to_index = m_channels.size();
        } else if (to_index > from_index) {
            to_index--;
        }
        
        // Clamp to valid range for insertion
        to_index = std::min(to_index, m_channels.size());
        
        m_channels.insert(m_channels.begin() + to_index, std::move(temp));
        // No need to rebuild map as pointers don't change with unique_ptr
    }

    inline void reorder_channel(const std::string& channel_name, size_t to_index) {
        auto it = std::find_if(m_channels.begin(), m_channels.end(),
                               [&channel_name](const std::unique_ptr<Channel>& ch) { return ch->name() == channel_name; });
        if (it == m_channels.end()) {
            throw std::out_of_range("Channel with the specified name does not exist");
        }
        size_t from_index = std::distance(m_channels.begin(), it);
        reorder_channel(from_index, to_index);
    }

    inline void reorder_channel(Id channel_id, size_t to_index) {
        auto it = std::find_if(m_channels.begin(), m_channels.end(),
                               [channel_id](const std::unique_ptr<Channel>& ch) { return ch->id() == channel_id; });
        if (it == m_channels.end()) {
            throw std::out_of_range("Channel with the specified ID does not exist");
        }
        size_t from_index = std::distance(m_channels.begin(), it);
        reorder_channel(from_index, to_index);
    }

    inline void clear() { 
        m_channel_map.clear();
        m_channels.clear(); 
    }

    inline void remove_channel(size_t index) {
        if (index >= m_channels.size()) {
            throw std::out_of_range("Channel index out of range");
        }
        // Remove from map first
        Id channel_id = m_channels[index]->id();
        m_channel_map.erase(channel_id);
        m_channels.erase(m_channels.begin() + index);
    }

    inline void remove_channel(const std::string& channel_name) {
        auto it = std::find_if(m_channels.begin(), m_channels.end(),
                               [&channel_name](const std::unique_ptr<Channel>& ch) { return ch->name() == channel_name; });
        if (it != m_channels.end()) {
            // Remove from map first
            Id channel_id = (*it)->id();
            m_channel_map.erase(channel_id);
            m_channels.erase(it);
        } else {
            throw std::out_of_range("Channel with the specified name does not exist");
        }
    }

    inline void remove_channel(Id channel_id) {
        auto map_it = m_channel_map.find(channel_id);
        if (map_it != m_channel_map.end()) {
            // Find the channel in the vector and remove it
            auto vec_it = std::find_if(m_channels.begin(), m_channels.end(),
                                       [channel_id](const std::unique_ptr<Channel>& ch) { return ch->id() == channel_id; });
            if (vec_it != m_channels.end()) {
                m_channels.erase(vec_it);
            }
            m_channel_map.erase(map_it);
        } else {
            throw std::out_of_range("Channel with the specified ID does not exist");
        }
    }

    std::vector<std::string> channel_names() const {
        std::vector<std::string> names;
        names.reserve(m_channels.size());
        for (const auto& channel : m_channels) {
            names.push_back(channel->name());
        }
        return names;
    }

    // For testing purposes - provides access to channels as vector of references
    std::vector<std::reference_wrapper<Channel>> channels() {
        std::vector<std::reference_wrapper<Channel>> refs;
        refs.reserve(m_channels.size());
        for (auto& channel_ptr : m_channels) {
            refs.emplace_back(*channel_ptr);
        }
        return refs;
    }

    std::vector<std::reference_wrapper<const Channel>> channels() const {
        std::vector<std::reference_wrapper<const Channel>> refs;
        refs.reserve(m_channels.size());
        for (const auto& channel_ptr : m_channels) {
            refs.emplace_back(*channel_ptr);
        }
        return refs;
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
    static uint64_t s_next_channel_id;
    std::string m_name;
    std::vector<std::unique_ptr<Channel>> m_channels;
    std::unordered_map<Id, Channel*> m_channel_map;
    double m_start_time { 0.0 };
    double m_end_time { 30.0 };
};

} // namespace anim

#endif // ANIM_ANIMATION_HPP