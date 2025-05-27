#ifndef ANIM_ANIMATION_HPP
#define ANIM_ANIMATION_HPP

#include "anim/channel.hpp"
#include <string>
#include <map>
#include <optional>
#include <vector>
#include <limits>
#include <algorithm>
#include <functional>

namespace anim {

class Animation {
public:
    Animation() = default;
    Channel* create_channel(const std::string& name);
    Channel* append_channel(const Channel& channel);
    Channel* insert_channel(size_t index, const Channel& channel);
    Channel* get_channel(size_t index);
    const Channel* get_channel(size_t index) const;
    Channel* get_channel(const std::string& name);
    const Channel* get_channel(const std::string& name) const;
    bool has_channel(const std::string& name) const;
    size_t get_channel_count() const;
    size_t num_channels() const;
    bool remove_channel(const std::string& name);
    bool remove_channel(size_t index);
    std::vector<std::string> get_channel_names() const;
    void clear_channels();
    size_t find_channel_index(const std::string& name) const;
    bool insert_keyframe(const std::string& channel_name, double time, double value, const BezierHandle& in_tangent, const BezierHandle& out_tangent, TangentMode mode);
    bool insert_keyframe(size_t channel_index, double time, double value, const BezierHandle& in_tangent, const BezierHandle& out_tangent, TangentMode mode);
    bool upsert_keyframe(const std::string& channel_name, double time, double value, const BezierHandle& in_tangent, const BezierHandle& out_tangent, TangentMode mode);
    bool upsert_keyframe(size_t channel_index, double time, double value, const BezierHandle& in_tangent, const BezierHandle& out_tangent, TangentMode mode);
    bool update_keyframe_at_time(const std::string& channel_name, double time, double new_value, const BezierHandle& new_in_tangent, const BezierHandle& new_out_tangent, TangentMode new_mode);
    bool update_keyframe_at_time(size_t channel_index, double time, double new_value, const BezierHandle& new_in_tangent, const BezierHandle& new_out_tangent, TangentMode new_mode);
    bool update_keyframe_at_time(const std::string& channel_name, double time, const std::optional<double>& new_value = std::nullopt, const std::optional<BezierHandle>& new_in_tangent = std::nullopt, const std::optional<BezierHandle>& new_out_tangent = std::nullopt, const std::optional<TangentMode>& new_mode = std::nullopt);
    bool update_keyframe_at_time(size_t channel_index, double time, const std::optional<double>& new_value = std::nullopt, const std::optional<BezierHandle>& new_in_tangent = std::nullopt, const std::optional<BezierHandle>& new_out_tangent = std::nullopt, const std::optional<TangentMode>& new_mode = std::nullopt);
    bool update_keyframe(const std::string& channel_name, size_t keyframe_index, double time, double value, const BezierHandle& in_tangent, const BezierHandle& out_tangent, TangentMode mode);
    bool update_keyframe(size_t channel_index, size_t keyframe_index, double time, double value, const BezierHandle& in_tangent, const BezierHandle& out_tangent, TangentMode mode);
    bool update_keyframe(const std::string& channel_name, size_t keyframe_index, const std::optional<double>& time = std::nullopt, const std::optional<double>& value = std::nullopt, const std::optional<BezierHandle>& in_tangent = std::nullopt, const std::optional<BezierHandle>& out_tangent = std::nullopt, const std::optional<TangentMode>& mode = std::nullopt);
    bool update_keyframe(size_t channel_index, size_t keyframe_index, const std::optional<double>& time = std::nullopt, const std::optional<double>& value = std::nullopt, const std::optional<BezierHandle>& in_tangent = std::nullopt, const std::optional<BezierHandle>& out_tangent = std::nullopt, const std::optional<TangentMode>& mode = std::nullopt);
    bool remove_keyframe(const std::string& channel_name, double time);
    bool remove_keyframe(size_t channel_index, double time);
    bool remove_keyframe_at_index(const std::string& channel_name, size_t keyframe_index);
    bool remove_keyframe_at_index(size_t channel_index, size_t keyframe_index);
    std::map<std::string, double> evaluate_channels(double time) const;
    std::map<std::string, std::vector<double>> evaluate_channels_range(double start_time, double end_time, int num_samples) const;
    std::map<std::string, std::vector<double>> evaluate_channels_range_by_rate(double start_time, double end_time, double sample_rate) const;
    std::optional<double> get_start_time() const;
    std::optional<double> get_end_time() const;
    double length() const;
    int num_samples(double sample_rate) const;
    bool is_empty() const;
    bool has_no_keyframes() const;

private:
    std::vector<Channel> m_channels;
};

} // namespace anim

#endif // ANIM_ANIMATION_HPP
