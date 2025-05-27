#include "anim/channel.hpp"

namespace anim {

void Channel::upsert_keyframe(double time, double value, 
                     const BezierHandle& in_handle, 
                     const BezierHandle& out_handle, 
                     TangentMode mode) {
    auto it = std::find_if(m_keyframes.begin(), m_keyframes.end(), 
                          [time](const Keyframe& kf) {
                              constexpr double epsilon = 1e-10;
                              return std::abs(kf.time() - time) < epsilon;
                          });
    if (it != m_keyframes.end()) {
        it->set_value(value);
        it->set_in_handle(in_handle);
        it->set_out_handle(out_handle);
        it->set_mode(mode);
    } else {
        m_keyframes.emplace_back(time, value, in_handle, out_handle, mode);
        sort_keyframes_internal();
    }
    recalculate_dependent_tangents_internal();
}

void Channel::set_keyframe_at_time(double time, double value, 
                     const BezierHandle& in_handle, 
                     const BezierHandle& out_handle, 
                     TangentMode mode) {
    // Maintain backward compatibility
    upsert_keyframe(time, value, in_handle, out_handle, mode);
}

bool Channel::insert_keyframe(double time, double value, 
                    const BezierHandle& in_handle, 
                    const BezierHandle& out_handle, 
                    TangentMode mode) {
    // Check if keyframe already exists at the given time
    if (has_keyframe_at_time(time)) {
        return false;
    }
    
    m_keyframes.emplace_back(time, value, in_handle, out_handle, mode);
    sort_keyframes_internal();
    recalculate_dependent_tangents_internal();
    return true;
}

bool Channel::append_keyframe(double time, double value, 
                    const BezierHandle& in_handle, 
                    const BezierHandle& out_handle, 
                    TangentMode mode) {
    // Check if keyframe already exists at the given time
    if (has_keyframe_at_time(time)) {
        return false;
    }
    
    // Check if time is greater than all existing keyframe times
    if (!m_keyframes.empty() && time <= m_keyframes.back().time()) {
        throw std::invalid_argument("Append time must be greater than all existing keyframe times");
    }
    
    m_keyframes.emplace_back(time, value, in_handle, out_handle, mode);
    recalculate_dependent_tangents_internal();
    return true;
}

void Channel::update_keyframe(size_t index, double time, double value, 
                         const BezierHandle& in_handle, 
                         const BezierHandle& out_handle, 
                         TangentMode mode) {
    if (index >= m_keyframes.size()) {
        throw std::out_of_range("Keyframe index out of range");
    }
    
    m_keyframes[index].set_time(time);
    m_keyframes[index].set_value(value);
    m_keyframes[index].set_in_handle(in_handle);
    m_keyframes[index].set_out_handle(out_handle);
    m_keyframes[index].set_mode(mode);
    
    sort_keyframes_internal();
    recalculate_dependent_tangents_internal();
}

void Channel::update_keyframe(size_t index, 
                         const std::optional<double>& time, 
                         const std::optional<double>& value, 
                         const std::optional<BezierHandle>& in_handle, 
                         const std::optional<BezierHandle>& out_handle, 
                         const std::optional<TangentMode>& mode) {
    if (index >= m_keyframes.size()) {
        throw std::out_of_range("Keyframe index out of range");
    }
    
    // Only update fields that are provided
    if (time) {
        m_keyframes[index].set_time(*time);
    }
    
    if (value) {
        m_keyframes[index].set_value(*value);
    }
    
    if (in_handle) {
        m_keyframes[index].set_in_handle(*in_handle);
    }
    
    if (out_handle) {
        m_keyframes[index].set_out_handle(*out_handle);
    }
    
    if (mode) {
        m_keyframes[index].set_mode(*mode);
    }
    
    sort_keyframes_internal();
    recalculate_dependent_tangents_internal();
}

bool Channel::update_keyframe_at_time(double time, 
                                 double new_value, 
                                 const BezierHandle& new_in_handle, 
                                 const BezierHandle& new_out_handle, 
                                 TangentMode new_mode) {
    auto it = std::find_if(m_keyframes.begin(), m_keyframes.end(), 
                          [time](const Keyframe& kf) {
                              constexpr double epsilon = 1e-10;
                              return std::abs(kf.time() - time) < epsilon;
                          });
    
    if (it == m_keyframes.end()) {
        return false;
    }
    
    it->set_value(new_value);
    it->set_in_handle(new_in_handle);
    it->set_out_handle(new_out_handle);
    it->set_mode(new_mode);
    
    recalculate_dependent_tangents_internal();
    return true;
}

bool Channel::update_keyframe_at_time(double time, 
                                 const std::optional<double>& new_value, 
                                 const std::optional<BezierHandle>& new_in_handle, 
                                 const std::optional<BezierHandle>& new_out_handle, 
                                 const std::optional<TangentMode>& new_mode) {
    auto it = std::find_if(m_keyframes.begin(), m_keyframes.end(), 
                          [time](const Keyframe& kf) {
                              constexpr double epsilon = 1e-10;
                              return std::abs(kf.time() - time) < epsilon;
                          });
    
    if (it == m_keyframes.end()) {
        return false;
    }
    
    // Only update fields that are provided
    if (new_value) {
        it->set_value(*new_value);
    }
    
    if (new_in_handle) {
        it->set_in_handle(*new_in_handle);
    }
    
    if (new_out_handle) {
        it->set_out_handle(*new_out_handle);
    }
    
    if (new_mode) {
        it->set_mode(*new_mode);
    }
    
    recalculate_dependent_tangents_internal();
    return true;
}

void Channel::set_keyframe(size_t index, double time, double new_value, 
                          const BezierHandle& new_in_handle, 
                          const BezierHandle& new_out_handle, 
                          TangentMode new_mode) {
    // Maintain backward compatibility
    if (index >= m_keyframes.size()) {
        throw std::out_of_range("Keyframe index out of range");
    }
    m_keyframes[index].set_time(time);
    m_keyframes[index].set_value(new_value);
    m_keyframes[index].set_in_handle(new_in_handle);
    m_keyframes[index].set_out_handle(new_out_handle);
    m_keyframes[index].set_mode(new_mode);
    
    recalculate_dependent_tangents_internal();
}

bool Channel::has_keyframe_at_time(double time) const {
    return std::any_of(m_keyframes.begin(), m_keyframes.end(), 
                       [time](const Keyframe& kf) {
                           constexpr double epsilon = 1e-10;
                           return std::abs(kf.time() - time) < epsilon;
                       });
}

bool Channel::has_keyframe(size_t index) const {
    return index < m_keyframes.size();
}

bool Channel::remove_keyframe_at_time(double time) {
    size_t original_size = m_keyframes.size();
    
    m_keyframes.erase(
        std::remove_if(m_keyframes.begin(), m_keyframes.end(),
                      [time](const Keyframe& kf) {
                          constexpr double epsilon = 1e-10;
                          return std::abs(kf.time() - time) < epsilon;
                      }),
        m_keyframes.end()
    );
    
    bool removed = m_keyframes.size() < original_size;
    if (removed) {
        recalculate_dependent_tangents_internal();
    }
    return removed;
}

bool Channel::remove_keyframe(size_t index) {
    if (index >= m_keyframes.size()) {
        return false;
    }
    
    m_keyframes.erase(m_keyframes.begin() + index);
    recalculate_dependent_tangents_internal();
    return true;
}

std::optional<Keyframe> Channel::get_keyframe_at_time(double time) const {
    auto it = std::find_if(m_keyframes.begin(), m_keyframes.end(), 
                          [time](const Keyframe& kf) {
                              constexpr double epsilon = 1e-10;
                              return std::abs(kf.time() - time) < epsilon;
                          });
    if (it != m_keyframes.end()) {
        return *it;
    }
    return std::nullopt;
}

Keyframe& Channel::get_keyframe(size_t index) {
    if (index >= m_keyframes.size()) {
        throw std::out_of_range("Keyframe index out of range");
    }
    return m_keyframes[index];
}

const std::vector<Keyframe>& Channel::get_all_keyframes() const {
    return m_keyframes;
}

double Channel::evaluate(double time) const {
    if (m_keyframes.empty()) {
        return 0.0;
    }
    
    if (m_keyframes.size() == 1) {
        return m_keyframes[0].value();
    }
    
    if (time <= m_keyframes.front().time()) {
        return m_keyframes.front().value();
    }
    
    if (time >= m_keyframes.back().time()) {
        return m_keyframes.back().value();
    }
    
    // Find the keyframes that bracket the requested time
    auto upper = std::upper_bound(m_keyframes.begin(), m_keyframes.end(), time,
                                 [](double t, const Keyframe& kf) {
                                     return t < kf.time();
                                 });
    auto lower = upper - 1;
    
    const Keyframe& start_kf = *lower;
    const Keyframe& end_kf = *upper;
    
    // Handle linear tangent mode
    if (start_kf.mode() == TangentMode::linear) {
        double t = (time - start_kf.time()) / (end_kf.time() - start_kf.time());
        return start_kf.value() + t * (end_kf.value() - start_kf.value());
    }

    // Handle constant tangent mode
    if (start_kf.mode() == TangentMode::constant) {
        return start_kf.value();
    }
    
    double start_time = start_kf.time();
    double end_time = end_kf.time();
    double t = (time - start_time) / (end_time - start_time);
    
    BezierHandle p0(start_kf.time(), start_kf.value());
    BezierHandle p1(start_kf.out_handle().time, start_kf.out_handle().value);
    BezierHandle p2(end_kf.in_handle().time, end_kf.in_handle().value);
    BezierHandle p3(end_kf.time(), end_kf.value());
    
    return bezier_utils::evaluate_cubic_bezier(p0, p1, p2, p3, t).value;
}

std::vector<double> Channel::evaluate_range(double start_time, double end_time, int num_samples) const {
    if (num_samples <= 1) {
        return {evaluate(start_time)};
    }
    if (start_time > end_time) {
        throw std::invalid_argument("Start time must be less than or equal to end time");
    }
    
    // For equal times, just return the value at that time
    if (start_time == end_time) {
        return {evaluate(start_time)};
    }
    
    std::vector<double> result(num_samples);
    double step = (end_time - start_time) / (num_samples - 1);
    
    for (int i = 0; i < num_samples; i++) {
        double time = start_time + i * step;
        result[i] = evaluate(time);
    }
    
    return result;
}

std::vector<double> Channel::evaluate_range_by_rate(double start_time, double end_time, double sample_rate) const {
    if (sample_rate <= 0.0) {
        throw std::invalid_argument("Sample rate must be positive");
    }
    if (start_time > end_time) {
        throw std::invalid_argument("Start time must be less than or equal to end time");
    }
    
    // For equal times, just return the value at that time
    if (start_time == end_time) {
        return {evaluate(start_time)};
    }
    
    int num_samples = static_cast<int>(std::ceil((end_time - start_time) * sample_rate)) + 1;
    return evaluate_range(start_time, end_time, num_samples);
}

bool Channel::is_empty() const {
    return m_keyframes.empty();
}

std::optional<double> Channel::get_start_time() const {
    if (m_keyframes.empty()) {
        return std::nullopt;
    }
    return m_keyframes.front().time();
}

std::optional<double> Channel::get_end_time() const {
    if (m_keyframes.empty()) {
        return std::nullopt;
    }
    return m_keyframes.back().time();
}

bool Channel::set_keyframe_time(double old_time, double new_time) {
    auto it = std::find_if(m_keyframes.begin(), m_keyframes.end(), 
                          [old_time](const Keyframe& kf) {
                              constexpr double epsilon = 1e-10;
                              return std::abs(kf.time() - old_time) < epsilon;
                          });
    
    if (it == m_keyframes.end()) {
        return false;
    }
    
    it->set_time(new_time);
    sort_keyframes_internal();
    recalculate_dependent_tangents_internal();
    return true;
}

bool Channel::set_keyframe_value(double time, double new_value) {
    auto it = std::find_if(m_keyframes.begin(), m_keyframes.end(), 
                          [time](const Keyframe& kf) {
                              constexpr double epsilon = 1e-10;
                              return std::abs(kf.time() - time) < epsilon;
                          });
    
    if (it == m_keyframes.end()) {
        return false;
    }
    
    it->set_value(new_value);
    recalculate_dependent_tangents_internal();
    return true;
}

bool Channel::set_keyframe_in_handle(double time, const BezierHandle& new_in_handle) {
    auto it = std::find_if(m_keyframes.begin(), m_keyframes.end(), 
                          [time](const Keyframe& kf) {
                              constexpr double epsilon = 1e-10;
                              return std::abs(kf.time() - time) < epsilon;
                          });
    
    if (it == m_keyframes.end()) {
        return false;
    }
    
    it->set_in_handle(new_in_handle);
    recalculate_dependent_tangents_internal();
    return true;
}

bool Channel::set_keyframe_out_handle(double time, const BezierHandle& new_out_handle) {
    auto it = std::find_if(m_keyframes.begin(), m_keyframes.end(), 
                          [time](const Keyframe& kf) {
                              constexpr double epsilon = 1e-10;
                              return std::abs(kf.time() - time) < epsilon;
                          });
    
    if (it == m_keyframes.end()) {
        return false;
    }
    
    it->set_out_handle(new_out_handle);
    recalculate_dependent_tangents_internal();
    return true;
}

bool Channel::set_keyframe_tangent_mode(double time, TangentMode new_mode) {
    auto it = std::find_if(m_keyframes.begin(), m_keyframes.end(), 
                          [time](const Keyframe& kf) {
                              constexpr double epsilon = 1e-10;
                              return std::abs(kf.time() - time) < epsilon;
                          });
    
    if (it == m_keyframes.end()) {
        return false;
    }
    
    it->set_mode(new_mode);
    recalculate_dependent_tangents_internal();
    return true;
}

void Channel::sort_keyframes_internal() {
    std::sort(m_keyframes.begin(), m_keyframes.end(),
             [](const Keyframe& a, const Keyframe& b) {
                 return a.time() < b.time();
             });
}

void Channel::recalculate_dependent_tangents_internal() {
    if (m_keyframes.size() < 2) {
        return;
    }
    
    for (size_t i = 0; i < m_keyframes.size(); i++) {
        Keyframe& current = m_keyframes[i];
          if (current.mode() == TangentMode::flat) {
            // For flat tangents, use horizontal handles
            BezierHandle current_point(current.time(), current.value());
            double time_offset = 0.1; // Default offset
            
            // Adjust offset based on neighbors if available
            if (i > 0) {
                time_offset = std::min(time_offset, (current.time() - m_keyframes[i-1].time()) / 3.0);
            }
            if (i < m_keyframes.size() - 1) {
                time_offset = std::min(time_offset, (m_keyframes[i+1].time() - current.time()) / 3.0);
            }
            
            // Use very small time offset for flat tangents to make curve flatter near the keyframe
            time_offset *= 0.1;
            
            BezierHandle in_handle(current.time() - time_offset, current.value());
            BezierHandle out_handle(current.time() + time_offset, current.value());
            
            current.set_in_handle(in_handle);
            current.set_out_handle(out_handle);
        }
        else if (current.mode() == TangentMode::manual) {
            // For manual, we ensure the handles are colinear and of equal magnitude
            // Get the current handles
            BezierHandle in_handle = current.in_handle();
            BezierHandle out_handle = current.out_handle();
            
            // Calculate vectors from keyframe to handles
            BezierHandle kf_point(current.time(), current.value());
            BezierHandle in_vec = in_handle - kf_point;
            BezierHandle out_vec = out_handle - kf_point;
            
            // Calculate magnitudes of both vectors
            double in_mag = std::sqrt(in_vec.time * in_vec.time + in_vec.value * in_vec.value);
            double out_mag = std::sqrt(out_vec.time * out_vec.time + out_vec.value * out_vec.value);
            
            // If either handle is very close to the keyframe, keep it as is
            if (in_mag < 1e-6 || out_mag < 1e-6) {
                continue;
            }
            
            // Use the average magnitude for both handles
            double avg_mag = (in_mag + out_mag) / 2.0;
            
            // Normalize and rescale vectors
            in_vec = in_vec * (avg_mag / in_mag);
            out_vec = out_vec * (avg_mag / out_mag);
            
            // Make out_vec opposite to in_vec to ensure collinearity
            out_vec = in_vec * -1;
            
            // Update the handles
            in_handle = kf_point + in_vec;
            out_handle = kf_point + out_vec;
            
            current.set_in_handle(in_handle);
            current.set_out_handle(out_handle);
        }
        else if (current.mode() == TangentMode::smooth) {
            BezierHandle current_point(current.time(), current.value());
            
            // Default handles point horizontally
            BezierHandle in_handle(current.time() - 0.1, current.value());
            BezierHandle out_handle(current.time() + 0.1, current.value());
            
            // Adjust handles based on previous and next keyframes if they exist
            if (i > 0 && i < m_keyframes.size() - 1) {
                const Keyframe& prev = m_keyframes[i-1];
                const Keyframe& next = m_keyframes[i+1];
                
                BezierHandle prev_point(prev.time(), prev.value());
                BezierHandle next_point(next.time(), next.value());
                
                // Calculate the slope as the average of the slopes to previous and next
                double in_slope = (current.value() - prev.value()) / (current.time() - prev.time());
                double out_slope = (next.value() - current.value()) / (next.time() - current.time());
                double avg_slope = (in_slope + out_slope) / 2.0;
                
                double tangent_length_in = (current.time() - prev.time()) / 3.0;
                double tangent_length_out = (next.time() - current.time()) / 3.0;
                
                in_handle.time = current.time() - tangent_length_in;
                in_handle.value = current.value() - tangent_length_in * avg_slope;
                
                out_handle.time = current.time() + tangent_length_out;
                out_handle.value = current.value() + tangent_length_out * avg_slope;
            }
            else if (i == 0 && m_keyframes.size() > 1) {
                // First keyframe - only use next
                const Keyframe& next = m_keyframes[i+1];
                
                double slope = (next.value() - current.value()) / (next.time() - current.time());
                double tangent_length = (next.time() - current.time()) / 3.0;
                
                in_handle.time = current.time() - tangent_length;
                in_handle.value = current.value() - tangent_length * slope;
                
                out_handle.time = current.time() + tangent_length;
                out_handle.value = current.value() + tangent_length * slope;
            }
            else if (i == m_keyframes.size() - 1 && m_keyframes.size() > 1) {
                // Last keyframe - only use previous
                const Keyframe& prev = m_keyframes[i-1];
                
                double slope = (current.value() - prev.value()) / (current.time() - prev.time());
                double tangent_length = (current.time() - prev.time()) / 3.0;
                
                in_handle.time = current.time() - tangent_length;
                in_handle.value = current.value() - tangent_length * slope;
                
                out_handle.time = current.time() + tangent_length;
                out_handle.value = current.value() + tangent_length * slope;
            }
            
            current.set_in_handle(in_handle);
            current.set_out_handle(out_handle);
        }
        // For BROKEN mode, no special handling needed as handles can be independently adjusted
    }
}

} // namespace anim
