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
    inline const std::string& name() const { return m_name; }
    inline void set_name(const std::string& name) { m_name = name; }

    const Keyframe& create_keyframe(double time, double value,
        Function function = Function::bezier, HandleMode handle_mode = HandleMode::smooth);

    const Keyframe& create_keyframe(const Point& position,
        Function function = Function::bezier, HandleMode handle_mode = HandleMode::smooth);

    const Keyframe& create_keyframe(double time, double value,
        const Point& in_handle, const Point& out_handle,
        Function function = Function::bezier, HandleMode handle_mode = HandleMode::aligned);

    const Keyframe& emplace_keyframe(Keyframe&& keyframe);
    
    bool has_keyframe(double time) const;
    void delete_keyframe(size_t index);
    const Keyframe& keyframe(size_t index) const;
    inline const Keyframe& operator[](size_t index) const { return keyframe(index); }
    const Keyframe& prev_keyframe(double time) const;
    const Keyframe& next_keyframe(double time) const;
    const Keyframe& closest_keyframe(double time) const;
    inline size_t size() const { return m_keyframes.size(); }
    inline size_t num_keyframes() const { return m_keyframes.size(); }
    inline bool empty() const { return m_keyframes.empty(); }

    void update_keyframe(size_t index, const Keyframe& keyframe);
    void set_keyframe_time(size_t index, double time);
    void set_keyframe_value(size_t index, double value);
    void set_keyframe_in_handle(size_t index, const Point& in_handle);
    void set_keyframe_out_handle(size_t index, const Point& out_handle);
    void set_keyframe_function(size_t index, Function function);
    void set_keyframe_handle_mode(size_t index, HandleMode handle_mode);
  
    double evaluate(double time) const;
    std::vector<double> evaluate_range(double start_time, double end_time, int num_samples) const;
    std::vector<double> evaluate_range_by_rate(double start_time, double end_time, double sample_rate) const;
    
    double start_time() const;
    double end_time() const;
    double length() const;
    size_t num_samples(double sample_rate) const;

private:
    std::string m_name;
    std::vector<Keyframe> m_keyframes;


    using KeyframeIt = std::vector<Keyframe>::iterator;

    const Keyframe& create_default_keyframe(const Point& position, Function function, HandleMode handle_mode);
    const Keyframe& insert_keyframe(Keyframe&& keyframe, bool source_is_out_handle = true);
    const Keyframe& insert_keyframe(KeyframeIt it, Keyframe&& keyframe, bool source_is_out_handle = true);    
    
    void update_local_handles(KeyframeIt it, bool source_is_out_handle = true);
    void update_handles(Keyframe& keyframe, Keyframe* prev_keyframe_ptr, Keyframe* next_keyframe_ptr, bool source_is_out_handle = true);
};

} // namespace anim

#endif // ANIM_CHANNEL_HPP
