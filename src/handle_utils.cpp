#include "anim/handle_utils.hpp"
#include <cmath>         
#include <stdexcept>     
#include <algorithm>     
#include <limits>
#include <numbers> 

namespace anim {

    bool nearly_equal(double a, double b, double epsilon) { 
        return std::abs(a - b) < epsilon;
    }

    double distance(const Point& p1, const Point& p2) {
        double dt = p1.time - p2.time;
        double dv = p1.value - p2.value;
        return std::sqrt(dt * dt + dv * dv);
    }

    Vector vector(const Point& p1, const Point& p2) {
        return Vector(p2.time - p1.time, p2.value - p1.value);
    }

    Vector normalize(const Vector& vec) {
        double length_sq = vec.time * vec.time + vec.value * vec.value;
        if (nearly_equal(length_sq, 0.0)) {
            throw std::domain_error("Cannot normalize a zero-length vector");
        }
        double length = std::sqrt(length_sq);
        return Vector(vec.time / length, vec.value / length);
    }
    
    double length_squared(const Vector& vec) {
        return vec.time * vec.time + vec.value * vec.value;
    }

    double length(const Vector& vec) {
        return std::sqrt(length_squared(vec));
    }

    Point midpoint(const Point& p1, const Point& p2) {
        return Point((p1.time + p2.time) / 2.0, (p1.value + p2.value) / 2.0);
    }

    Point scale(const Point& p, double scalar) {
        return Point(p.time * scalar, p.value * scalar);
    }

    Point translate(const Point& p, const Vector& vec) {
        return Point(p.time + vec.time, p.value + vec.value);
    }

    Point rotate(const Point& p, double angle_degrees) {
        double rad = angle_degrees * std::numbers::pi / 180.0; 
        double cos_angle = std::cos(rad);
        double sin_angle = std::sin(rad);
        return Point(
            p.time * cos_angle - p.value * sin_angle,
            p.time * sin_angle + p.value * cos_angle
        );
    }

    double dot_product(const Vector& v1, const Vector& v2) {
        return v1.time * v2.time + v1.value * v2.value;
    }

    double cross_product(const Vector& v1, const Vector& v2) {
        return v1.time * v2.value - v1.value * v2.time;
    }

    Vector reflect(const Vector& vec, const Vector& normal_unit_vector) {
        double dp = dot_product(vec, normal_unit_vector);
        return Vector(
            vec.time - 2 * dp * normal_unit_vector.time,
            vec.value - 2 * dp * normal_unit_vector.value
        );
    }

    Vector invert(const Vector& vec) {
        return Vector(-vec.time, -vec.value);
    }
    
    Point constrain_point_time_preserve_slope(const Point& origin, const Point& p, double min_time, double max_time) {
        if (p.time >= min_time && p.time <= max_time) {
            return p;
        }

        if (nearly_equal(p.time, origin.time)) {
            if (p.time < min_time) return Point(min_time, p.value);
            return Point(max_time, p.value);
        }

        double slope = (p.value - origin.value) / (p.time - origin.time);

        if (p.time < min_time) {
            return Point(min_time, origin.value + slope * (min_time - origin.time));
        } else {
            return Point(max_time, origin.value + slope * (max_time - origin.time));
        }
    }

    void constrain_in_handle_time(Keyframe& keyframe, const Keyframe& prev_keyframe) {
        keyframe.in_handle = constrain_point_time_preserve_slope(
            keyframe.position, 
            keyframe.in_handle, 
            prev_keyframe.position.time, 
            keyframe.position.time
        );
    }

    void constrain_out_handle_time(Keyframe& keyframe, const Keyframe& next_keyframe) {
        keyframe.out_handle = constrain_point_time_preserve_slope(
            keyframe.position, 
            keyframe.out_handle, 
            keyframe.position.time, 
            next_keyframe.position.time
        );
    }

    void constrain_handles(Keyframe& keyframe, const Keyframe* prev_keyframe_ptr, const Keyframe* next_keyframe_ptr) {
        if (prev_keyframe_ptr) {
            constrain_in_handle_time(keyframe, *prev_keyframe_ptr);
        }
        if (next_keyframe_ptr) {
            constrain_out_handle_time(keyframe, *next_keyframe_ptr);
        }
    }

    void calculate_flat_handles(Keyframe& keyframe, const Keyframe* prev_keyframe_ptr, const Keyframe* next_keyframe_ptr) {
        if (prev_keyframe_ptr) {
            double clamped_time = std::clamp(keyframe.in_handle.time, prev_keyframe_ptr->position.time, keyframe.position.time);
            keyframe.in_handle = Point(clamped_time, keyframe.position.value);
        } else {
            keyframe.in_handle = keyframe.position;
        }

        if (next_keyframe_ptr) {
            double clamped_time = std::clamp(keyframe.out_handle.time, keyframe.position.time, next_keyframe_ptr->position.time);
            keyframe.out_handle = Point(clamped_time, keyframe.position.value);
        } else {
            keyframe.out_handle = keyframe.position;
        }
    }

    void calculate_smooth_handles(Keyframe& keyframe, const Keyframe* prev_keyframe_ptr, const Keyframe* next_keyframe_ptr, double smooth_factor) {
        if (prev_keyframe_ptr && next_keyframe_ptr) {
            Vector vec_prev_to_curr = vector(prev_keyframe_ptr->position, keyframe.position);
            Vector vec_curr_to_next = vector(keyframe.position, next_keyframe_ptr->position);

            Vector tangent_dir_vec = vector(prev_keyframe_ptr->position, next_keyframe_ptr->position);
            
            if (nearly_equal(length_squared(tangent_dir_vec), 0.0)) {
                calculate_flat_handles(keyframe, prev_keyframe_ptr, next_keyframe_ptr);
                return;
            }
            Vector normalized_tangent = normalize(tangent_dir_vec);

            double dist_prev = distance(prev_keyframe_ptr->position, keyframe.position);
            double dist_next = distance(keyframe.position, next_keyframe_ptr->position);
            
            Vector in_handle_offset = normalized_tangent * (dist_prev * smooth_factor);
            Vector out_handle_offset = normalized_tangent * (dist_next * smooth_factor);

            keyframe.in_handle  = keyframe.position - in_handle_offset;
            keyframe.out_handle = keyframe.position + out_handle_offset;

        } else if (prev_keyframe_ptr) {
            Vector vec_prev_to_curr = vector(prev_keyframe_ptr->position, keyframe.position);
             if (nearly_equal(length_squared(vec_prev_to_curr), 0.0)) {
                calculate_flat_handles(keyframe, prev_keyframe_ptr, next_keyframe_ptr);
                return;
            }
            Vector normalized_tangent = normalize(vec_prev_to_curr);
            double dist_prev = length(vec_prev_to_curr);

            Vector handle_offset = normalized_tangent * (dist_prev * smooth_factor);
            keyframe.in_handle  = keyframe.position - handle_offset;
            keyframe.out_handle = keyframe.position + handle_offset;

        } else if (next_keyframe_ptr) {
            Vector vec_curr_to_next = vector(keyframe.position, next_keyframe_ptr->position);
            if (nearly_equal(length_squared(vec_curr_to_next), 0.0)) {
                calculate_flat_handles(keyframe, prev_keyframe_ptr, next_keyframe_ptr);
                return;
            }
            Vector normalized_tangent = normalize(vec_curr_to_next);
            double dist_next = length(vec_curr_to_next);

            Vector handle_offset = normalized_tangent * (dist_next * smooth_factor);
            keyframe.in_handle  = keyframe.position - handle_offset;
            keyframe.out_handle = keyframe.position + handle_offset;
            
        } else {
            keyframe.in_handle  = keyframe.position;
            keyframe.out_handle = keyframe.position;
        }
    }

    void enforce_aligned_handles(Keyframe& keyframe, bool source_is_out_handle) {
        if (source_is_out_handle) {
            Vector out_tangent_vec = vector(keyframe.position, keyframe.out_handle);
            double out_length = length(out_tangent_vec);

            if (nearly_equal(out_length, 0.0)) {
                keyframe.in_handle = keyframe.position;
                return;
            }

            Vector in_tangent_dir = normalize(invert(out_tangent_vec));
            double in_handle_current_dist = distance(keyframe.position, keyframe.in_handle);
            
            keyframe.in_handle = keyframe.position + (in_tangent_dir * in_handle_current_dist);
        } else {
            Vector in_tangent_vec = vector(keyframe.position, keyframe.in_handle);
            double in_length = length(in_tangent_vec);

            if (nearly_equal(in_length, 0.0)) {
                keyframe.out_handle = keyframe.position;
                return;
            }
            
            Vector out_tangent_dir = normalize(invert(in_tangent_vec));
            double out_handle_current_dist = distance(keyframe.position, keyframe.out_handle);
            
            keyframe.out_handle = keyframe.position + (out_tangent_dir * out_handle_current_dist) ;
        }
    }
    
    void initialize_handles_for_aligned_mode(Keyframe& keyframe, const Keyframe* prev_keyframe_ptr, const Keyframe* next_keyframe_ptr) {
        calculate_smooth_handles(keyframe, prev_keyframe_ptr, next_keyframe_ptr);
    }

} // namespace anim

