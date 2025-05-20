#ifndef ANIM_ANIMATION_CHANNEL_H
#define ANIM_ANIMATION_CHANNEL_H

#include "keyframe.h"
#include "bezier_utils.h"
#include <vector>
#include <algorithm>
#include <optional>
#include <stdexcept>
#include <cmath>

namespace anim {

/**
 * @brief Manages a sequence of Keyframe objects for a single animatable property.
 */
class AnimationChannel {
public:
    /**
     * @brief Constructor for an empty animation channel
     */
    AnimationChannel() = default;

    /**
     * @brief Add or replace a keyframe at the specified time
     * 
     * @param time The time value of the keyframe
     * @param value The animated value at the keyframe
     * @param in_tangent The in-tangent handle (absolute time/value coordinates)
     * @param out_tangent The out-tangent handle (absolute time/value coordinates)
     * @param mode The tangent mode for this keyframe
     */
    void set_keyframe(double time, double value, 
                     const BezierHandle& in_tangent, 
                     const BezierHandle& out_tangent, 
                     TangentMode mode) {        // Find if a keyframe at this time already exists
        auto it = std::find_if(m_keyframes.begin(), m_keyframes.end(), 
                              [time](const Keyframe& kf) {
                                  constexpr double epsilon = 1e-10;
                                  return std::abs(kf.time() - time) < epsilon;
                              });
          // Create the new keyframe
        Keyframe new_keyframe(time, value, in_tangent, out_tangent, mode);
        
        // Either replace or add the keyframe
        if (it != m_keyframes.end()) {
            *it = new_keyframe;
        } else {
            m_keyframes.push_back(new_keyframe);
            sort_keyframes_internal();
        }
        
        // Recalculate dependent tangents
        recalculate_dependent_tangents_internal();
    }

    /**
     * @brief Change a keyframe's time value
     * 
     * @param old_time The current time of the keyframe to change
     * @param new_time The new time to set
     * @throws std::out_of_range if no keyframe exists at old_time
     */
    void set_keyframe_time(double old_time, double new_time) {
        auto kf_opt = get_keyframe(old_time);
        if (!kf_opt) {
            throw std::out_of_range("No keyframe found at specified time");
        }
        
        const Keyframe& kf = *kf_opt;
        
        // Create a new keyframe with the updated time
        Keyframe new_keyframe(
            new_time, kf.value(), 
            BezierHandle(kf.in_tangent().time - old_time + new_time, kf.in_tangent().value),
            BezierHandle(kf.out_tangent().time - old_time + new_time, kf.out_tangent().value),
            kf.mode()
        );
        
        // Remove the old keyframe
        remove_keyframe(old_time);
        
        // Add the new keyframe
        m_keyframes.push_back(new_keyframe);
        sort_keyframes_internal();
        
        // Recalculate dependent tangents
        recalculate_dependent_tangents_internal();
    }

    /**
     * @brief Change a keyframe's value
     * 
     * @param time The time of the keyframe to change
     * @param new_value The new value to set
     * @throws std::out_of_range if no keyframe exists at the specified time
     */
    void set_keyframe_value(double time, double new_value) {
        auto kf_opt = get_keyframe(time);
        if (!kf_opt) {
            throw std::out_of_range("No keyframe found at specified time");
        }
        
        const Keyframe& kf = *kf_opt;
        
        // Adjust tangent values accordingly
        double value_diff = new_value - kf.value();
        
        BezierHandle new_in_tangent(kf.in_tangent().time, kf.in_tangent().value + value_diff);
        BezierHandle new_out_tangent(kf.out_tangent().time, kf.out_tangent().value + value_diff);
        
        // Create a new keyframe with the updated value and tangents
        Keyframe new_keyframe(
            kf.time(), new_value, 
            new_in_tangent, new_out_tangent,
            kf.mode()
        );
        
        // Replace the old keyframe
        auto it = std::find_if(m_keyframes.begin(), m_keyframes.end(), 
                              [time](const Keyframe& k) {
                                  constexpr double epsilon = 1e-10;
                                  return std::abs(k.time() - time) < epsilon;
                              });
        
        if (it != m_keyframes.end()) {
            *it = new_keyframe;
        }
        
        // Recalculate dependent tangents
        recalculate_dependent_tangents_internal();
    }

    /**
     * @brief Change a keyframe's in-tangent handle
     * 
     * @param time The time of the keyframe to change
     * @param new_in_tangent The new in-tangent handle
     * @throws std::out_of_range if no keyframe exists at the specified time
     */
    void set_keyframe_in_tangent(double time, const BezierHandle& new_in_tangent) {
        auto kf_opt = get_keyframe(time);
        if (!kf_opt) {
            throw std::out_of_range("No keyframe found at specified time");
        }
        
        const Keyframe& kf = *kf_opt;
        BezierHandle out_tangent = kf.out_tangent();
          // If mode is SMOOTH_MANUAL, adjust out-tangent to maintain co-linearity
        if (kf.mode() == TangentMode::smoothManual) {
            // Calculate the direction from keyframe to in-tangent
            Point2D kf_point(kf.time(), kf.value());
            Point2D direction = new_in_tangent - kf_point;
            
            // Flip the direction for out-tangent
            direction = direction * -1.0;
            
            // Maintain the original out-tangent length
            double original_length = std::sqrt(
                std::pow(out_tangent.time - kf.time(), 2) +
                std::pow(out_tangent.value - kf.value(), 2)
            );
            
            double new_length = std::sqrt(
                std::pow(direction.time, 2) + std::pow(direction.value, 2)
            );
            
            if (new_length > 1e-10) {
                direction = direction * (original_length / new_length);
            }
            
            // Calculate the new out-tangent
            out_tangent = kf_point + direction;
        }
        
        // Create a new keyframe with the updated tangents
        Keyframe new_keyframe(
            kf.time(), kf.value(), 
            new_in_tangent, out_tangent,
            kf.mode()
        );
        
        // Replace the old keyframe
        auto it = std::find_if(m_keyframes.begin(), m_keyframes.end(), 
                              [time](const Keyframe& k) {
                                  constexpr double epsilon = 1e-10;
                                  return std::abs(k.time() - time) < epsilon;
                              });
        
        if (it != m_keyframes.end()) {
            *it = new_keyframe;
        }
        
        // Recalculate dependent tangents
        recalculate_dependent_tangents_internal();
    }

    /**
     * @brief Change a keyframe's out-tangent handle
     * 
     * @param time The time of the keyframe to change
     * @param new_out_tangent The new out-tangent handle
     * @throws std::out_of_range if no keyframe exists at the specified time
     */
    void set_keyframe_out_tangent(double time, const BezierHandle& new_out_tangent) {
        auto kf_opt = get_keyframe(time);
        if (!kf_opt) {
            throw std::out_of_range("No keyframe found at specified time");
        }
        
        const Keyframe& kf = *kf_opt;
        BezierHandle in_tangent = kf.in_tangent();
          // If mode is SMOOTH_MANUAL, adjust in-tangent to maintain co-linearity
        if (kf.mode() == TangentMode::smoothManual) {
            // Calculate the direction from keyframe to out-tangent
            Point2D kf_point(kf.time(), kf.value());
            Point2D direction = new_out_tangent - kf_point;
            
            // Flip the direction for in-tangent
            direction = direction * -1.0;
            
            // Maintain the original in-tangent length
            double original_length = std::sqrt(
                std::pow(in_tangent.time - kf.time(), 2) +
                std::pow(in_tangent.value - kf.value(), 2)
            );
            
            double new_length = std::sqrt(
                std::pow(direction.time, 2) + std::pow(direction.value, 2)
            );
            
            if (new_length > 1e-10) {
                direction = direction * (original_length / new_length);
            }
            
            // Calculate the new in-tangent
            in_tangent = kf_point + direction;
        }
        
        // Create a new keyframe with the updated tangents
        Keyframe new_keyframe(
            kf.time(), kf.value(), 
            in_tangent, new_out_tangent,
            kf.mode()
        );
        
        // Replace the old keyframe
        auto it = std::find_if(m_keyframes.begin(), m_keyframes.end(), 
                              [time](const Keyframe& k) {
                                  constexpr double epsilon = 1e-10;
                                  return std::abs(k.time() - time) < epsilon;
                              });
        
        if (it != m_keyframes.end()) {
            *it = new_keyframe;
        }
        
        // Recalculate dependent tangents
        recalculate_dependent_tangents_internal();
    }

    /**
     * @brief Change a keyframe's tangent mode
     * 
     * @param time The time of the keyframe to change
     * @param new_mode The new tangent mode
     * @throws std::out_of_range if no keyframe exists at the specified time
     */
    void set_keyframe_tangent_mode(double time, TangentMode new_mode) {
        auto kf_opt = get_keyframe(time);
        if (!kf_opt) {
            throw std::out_of_range("No keyframe found at specified time");
        }
        
        const Keyframe& kf = *kf_opt;
        
        // Create a new keyframe with the updated mode
        Keyframe new_keyframe(
            kf.time(), kf.value(), 
            kf.in_tangent(), kf.out_tangent(),
            new_mode
        );
        
        // Replace the old keyframe
        auto it = std::find_if(m_keyframes.begin(), m_keyframes.end(), 
                              [time](const Keyframe& k) {
                                  constexpr double epsilon = 1e-10;
                                  return std::abs(k.time() - time) < epsilon;
                              });
        
        if (it != m_keyframes.end()) {
            *it = new_keyframe;
        }
        
        // Recalculate dependent tangents
        recalculate_dependent_tangents_internal();
    }

    /**
     * @brief Remove a keyframe at the specified time
     * 
     * @param time The time of the keyframe to remove
     * @return bool True if a keyframe was removed, false if no keyframe was found
     */
    bool remove_keyframe(double time) {
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
    }    /**
     * @brief Get a keyframe at the specified time
     * 
     * @param time The time of the keyframe to get
     * @return std::optional<Keyframe> The keyframe if found, or std::nullopt
     */
    std::optional<Keyframe> get_keyframe(double time) const {
        auto it = std::find_if(m_keyframes.begin(), m_keyframes.end(), 
                              [time](const Keyframe& kf) {
                                  constexpr double epsilon = 1e-10;
                                  return std::abs(kf.time() - time) < epsilon;
                              });
        
        if (it != m_keyframes.end()) {
            return *it;
        }
        
        return std::nullopt;
    }    /**
     * @brief Get all keyframes in the channel
     * 
     * @return const std::vector<Keyframe>& All keyframes
     */
    const std::vector<Keyframe>& get_all_keyframes() const {
        return m_keyframes;
    }

    /**
     * @brief Evaluate the animation channel at a specific time
     * 
     * @param time The time to evaluate at
     * @return double The animated value at the specified time
     */
    double evaluate(double time) const {
        // Handle empty channel
        if (m_keyframes.empty()) {
            return 0.0;
        }
        
        // Handle single keyframe
        if (m_keyframes.size() == 1) {
            return m_keyframes[0].value();
        }
        
        // Handle time before first keyframe
        if (time <= m_keyframes.front().time()) {
            return m_keyframes.front().value();
        }
        
        // Handle time after last keyframe
        if (time >= m_keyframes.back().time()) {
            return m_keyframes.back().value();
        }
        
        // Find the segment containing the time
        auto next_it = std::find_if(m_keyframes.begin(), m_keyframes.end(),
                                    [time](const Keyframe& kf) {
                                        return kf.time() > time;
                                    });
        
        if (next_it == m_keyframes.begin()) {
            // This shouldn't happen if the checks above work correctly
            return next_it->value();
        }
        
        auto prev_it = next_it - 1;
        const Keyframe& prev_kf = *prev_it;
        const Keyframe& next_kf = *next_it;
          // Handle STEPPED mode
        if (prev_kf.mode() == TangentMode::stepped) {
            return prev_kf.value();
        }
        
        // Handle LINEAR mode by direct interpolation
        if (prev_kf.mode() == TangentMode::linear) {
            double t = (time - prev_kf.time()) / (next_kf.time() - prev_kf.time());
            return prev_kf.value() + t * (next_kf.value() - prev_kf.value());
        }
        
        // For the rest, evaluate as cubic Bézier curve
        Point2D p0(prev_kf.time(), prev_kf.value());
        Point2D p1 = prev_kf.out_tangent();
        Point2D p2 = next_kf.in_tangent();
        Point2D p3(next_kf.time(), next_kf.value());
        
        try {
            // Find the parameter t where the curve's time matches our target time
            double param = bezier_utils::find_parameter_for_time(p0, p1, p2, p3, time);
            
            // Evaluate the Bézier curve at that parameter
            Point2D result = bezier_utils::evaluate_cubic_bezier(p0, p1, p2, p3, param);
            return result.value;
        } catch (const std::exception&) {
            // If something goes wrong with Bézier evaluation, fall back to linear
            double t = (time - prev_kf.time()) / (next_kf.time() - prev_kf.time());
            return prev_kf.value() + t * (next_kf.value() - prev_kf.value());
        }
    }

    /**
     * @brief Evaluate the animation channel over a range with a fixed number of samples
     * 
     * @param start_time Start of the time range
     * @param end_time End of the time range
     * @param num_samples Number of samples to generate
     * @return std::vector<double> The sampled values
     */
    std::vector<double> evaluate_range(double start_time, double end_time, int num_samples) const {
        if (num_samples <= 0) {
            return {};
        }
        
        if (num_samples == 1) {
            return {evaluate(start_time)};
        }
        
        std::vector<double> result;
        result.reserve(num_samples);
        
        double time_step = (end_time - start_time) / (num_samples - 1);
        
        for (int i = 0; i < num_samples; ++i) {
            double time = start_time + i * time_step;
            result.push_back(evaluate(time));
        }
        
        return result;
    }

    /**
     * @brief Evaluate the animation channel over a range with a specific sample rate
     * 
     * @param start_time Start of the time range
     * @param end_time End of the time range
     * @param sample_rate Number of samples per unit of time
     * @return std::vector<double> The sampled values
     */
    std::vector<double> evaluate_range_by_rate(double start_time, double end_time, double sample_rate) const {
        if (sample_rate <= 0.0) {
            return {};
        }
        
        if (start_time >= end_time) {
            return {evaluate(start_time)};
        }
        
        double time_range = end_time - start_time;
        int num_samples = static_cast<int>(std::ceil(time_range * sample_rate)) + 1;
        
        std::vector<double> result;
        result.reserve(num_samples);
        
        for (int i = 0; i < num_samples; ++i) {
            // Calculate time directly from the index to avoid floating-point accumulation errors
            double time = start_time + (i / sample_rate);
            
            // Make sure we don't go past end_time due to floating-point issues
            if (time > end_time) {
                time = end_time;
            }
            
            result.push_back(evaluate(time));
        }
        
        return result;
    }

    /**
     * @brief Check if the channel has no keyframes
     * 
     * @return bool True if the channel is empty
     */
    bool is_empty() const {
        return m_keyframes.empty();
    }

    /**
     * @brief Get the time of the first keyframe
     * 
     * @return std::optional<double> The start time, or std::nullopt if empty
     */
    std::optional<double> get_start_time() const {
        if (m_keyframes.empty()) {
            return std::nullopt;
        }
        return m_keyframes.front().time();
    }

    /**
     * @brief Get the time of the last keyframe
     * 
     * @return std::optional<double> The end time, or std::nullopt if empty
     */
    std::optional<double> get_end_time() const {
        if (m_keyframes.empty()) {
            return std::nullopt;
        }
        return m_keyframes.back().time();
    }

private:
    std::vector<Keyframe> m_keyframes; ///< The keyframes, always sorted by time

    /**
     * @brief Sort keyframes by time
     */
    void sort_keyframes_internal() {
        std::sort(m_keyframes.begin(), m_keyframes.end(),
                 [](const Keyframe& a, const Keyframe& b) {
                     return a.time() < b.time();
                 });
    }

    /**
     * @brief Recalculate tangents for keyframes based on their modes and neighbors
     */
    void recalculate_dependent_tangents_internal() {
        if (m_keyframes.empty()) {
            return;
        }
          // Make a copy of the keyframes for modification
        std::vector<Keyframe> new_keyframes = m_keyframes;
        bool tangents_changed;
        
        // Iterate until no more tangent changes are required
        do {
            tangents_changed = false;
            
            for (size_t i = 0; i < new_keyframes.size(); ++i) {
                const Keyframe& kf = new_keyframes[i];
                Point2D kf_point(kf.time(), kf.value());
                
                // Initialize new tangents with current values
                BezierHandle new_in_tangent = kf.in_tangent();
                BezierHandle new_out_tangent = kf.out_tangent();
                bool needs_update = false;
                  switch (kf.mode()) {
                    case TangentMode::flat: {
                        // Flat tangents are horizontal
                        double time_offset = 0.1; // Default offset
                        
                        // Adjust offset based on neighbors if available
                        if (i > 0) {
                            time_offset = std::min(time_offset, (kf.time() - new_keyframes[i-1].time()) / 3.0);
                        }
                        if (i < new_keyframes.size() - 1) {
                            time_offset = std::min(time_offset, (new_keyframes[i+1].time() - kf.time()) / 3.0);
                        }
                        
                        bezier_utils::create_flat_bezier_handles(
                            kf_point, time_offset, new_in_tangent, new_out_tangent);
                        needs_update = true;
                        break;
                    }
                    
                    case TangentMode::linear: {
                        // Linear tangents point to adjacent keyframes
                        if (i > 0) {
                            Point2D prev_point(new_keyframes[i-1].time(), new_keyframes[i-1].value());
                            double t_diff = kf.time() - prev_point.time;
                            double v_diff = kf.value() - prev_point.value;
                            new_in_tangent = Point2D(kf.time() - t_diff / 3.0, kf.value() - v_diff / 3.0);
                        }
                        
                        if (i < new_keyframes.size() - 1) {
                            Point2D next_point(new_keyframes[i+1].time(), new_keyframes[i+1].value());
                            double t_diff = next_point.time - kf.time();
                            double v_diff = next_point.value - kf.value();
                            new_out_tangent = Point2D(kf.time() + t_diff / 3.0, kf.value() + v_diff / 3.0);
                        }
                        
                        needs_update = true;
                        break;                    }
                    
                    case TangentMode::smoothAuto: {
                        // Calculate smooth tangents based on neighbors
                        if (new_keyframes.size() < 2) {
                            // Not enough keyframes for auto tangents
                            break;
                        }
                        
                        if (i == 0) {
                            // First keyframe
                            if (i + 1 < new_keyframes.size()) {
                                // Use next keyframe for both tangents
                                Point2D next_point(new_keyframes[i+1].time(), new_keyframes[i+1].value());
                                double t_diff = next_point.time - kf.time();
                                double v_diff = next_point.value - kf.value();
                                
                                new_out_tangent = Point2D(kf.time() + t_diff / 3.0, kf.value() + v_diff / 3.0);
                                new_in_tangent = Point2D(kf.time() - t_diff / 3.0, kf.value() - v_diff / 3.0);
                                needs_update = true;
                            }
                        } else if (i == new_keyframes.size() - 1) {
                            // Last keyframe
                            Point2D prev_point(new_keyframes[i-1].time(), new_keyframes[i-1].value());
                            double t_diff = kf.time() - prev_point.time;
                            double v_diff = kf.value() - prev_point.value;
                            
                            new_in_tangent = Point2D(kf.time() - t_diff / 3.0, kf.value() - v_diff / 3.0);
                            new_out_tangent = Point2D(kf.time() + t_diff / 3.0, kf.value() + v_diff / 3.0);
                            needs_update = true;
                        } else {
                            // Middle keyframe - use Catmull-Rom style tangent calculation
                            Point2D prev_point(new_keyframes[i-1].time(), new_keyframes[i-1].value());
                            Point2D next_point(new_keyframes[i+1].time(), new_keyframes[i+1].value());
                            
                            // Calculate the tangent direction (average of vectors to prev and next)
                            double next_time_diff = next_point.time - kf.time();
                            double next_value_diff = next_point.value - kf.value();
                            double prev_time_diff = kf.time() - prev_point.time;
                            double prev_value_diff = kf.value() - prev_point.value;
                            
                            // Normalize the time diffs to account for uneven spacing
                            double total_time = next_time_diff + prev_time_diff;
                            double prev_weight = next_time_diff / total_time;
                            double next_weight = prev_time_diff / total_time;
                            
                            // Calculate slope
                            double tangent_slope = 
                                (next_value_diff * next_weight + prev_value_diff * prev_weight) / 
                                (next_time_diff * next_weight + prev_time_diff * prev_weight);
                            
                            // Create tangent handles using the slope
                            double handle_time_length = std::min(next_time_diff, prev_time_diff) / 3.0;
                            new_out_tangent = Point2D(
                                kf.time() + handle_time_length, 
                                kf.value() + handle_time_length * tangent_slope
                            );
                            new_in_tangent = Point2D(
                                kf.time() - handle_time_length, 
                                kf.value() - handle_time_length * tangent_slope
                            );
                            needs_update = true;
                        }
                        break;                    }
                    
                    case TangentMode::smoothManual: {
                        // In SMOOTH_MANUAL, if one tangent changes, the other should adjust to maintain co-linearity
                        // The length of each tangent is preserved
                        
                        // This is mostly handled in set_keyframe_in/out_tangent
                        // but we need to adjust if a neighbor changed
                        
                        // Note: Do nothing here as the manual aspect implies user control
                        // This case may be revisited if more complex behavior is needed
                        break;
                    }
                    
                    case TangentMode::stepped:
                    case TangentMode::broken:
                        // STEPPED and BROKEN modes don't require automatic tangent adjustment
                        break;
                }
                
                // If tangents were updated, create a new keyframe
                if (needs_update && 
                    (new_in_tangent != kf.in_tangent() || new_out_tangent != kf.out_tangent())) {
                    new_keyframes[i] = Keyframe(
                        kf.time(), kf.value(), new_in_tangent, new_out_tangent, kf.mode());
                    tangents_changed = true;
                }
            }
        } while (tangents_changed);
        
        // Replace the original keyframes with the updated ones
        m_keyframes = std::move(new_keyframes);
    }
};

} // namespace anim

#endif // ANIM_ANIMATION_CHANNEL_H
