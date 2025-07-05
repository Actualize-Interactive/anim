#ifndef ANIM_CHANNEL_HPP
#define ANIM_CHANNEL_HPP

#include "anim/keyframe.hpp"
#include "anim/handle_utils.hpp"
#include "anim/id.hpp"
#include "anim/extend.hpp"
#include <vector>
#include <algorithm>
#include <optional>
#include <stdexcept>
#include <cmath>
#include <string>
#include <memory>

namespace anim {

// Forward declaration
class Animation;

class Channel {
    friend class Animation; // Allow Animation to access protected constructor

protected:
    // Delete public constructors and copy operations to prevent direct creation and ID duplication
    Channel() = delete;
    Channel(const Channel&) = delete;
    Channel& operator=(const Channel&) = delete;
    Channel(Channel&&) = default;
    Channel& operator=(Channel&&) = default;
    
    // Only Animation can create Channel objects with specific IDs
    explicit Channel(const std::string& name, uint64_t id) : m_name(name), m_id(id) {}

public:
    inline const std::string& name() const { return m_name; }
    inline void set_name(const std::string& name) { m_name = name; }
    
    // ID accessor
    inline Id id() const { return m_id; }

    const Keyframe& create_keyframe(double time, double value,
        Function function = Function::Bezier, HandleMode handle_mode = HandleMode::Smooth);

    const Keyframe& create_keyframe(const Point& position,
        Function function = Function::Bezier, HandleMode handle_mode = HandleMode::Smooth);

    const Keyframe& create_keyframe(double time, double value,
        const Point& in_handle, const Point& out_handle,
        Function function = Function::Bezier, HandleMode handle_mode = HandleMode::Aligned);

    const Keyframe& create_keyframe(const Point& position,
        const Point& in_handle, const Point& out_handle,
        Function function = Function::Bezier, HandleMode handle_mode = HandleMode::Aligned);

    const Keyframe& create_keyframe(const Keyframe& reference_keyframe);

    const Keyframe& emplace_keyframe(Keyframe&& keyframe);
    
    bool has_keyframe(double time) const;
    void delete_keyframe(size_t index);
    const Keyframe& keyframe(size_t index) const;
    inline const Keyframe& operator[](size_t index) const { return keyframe(index); }
    const Keyframe& prev_keyframe(double time) const;
    const Keyframe& next_keyframe(double time) const;
    const Keyframe& closest_keyframe(double time) const;
    const std::vector<Keyframe>& keyframes() const;
    inline size_t size() const { return m_keyframes.size(); }
    inline size_t num_keyframes() const { return m_keyframes.size(); }
    inline bool empty() const { return m_keyframes.empty(); }

    void update_keyframe(size_t index, const Keyframe& keyframe);
    void set_keyframe_time(size_t index, double time);
    void set_keyframe_value(size_t index, double value);
    void set_keyframe_position(size_t index, const Point& position);
    void set_keyframe_position(size_t index, double time, double value);
    void set_keyframe_in_handle(size_t index, const Point& in_handle);
    void set_keyframe_out_handle(size_t index, const Point& out_handle);
    void set_keyframe_function(size_t index, Function function);
    void set_keyframe_handle_mode(size_t index, HandleMode handle_mode);
  
    double evaluate(double time, double* prev_t = nullptr) const;
    std::vector<double> evaluate_range(double start_time, double end_time, int num_samples) const;
    std::vector<double> evaluate_range_by_rate(double start_time, double end_time, double sample_rate) const;
    
    double start_time() const;
    double end_time() const;
    double length() const;
    size_t num_samples(double sample_rate) const;

    // Extend behavior methods
    Extend extend_start() const;
    Extend extend_end() const;
    void set_extend_start(Extend extend);
    void set_extend_end(Extend extend);

    // Copy keyframes from another channel (for use by Animation::copy_channel)
    void copy_keyframes_from(const Channel& source);

    // Equality operators
    bool operator==(const Channel& other) const;
    bool operator!=(const Channel& other) const;

private:
    std::string m_name;
    const Id m_id; // Immutable ID - each channel has a unique identity
    mutable std::vector<Keyframe> m_keyframes;
    mutable Keyframe m_last_keyframe_cache; // Cache for the last keyframe's original state
    mutable bool m_cache_valid = false; // Track if cache contains valid data
    mutable size_t m_cached_keyframe_index = SIZE_MAX; // Index of the keyframe in cache
    
    // Extend behavior settings
    Extend m_extend_start = Extend::Hold;
    Extend m_extend_end = Extend::Hold;

    using KeyframeIt = std::vector<Keyframe>::iterator;

    const Keyframe& create_default_keyframe(const Point& position, Function function, HandleMode handle_mode);
    const Keyframe& insert_keyframe(Keyframe&& keyframe, GrabbedHandle grabbed_handle = GrabbedHandle::none);
    const Keyframe& insert_keyframe(KeyframeIt it, Keyframe&& keyframe, GrabbedHandle grabbed_handle = GrabbedHandle::none);    

    void update_keyframe_position(KeyframeIt it, const Point& position);
    void clamp_keyframe_time(KeyframeIt it, double time);
    
    void update_local_handles(KeyframeIt it, GrabbedHandle grabbed_handle = GrabbedHandle::none);
    void update_handles(
        Keyframe& keyframe, 
        Keyframe* prev_keyframe_ptr, Keyframe* next_keyframe_ptr, 
        GrabbedHandle grabbed_handle = GrabbedHandle::none);
    
    void apply_last_keyframe_inheritance(bool restore_cache = true) const;
    void invalidate_cache() const;
};

} // namespace anim

#endif // ANIM_CHANNEL_HPP
