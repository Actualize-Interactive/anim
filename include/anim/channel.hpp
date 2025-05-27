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
    void upsert_keyframe(double time, double value, const BezierHandle& in_handle, const BezierHandle& out_handle, TangentMode mode);
    void set_keyframe_at_time(double time, double value, const BezierHandle& in_handle, const BezierHandle& out_handle, TangentMode mode);
    bool insert_keyframe(double time, double value, const BezierHandle& in_handle, const BezierHandle& out_handle, TangentMode mode);
    bool append_keyframe(double time, double value, const BezierHandle& in_handle, const BezierHandle& out_handle, TangentMode mode);
    void update_keyframe(size_t index, double time, double value, const BezierHandle& in_handle, const BezierHandle& out_handle, TangentMode mode);
    void update_keyframe(size_t index, const std::optional<double>& time = std::nullopt, const std::optional<double>& value = std::nullopt, const std::optional<BezierHandle>& in_handle = std::nullopt, const std::optional<BezierHandle>& out_handle = std::nullopt, const std::optional<TangentMode>& mode = std::nullopt);
    bool update_keyframe_at_time(double time, double new_value, const BezierHandle& new_in_handle, const BezierHandle& new_out_handle, TangentMode new_mode);
    bool update_keyframe_at_time(double time, const std::optional<double>& new_value = std::nullopt, const std::optional<BezierHandle>& new_in_handle = std::nullopt, const std::optional<BezierHandle>& new_out_handle = std::nullopt, const std::optional<TangentMode>& new_mode = std::nullopt);
    void set_keyframe(size_t index, double time, double new_value, const BezierHandle& new_in_handle, const BezierHandle& new_out_handle, TangentMode new_mode);
    bool has_keyframe_at_time(double time) const;
    bool has_keyframe(size_t index) const;
    size_t keyframe_count() const { return m_keyframes.size(); }
    bool remove_keyframe_at_time(double time);
    bool remove_keyframe(size_t index);
    std::optional<Keyframe> get_keyframe_at_time(double time) const;
    Keyframe& get_keyframe(size_t index);
    const std::vector<Keyframe>& get_all_keyframes() const;
    double evaluate(double time) const;
    std::vector<double> evaluate_range(double start_time, double end_time, int num_samples) const;
    std::vector<double> evaluate_range_by_rate(double start_time, double end_time, double sample_rate) const;
    bool is_empty() const;
    std::optional<double> get_start_time() const;
    std::optional<double> get_end_time() const;
    const std::string& name() const { return m_name; }
    void set_name(const std::string& name) { m_name = name; }
    bool set_keyframe_time(double old_time, double new_time);
    bool set_keyframe_value(double time, double new_value);
    bool set_keyframe_in_handle(double time, const BezierHandle& new_in_handle);
    bool set_keyframe_out_handle(double time, const BezierHandle& new_out_handle);
    bool set_keyframe_tangent_mode(double time, TangentMode new_mode);

private:
    std::string m_name;
    std::vector<Keyframe> m_keyframes;
    void sort_keyframes_internal();
    void recalculate_dependent_tangents_internal();
};

} // namespace anim

#endif // ANIM_CHANNEL_HPP
