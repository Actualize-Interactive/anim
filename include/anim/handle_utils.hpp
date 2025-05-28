#ifndef ANIM_HANDLE_UTILS_HPP
#define ANIM_HANDLE_UTILS_HPP

#include "point.hpp"    
#include "keyframe.hpp"  
#include <cmath>         
#include <stdexcept>     
#include <algorithm>     
#include <limits>
#include <numbers> 


namespace anim {


    bool nearly_equal(double a, double b, double epsilon = std::numeric_limits<double>::epsilon()) { 
        return std::abs(a - b) < epsilon;
    }

    // Calculates the Euclidean distance between two points.
    double distance(const Point& p1, const Point& p2) {
        double dt = p1.time - p2.time;
        double dv = p1.value - p2.value;
        return std::sqrt(dt * dt + dv * dv);
    }

    Vector vector(const Point& p1, const Point& p2) {
        return Vector(p2.time - p1.time, p2.value - p1.value);
    }

    Vector normalize(const Vector& vec) {
        double length_sq = vec.time * vec.time + vec.value * vec.value; // Calculate squared length first
        if (nearly_equal(length_sq, 0.0)) { // Check against squared length to avoid sqrt(0) issues
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

    // 'normal' is assumed to be a unit vector.
    Vector reflect(const Vector& vec, const Vector& normal_unit_vector) {
        // Formula: R = V - 2 * dot(V, N) * N
        double dp = dot_product(vec, normal_unit_vector);
        return Vector(
            vec.time - 2 * dp * normal_unit_vector.time,
            vec.value - 2 * dp * normal_unit_vector.value
        );
    }

    Vector invert(const Vector& vec) {
        return Vector(-vec.time, -vec.value);
    }
    
    // Constrains a point 'p' (typically a handle) to be within [min_time, max_time]
    // while maintaining its slope relative to an 'origin' point (typically the keyframe).
    Point constrain_point_time_preserve_slope(const Point& origin, const Point& p, double min_time, double max_time) {
        if (p.time >= min_time && p.time <= max_time) {
            return p; // Already within bounds
        }

        // Handle vertical line case (p.time is very close to origin.time)
        if (nearly_equal(p.time, origin.time)) {
            // If the handle is vertically aligned with the keyframe and needs clamping.
            // Option 1: Snap to keyframe (results in zero-length handle if clamped)
            // return origin; 
            // Option 2: Create a vertical handle at the clamped time, preserving original value.
            if (p.time < min_time) return Point(min_time, p.value);
            return Point(max_time, p.value); // p.time > max_time
        }

        // p.time is different from origin.time, so slope is well-defined.
        double slope = (p.value - origin.value) / (p.time - origin.time);

        if (p.time < min_time) {
            return Point(min_time, origin.value + slope * (min_time - origin.time));
        } else { // p.time > max_time (because first if failed and min_time <= max_time is assumed)
            return Point(max_time, origin.value + slope * (max_time - origin.time));
        }
    }

    // Constrains the in-handle's time of a keyframe.
    void constrain_in_handle_time(Keyframe& keyframe, const Keyframe& prev_keyframe) {
        // The in-handle's time should be between prev_keyframe.position.time and keyframe.position.time.
        keyframe.in_handle = constrain_point_time_preserve_slope(
            keyframe.position, 
            keyframe.in_handle, 
            prev_keyframe.position.time, 
            keyframe.position.time
        );
    }

    // Constrains the out-handle's time of a keyframe.
    void constrain_out_handle_time(Keyframe& keyframe, const Keyframe& next_keyframe) {
        // The out-handle's time should be between keyframe.position.time and next_keyframe.position.time.
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

    // Constraints the in and out handles of a keyframe based on its handle mode.

    // --- Handle Mode Calculation Functions ---
    // These functions modify the keyframe's in_handle and out_handle based on the mode.
    // They assume Point and Vector types have overloaded operators for basic arithmetic
    // (e.g., Point - Vector, Point + Vector, Vector * double, Vector + Vector)
    // if not directly using component-wise operations.

    // HandleMode::flat
    // Handles have the same 'value' as the keyframe's position (parallel to the time axis).
    // Their 'time' values are independently adjustable but clamped.
    void calculate_flat_handles(Keyframe& keyframe, const Keyframe* prev_keyframe_ptr, const Keyframe* next_keyframe_ptr) {
        // Set in-handle
        if (prev_keyframe_ptr) {
            // Clamp time between previous keyframe and current keyframe. Value is keyframe's value.
            double clamped_time = std::clamp(keyframe.in_handle.time, prev_keyframe_ptr->position.time, keyframe.position.time);
            keyframe.in_handle = Point(clamped_time, keyframe.position.value);
        } else {
            // No previous keyframe, in-handle is at the keyframe position (zero length).
            keyframe.in_handle = keyframe.position;
        }

        // Set out-handle
        if (next_keyframe_ptr) {
            // Clamp time between current keyframe and next keyframe. Value is keyframe's value.
            double clamped_time = std::clamp(keyframe.out_handle.time, keyframe.position.time, next_keyframe_ptr->position.time);
            keyframe.out_handle = Point(clamped_time, keyframe.position.value);
        } else {
            // No next keyframe, out-handle is at the keyframe position (zero length).
            keyframe.out_handle = keyframe.position;
        }
    }


    // HandleMode::smooth 
    // Handles are auto-calculated to create a smooth (C1 continuous) transition.
    // They are collinear, and their magnitude is typically a factor of the distance to adjacent keyframes.
    void calculate_smooth_handles(Keyframe& keyframe, const Keyframe* prev_keyframe_ptr, const Keyframe* next_keyframe_ptr, double smooth_factor = 1.0/3.0) {
        if (prev_keyframe_ptr && next_keyframe_ptr) { // Keyframe has two neighbors
            Vector vec_prev_to_curr = vector(prev_keyframe_ptr->position, keyframe.position);
            Vector vec_curr_to_next = vector(keyframe.position, next_keyframe_ptr->position);

            // Catmull-Rom like tangent calculation (or a common variant for Bezier smoothing)
            // Tangent at keyframe is parallel to vector from prev_keyframe_ptr->position to next_keyframe_ptr->position
            Vector tangent_dir_vec = vector(prev_keyframe_ptr->position, next_keyframe_ptr->position);
            
            if (nearly_equal(length_squared(tangent_dir_vec), 0.0)) { // Prev and Next are at the same spot
                // Fallback to flat handles or some other default if prev and next coincide
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

        } else if (prev_keyframe_ptr) { // Only previous keyframe (endpoint)
            Vector vec_prev_to_curr = vector(prev_keyframe_ptr->position, keyframe.position);
             if (nearly_equal(length_squared(vec_prev_to_curr), 0.0)) {
                calculate_flat_handles(keyframe, prev_keyframe_ptr, next_keyframe_ptr);
                return;
            }
            Vector normalized_tangent = normalize(vec_prev_to_curr);
            double dist_prev = length(vec_prev_to_curr); // distance() would also work

            Vector handle_offset = normalized_tangent * (dist_prev * smooth_factor);
            keyframe.in_handle  = keyframe.position - handle_offset;
            keyframe.out_handle = keyframe.position + handle_offset; // Make symmetric for endpoint

        } else if (next_keyframe_ptr) { // Only next keyframe (startpoint)
            Vector vec_curr_to_next = vector(keyframe.position, next_keyframe_ptr->position);
            if (nearly_equal(length_squared(vec_curr_to_next), 0.0)) {
                calculate_flat_handles(keyframe, prev_keyframe_ptr, next_keyframe_ptr);
                return;
            }
            Vector normalized_tangent = normalize(vec_curr_to_next);
            double dist_next = length(vec_curr_to_next);

            Vector handle_offset = normalized_tangent * (dist_next * smooth_factor);
            keyframe.in_handle  = keyframe.position - handle_offset; // Make symmetric for startpoint
            keyframe.out_handle = keyframe.position + handle_offset;
            
        } else { // Isolated keyframe
            keyframe.in_handle  = keyframe.position;
            keyframe.out_handle = keyframe.position; // Zero-length handles (effectively flat)
        }
    }

    // HandleMode::aligned
    // This function is intended to be called when a user adjusts one handle,
    // and the other handle needs to be updated to remain collinear and opposite.
    // It does NOT auto-calculate tangents from neighbors like 'smooth'.
    // `source_is_out_handle` = true if out_handle was just moved by user, false if in_handle was.
    // out_handle is the source when updating from state
    void enforce_aligned_handles(Keyframe& keyframe, bool source_is_out_handle = true) {
        if (source_is_out_handle) { // out_handle is the source
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

            if (nearly_equal(in_length, 0.0)) { // If in-handle is at keyframe position
                // Option 1: Make out-handle also zero-length
                keyframe.out_handle = keyframe.position;
                // Option 2: Or, if out-handle had a length, try to preserve its direction but make it flat?
                // This depends on desired UX. Snapping to zero is simpler.
                return;
            }
            
            Vector out_tangent_dir = normalize(invert(in_tangent_vec));
            double out_handle_current_dist = distance(keyframe.position, keyframe.out_handle);
            
            // Preserve the existing length of the out-handle, but align its direction.
            keyframe.out_handle = keyframe.position + (out_tangent_dir * out_handle_current_dist) ;
        }
    }
    
    // Call this when switching TO aligned mode to set an initial state.
    // This uses the smooth calculation as a reasonable default.
    void initialize_handles_for_aligned_mode(Keyframe& keyframe, const Keyframe* prev_keyframe_ptr, const Keyframe* next_keyframe_ptr) {
        calculate_smooth_handles(keyframe, prev_keyframe_ptr, next_keyframe_ptr);
        // After smooth calculation, they are already collinear. No further action needed for initialization.
        // If you wanted a different initial aligned state (e.g., based on chords), implement here.
    }

} // namespace anim

#endif // ANIM_HANDLE_UTILS_HPP
