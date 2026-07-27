#include "anim/animation.hpp"
#include "sampling.hpp"

namespace anim {

// Static member definition
uint64_t Animation::s_next_channel_id = 1;

// not as concise as std::make_unique, but necessary due to protected constructor
Channel& Animation::create_channel(const std::string& channel_name) {
    uint64_t new_id = next_channel_id();
    auto channel_ptr = std::unique_ptr<Channel>(new Channel(channel_name, new_id));
    Channel* raw_ptr = channel_ptr.get();
    m_channel_map[Id(new_id)] = raw_ptr;
    m_channels.emplace_back(std::move(channel_ptr));
    return *raw_ptr;
}

Channel& Animation::create_channel(const std::string& channel_name, size_t index) {
    if (index > m_channels.size()) {
        throw std::out_of_range("Channel index out of range");
    }
    uint64_t new_id = next_channel_id();
    auto channel_ptr = std::unique_ptr<Channel>(new Channel(channel_name, new_id));
    Channel* raw_ptr = channel_ptr.get();
    m_channel_map[Id(new_id)] = raw_ptr;
    m_channels.insert(m_channels.begin() + index, std::move(channel_ptr));
    return *raw_ptr;
}

Channel& Animation::copy_channel(const Channel& source_channel, const std::string& new_name) {
    uint64_t new_id = next_channel_id();
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

Animation Animation::copy() const {
    return copy(m_name + "_copy");
}

Animation Animation::copy(const std::string& new_name) const {
    Animation new_animation(new_name);
    
    // Copy basic properties
    new_animation.m_start_time = m_start_time;
    new_animation.m_end_time = m_end_time;
    
    // Copy all channels using existing copy_channel method
    for (const auto& channel : m_channels) {
        new_animation.copy_channel(*channel, channel->name());
    }
    
    return new_animation;
}

const Channel& Animation::channel(size_t index) const {
    if (index >= m_channels.size()) {
        throw std::out_of_range("Channel index out of range");
    }
    return *m_channels[index];
}

const Channel& Animation::operator[](size_t index) const {
    return channel(index);
}

Channel& Animation::channel(size_t index) {
    if (index >= m_channels.size()) {
        throw std::out_of_range("Channel index out of range");
    }
    return *m_channels[index];
}

Channel& Animation::operator[](size_t index) {
    return channel(index);
}

Channel& Animation::channel(const std::string& channel_name) {
    auto it = std::find_if(m_channels.begin(), m_channels.end(),
                           [&channel_name](const std::unique_ptr<Channel>& ch) { return ch->name() == channel_name; });
    if (it != m_channels.end()) {
        return **it;
    } else {
        throw std::out_of_range("Channel with the specified name does not exist");
    }
}

Channel& Animation::operator[](const std::string& channel_name) {
    return channel(channel_name);
}

Channel& Animation::channel(Id channel_id) {
    return *m_channel_map.at(channel_id);
}

Channel& Animation::operator[](Id channel_id) {
    return channel(channel_id);
}

const Channel& Animation::channel(const std::string& channel_name) const {
    auto it = std::find_if(m_channels.begin(), m_channels.end(),
                           [&channel_name](const std::unique_ptr<Channel>& ch) { return ch->name() == channel_name; });
    if (it != m_channels.end()) {
        return **it;
    } else {
        throw std::out_of_range("Channel with the specified name does not exist");
    }
}

const Channel& Animation::operator[](const std::string& channel_name) const {
    return channel(channel_name);
}

const Channel& Animation::channel(Id channel_id) const {
    return *m_channel_map.at(channel_id);
}

const Channel& Animation::operator[](Id channel_id) const {
    return channel(channel_id);
}

bool Animation::has_channel(const std::string& channel_name) const {
    return std::any_of(m_channels.begin(), m_channels.end(),
                       [&channel_name](const std::unique_ptr<Channel>& ch) { return ch->name() == channel_name; });
}

void Animation::reorder_channel(size_t from_index, size_t to_index) {
    if (from_index >= m_channels.size() || to_index >= m_channels.size()) {
        throw std::out_of_range("Channel index out of range");
    }
    if (from_index == to_index) return;
    
    size_t original_size = m_channels.size(); // Store original size
    auto temp = std::move(m_channels[from_index]);
    m_channels.erase(m_channels.begin() + from_index); // m_channels size is now original_size - 1
    
    size_t new_index = to_index;

    if (to_index == original_size - 1) { // to_index was the last index
        new_index = m_channels.size(); 
    } else if (to_index > from_index) {
        new_index--; // vector is shorter now, so shift left by one
    }

    new_index = std::min(new_index, m_channels.size());
    m_channels.insert(m_channels.begin() + new_index, std::move(temp));
}

void Animation::reorder_channel(const std::string& channel_name, size_t to_index) {
    auto it = std::find_if(m_channels.begin(), m_channels.end(),
                           [&channel_name](const std::unique_ptr<Channel>& ch) { return ch->name() == channel_name; });
    if (it == m_channels.end()) {
        throw std::out_of_range("Channel with the specified name does not exist");
    }
    size_t from_index = std::distance(m_channels.begin(), it);
    reorder_channel(from_index, to_index);
}

void Animation::reorder_channel(Id channel_id, size_t to_index) {
    auto it = std::find_if(m_channels.begin(), m_channels.end(),
                           [channel_id](const std::unique_ptr<Channel>& ch) { return ch->id() == channel_id; });
    if (it == m_channels.end()) {
        throw std::out_of_range("Channel with the specified ID does not exist");
    }
    size_t from_index = std::distance(m_channels.begin(), it);
    reorder_channel(from_index, to_index);
}

void Animation::sort_channels() {
    sort_channels([](const Channel& lhs, const Channel& rhs) {
        return lhs.name() < rhs.name();
    });
}

void Animation::sort_channels(const std::function<bool(const Channel&, const Channel&)>& comparator) {
    // Reordering the owning pointers leaves the Channel objects themselves in
    // place, so m_channel_map stays valid and so does anything the caller is
    // already holding a reference to.
    std::stable_sort(m_channels.begin(), m_channels.end(),
                     [&comparator](const std::unique_ptr<Channel>& lhs,
                                   const std::unique_ptr<Channel>& rhs) {
                         return comparator(*lhs, *rhs);
                     });
}

void Animation::clear() {
    m_channel_map.clear();
    m_channels.clear();
}

void Animation::remove_channel(size_t index) {
    if (index >= m_channels.size()) {
        throw std::out_of_range("Channel index out of range");
    }
    // Remove from map first
    Id channel_id = m_channels[index]->id();
    m_channel_map.erase(channel_id);
    m_channels.erase(m_channels.begin() + index);
}

void Animation::remove_channel(const std::string& channel_name) {
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

void Animation::remove_channel(Id channel_id) {
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

std::vector<std::string> Animation::channel_names() const {
    std::vector<std::string> names;
    names.reserve(m_channels.size());
    for (const auto& channel : m_channels) {
        names.push_back(channel->name());
    }
    return names;
}

const std::vector<std::unique_ptr<Channel>>& Animation::channels() {
    return m_channels;
}

const std::vector<std::unique_ptr<Channel>>& Animation::channels() const {
    return m_channels;
}

void Animation::set_start_time(double start_time) { 
    m_start_time = std::min(start_time, m_end_time); 
}

void Animation::set_end_time(double end_time) { 
    m_end_time = std::max(end_time, m_start_time); 
}

double Animation::length() const { 
    if (m_start_time > m_end_time) {
        throw std::invalid_argument("Start time must be less than or equal to end time");
    }
    return m_end_time - m_start_time;
}

void Animation::set_length(double length) {
    if (length < 0.0) {
        throw std::invalid_argument("Length cannot be negative");
    }
    m_end_time = m_start_time + length;
}

size_t Animation::num_samples(double sample_rate, RangeEnd range_end) const {
    if (sample_rate <= 0.0) {
        throw std::invalid_argument("Sample rate must be positive");
    }
    if (m_channels.empty()) {
        return 0;
    }
    if (length() == 0.0) {
        // No span to divide, so the single sample at the start time.
        return 1;
    }
    return detail::sample_count(length(), sample_rate, range_end);
}

SampleTimes Animation::sample_times(int num_samples, RangeEnd range_end) const {
    return anim::sample_times(m_start_time, m_end_time, num_samples, range_end);
}

SampleTimes Animation::sample_times_by_rate(double sample_rate, RangeEnd range_end) const {
    if (sample_rate <= 0.0) {
        throw std::invalid_argument("Sample rate must be positive");
    }
    // Matches num_samples, which reports nothing to sample until there is a
    // channel to sample, whatever the animation's time range says.
    if (m_channels.empty()) {
        return SampleTimes(m_start_time, 0.0, 0);
    }
    return anim::sample_times_by_rate(m_start_time, m_end_time, sample_rate, range_end);
}

bool Animation::operator==(const Animation& other) const {
    // Compare all settings and channels
    if (m_name != other.m_name) {
        return false;
    }
    
    if (m_start_time != other.m_start_time) {
        return false;
    }
    
    if (m_end_time != other.m_end_time) {
        return false;
    }
    
    if (m_channels.size() != other.m_channels.size()) {
        return false;
    }
    
    // Compare all channels in order
    for (size_t i = 0; i < m_channels.size(); ++i) {
        if (*m_channels[i] != *other.m_channels[i]) {
            return false;
        }
    }
    
    return true;
}

bool Animation::operator!=(const Animation& other) const {
    return !(*this == other);
}

} // namespace anim
