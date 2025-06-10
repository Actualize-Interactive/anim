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
    inline void set_name(const std::string& name) { m_name = name; }

    Channel& create_channel(const std::string& channel_name);
    Channel& create_channel(const std::string& channel_name, size_t index);
    Channel& copy_channel(const Channel& source_channel, const std::string& new_name = "");

    // Animation copy methods
    Animation copy() const;
    Animation copy(const std::string& new_name) const;

    const Channel& channel(size_t index) const;
    const Channel& operator[](size_t index) const;
    Channel& channel(size_t index);
    Channel& operator[](size_t index);

    Channel& channel(const std::string& channel_name);
    Channel& operator[](const std::string& channel_name);
    Channel* channel(Id channel_id);
    Channel* operator[](Id channel_id);

    const Channel& channel(const std::string& channel_name) const;
    const Channel& operator[](const std::string& channel_name) const;
    const Channel* channel(Id channel_id) const;
    const Channel* operator[](Id channel_id) const;

    inline size_t size() const { return m_channels.size(); }
    inline size_t num_channels() const { return m_channels.size(); }
    bool has_channel(const std::string& channel_name) const;
    inline bool empty() const { return m_channels.empty(); }

    void reorder_channel(size_t from_index, size_t to_index);
    void reorder_channel(const std::string& channel_name, size_t to_index);
    void reorder_channel(Id channel_id, size_t to_index);

    void clear();
    void remove_channel(size_t index);
    void remove_channel(const std::string& channel_name);
    void remove_channel(Id channel_id);

    std::vector<std::string> channel_names() const;
    const std::vector<std::unique_ptr<Channel>>& channels();
    const std::vector<std::unique_ptr<Channel>>& channels() const;

    inline double start_time() const { return m_start_time; }
    void set_start_time(double start_time);
    
    inline double end_time() const { return m_end_time; }
    void set_end_time(double end_time);

    double length() const;
    void set_length(double length);

    int num_samples(double sample_rate) const;

private:
    static uint64_t s_next_channel_id;
    std::string m_name;
    std::vector<std::unique_ptr<Channel>> m_channels;
    std::unordered_map<Id, Channel*> m_channel_map;
    double m_start_time { 0.0 };
    double m_end_time { 30.0 };

    uint64_t next_channel_id() {
        return s_next_channel_id++;
    }
};

} // namespace anim

#endif // ANIM_ANIMATION_HPP