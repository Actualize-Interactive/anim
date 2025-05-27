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

class Channel {
public:
    Channel() = default;
    explicit Channel(const std::string& name) : m_name(name) {}
    const std::string& name() const { return m_name; }
    void set_name(const std::string& name) { m_name = name; }

    const Keyframe& create_keyframe(const Point& position, 
        const Point& in_handle = Point(), 
        const Point& out_handle = Point(),
        FunctionType function_type = FunctionType::bezier,
        HandleType handle_type = HandleType::smooth) {
        return insert_keyframe(Keyframe(position, in_handle, out_handle, function_type, handle_type));
    }

    const Keyframe& create_keyframe(double time, double value,
        const Point& in_handle = Point(),
        const Point& out_handle = Point(),
        FunctionType function_type = FunctionType::bezier,
        HandleType handle_type = HandleType::smooth) {
        return insert_keyframe(Keyframe(time, value, in_handle, out_handle, function_type, handle_type));
    }

    const Keyframe& emplace_keyframe(Keyframe&& keyframe) {
        return insert_keyframe(std::move(keyframe));
    }
    
    bool has_keyframe(double time) const;
    void delete_keyframe(size_t index);
    const Keyframe& keyframe(size_t index) const;
    const Keyframe& prev_keyframe(double time) const;
    const Keyframe& next_keyframe(double time) const;
    const Keyframe& closest_keyframe(double time) const;
    size_t size() const { return m_keyframes.size(); }
    bool empty() const { return m_keyframes.empty(); }

    void update_keyframe(size_t index, const Keyframe& keyframe);
    void set_keyframe_time(size_t index, double time);
    void set_keyframe_value(size_t index, double value);
    void set_keyframe_in_handle(size_t index, const Point& in_handle);
    void set_keyframe_out_handle(size_t index, const Point& out_handle);
    void set_keyframe_function_type(size_t index, FunctionType function_type);
    void set_keyframe_handle_type(size_t index, HandleType handle_type);
  
    double evaluate(double time) const;
    std::vector<double> evaluate_range(double start_time, double end_time, int num_samples) const;
    std::vector<double> evaluate_range_by_rate(double start_time, double end_time, double sample_rate) const;
    
    double start_time() const;
    double end_time() const;
    size_t num_samples(double sample_rate) const;

private:
    std::string m_name;
    std::vector<Keyframe> m_keyframes;

    const Keyframe& insert_keyframe(Keyframe&& keyframe);

    using KeyframeIt = std::vector<Keyframe>::iterator;
    void update_local_handles(KeyframeIt it);

    void update_prev_out_handle(Keyframe& keyframe, const Keyframe& next_keyframe);
    void update_next_in_handle(Keyframe& keyframe, const Keyframe& prev_keyframe);

    void update_handles(Keyframe& keyframe, Keyframe* prev_keyframe_ptr, Keyframe* next_keyframe_ptr);
};

} // namespace anim

#endif // ANIM_CHANNEL_HPP
