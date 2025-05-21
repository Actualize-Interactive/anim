#include "anim/channel.hpp"

namespace anim {

void Channel::set_keyframe(double time, double value, 
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

void Channel::set_keyframe_time(double old_time, double new_time) {
    auto it = std::find_if(m_keyframes.begin(), m_keyframes.end(), 
                          [old_time](const Keyframe& kf) {
                              constexpr double epsilon = 1e-10;
                              return std::abs(kf.time() - old_time) < epsilon;
                          });
    if (it == m_keyframes.end()) {
        throw std::out_of_range("No keyframe found at specified time");
    }
    double value = it->value();
    BezierHandle in_tangent(it->in_tangent().time - old_time + new_time, it->in_tangent().value);
    BezierHandle out_tangent(it->out_tangent().time - old_time + new_time, it->out_tangent().value);
    TangentMode mode = it->mode();
    it->set_time(new_time);
    it->set_in_tangent(in_tangent);
    it->set_out_tangent(out_tangent);
    // Resort after time change
    sort_keyframes_internal();
    recalculate_dependent_tangents_internal();
}

void Channel::set_keyframe_value(double time, double new_value) {
    auto it = std::find_if(m_keyframes.begin(), m_keyframes.end(), 
                          [time](const Keyframe& kf) {
                              constexpr double epsilon = 1e-10;
                              return std::abs(kf.time() - time) < epsilon;
                          });
    if (it == m_keyframes.end()) {
        throw std::out_of_range("No keyframe found at specified time");
    }
    double value_diff = new_value - it->value();
    it->set_value(new_value);
    it->set_in_tangent(BezierHandle(it->in_tangent().time, it->in_tangent().value + value_diff));
    it->set_out_tangent(BezierHandle(it->out_tangent().time, it->out_tangent().value + value_diff));
    recalculate_dependent_tangents_internal();
}

void Channel::set_keyframe_in_tangent(double time, const BezierHandle& new_in_tangent) {
    auto it = std::find_if(m_keyframes.begin(), m_keyframes.end(), 
                          [time](const Keyframe& kf) {
                              constexpr double epsilon = 1e-10;
                              return std::abs(kf.time() - time) < epsilon;
                          });
    if (it == m_keyframes.end()) {
        throw std::out_of_range("No keyframe found at specified time");
    }
    BezierHandle out_tangent = it->out_tangent();
    if (it->mode() == TangentMode::smoothManual) {
        Point2D kf_point(it->time(), it->value());
        Point2D direction = new_in_tangent - kf_point;
        direction = direction * -1.0;
        double original_length = std::sqrt(
            std::pow(out_tangent.time - it->time(), 2) +
            std::pow(out_tangent.value - it->value(), 2)
        );
        double new_length = std::sqrt(
            std::pow(direction.time, 2) + std::pow(direction.value, 2)
        );
        if (new_length > 1e-10) {
            direction = direction * (original_length / new_length);
        }
        out_tangent = kf_point + direction;
    }
    it->set_in_tangent(new_in_tangent);
    it->set_out_tangent(out_tangent);
    recalculate_dependent_tangents_internal();
}

void Channel::set_keyframe_out_tangent(double time, const BezierHandle& new_out_tangent) {
    auto it = std::find_if(m_keyframes.begin(), m_keyframes.end(), 
                          [time](const Keyframe& kf) {
                              constexpr double epsilon = 1e-10;
                              return std::abs(kf.time() - time) < epsilon;
                          });
    if (it == m_keyframes.end()) {
        throw std::out_of_range("No keyframe found at specified time");
    }
    BezierHandle in_tangent = it->in_tangent();
    if (it->mode() == TangentMode::smoothManual) {
        Point2D kf_point(it->time(), it->value());
        Point2D direction = new_out_tangent - kf_point;
        direction = direction * -1.0;
        double original_length = std::sqrt(
            std::pow(in_tangent.time - it->time(), 2) +
            std::pow(in_tangent.value - it->value(), 2)
        );
        double new_length = std::sqrt(
            std::pow(direction.time, 2) + std::pow(direction.value, 2)
        );
        if (new_length > 1e-10) {
            direction = direction * (original_length / new_length);
        }
        in_tangent = kf_point + direction;
    }
    it->set_in_tangent(in_tangent);
    it->set_out_tangent(new_out_tangent);
    recalculate_dependent_tangents_internal();
}

void Channel::set_keyframe_tangent_mode(double time, TangentMode new_mode) {
    auto it = std::find_if(m_keyframes.begin(), m_keyframes.end(), 
                          [time](const Keyframe& kf) {
                              constexpr double epsilon = 1e-10;
                              return std::abs(kf.time() - time) < epsilon;
                          });
    if (it == m_keyframes.end()) {
        throw std::out_of_range("No keyframe found at specified time");
    }
    it->set_mode(new_mode);
    recalculate_dependent_tangents_internal();
}

bool Channel::remove_keyframe(double time) {
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

std::optional<Keyframe> Channel::get_keyframe(double time) const {
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
