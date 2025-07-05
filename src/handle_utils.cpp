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

    // Better for really small numbers, but we don't need it for our use case.
    // bool nearly_equal(double a, double b, double epsilon) { 
    //     if (epsilon <= 0.0) {
    //         epsilon = std::numeric_limits<double>::epsilon();
    //     }
        
    //     // For very small numbers, use absolute comparison
    //     if (std::abs(a) < 1e-12 && std::abs(b) < 1e-12) {
    //         return std::abs(a - b) < epsilon;
    //     }
        
    //     // For larger numbers, use relative comparison
    //     double max_val = std::max(std::abs(a), std::abs(b));
    //     return std::abs(a - b) < epsilon * max_val;
    // }

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
    
    void constrain_in_handle_time(Keyframe& keyframe, const Keyframe& prev_keyframe) {
        keyframe.in_handle.time = std::clamp(
            keyframe.in_handle.time, 
            prev_keyframe.position.time, 
            keyframe.position.time
        );
    }

    void constrain_out_handle_time(Keyframe& keyframe, const Keyframe& next_keyframe) {
        keyframe.out_handle.time = std::clamp(
            keyframe.out_handle.time, 
            keyframe.position.time, 
            next_keyframe.position.time
        );
    }

    void constrain_handles_time(Keyframe& keyframe, const Keyframe* prev_keyframe_ptr, const Keyframe* next_keyframe_ptr) {
        if (prev_keyframe_ptr) {
            constrain_in_handle_time(keyframe, *prev_keyframe_ptr);
        }
        if (next_keyframe_ptr) {
            constrain_out_handle_time(keyframe, *next_keyframe_ptr);
        }
    }

    // Adjusts a handle so its time does not go past a boundary keyframe's time.
    // The handle is kept on the line defined by the keyframe and the original handle position.
    void ensure_handle_time_boundary(
        const Point& keyframe_pos,
        Point& handle_to_adjust,
        const Point& boundary_pos,
        bool is_in_handle
    ) {
        bool violation = false;
        if (is_in_handle) {
            if (handle_to_adjust.time < boundary_pos.time) {
                violation = true;
            }
        } else {
            if (handle_to_adjust.time > boundary_pos.time) {
                violation = true;
            }
        }

        if (violation) {
            Vector vec_kf_to_handle = vector(keyframe_pos, handle_to_adjust);
            if (nearly_equal(vec_kf_to_handle.time, 0.0)) {
                return;
            }
            double scale_factor = (boundary_pos.time - keyframe_pos.time) / vec_kf_to_handle.time;
            handle_to_adjust = keyframe_pos + (vec_kf_to_handle * scale_factor);
        }
    }

    void ensure_linear_handles_time_boundary(Keyframe& keyframe, const Keyframe* prev_keyframe_ptr, const Keyframe* next_keyframe_ptr) {
        if (prev_keyframe_ptr && next_keyframe_ptr) { // keyframe is in the middle of two keyframes
            ensure_handle_time_boundary(keyframe.position, keyframe.in_handle, prev_keyframe_ptr->position, true);
            ensure_handle_time_boundary(keyframe.position, keyframe.out_handle, next_keyframe_ptr->position, false);
        } else if (prev_keyframe_ptr) { // keyframe is the last keyframe
            ensure_handle_time_boundary(keyframe.position, keyframe.in_handle, prev_keyframe_ptr->position, true);
        } else if (next_keyframe_ptr) { // keyframe is the first keyframe
            ensure_handle_time_boundary(keyframe.position, keyframe.out_handle, next_keyframe_ptr->position, false);
        } else { // keyframe is the only keyframe
            // No handles to adjust
        }
        
        // For linear and constant functions, ensure in_handle doesn't go past keyframe time
        // and out_handle doesn't go before keyframe time
        constrain_handles_time(keyframe, prev_keyframe_ptr, next_keyframe_ptr);
    }


    void apply_flat_handles(Keyframe& keyframe, const Keyframe* prev_keyframe_ptr, const Keyframe* next_keyframe_ptr) {
        if (prev_keyframe_ptr && next_keyframe_ptr) { // keyframe is in the middle of two keyframes
            double in_time_offset = (prev_keyframe_ptr->position.time - keyframe.position.time) / 3.0;
            keyframe.in_handle  = Point(keyframe.position.time + in_time_offset, keyframe.position.value);
            double out_time_offset = (next_keyframe_ptr->position.time - keyframe.position.time) / 3.0;
            keyframe.out_handle = Point(keyframe.position.time + out_time_offset, keyframe.position.value);
            ensure_handle_time_boundary(keyframe.position, keyframe.in_handle, prev_keyframe_ptr->position, true);
            ensure_handle_time_boundary(keyframe.position, keyframe.out_handle, next_keyframe_ptr->position, false);
        } else if (prev_keyframe_ptr) { // keyframe is the last keyframe
            double time_offset = (prev_keyframe_ptr->position.time - keyframe.position.time) / 3.0;
            keyframe.in_handle  = Point(keyframe.position.time + time_offset, keyframe.position.value);
            keyframe.out_handle = Point(keyframe.position.time - time_offset, keyframe.position.value);
            ensure_handle_time_boundary(keyframe.position, keyframe.in_handle, prev_keyframe_ptr->position, true);
        } else if (next_keyframe_ptr) { // keyframe is the first keyframe
            double time_offset = (next_keyframe_ptr->position.time - keyframe.position.time) / 3.0;
            keyframe.in_handle = Point(keyframe.position.time + time_offset, keyframe.position.value);
            keyframe.out_handle = Point(keyframe.position.time + time_offset, keyframe.position.value);
            ensure_handle_time_boundary(keyframe.position, keyframe.out_handle, next_keyframe_ptr->position, false);
        } else { // keyframe is the only keyframe
            keyframe.in_handle = Point(keyframe.position.time - 1.0, keyframe.position.value);
            keyframe.out_handle = Point(keyframe.position.time + 1.0, keyframe.position.value);
        }
        
        // For flat handles, ensure in_handle doesn't go past keyframe time
        // and out_handle doesn't go before keyframe time
        constrain_handles_time(keyframe, prev_keyframe_ptr, next_keyframe_ptr);
    }

    void apply_smooth_handles(Keyframe& keyframe, const Keyframe* prev_keyframe_ptr, const Keyframe* next_keyframe_ptr, double smooth_factor) {
        if (prev_keyframe_ptr && next_keyframe_ptr) { // keyframe is in the middle of two keyframes
            Vector vec_prev_to_curr = vector(prev_keyframe_ptr->position, keyframe.position);
            Vector vec_curr_to_next = vector(keyframe.position, next_keyframe_ptr->position);
            Vector tangent_dir_vec = vector(prev_keyframe_ptr->position, next_keyframe_ptr->position);
            
            if (nearly_equal(length_squared(tangent_dir_vec), 0.0)) {
                apply_flat_handles(keyframe, prev_keyframe_ptr, next_keyframe_ptr);
                return;
            }
            Vector normalized_tangent = normalize(tangent_dir_vec);
            double dist_prev = length(vec_prev_to_curr);
            double dist_next = length(vec_curr_to_next);
            
            Vector in_handle_offset = normalized_tangent * (dist_prev * smooth_factor);
            Vector out_handle_offset = normalized_tangent * (dist_next * smooth_factor);

            keyframe.in_handle  = keyframe.position - in_handle_offset;
            keyframe.out_handle = keyframe.position + out_handle_offset;

            ensure_handle_time_boundary(keyframe.position, keyframe.in_handle, prev_keyframe_ptr->position, true);
            ensure_handle_time_boundary(keyframe.position, keyframe.out_handle, next_keyframe_ptr->position, false);

        } else if (prev_keyframe_ptr) { // keyframe is the last keyframe
            Vector vec_prev_to_curr = vector(prev_keyframe_ptr->position, keyframe.position);
            if (nearly_equal(length_squared(vec_prev_to_curr), 0.0)) {
                apply_flat_handles(keyframe, prev_keyframe_ptr, next_keyframe_ptr);
                return;
            }
            Vector normalized_tangent = normalize(vec_prev_to_curr);
            double dist_prev = length(vec_prev_to_curr);

            Vector handle_offset = normalized_tangent * (dist_prev * smooth_factor);
            keyframe.in_handle  = keyframe.position - handle_offset;
            keyframe.out_handle = keyframe.position + handle_offset;

            ensure_handle_time_boundary(keyframe.position, keyframe.in_handle, prev_keyframe_ptr->position, true);
        


        } else if (next_keyframe_ptr) { // keyframe is the first keyframe
            Vector vec_curr_to_next = vector(keyframe.position, next_keyframe_ptr->position);
            if (nearly_equal(length_squared(vec_curr_to_next), 0.0)) {
                apply_flat_handles(keyframe, prev_keyframe_ptr, next_keyframe_ptr);
                return;
            }
            Vector normalized_tangent = normalize(vec_curr_to_next);
            double dist_next = length(vec_curr_to_next);

            Vector handle_offset = normalized_tangent * (dist_next * smooth_factor);
            keyframe.in_handle  = keyframe.position - handle_offset;
            keyframe.out_handle = keyframe.position + handle_offset;

            ensure_handle_time_boundary(keyframe.position, keyframe.out_handle, next_keyframe_ptr->position, false);
            
        } else { // keyframe is the only keyframe
            keyframe.in_handle  = Point(keyframe.position.time - 1.0, keyframe.position.value);
            keyframe.out_handle = Point(keyframe.position.time + 1.0, keyframe.position.value);
        }
        
        // For smooth handles, ensure in_handle doesn't go past keyframe time
        // and out_handle doesn't go before keyframe time
        constrain_handles_time(keyframe, prev_keyframe_ptr, next_keyframe_ptr);
    }


    /**
     * @brief Calculates the maximum allowed magnitude along a direction.
     *
     * It considers the keyframe position, the handle direction, adjacent keyframes,
     * and whether it's an 'in' or 'out' handle to find the furthest point
     * allowed by time boundaries and returns the corresponding magnitude.
     *
     * @param kf_pos The keyframe's position.
     * @param dir The normalized direction vector of the handle.
     * @param prev_kf Pointer to the previous keyframe (can be nullptr).
     * @param next_kf Pointer to the next keyframe (can be nullptr).
     * @param is_out_handle True if 'dir' is for an 'out' handle, false for 'in'.
     * @return The maximum allowed magnitude along 'dir'.
     */
    double calculate_max_magnitude(
        const Point& kf_pos,
        const Vector& dir,
        const Keyframe* prev_kf,
        const Keyframe* next_kf,
        bool is_out_handle
    ) {
        const double infinity = std::numeric_limits<double>::infinity();

        // If handle is vertical (dir.time is zero), time constraints don't limit magnitude.
        if (nearly_equal(dir.time, 0.0)) {
            return infinity;
        }

        double boundary_time;

        if (is_out_handle) {
            // Out handle: Must be >= kf_pos.time and <= next_kf.time.
            // Expect dir.time > 0. If not, something's wrong -> 0 magnitude.
            if (dir.time < 0) return 0.0;
            boundary_time = next_kf ? next_kf->position.time : infinity;
            // The effective boundary is the next keyframe, but no less than the current one.
            boundary_time = std::max(kf_pos.time, boundary_time);
        } else {
            // In handle: Must be <= kf_pos.time and >= prev_kf.time.
            // Expect dir.time < 0. If not, something's wrong -> 0 magnitude.
            if (dir.time > 0) return 0.0;
            boundary_time = prev_kf ? prev_kf->position.time : -infinity;
            // The effective boundary is the prev keyframe, but no more than the current one.
            boundary_time = std::min(kf_pos.time, boundary_time);
        }

        // If boundary is effectively infinite, max magnitude is infinite.
        if (!std::isfinite(boundary_time)) {
            return infinity;
        }

        // Calculate magnitude: M = (boundary_time - kf_pos.time) / dir.time
        double max_mag = (boundary_time - kf_pos.time) / dir.time;

        // Magnitude should always be positive. If somehow negative, return 0.
        return std::max(0.0, max_mag);
    }

    void apply_aligned_handles(
        Keyframe& keyframe,
        const Keyframe* prev_keyframe_ptr,
        const Keyframe* next_keyframe_ptr,
        GrabbedHandle grabbed_handle
    ) {
        Point* source_handle_ptr;
        Point* target_handle_ptr;
        bool source_is_out;


        // Determine source and target handles
        if (grabbed_handle == GrabbedHandle::out_handle) {
            source_handle_ptr = &(keyframe.out_handle);
            target_handle_ptr = &(keyframe.in_handle);
            source_is_out = true;
        } else if (grabbed_handle == GrabbedHandle::in_handle) {
            source_handle_ptr = &(keyframe.in_handle);
            target_handle_ptr = &(keyframe.out_handle);
            source_is_out = false;
        } else { 
            // If no handle is grabbed, we need to determine which handle to use as source.
            // Use the one with the larger magnitude from the keyframe position.
            double dist_out = distance(keyframe.position, keyframe.out_handle);
            double dist_in = distance(keyframe.position, keyframe.in_handle);

            if (dist_out >= dist_in) {
                source_handle_ptr = &(keyframe.out_handle);
                target_handle_ptr = &(keyframe.in_handle);
                source_is_out = true;
            } else {
                source_handle_ptr = &(keyframe.in_handle);
                target_handle_ptr = &(keyframe.out_handle);
                source_is_out = false;
            }
            
        }

        Point& source_handle = *source_handle_ptr;
        Point& target_handle = *target_handle_ptr;
        const Point& kf_pos = keyframe.position;

        // --- Basic Time Check (Snap if source crosses kf_pos.time) ---
        if (source_is_out && source_handle.time < kf_pos.time) {
            keyframe.out_handle.time = kf_pos.time;
        }
        if (!source_is_out && source_handle.time > kf_pos.time) {
            keyframe.in_handle.time = kf_pos.time;
        }

        Vector vec_kf_to_source = vector(kf_pos, source_handle);
        double L_sq_kf_to_source = length_squared(vec_kf_to_source);

        Vector vec_kf_to_target = vector(kf_pos, target_handle);
        double L_sq_kf_to_target = length_squared(vec_kf_to_target);

        // --- Zero Length Check ---
        bool source_is_zero_length = nearly_equal(L_sq_kf_to_source, 0.0);
        bool target_is_zero_length = nearly_equal(L_sq_kf_to_target, 0.0);
        if (source_is_zero_length || target_is_zero_length) {
            if (source_is_zero_length) {
                if (target_is_zero_length) {
                    return;
                }
                vec_kf_to_source = invert(vec_kf_to_target); // Use target's direction
            }
            if (target_is_zero_length) {
                if (source_is_zero_length) {
                    return;
                }
                vec_kf_to_target = invert(vec_kf_to_source); // Use source's direction
            }
        }

        double current_magnitude = keyframe.handle_mode != HandleMode::AlignAdjustable ? length(vec_kf_to_source) : length(vec_kf_to_target);
        Vector dir_source = normalize(vec_kf_to_source);
        Vector dir_target = invert(dir_source);
        const double infinity = std::numeric_limits<double>::infinity();

        // --- Option 2: Maintain Symmetry ---
        if (keyframe.handle_mode == HandleMode::AlignStrict) {
            double max_mag_source = calculate_max_magnitude(kf_pos, dir_source, prev_keyframe_ptr, next_keyframe_ptr, source_is_out);
            double max_mag_target = calculate_max_magnitude(kf_pos, dir_target, prev_keyframe_ptr, next_keyframe_ptr, !source_is_out);

            // Use the smallest of current mag, max source mag, and max target mag.
            double final_magnitude = std::min({current_magnitude, max_mag_source, max_mag_target});
            final_magnitude = std::max(0.0, final_magnitude); // Ensure non-negative

            // Set both handles using the final, constrained magnitude.
            keyframe.out_handle = kf_pos + ((source_is_out ? dir_source : dir_target) * final_magnitude);
            keyframe.in_handle = kf_pos + ((source_is_out ? dir_target : dir_source) * final_magnitude);
        }
        // --- Option 1: Break Symmetry ---
        else { // HandleConstraintOption::BreakSymmetry
            // Calculate source boundaries
            double source_min_time = source_is_out ? kf_pos.time : (prev_keyframe_ptr ? prev_keyframe_ptr->position.time : -infinity);
            double source_max_time = source_is_out ? (next_keyframe_ptr ? next_keyframe_ptr->position.time : infinity) : kf_pos.time;

            // Calculate target boundaries
            double target_min_time = !source_is_out ? kf_pos.time : (prev_keyframe_ptr ? prev_keyframe_ptr->position.time : -infinity);
            double target_max_time = !source_is_out ? (next_keyframe_ptr ? next_keyframe_ptr->position.time : infinity) : kf_pos.time;

            Point final_source_pos = source_handle; // Start with current source
            Point final_target_pos = kf_pos + (dir_target * current_magnitude); // Start with ideal target

            // Helper function to clamp time and recalculate value if clamped.
            auto clamp_and_recalc = [&](Point& p, const Vector& dir, double min_t, double max_t, bool is_out_handle = true) {
                double original_time = p.time;
                p.time = std::clamp(p.time, min_t, max_t);

                // If time was actually clamped...
                if (!nearly_equal(p.time, original_time)) {
                    if (nearly_equal(dir.time, 0.0)) {
                        p = kf_pos;
                    } else if (nearly_equal(p.time, kf_pos.time)) {
                        p = kf_pos;
                    } else {
                        // Recalculate value to maintain slope: V = V_kf + (dV/dT) * (T_clamped - T_kf)
                        double slope = dir.value / dir.time;
                        p.value = kf_pos.value + slope * (p.time - kf_pos.time);
                    }
                }
            };

            // Clamp source and target independently
            clamp_and_recalc(final_source_pos, dir_source, source_min_time, source_max_time, source_is_out);
            clamp_and_recalc(final_target_pos, dir_target, target_min_time, target_max_time, !source_is_out);
            *source_handle_ptr = final_source_pos;
            *target_handle_ptr = final_target_pos;
        }
    }

} // namespace anim

