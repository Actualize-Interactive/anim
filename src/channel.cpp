#include "anim/channel.hpp"

namespace anim {

void Channel::set_keyframe_at_time(double time, double value, 
                         const BezierHandle& in_tangent, 
                         const BezierHandle& out_tangent, 
                         TangentMode mode) {
    auto it = std::find_if(m_keyframes.begin(), m_keyframes.end(), 
                          [time](const Keyframe& kf) {
                              constexpr double epsilon = 1e-10;
                              return std::abs(kf.time() - time) < epsilon;
                          });
    if (it != m_keyframes.end()) {
        it->set_value(value);
        it->set_in_tangent(in_tangent);
        it->set_out_tangent(out_tangent);
        it->set_mode(mode);
    } else {
        m_keyframes.emplace_back(time, value, in_tangent, out_tangent, mode);
        sort_keyframes_internal();
    }
    recalculate_dependent_tangents_internal();
}
void Channel::set_keyframe(size_t index, double time, double new_value, 
                          const BezierHandle& new_in_tangent, 
                          const BezierHandle& new_out_tangent, 
                          TangentMode new_mode) {
    if (index >= m_keyframes.size()) {
        throw std::out_of_range("Keyframe index out of range");
    }
    m_keyframes[index].set_time(time);
    m_keyframes[index].set_value(new_value);
    m_keyframes[index].set_in_tangent(new_in_tangent);
    m_keyframes[index].set_out_tangent(new_out_tangent);
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

    // Handle stepped tangent mode
    if (start_kf.mode() == TangentMode::stepped) {
        return start_kf.value();
    }
    
    double start_time = start_kf.time();
    double end_time = end_kf.time();
    double t = (time - start_time) / (end_time - start_time);
    
    Point2D p0(start_kf.time(), start_kf.value());
    Point2D p1(start_kf.out_tangent().time, start_kf.out_tangent().value);
    Point2D p2(end_kf.in_tangent().time, end_kf.in_tangent().value);
    Point2D p3(end_kf.time(), end_kf.value());
    
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
            Point2D current_point(current.time(), current.value());
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
            
            BezierHandle in_tangent(current.time() - time_offset, current.value());
            BezierHandle out_tangent(current.time() + time_offset, current.value());
            
            current.set_in_tangent(in_tangent);
            current.set_out_tangent(out_tangent);
        }
        else if (current.mode() == TangentMode::smoothAuto) {
            Point2D current_point(current.time(), current.value());
            
            // Default tangents point horizontally
            BezierHandle in_tangent(current.time() - 0.1, current.value());
            BezierHandle out_tangent(current.time() + 0.1, current.value());
            
            // Adjust tangents based on previous and next keyframes if they exist
            if (i > 0 && i < m_keyframes.size() - 1) {
                const Keyframe& prev = m_keyframes[i-1];
                const Keyframe& next = m_keyframes[i+1];
                
                Point2D prev_point(prev.time(), prev.value());
                Point2D next_point(next.time(), next.value());
                
                // Calculate the slope as the average of the slopes to previous and next
                double in_slope = (current.value() - prev.value()) / (current.time() - prev.time());
                double out_slope = (next.value() - current.value()) / (next.time() - current.time());
                double avg_slope = (in_slope + out_slope) / 2.0;
                
                double tangent_length_in = (current.time() - prev.time()) / 3.0;
                double tangent_length_out = (next.time() - current.time()) / 3.0;
                
                in_tangent.time = current.time() - tangent_length_in;
                in_tangent.value = current.value() - tangent_length_in * avg_slope;
                
                out_tangent.time = current.time() + tangent_length_out;
                out_tangent.value = current.value() + tangent_length_out * avg_slope;
            }
            else if (i == 0 && m_keyframes.size() > 1) {
                // First keyframe - only use next
                const Keyframe& next = m_keyframes[i+1];
                
                double slope = (next.value() - current.value()) / (next.time() - current.time());
                double tangent_length = (next.time() - current.time()) / 3.0;
                
                in_tangent.time = current.time() - tangent_length;
                in_tangent.value = current.value() - tangent_length * slope;
                
                out_tangent.time = current.time() + tangent_length;
                out_tangent.value = current.value() + tangent_length * slope;
            }
            else if (i == m_keyframes.size() - 1 && m_keyframes.size() > 1) {
                // Last keyframe - only use previous
                const Keyframe& prev = m_keyframes[i-1];
                
                double slope = (current.value() - prev.value()) / (current.time() - prev.time());
                double tangent_length = (current.time() - prev.time()) / 3.0;
                
                in_tangent.time = current.time() - tangent_length;
                in_tangent.value = current.value() - tangent_length * slope;
                
                out_tangent.time = current.time() + tangent_length;
                out_tangent.value = current.value() + tangent_length * slope;
            }
            
            current.set_in_tangent(in_tangent);
            current.set_out_tangent(out_tangent);
        }
    }
}

} // namespace anim
