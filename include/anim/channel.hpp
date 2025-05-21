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

    void set_keyframe(double time, double value, 
                     const BezierHandle& in_tangent, 
                     const BezierHandle& out_tangent, 
                     TangentMode mode);

    void set_keyframe_time(double old_time, double new_time);
    void set_keyframe_value(double time, double new_value);
    void set_keyframe_in_tangent(double time, const BezierHandle& new_in_tangent);
    void set_keyframe_out_tangent(double time, const BezierHandle& new_out_tangent);
    void set_keyframe_tangent_mode(double time, TangentMode new_mode);
    bool remove_keyframe(double time);
    
    std::optional<Keyframe> get_keyframe(double time) const;
    const std::vector<Keyframe>& get_all_keyframes() const;

    double evaluate(double time) const;
    std::vector<double> evaluate_range(double start_time, double end_time, int num_samples) const;
    std::vector<double> evaluate_range_by_rate(double start_time, double end_time, double sample_rate) const;

    bool is_empty() const;
    std::optional<double> get_start_time() const;
    std::optional<double> get_end_time() const;

    // Name property getters and setters
    const std::string& name() const { return m_name; }
    void set_name(const std::string& name) { m_name = name; }

private:
    std::string m_name;
    std::vector<Keyframe> m_keyframes;

    void sort_keyframes_internal();
    void recalculate_dependent_tangents_internal();
};

} // namespace anim

#endif // ANIM_CHANNEL_HPP
